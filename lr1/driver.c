#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/jiffies.h>
#include <linux/uaccess.h>
#include <linux/stddef.h>
#include <linux/string.h>

#define DEVICE_NAME "lr1_device"
#define CLASS_NAME "lr1_class"
#define NUM_BINS 20
#define BIN_SIZE_US 50  // 50 микросекунд

static int buf;
static int buf_is_empty = 1;  // 1 = empty, 0 = not empty
static unsigned long last_write_time;
static size_t histogram[NUM_BINS] = {0};
static size_t hist_len = 0;  // Текущий бин
static unsigned long time_from_bin_start = 0;  // Накопленное время для текущего бина

static dev_t dev_no;
static struct cdev dev;
static struct class *dev_class;
static struct device *device;

static int device_open(struct inode *inode, struct file *file) {
    pr_info("%s: device opened\n", DEVICE_NAME);
    // Сбрасываем состояние гистограммы при открытии устройства
    // Это позволяет каждому приложению начинать с чистого состояния
    hist_len = 0;
    time_from_bin_start = 0;
    last_write_time = jiffies;  // Инициализируем время при открытии
    memset(histogram, 0, sizeof(histogram));
    return 0;
}

static int device_release(struct inode *inode, struct file *file) {
    pr_info("%s: device released\n", DEVICE_NAME);
    return 0;
}

static ssize_t device_read(struct file *flip, char __user *user_buf, size_t count, loff_t *offset) {
    pr_info("%s: device_read: count=%zu\n", DEVICE_NAME, count);

    if (count != sizeof(int)) {
        pr_err("%s: device_read: count must be %zu\n", DEVICE_NAME, sizeof(int));
        return -EINVAL;
    }

    if (buf_is_empty == 1) {
        pr_info("%s: device_read: buffer is empty\n", DEVICE_NAME);
        return 0;
    }

    if (copy_to_user(user_buf, &buf, sizeof(int))) {
        pr_err("%s: device_read: copy_to_user failed\n", DEVICE_NAME);
        return -EFAULT;
    }

    // Измеряем время между write и read
    unsigned long time_between_write_and_read = jiffies - last_write_time;
    
    // Накапливаем время для текущего бина
    time_from_bin_start += time_between_write_and_read;
    
    // Увеличиваем счетчик в текущем бине
    if (hist_len < NUM_BINS) {
        histogram[hist_len]++;
    }
    
    // Если накопилось >= 50 мкс, переходим к следующему бину
    if (jiffies_to_usecs(time_from_bin_start) >= BIN_SIZE_US) {
        time_from_bin_start = 0;
        if (hist_len < NUM_BINS - 1) {
            hist_len++;
        }
    }

    pr_info("%s: device_read: successfully read %d, time_diff=%lu jiffies, bin=%zu\n", 
            DEVICE_NAME, buf, time_between_write_and_read, hist_len);

    return sizeof(int);
}

static ssize_t device_write(struct file *flip, const char __user *user_buf, size_t count, loff_t *offset) {
    pr_info("%s: device_write: count=%zu\n", DEVICE_NAME, count);

    if (count != sizeof(int)) {
        pr_err("%s: device_write: count must be %zu\n", DEVICE_NAME, sizeof(int));
        return -EINVAL;
    }

    if (copy_from_user(&buf, user_buf, sizeof(int))) {
        pr_err("%s: device_write: copy_from_user failed\n", DEVICE_NAME);
        return -EFAULT;
    }

    buf_is_empty = 0;
    last_write_time = jiffies;

    pr_info("%s: device_write: successfully wrote %d\n", DEVICE_NAME, buf);

    return sizeof(int);
}

static void print_histogram(void) {
    pr_info("%s: ========== Histogram ==========\n", DEVICE_NAME);
    pr_info("%s: Bin size: %d microseconds\n", DEVICE_NAME, BIN_SIZE_US);
    pr_info("%s: Number of bins used: %zu\n", DEVICE_NAME, hist_len + 1);
    pr_info("%s: ----------------------------------------\n", DEVICE_NAME);
    
    // Выводим только заполненные бины
    for (size_t i = 0; i <= hist_len && i < NUM_BINS; i++) {
        pr_info("%s: Bin %2zu: %zu operations\n", DEVICE_NAME, i, histogram[i]);
    }
    
    pr_info("%s: =======================================\n", DEVICE_NAME);
}

static long device_ioctl(struct file *flip, unsigned int cmd, unsigned long arg) {
    pr_info("%s: device_ioctl: cmd=0x%x\n", DEVICE_NAME, cmd);

    switch (cmd) {
        case 0x01:  // Команда для вывода гистограммы
            pr_info("%s: device_ioctl: Printing histogram\n", DEVICE_NAME);
            print_histogram();
            break;
        default:
            pr_err("%s: device_ioctl: Unknown command: 0x%x\n", DEVICE_NAME, cmd);
            return -EINVAL;
    }

    return 0;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = device_open,
    .release = device_release,
    .read = device_read,
    .write = device_write,
    .unlocked_ioctl = device_ioctl,
};

static int __init lr1_init(void) {
    int res;

    pr_info("%s: Initializing driver\n", DEVICE_NAME);

    // Выделяем номера устройств
    res = alloc_chrdev_region(&dev_no, 0, 1, DEVICE_NAME);
    if (res < 0) {
        pr_err("%s: Failed to allocate chrdev region: %d\n", DEVICE_NAME, res);
        return res;
    }

    // Инициализируем символьное устройство
    cdev_init(&dev, &fops);
    dev.owner = THIS_MODULE;

    // Добавляем устройство в систему
    res = cdev_add(&dev, dev_no, 1);
    if (res < 0) {
        pr_err("%s: Failed to add cdev: %d\n", DEVICE_NAME, res);
        unregister_chrdev_region(dev_no, 1);
        return res;
    }

    // Создаем класс устройства
    dev_class = class_create(CLASS_NAME);
    if (IS_ERR(dev_class)) {
        pr_err("%s: Failed to create device class\n", DEVICE_NAME);
        res = PTR_ERR(dev_class);
        cdev_del(&dev);
        unregister_chrdev_region(dev_no, 1);
        return res;
    }

    // Автоматически создаем файл устройства
    device = device_create(dev_class, NULL, dev_no, NULL, DEVICE_NAME);
    if (IS_ERR(device)) {
        pr_err("%s: Failed to create device node\n", DEVICE_NAME);
        res = PTR_ERR(device);
        class_destroy(dev_class);
        cdev_del(&dev);
        unregister_chrdev_region(dev_no, 1);
        return res;
    }

    pr_info("%s: Driver initialized successfully, major=%d\n", 
            DEVICE_NAME, MAJOR(dev_no));

    return 0;
}

static void __exit lr1_exit(void) {
    pr_info("%s: Cleaning up driver\n", DEVICE_NAME);

    // Выводим финальную гистограмму
    print_histogram();

    device_destroy(dev_class, dev_no);
    class_destroy(dev_class);
    cdev_del(&dev);
    unregister_chrdev_region(dev_no, 1);

    pr_info("%s: Driver unloaded\n", DEVICE_NAME);
}

module_init(lr1_init);
module_exit(lr1_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("MAI Team");
MODULE_DESCRIPTION("Lab 1: Character driver with histogram");

