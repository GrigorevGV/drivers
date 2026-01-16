#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define DEVICE_PATH "/dev/lr1_device"
#define IOCTL_PRINT_HISTOGRAM 0x01

int main(void) {
    int writer_fd, reader_fd;
    int value;
    int read_value;

    // Открываем устройство для записи
    writer_fd = open(DEVICE_PATH, O_WRONLY);
    if (writer_fd < 0) {
        perror("Failed to open device for writing");
        return 1;
    }

    // Открываем устройство для чтения
    reader_fd = open(DEVICE_PATH, O_RDONLY);
    if (reader_fd < 0) {
        perror("Failed to open device for reading");
        close(writer_fd);
        return 1;
    }

    printf("Device opened successfully (writer_fd=%d, reader_fd=%d)\n", writer_fd, reader_fd);
    printf("Writing and reading integers...\n\n");

    // Выполняем цикл записи и чтения
    for (int i = 0; i < 1000; i++) {
        value = i;

        // Записываем значение
        if (write(writer_fd, &value, sizeof(int)) != sizeof(int)) {
            perror("Write failed");
            close(writer_fd);
            close(reader_fd);
            return 1;
        }

        // Читаем значение
        if (read(reader_fd, &read_value, sizeof(int)) != sizeof(int)) {
            perror("Read failed");
            close(writer_fd);
            close(reader_fd);
            return 1;
        }

        if (i % 100 == 0) {
            printf("Write: %d, Read: %d\n", value, read_value);
        }
    }

    printf("\nRequesting histogram via ioctl...\n");
    
    // Запрашиваем вывод гистограммы через ioctl
    if (ioctl(reader_fd, IOCTL_PRINT_HISTOGRAM) < 0) {
        perror("ioctl failed");
        close(writer_fd);
        close(reader_fd);
        return 1;
    }

    printf("Histogram printed to kernel log (check with: sudo dmesg | tail -30)\n");

    close(writer_fd);
    close(reader_fd);
    return 0;
}

