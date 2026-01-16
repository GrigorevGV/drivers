# Драйвер для передачи CAN пакетов через Ethernet

## Описание

Модуль ядра Linux, создающий мост между интерфейсом CAN (SocketCAN) и сетью Ethernet. Драйвер передает CAN-фреймы через UDP/IP протокол.

## Требования

- Linux ядро (версия 5.9 или выше)
- SocketCAN поддержка в ядре
- Заголовочные файлы ядра

## Установка зависимостей

```bash
sudo apt-get update
sudo apt-get install build-essential linux-headers-$(uname -r) can-utils
```

## Сборка

```bash
make
```

## Загрузка модуля

```bash
sudo insmod can_ethernet.ko can_interface_name=vcan0 remote_ip=192.168.1.100 remote_port=12345
```

**Параметры:**
- `can_interface_name` - имя CAN интерфейса (по умолчанию: can0)
- `remote_ip` - IP адрес получателя (по умолчанию: 192.168.1.100)
- `remote_port` - UDP порт (по умолчанию: 12345)

## Тестирование

```bash
# Создание виртуального CAN интерфейса
sudo modprobe vcan
sudo ip link add dev vcan0 type vcan
sudo ip link set up vcan0

# Загрузка модуля
sudo insmod can_ethernet.ko can_interface_name=vcan0 remote_ip=127.0.0.1 remote_port=12345

# Отправка тестового сообщения
cansend vcan0 123#DEADBEEF

# Просмотр логов
dmesg | tail -20
```

## Выгрузка модуля

```bash
sudo rmmod can_ethernet
```

## Формат пакетов

CAN-фреймы инкапсулируются в UDP пакеты:
- CAN ID (4 байта, big-endian)
- CAN DLC (1 байт)
- Флаги (1 байт)
- Данные CAN (до 8 байт)
