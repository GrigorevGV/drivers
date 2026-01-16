#include <linux/fs.h>
#include <linux/module.h>
#include <linux/printk.h>

static int device_open(struct inode *inode, struct file *file) {
    pr_info("INFO! Open device\n");
    return 0;
}

static int device_release(struct inode *inode, struct file *file) {
    pr_info("INFO! Release device\n");
    return 0;
}

const struct file_operations fops = {
    .open = device_open,
    .release = device_release,
};

unsigned int Major;

int init_module(void) {
    pr_info("INFO! start init module custom driver\n");

    Major = register_chrdev(0, "foo", &fops);
    if (Major < 0) {
        printk(KERN_ALERT "Registering char device failed with %d\n", Major);
        return Major;
    }

    pr_info("INFO! end init module custom driver\n");
    pr_info("Major number: %d\n", Major);

    return 0;
}

void cleanup_module(void) {
    pr_info("Goodbye world 1.\n");
    unregister_chrdev(Major, "foo");
}

MODULE_LICENSE("GPL");

