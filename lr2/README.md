# Лабораторная работа 2

## Задание

1. Собрать образ для BeagleBone Black при помощи buildroot
2. Загрузить образ
3. Убедиться, что образ загрузился

## Клонирование и настройка buildroot

```bash
git clone https://git.buildroot.net/buildroot
cd buildroot
make beaglebone_defconfig
make menuconfig
```

## Конфигурация ядра

Для поддержки сети поверх USB:

```bash
make linux-menuconfig
```

Необходимые опции:
- CONFIG_USB_GADGET=y
- CONFIG_USB_MUSB_HDRC=y
- CONFIG_USB_MUSB_GADGET=y
- CONFIG_USB_MUSB_DSPS=y
- CONFIG_NOP_USB_XCEIV=y
- CONFIG_AM335X_PHY_USB=y
- CONFIG_USB_ETH=y

## Компиляция

```bash
make -j$(nproc) 2>&1 | tee build.log
```

После компиляции в `output/images/` появятся файлы: `am335x-boneblack.dtb`, `MLO`, `rootfs.tar`, `u-boot.img`, `zImage`.

## Запуск в QEMU

```bash
./lr2/run_qemu.sh buildroot/output/images
```

## Подготовка SD-карты (для физической платы)

1. Создайте разделы на SD-карте
2. Скопируйте файлы загрузки в boot раздел
3. Распакуйте rootfs в rootfs раздел
4. Вставьте SD-карту в BeagleBone Black и загрузите

## Проверка загрузки

```bash
uname -r
hostname
df -h
```
