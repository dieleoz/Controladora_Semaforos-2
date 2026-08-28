# -*- coding: utf-8 -*-
def crc8_maxim(data: bytes) -> int:
    crc = 0x00
    for byte in data:
        crc ^= byte
        for _ in range(8):
            if crc & 0x80:
                crc = (crc << 1) ^ 0x31
            else:
                crc = crc << 1
            crc &= 0xFF
    return crc

def xor_checksum(data: bytes) -> int:
    crc = 0
    for byte in data:
        crc ^= byte
    return crc

print("=== PRUEBA DE VULNERABILIDAD XOR vs CRC-8 MAXIM ===")
trama_original = bytes([0x01, 0x02, 0x03])
trama_corrupta = bytes([0x02, 0x01, 0x03]) # Bytes invertidos por ruido

print("Trama Original:", list(trama_original))
print("Trama Corrupta (Ruido):", list(trama_corrupta))

print("\n--- Con el antiguo XOR (Falso Positivo) ---")
print("XOR Original: 0x{:02X}".format(xor_checksum(trama_original)))
print("XOR Corrupta: 0x{:02X}".format(xor_checksum(trama_corrupta)))
if xor_checksum(trama_original) == xor_checksum(trama_corrupta):
    print("-> PELIGRO: El XOR deja pasar la trama corrupta como valida.")

print("\n--- Con el nuevo CRC-8 Maxim (V7) ---")
print("CRC-8 Original: 0x{:02X}".format(crc8_maxim(trama_original)))
print("CRC-8 Corrupta: 0x{:02X}".format(crc8_maxim(trama_corrupta)))
if crc8_maxim(trama_original) != crc8_maxim(trama_corrupta):
    print("-> EXITO: El CRC-8 Maxim detecta la corrupcion y descarta el paquete.")
