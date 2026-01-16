# Makefile для сборки модуля ядра can_ethernet

obj-m += can_ethernet.o

# Путь к исходникам ядра (обычно /lib/modules/$(shell uname -r)/build)
KDIR := /lib/modules/$(shell uname -r)/build

# Текущая директория
PWD := $(shell pwd)

# Флаги компиляции
ccflags-y := -Wall -Wextra

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	rm -f *.o *.ko *.mod.c *.mod *.symvers *.order

load:
	sudo insmod can_ethernet.ko

unload:
	sudo rmmod can_ethernet

reload: unload load

help:
	@echo "Доступные команды:"
	@echo "  make          - Собрать модуль"
	@echo "  make clean    - Очистить собранные файлы"
	@echo "  make load     - Загрузить модуль в ядро"
	@echo "  make unload   - Выгрузить модуль из ядра"
	@echo "  make reload   - Перезагрузить модуль"
	@echo "  make help     - Показать эту справку"

.PHONY: all clean load unload reload help

