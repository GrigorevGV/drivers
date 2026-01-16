#!/bin/bash

if [ -n "$1" ]; then
    IMAGES_DIR="$1"
else
    SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
    if [ -d "$SCRIPT_DIR/../buildroot/output/images" ]; then
        IMAGES_DIR="$SCRIPT_DIR/../buildroot/output/images"
    elif [ -d "buildroot/output/images" ]; then
        IMAGES_DIR="buildroot/output/images"
    else
        echo "Ошибка: Не найдена папка с образами."
        echo "Использование: $0 [путь_к_buildroot_output_images]"
        exit 1
    fi
fi

cd "$IMAGES_DIR" || exit 1

if [ ! -f "zImage" ]; then
    echo "Ошибка: Не найден zImage в $IMAGES_DIR"
    exit 1
fi

if [ ! -f "rootfs.img" ] && [ ! -f "rootfs.ext2" ] && [ ! -f "rootfs.tar" ]; then
    echo "Предупреждение: Не найден образ rootfs."
    echo "Создайте rootfs.img или используйте rootfs.ext2/rootfs.tar"
    echo ""
    echo "Для создания rootfs.img из rootfs.tar:"
    echo "  qemu-img create -f raw rootfs.img 512M"
    echo "  sudo mkfs.ext4 -F rootfs.img"
    echo "  sudo mkdir -p /mnt/qemu_rootfs"
    echo "  sudo mount -o loop rootfs.img /mnt/qemu_rootfs"
    echo "  sudo tar -C /mnt/qemu_rootfs -xf rootfs.tar"
    echo "  sudo umount /mnt/qemu_rootfs"
    exit 1
fi

if [ -f "rootfs.ext2" ]; then
    ROOTFS="rootfs.ext2"
elif [ -f "rootfs.img" ]; then
    ROOTFS="rootfs.img"
else
    echo "Используется rootfs.ext2 по умолчанию (может не существовать)"
    ROOTFS="rootfs.ext2"
fi

if [ -f "am335x-boneblack.dtb" ]; then
    DTB="am335x-boneblack.dtb"
    MACHINE="vexpress-a9"
    echo "Используется DTB для BeagleBone Black (может не работать с vexpress-a9)"
elif [ -f "vexpress-v2p-ca9.dtb" ]; then
    DTB="vexpress-v2p-ca9.dtb"
    MACHINE="vexpress-a9"
    echo "Используется DTB для vexpress-a9"
else
    DTB=""
    MACHINE="vexpress-a9"
    echo "Предупреждение: DTB не найден, запуск без DTB"
fi

QEMU="qemu-system-arm"
CPU="cortex-a9"
MEMORY="512M"
CONSOLE="console=ttyAMA0,115200"
ROOT_DEVICE="root=/dev/mmcblk0 rw rootwait"

echo "Запуск QEMU..."
echo "Машина: $MACHINE"
echo "CPU: $CPU"
echo "Память: $MEMORY"
echo "Ядро: zImage"
echo "DTB: ${DTB:-не используется}"
echo "RootFS: $ROOTFS"
echo ""
echo "Для выхода: Ctrl+A, затем X"
echo ""

QEMU_CMD="$QEMU -M $MACHINE -cpu $CPU -m $MEMORY -kernel zImage"

if [ -n "$DTB" ]; then
    QEMU_CMD="$QEMU_CMD -dtb $DTB"
fi

QEMU_CMD="$QEMU_CMD -drive file=$ROOTFS,if=sd,format=raw"
QEMU_CMD="$QEMU_CMD -append \"$CONSOLE $ROOT_DEVICE\""
QEMU_CMD="$QEMU_CMD -serial stdio"
QEMU_CMD="$QEMU_CMD -netdev user,id=net0"
QEMU_CMD="$QEMU_CMD -device virtio-net-device,netdev=net0"
QEMU_CMD="$QEMU_CMD -no-reboot"

eval "$QEMU_CMD"

