# 📁 DOCUMENTACIÓN FUNCIONAL Y MANUALES DE CAMPO (05_Funcional)

Esta carpeta centraliza los manuales de operación, guía de cableado, protocolo de pruebas y configuraciones de radio para el personal funcional y técnicos en terreno.

---

> ## 🚨 EMPIECE POR AQUÍ
>
> **1.** Reconfigure las **4 radios** a `2.4 kbps` de Air Data Rate → **[`4_Manual_Configuracion_Radios.md`](4_Manual_Configuracion_Radios.md)**
> **2.** Cargue el firmware **en las dos tarjetas, la MISMA versión** → [`2_Manual_Hardware_y_Pruebas.md`](2_Manual_Hardware_y_Pruebas.md) §4
> **3.** Ejecute el checklist y firme el acta → **[`3_Protocolo_Pruebas_Rigurosas.md`](3_Protocolo_Pruebas_Rigurosas.md)**
>
> El paso 1 **no es opcional**: es la corrección de la causa raíz del fallo de comunicación que aparecía
> al paso de cada ciclo, en los tres modos, y del repetidor que no enlazaba. Sin él, el firmware nuevo
> no resuelve el síntoma.

---

> ## 📌 EL SISTEMA CAMBIÓ EL 01/08/2026 — LEA ESTO ANTES DE USAR LOS MANUALES
>
> Los manuales se escribieron cuando el equipo tenía **4 opciones de menú en una lista plana y ningún
> modo por reloj**. Ya no es así:
>
> | Novedad | Dónde está documentada |
> |---|---|
> | **Menú en dos niveles** (`CONFIGURACION` como cuarta opción) | `1_Manual_Usuario.md §3` |
> | Pantalla **AJUSTAR HORA** y **sincronización horaria por radio** | `1_Manual_Usuario.md §4` |
> | **MODO DEGRADADO** — operación por reloj sin radio | **`8_Procedimiento_Modo_Degradado.md`** |
> | **Mando de 4 relés** y sus secuencias desde el piso | `1_Manual_Usuario.md §6` · `2_Manual_Hardware_y_Pruebas.md §6` |
> | El **Esclavo ahora tiene pantalla y menú propios** | `1_Manual_Usuario.md §7` |
> | **Pila `CR2032`** del reloj en ambas tarjetas | `2_Manual_Hardware_y_Pruebas.md §5` |
>
> **La versión que corre en campo hoy es la V8.4**, que **no** incluye nada de lo anterior. Todo lo
> nuevo está construido y validado en simulador, pero **sin prueba de banco ni de campo**. Las
> Secciones 7 a 10 del protocolo de pruebas son su primera verificación física.

> ## ⚠️ SI VA A OPERAR EL MODO DEGRADADO
>
> **Lea completo el [`8_Procedimiento_Modo_Degradado.md`](8_Procedimiento_Modo_Degradado.md) antes de
> tocar nada.** Ese modo da verde **sin confirmación del otro extremo** —con el radio muerto es
> inevitable— y tiene riesgos residuales que el cliente aceptó por escrito. Tres cosas que hay que
> saber de entrada:
>
> - Se activa **en las dos puntas** y **exige verificación visual de ambas**, al entrar **y al salir**.
> - **Hoy el Esclavo no tiene receptor de mando:** activarlo en esa punta **obliga a subir al gabinete**.
> - **Límite duro de 48 h:** pasado ese tiempo sin resincronizar, el modo **cae solo a ámbar**.

---

