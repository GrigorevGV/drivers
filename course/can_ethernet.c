/*
 * can_ethernet.c - Драйвер для передачи CAN пакетов через Ethernet
 * 
 * Этот модуль создает мост между SocketCAN интерфейсом и Ethernet,
 * позволяя передавать CAN фреймы через сеть UDP/IP.
 * 
 * Основан на примерах из ядра Linux: SocketCAN, UDP sockets
 * Адаптирован для ядра Linux 6.x
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/skbuff.h>
#include <linux/inet.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <linux/socket.h>
#include <net/sock.h>
#include <linux/kthread.h>
#include <linux/netdevice.h>
#include <linux/string.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <uapi/linux/in.h>

#define DRIVER_NAME "can_ethernet"
#define CAN_ETH_UDP_PORT 12345
#define MAX_PACKET_SIZE 1024

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Student");
MODULE_DESCRIPTION("Driver for transmitting CAN packets over Ethernet");
MODULE_VERSION("1.0");

static char *can_interface_name = "can0";
module_param(can_interface_name, charp, 0644);
MODULE_PARM_DESC(can_interface_name, "CAN interface name (e.g., can0, vcan0)");

static char *remote_ip = "192.168.1.100";
module_param(remote_ip, charp, 0644);
MODULE_PARM_DESC(remote_ip, "Remote IP address for CAN packets");

static int remote_port = CAN_ETH_UDP_PORT;
module_param(remote_port, int, 0644);
MODULE_PARM_DESC(remote_port, "Remote UDP port");

static struct socket *can_sock;
static struct socket *udp_sock;
static struct task_struct *rx_thread;
static int thread_running = 0;

/* Структура для передачи CAN фрейма через UDP */
struct can_eth_packet {
    __be32 can_id;
    __u8 can_dlc;
    __u8 flags;
    __u8 data[CAN_MAX_DLEN];
} __packed;

/*
 * Отправка CAN фрейма через UDP (совместимо с ядром 6.x)
 */
static int send_can_frame_over_udp(struct can_frame *frame)
{
    struct msghdr msg;
    struct kvec iov;
    struct sockaddr_in addr;
    struct can_eth_packet packet;
    int err;
    __be32 ip_addr;

    if (!udp_sock) {
        printk(KERN_ERR DRIVER_NAME ": UDP socket not initialized\n");
        return -ENOTCONN;
    }

    /* Преобразуем IP адрес */
    if (in4_pton(remote_ip, -1, (u8 *)&ip_addr, '\0', NULL) <= 0) {
        printk(KERN_ERR DRIVER_NAME ": Invalid IP address: %s\n", remote_ip);
        return -EINVAL;
    }

    /* Подготавливаем пакет */
    packet.can_id = cpu_to_be32(frame->can_id);
    packet.can_dlc = frame->can_dlc;
    packet.flags = 0;
    if (frame->can_id & CAN_RTR_FLAG)
        packet.flags |= 0x01;
    if (frame->can_id & CAN_EFF_FLAG)
        packet.flags |= 0x02;
    
    memcpy(packet.data, frame->data, frame->can_dlc);

    /* Заполняем адрес получателя */
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(remote_port);
    addr.sin_addr.s_addr = ip_addr;

    /* Подготавливаем iovec */
    iov.iov_base = &packet;
    iov.iov_len = sizeof(packet.can_id) + sizeof(packet.can_dlc) + 
                  sizeof(packet.flags) + frame->can_dlc;

    /* Подготавливаем сообщение */
    memset(&msg, 0, sizeof(msg));
    msg.msg_name = &addr;
    msg.msg_namelen = sizeof(addr);
    msg.msg_flags = 0;

    /* Отправляем пакет через kernel_sendmsg (использует kvec напрямую) */
    err = kernel_sendmsg(udp_sock, &msg, &iov, 1, iov.iov_len);

    if (err < 0) {
        printk(KERN_ERR DRIVER_NAME ": Failed to send UDP packet: %d\n", err);
    } else {
        printk(KERN_DEBUG DRIVER_NAME ": Sent CAN frame ID=0x%03X, DLC=%d\n",
               frame->can_id & CAN_SFF_MASK, frame->can_dlc);
    }

    return err;
}

/*
 * Поток для приема CAN фреймов и отправки их через Ethernet
 */
static int can_rx_thread(void *data)
{
    struct msghdr msg;
    struct kvec iov;
    struct can_frame frame;
    int err;
    struct sockaddr_can addr;
    int len = sizeof(addr);

    printk(KERN_INFO DRIVER_NAME ": CAN receive thread started\n");

    while (!kthread_should_stop() && thread_running) {
        /* Подготавливаем буфер для приема */
        iov.iov_base = &frame;
        iov.iov_len = sizeof(frame);

        /* Подготавливаем сообщение */
        memset(&msg, 0, sizeof(msg));
        msg.msg_name = &addr;
        msg.msg_namelen = len;
        msg.msg_flags = MSG_DONTWAIT;

        /* Принимаем CAN фрейм через kernel_recvmsg (использует kvec напрямую) */
        err = kernel_recvmsg(can_sock, &msg, &iov, 1, iov.iov_len, msg.msg_flags);

        if (err > 0) {
            /* Отправляем через UDP */
            send_can_frame_over_udp(&frame);
        } else if (err != -EAGAIN && err != -EWOULDBLOCK) {
            if (!kthread_should_stop()) {
                printk(KERN_ERR DRIVER_NAME ": Error receiving CAN frame: %d\n", err);
            }
            msleep(100); /* Небольшая задержка при ошибке */
        } else {
            msleep(10); /* Короткая задержка если нет данных */
        }
    }

    printk(KERN_INFO DRIVER_NAME ": CAN receive thread stopped\n");
    return 0;
}

