import numpy as np
import datetime
import cv2
import time
import serial
from ultralytics import YOLO
from shapely.geometry import Point, Polygon
from helper import create_video_writer

# ==========================================
# CONFIGURACIÓN (A ajustar por Funcional)
# ==========================================
SERIAL_PORT = "COM5" # En Raspberry Pi puede ser /dev/ttyUSB0 o /dev/ttyS0
BAUD_RATE = 115200
SEND_INTERVAL_SEC = 1.0 
CONF_THRESHOLD = 0.5
VEHICLE_CLASSES = [2, 3, 5, 7] # YOLO COCO: car, motorcycle, bus, truck

# Zona de interés (Polígono donde los autos esperan su turno)
ROI_POLYGON = Polygon([(200, 300), (800, 300), (1000, 600), (100, 600)])
roi_pts = np.array([[200, 300], [800, 300], [1000, 600], [100, 600]], np.int32)
roi_pts = roi_pts.reshape((-1, 1, 2))

# ==========================================
# INICIALIZACIÓN
# ==========================================
print(f"[*] Iniciando modelo YOLOv8...")
model = YOLO("yolov8s.pt")

print(f"[*] Abriendo conexión Serial en {SERIAL_PORT}...")
try:
    ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
except Exception as e:
    print(f"[WARNING] No se pudo abrir puerto Serial. Modo Simulación activo: {e}")
    ser = None

video_cap = cv2.VideoCapture("1.mp4") # Cambiar a RTSP o 0 para Webcam
writer = create_video_writer(video_cap, "output.mp4")

last_send_time = time.time()

# ==========================================
# BUCLE PRINCIPAL
# ==========================================
while video_cap.isOpened():
    success, frame = video_cap.read()
    if not success:
        print("[INFO] Fin del video o desconexión de cámara. Reiniciando...")
        video_cap.set(cv2.CAP_PROP_POS_FRAMES, 0) # Loop temporal para pruebas
        continue

    # Detección
    results = model.predict(frame, classes=VEHICLE_CLASSES, conf=CONF_THRESHOLD, verbose=False)
    
    vehiculos_en_espera = 0
    
    for result in results:
        boxes = result.boxes
        for box in boxes:
            x1, y1, x2, y2 = map(int, box.xyxy[0])
            cx, cy = (x1 + x2) // 2, (y1 + y2) // 2
            
            # Chequear si el centro del vehículo está en la zona de espera
            if ROI_POLYGON.contains(Point(cx, cy)):
                vehiculos_en_espera += 1
                cv2.rectangle(frame, (x1, y1), (x2, y2), (0, 0, 255), 2) # Rojo si está esperando
            else:
                cv2.rectangle(frame, (x1, y1), (x2, y2), (0, 255, 0), 2) # Verde si solo pasa

    # Dibujar ROI
    cv2.polylines(frame, [roi_pts], isClosed=True, color=(255, 255, 0), thickness=2)
    cv2.putText(frame, f"Autos Esperando: {vehiculos_en_espera}", (50, 50), 
                cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 0, 255), 3)

    # ==========================================
    # COMUNICACIÓN SERIAL AL STM32
    # ==========================================
    if time.time() - last_send_time >= SEND_INTERVAL_SEC:
        cmd = f"AI_CARS:{vehiculos_en_espera}\n"
        print(f"[UART TX] -> {cmd.strip()}")
        if ser and ser.is_open:
            try:
                ser.write(cmd.encode('utf-8'))
            except Exception as e:
                print(f"[ERROR UART] {e}")
                # Aquí iría el intento de reconexión del Watchdog de software
        last_send_time = time.time()

    writer.write(frame)
    cv2.imshow("Semáforo Inteligente V2", cv2.resize(frame, (800, 600)))
    
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

video_cap.release()
writer.release()
if ser:
    ser.close()
cv2.destroyAllWindows()