> ## 📷 CÓMO DETECTA VEHÍCULOS EL EQUIPO — CORRECCIÓN DEL 28/08/2026
>
> **Este README anunciaba «integración con visión artificial (YOLOv8)». Se corrige, y queda escrito
> qué decía**, porque lo que se borra en silencio se vuelve a proponer.
>
> **Lo que el firmware hace de verdad hoy: lee CONTACTOS SECOS, y nada más.** La analítica de vídeo
> la hace **el DSP de la propia cámara Hikvision AcuSense**; el equipo solo ve un pulso de 1 segundo.
>
> **Desde el 31/08 son TRES entradas por punta, no una** —este README decía *«lee UN contacto seco
> en `PB0`»*—:
>
> | entrada | pin | bornera | antirrebote de placa | estado |
> |---|---|---|---|---|
> | `CAM_DEMANDA_PIN` | `PB0` | `J14` | ✅ `R64` 10 kΩ + `C25` 100 nF | ✅ **cableable hoy** |
> | `CAM_C_PIN` | `PB14` | `J16` p10 | ❌ ninguno | 🟠 firmware listo — **NO cablear hasta `M3`** |
> | `CAM_D_PIN` | `PB15` | `J16` p12 | ❌ ninguno | 🟠 firmware listo — **NO cablear hasta `M3`** |
>
> **Las tres son de DEMANDA y activas en ALTO**: el contacto cierra contra **3,3 V**, no contra masa.
>
> | | |
> |---|---|
> | **NO hay** visión artificial embarcada en el STM32 | 64 KB de flash; el Maestro ya va al ~86 % |
> | **NO hay** YOLO, Raspberry Pi, Jetson ni PC en obra | Descartado **por decisión escrita** en `6_Preguntas_Diseno_Funcional.md` §2: *«Cero Computadores Edge Externos (CERRADO)»* |
> | **NO existe** el puerto serie de cámara IA (`AI_CARS`) | **`AiBus` y sus tres funciones están RETIRADOS**, no huérfanos: `grep AiBus` sobre las dos puntas sólo devuelve comentarios de historia. Colgaba del mismo USART1 que el Bluetooth, así que *«el puerto IA a 115200» nunca existió*; el enlazador ya descartaba las funciones, pero el objeto costaba **280 B de RAM por punta** en cada arranque |
> | **NO está construida** la cámara de umbral (despeje de tramo) | *Especificado, sin construir.* Falta una entrada física y un comando de radio. El despeje se hace **por tiempo** (`cfgDespejeSeg`), que es el criterio conservador. Ver `9_Manual_Parametrizacion_Camara_IA.md` |
>
> **Y el manual `04_Manuales/MANUAL_CONFIGURACION_CAMARAS_IA.md` salió el 26/08 con dos errores de
> pin**, corregidos el 28/08 y registrados en su §0: asignaba la cámara de demanda a **`PB9`**.
>
> 🔴 **`PB9` y `PB13` son hoy `MANDO_A` y `MANDO_B`, los canales del mando de relés** (`J16` p5 y
> p8), y `PB8` es el **LED testigo `D5`**, no una bornera. **Cablear una cámara a `PB9`/`PB13` no
> inyecta «pulsaciones de menú»: compone SECUENCIAS DE MANDO.** Tres pulsos de tráfico en 12 s hacen
> `A·A·A` o `B·B·B` y **el semáforo cambia de modo solo**; cuatro alternos en 18 s lo meten en
> Degradado.
>
> **Evidencia:** todo lo anterior está **MEDIDO** sobre el fuente y el esquemático (`grep` de las
> llamadas y `pines.h` de las dos puntas). **Ninguna línea está VERIFICADA EN LA PLACA todavía**:
> la sesión de banco es su primera comprobación física.
>
> Referencia de campo vigente: **[`9_Manual_Parametrizacion_Camara_IA.md`](9_Manual_Parametrizacion_Camara_IA.md)**
> y **[`15_Lista_de_Compras_Hardware.md`](15_Lista_de_Compras_Hardware.md)**. **2 cámaras es el
> montaje mínimo y el único cableable hoy** —una por poste, en `J14`—; las dos entradas de `J16` de
> cada punta están en el firmware y **esperan la medida `M3`**.

---

## 📄 Índice de Manuales y Documentos Core:

