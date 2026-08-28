import serial

puerto  = "COM7"    # <-- cambia esto por tu puerto
baudrate = 115600    # <-- cambia si es necesario

ser = serial.Serial(puerto, baudrate, timeout=1)
print(f"Leyendo {puerto}... (Ctrl+C para salir)\n")

try:
    while True:
        if ser.in_waiting > 0:
            datos = ser.read(ser.in_waiting)
            print(datos.decode("utf-8", errors="replace"), end="", flush=True)
except KeyboardInterrupt:
    print("\nCerrado.")
    ser.close()