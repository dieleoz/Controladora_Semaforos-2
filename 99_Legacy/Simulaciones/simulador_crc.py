import sys

def calcular_crc(texto: str) -> int:
    crc = 0
    for char in texto:
        crc ^= ord(char)
    return crc

def simular_envio(mensaje_base: str, msg_id: int):
    payload = f"{mensaje_base},{msg_id}"
    crc = calcular_crc(payload)
    paquete = f"{payload},{crc:02X}"
    print(f"[TX STM32] Mensaje Original: '{mensaje_base}' -> Paquete al aire: '{paquete}'")
    return paquete

def simular_recepcion(paquete_recibido: str, ultimo_id: int):
    print(f"\n[RX STM32] Llegó del aire: '{paquete_recibido}'")
    
    partes = paquete_recibido.rsplit(',', 2)
    if len(partes) != 3:
        print("  -> ERROR: Formato inválido. Descartado.")
        return ultimo_id
        
    texto, id_str, crc_str = partes
    
    try:
        crc_recibido = int(crc_str, 16)
        id_recibido = int(id_str)
    except ValueError:
        print("  -> ERROR: ID o CRC no numérico. Descartado.")
        return ultimo_id
        
    payload_validar = f"{texto},{id_str}"
    crc_calculado = calcular_crc(payload_validar)
    
    if crc_recibido != crc_calculado:
        print(f"  -> ERROR CRC: Esperaba {crc_recibido:02X}, Calculó {crc_calculado:02X}. (Ruido detectado). Descartado.")
        return ultimo_id
        
    if id_recibido == ultimo_id:
        print(f"  -> ERROR REPLAY: El ID {id_recibido} ya fue procesado. (Eco del repetidor). Descartado.")
        return ultimo_id
        
    print(f"  -> ÉXITO: Mensaje '{texto}' validado correctamente. Nuevo ID: {id_recibido}")
    return id_recibido

if __name__ == "__main__":
    print("=== SIMULACIÓN DE CRC Y REPLAY ATTACK (SFTY-3) ===\n")
    
    ultimo_id_esclavo = 0
    
    # 1. Envio normal
    paquete1 = simular_envio("GO_GREEN", 100)
    ultimo_id_esclavo = simular_recepcion(paquete1, ultimo_id_esclavo)
    
    # 2. Ruido corrompe un caracter en el aire (GO_GREEN -> GO_GREXN)
    paquete2 = "GO_GREXN,101,2F"
    ultimo_id_esclavo = simular_recepcion(paquete2, ultimo_id_esclavo)
    
    # 3. Ruido corrompe el CRC
    paquete3 = "GO_RED,102,FF"
    ultimo_id_esclavo = simular_recepcion(paquete3, ultimo_id_esclavo)
    
    # 4. El repetidor hace eco del paquete 1 (Replay Attack)
    ultimo_id_esclavo = simular_recepcion(paquete1, ultimo_id_esclavo)
    
    # 5. Nuevo mensaje válido
    paquete4 = simular_envio("GO_RED", 103)
    ultimo_id_esclavo = simular_recepcion(paquete4, ultimo_id_esclavo)
    
    print("\nSimulación finalizada. SFTY-3 Funciona correctamente.")
