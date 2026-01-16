#!/usr/bin/env python3

import socket
import sys

UDP_PORT = 12345

def main():
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    
    try:
        sock.bind(('', UDP_PORT))
        print(f"Ожидание UDP сообщений на порту {UDP_PORT}...")
        print("Отправляйте CAN сообщения используя cansend")
        print("Для остановки нажмите Ctrl+C\n")
        
        while True:
            data, addr = sock.recvfrom(1024)
            
            hex_data = ' '.join(f'{b:02x}' for b in data)
            print(f"Получено {len(data)} байт от {addr[0]}:{addr[1]}")
            print(f"Hex: {hex_data}")
            print(f"ASCII: {repr(data)}")
            print("-" * 60)
            
    except KeyboardInterrupt:
        print("\nПрограмма остановлена пользователем")
    except Exception as e:
        print(f"Ошибка: {e}", file=sys.stderr)
    finally:
        sock.close()

if __name__ == '__main__':
    main()

