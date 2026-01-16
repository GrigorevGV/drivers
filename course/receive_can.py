#!/usr/bin/env python3
"""
Простой скрипт для приема UDP пакетов с CAN данными и вывода в hex формате
Использование: python3 receive_can.py
"""

import socket
import sys

UDP_PORT = 12345

def main():
    # Создаем UDP сокет
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    
    try:
        # Привязываем к порту
        sock.bind(('', UDP_PORT))
        print(f"Ожидание UDP пакетов на порту {UDP_PORT}...")
        print("Отправьте CAN сообщение через cansend")
        print("Для выхода нажмите Ctrl+C\n")
        
        while True:
            # Принимаем данные (максимум 1024 байта)
            data, addr = sock.recvfrom(1024)
            
            # Выводим в hex формате
            hex_data = ' '.join(f'{b:02x}' for b in data)
            print(f"Получено {len(data)} байт от {addr[0]}:{addr[1]}")
            print(f"Hex: {hex_data}")
            print(f"ASCII: {repr(data)}")
            print("-" * 60)
            
    except KeyboardInterrupt:
        print("\nОстановлено пользователем")
    except Exception as e:
        print(f"Ошибка: {e}", file=sys.stderr)
    finally:
        sock.close()

if __name__ == '__main__':
    main()

