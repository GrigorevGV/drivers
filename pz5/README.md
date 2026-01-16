# Практическое задание 5

## Задание
Создать сетевой драйвер и приложение для отправки пакетов.

## Компиляция драйвера
```bash
cd driver
make
```

## Загрузка драйвера
```bash
sudo insmod net_driver.ko
```

## Проверка интерфейса
```bash
ip link show | grep netdemo
```

## Включение интерфейса
```bash
sudo ip link set netdemo0 up
```

## Компиляция приложения
```bash
cd app
make
```

## Запуск приложения
```bash
sudo ./rawexample
```

## Просмотр логов
```bash
sudo dmesg | tail -50
```

## Выгрузка драйвера
```bash
sudo rmmod net_driver
```

