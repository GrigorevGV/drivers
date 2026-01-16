#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

int main(void) {
    const char* const device_name = "/dev/foo_device";
    const int fd = open(device_name, O_RDWR);
    if (fd < 0) {
        printf("Failed to open '%s': %d\n", device_name, fd);
        return fd;
    }

    printf("Opened '%s' device with fd=%d\n", device_name, fd);

    close(fd);

    return 0;
}