1. 📘 **[1_Manual_Usuario.md](file:///d:/@Proyect/Controladora_Semaforos/05_Funcional/1_Manual_Usuario.md)** / **`1_Manual_Usuario.docx`**  
   Manual de operación y secuencia de luces seguras bajo la **Resolución 2024 de MinTransporte Colombia**.
   Incluye el **menú de dos niveles**, **AJUSTAR HORA**, el **mando de 4 relés** y el **menú propio del Esclavo**.
2. 📘 **[2_Manual_Hardware_y_Pruebas.md](file:///d:/@Proyect/Controladora_Semaforos/05_Funcional/2_Manual_Hardware_y_Pruebas.md)** / **`2_Manual_Hardware_y_Pruebas.docx`**  
   Guía de ensamblaje, cableado de borneras RS485 `485_A` / `485_B` (A a A, B a B) y flasheo en PlatformIO.
   Incluye la **pila `CR2032` del reloj** (§5) y el **mando de relés**, con la advertencia de que **el Esclavo no tiene receptor** (§6).
3. 📘 **[3_Protocolo_Pruebas_Rigurosas.md](file:///d:/@Proyect/Controladora_Semaforos/05_Funcional/3_Protocolo_Pruebas_Rigurosas.md)** / **`3_Protocolo_Pruebas_Rigurosas.docx`**  
   Checklist obligatorio de pruebas de laboratorio y campo para certificar el equipo antes de puesta en marcha.
   **68 pruebas**, con las Secciones **7 (reloj y sincronización)**, **8 (mando)**, **9 (Modo Degradado)** y **10 (interfaz del Esclavo)** nuevas.
4. 📘 **[4_Manual_Configuracion_Radios.md](file:///d:/@Proyect/Controladora_Semaforos/05_Funcional/4_Manual_Configuracion_Radios.md)** / **`4_Manual_Configuracion_Radios.docx`**  
   Configuración de radios industriales **E90-DTU** con `RF_Setting4.6.exe` y DIP switches `M0`/`M1`.
5. 📘 **[5_Manual_Puente_ESP32.md](file:///d:/@Proyect/Controladora_Semaforos/05_Funcional/5_Manual_Puente_ESP32.md)** / **`5_Manual_Puente_ESP32.docx`**  
   Instrucciones para la instalación del puente repetidor con ESP32 (Modo 4 Radios para curvas ciegas).
6. 📘 **[6_Preguntas_Diseno_Funcional.md](file:///d:/@Proyect/Controladora_Semaforos/05_Funcional/6_Preguntas_Diseno_Funcional.md)** / **`6_Preguntas_Diseno_Funcional.docx`**  
   Cuestionario y parámetros de diseño de obra, y la **detección vehicular por contacto seco**.
   Contiene la decisión **CERRADA** de *«Cero Computadores Edge Externos»* (§2): la analítica corre
   **dentro de la cámara**, y al equipo llega **un pulso de hardware** — no vídeo ni datos.
7. 📡 **[7_Especificacion_Antenas.md](file:///d:/@Proyect/Controladora_Semaforos/05_Funcional/7_Especificacion_Antenas.md)** / **`7_Especificacion_Antenas.docx`**  
   **Especificación para fabricación de antenas bajo pedido.** Documento para entregar al proveedor:
   sintonía a **171 MHz**, ROE ≤ 1,5:1 en 168–174 MHz, sin plano de tierra y con **reporte de medición
   de ROE exigido como entregable**. Resuelve la causa del alcance de 3 cuadras medido el 31/07.
8. 🕹️ **[8_Procedimiento_Modo_Degradado.md](file:///d:/@Proyect/Controladora_Semaforos/05_Funcional/8_Procedimiento_Modo_Degradado.md)** / **`8_Procedimiento_Modo_Degradado.docx`**  
   **Procedimiento de campo del MODO DEGRADADO.** Requisitos previos, activación **en las dos puntas**
   con verificación visual de ambas, el **límite duro de 48 h**, la salida —también verificada en
   ambas puntas— y los **riesgos residuales aceptados por el cliente**. **Obligatorio leerlo antes de
   operar ese modo.**

---

## 🔄 Regeneración de los entregables Word

Los `.docx` **se generan desde los `.md`**, que son la única fuente de verdad:

```bash
python 05_Funcional/convertir_a_word.py        # regenera los 8
python 05_Funcional/convertir_a_word.py 3 8    # solo los numerados 3 y 8
```

> El script descubre solo los `.md` numerados de la carpeta, así que el documento **8** se genera sin
> tocar nada del script.

> **No edite los `.docx` a mano:** cualquier cambio se pierde al regenerarlos. Edite el `.md`
> correspondiente y vuelva a ejecutar el script.
>
> *Archivo obsoleto:* `2_Manual_Hardware.docx` es un remanente del nombre anterior del documento 2.
> El vigente es `2_Manual_Hardware_y_Pruebas.docx`.

---

## 🔗 Matriz de Referencia Cruzada:
- **`ARQUITECTURA.map` $\leftrightarrow$ `05_Funcional`:** Mapeo directo entre la topología de red RS485/LoRa y las instrucciones físicas de conexión.
- **`README.md` & `roadmap.md` $\leftrightarrow$ `05_Funcional`:** Garantiza el cumplimiento de las fases de desarrollo (Fase 1 Lógica Core, Fase 2 «AI Edge», Fase 3 Protocolo Binario V7 y Fase 4 Pruebas Físicas).
  > *Nota (28/08/2026):* el nombre **«Fase 2 AI Edge»** es histórico y **no describe lo construido**.
  > Lo que quedó de esa fase es la lectura de **contactos secos** —`PB0` en `J14`, y `PB14`/`PB15`
  > en `J16` desde el 31/08— ver el aviso de cámaras más arriba. No hay computador edge ni
  > inferencia en el microcontrolador.
