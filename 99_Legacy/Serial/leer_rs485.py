import serial
import serial.tools.list_ports

def listar_puertos():
    puertos = serial.tools.list_ports.comports()
    print("Puertos disponibles:")
    for p in puertos:
        print(f"  {p.device} - {p.description}")
    print()

def main():
    listar_puertos()

    puerto = input("Ingresa el puerto COM (ej: COM5): ").strip()
    baudrate = 9600

    try:
        ser = serial.Serial(puerto, baudrate, timeout=1)
        print(f"Escuchando en {puerto} a {baudrate} baudios...")
        print("Presiona Ctrl+C para salir.\n")

        while True:
            if ser.in_waiting > 0:
                linea = ser.readline().decode('utf-8', errors='replace').strip()
                if linea:
                    print(f"Recibido: {linea}")

    except serial.SerialException as e:
        print(f"Error abriendo el puerto: {e}")
    except KeyboardInterrupt:
        print("\nCerrando conexión...")
    finally:
        if 'ser' in locals() and ser.is_open:
            ser.close()

if __name__ == "__main__":
    main()