/*
 * Инициализация CAN сокета
 */
static int init_can_socket(void)
{
    struct sockaddr_can addr;
    struct can_filter filter = {0, 0}; /* Принимаем все пакеты */
    struct net_device *can_dev;
    int err;

    /* Создаем CAN сокет */
    err = sock_create_kern(&init_net, PF_CAN, SOCK_RAW, CAN_RAW, &can_sock);
    if (err < 0) {
        printk(KERN_ERR DRIVER_NAME ": Failed to create CAN socket: %d\n", err);
        return err;
    }

    /* Находим CAN интерфейс */
    can_dev = dev_get_by_name(&init_net, can_interface_name);
    if (!can_dev) {
        printk(KERN_ERR DRIVER_NAME ": CAN interface '%s' not found\n", can_interface_name);
        sock_release(can_sock);
        can_sock = NULL;
        return -ENODEV;
    }

    /* Настраиваем адрес */
    addr.can_family = AF_CAN;
    addr.can_ifindex = can_dev->ifindex;

    /* Устанавливаем фильтр через sock_setsockopt с использованием sockptr_t */
    {
        sockptr_t optval = KERNEL_SOCKPTR(&filter);
        err = sock_setsockopt(can_sock, SOL_CAN_RAW, CAN_RAW_FILTER, optval, sizeof(filter));
    }
    if (err < 0) {
        printk(KERN_ERR DRIVER_NAME ": Failed to set CAN filter: %d\n", err);
        dev_put(can_dev);
        sock_release(can_sock);
        can_sock = NULL;
        return err;
    }

    /* Привязываем сокет */
    err = kernel_bind(can_sock, (struct sockaddr *)&addr, sizeof(addr));
    dev_put(can_dev);

    if (err < 0) {
        printk(KERN_ERR DRIVER_NAME ": Failed to bind CAN socket: %d\n", err);
        sock_release(can_sock);
        can_sock = NULL;
        return err;
    }

    printk(KERN_INFO DRIVER_NAME ": CAN socket initialized for interface '%s'\n", can_interface_name);
    return 0;
}

/*
 * Инициализация UDP сокета
 */
static int init_udp_socket(void)
{
    int err;

    err = sock_create_kern(&init_net, AF_INET, SOCK_DGRAM, IPPROTO_UDP, &udp_sock);
    if (err < 0) {
        printk(KERN_ERR DRIVER_NAME ": Failed to create UDP socket: %d\n", err);
        return err;
    }

    printk(KERN_INFO DRIVER_NAME ": UDP socket initialized (remote: %s:%d)\n", 
           remote_ip, remote_port);
    return 0;
}

/*
 * Инициализация модуля
 */
static int __init can_ethernet_init(void)
{
    int err;

    printk(KERN_INFO DRIVER_NAME ": Initializing CAN-Ethernet driver\n");
    printk(KERN_INFO DRIVER_NAME ": CAN interface: %s\n", can_interface_name);
    printk(KERN_INFO DRIVER_NAME ": Remote: %s:%d\n", remote_ip, remote_port);

    /* Инициализируем UDP сокет */
    err = init_udp_socket();
    if (err < 0) {
        return err;
    }

    /* Инициализируем CAN сокет */
    err = init_can_socket();
    if (err < 0) {
        sock_release(udp_sock);
        udp_sock = NULL;
        return err;
    }

    /* Запускаем поток приема CAN фреймов */
    thread_running = 1;
    rx_thread = kthread_run(can_rx_thread, NULL, "can_eth_rx");
    if (IS_ERR(rx_thread)) {
        printk(KERN_ERR DRIVER_NAME ": Failed to create receive thread\n");
        sock_release(can_sock);
        sock_release(udp_sock);
        can_sock = NULL;
        udp_sock = NULL;
        thread_running = 0;
        return PTR_ERR(rx_thread);
    }

    printk(KERN_INFO DRIVER_NAME ": CAN-Ethernet driver initialized successfully\n");
    return 0;
}

/*
 * Очистка модуля
 */
static void __exit can_ethernet_exit(void)
{
    printk(KERN_INFO DRIVER_NAME ": Unloading module\n");

    /* Останавливаем поток */
    thread_running = 0;
    if (rx_thread && !IS_ERR(rx_thread)) {
        kthread_stop(rx_thread);
        rx_thread = NULL;
    }

    /* Закрываем сокеты */
    if (can_sock) {
        sock_release(can_sock);
        can_sock = NULL;
    }

    if (udp_sock) {
        sock_release(udp_sock);
        udp_sock = NULL;
    }

    printk(KERN_INFO DRIVER_NAME ": Module unloaded\n");
}

module_init(can_ethernet_init);
module_exit(can_ethernet_exit);
