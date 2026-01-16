# Пошаговая инструкция по тестированию драйвера

## Шаг 1: Подготовка окружения

Создайте виртуальный CAN интерфейс для тестирования:

```bash
sudo modprobe vcan
sudo ip link add dev vcan0 type vcan
sudo ip link set up vcan0
```

Проверьте, что интерфейс создан:
```bash
ip link show vcan0
```

## Шаг 2: Загрузка модуля

Загрузите модуль с параметрами (для тестирования используем локальный IP):
```bash
sudo insmod can_ethernet.ko can_interface_name=vcan0 remote_ip=127.0.0.1 remote_port=12345
```

Проверьте, что модуль загружен:
```bash
lsmod | grep can_ethernet
```

Просмотрите логи:
```bash
dmesg | tail -20
```

Вы должны увидеть сообщения о инициализации драйвера.

## Шаг 3: Тестирование (в двух терминалах)

### Терминал 1: Прием UDP пакетов
Откройте первый терминал и запустите приемник UDP с выводом в hex формате.

**Вариант 1 (рекомендуется - через Python скрипт):**
```bash
python3 receive_can.py
```
Или одной строкой:
```bash
python3 -c "import socket; s=socket.socket(socket.AF_INET, socket.SOCK_DGRAM); s.bind(('', 12345)); [print(' '.join(f'{b:02x}' for b in s.recv(1024))) for _ in iter(int, 1)]"
```

**Вариант 2 (сохранить в файл - самый надежный):**
```bash
nc -u -l 12345 > /tmp/can_data.bin
```
Затем после отправки CAN сообщения нажмите Ctrl+C в этом терминале, и в другом терминале:
```bash
hexdump -C /tmp/can_data.bin
```

**Вариант 3 (через xxd):**
```bash
nc -u -l 12345 | xxd
```

**Примечание:** Если `nc -u -l 12345` показывает данные (даже нечитаемые символы), значит драйвер работает правильно! Для просмотра в hex используйте один из вариантов выше.

### Терминал 2: Отправка CAN сообщений
Откройте второй терминал и отправьте тестовое CAN сообщение:
```bash
cansend vcan0 123#DEADBEEF
```

В первом терминале (с nc или socat) вы должны увидеть полученные данные.

## Шаг 4: Дополнительные тесты

Отправьте несколько разных CAN сообщений:
```bash
# Стандартный CAN фрейм
cansend vcan0 123#DEADBEEF

# Расширенный CAN фрейм
cansend vcan0 12345678#1122334455667788

# CAN фрейм с разной длиной данных
cansend vcan0 456#CAFE
cansend vcan0 789#1234567890ABCDEF
```

Проверьте логи в реальном времени:
```bash
sudo dmesg -w | grep can_ethernet
```

## Шаг 5: Проверка параметров модуля

Просмотрите текущие параметры модуля:
```bash
cat /sys/module/can_ethernet/parameters/can_interface_name
cat /sys/module/can_ethernet/parameters/remote_ip
cat /sys/module/can_ethernet/parameters/remote_port
```

## Шаг 6: Выгрузка модуля

После тестирования выгрузите модуль:
```bash
sudo rmmod can_ethernet
```

Проверьте логи:
```bash
dmesg | tail -10
```

## Устранение проблем

### Модуль не загружается
- Проверьте наличие CAN интерфейса: `ip link show | grep can`
- Убедитесь, что интерфейс включен: `sudo ip link set vcan0 up`
- Проверьте логи: `dmesg | tail -30`

### Нет данных в UDP приемнике
- Проверьте, что модуль загружен: `lsmod | grep can_ethernet`
- Проверьте логи драйвера: `dmesg | grep can_ethernet`
- Убедитесь, что порт 12345 не занят: `sudo netstat -ulnp | grep 12345`
- Попробуйте использовать другой порт при загрузке модуля

### Ошибки при отправке CAN сообщений
- Убедитесь, что vcan0 создан и включен
- Проверьте, что can-utils установлены: `which cansend`
- Попробуйте мониторинг: `candump vcan0` в отдельном терминале

