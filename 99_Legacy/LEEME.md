# 99_Legacy — la caja de lo que ya no aplica

**Nada de aquí dentro se lee ni se ejecuta hoy.** Está para explicar un *porqué*, no para
consultarse: la regla del repositorio es que **una causa que desaparece en silencio vuelve a
proponerse**, y la segunda vez ya nadie recuerda que se comprobó.

Si busca lo vigente: firmware en `01_Firmware/Maestro`, `Esclavo`, `Repetidor`,
`ESP32_Expansion`; manuales en `04_Manuales/` y `05_Funcional/`; el entregable de esta semana
es el `.zip` de la raíz y su `LEEME_PRIMERO.md`.

---

## 0. 🔴 LA TRAMPA DE ESTA CARPETA, y ya cobró un hallazgo falso

`Controladora_Semaforos-backups/` contiene **cinco `.kicad_pcb` de 78 bytes** —cabecera y cierre,
sin una sola pista—. Medido hoy con `find -printf '%s'`:

```
78        99_Legacy/Controladora_Semaforos-backups/Controladora_Semaforos-2026-05-05_*/Controladora_Semaforos.kicad_pcb   (x5)
2158421   01_Firmware/Controladora_Semaforos/Controladora_Semaforos/Controladora_Semaforos.kicad_pcb   <-- EL BUENO
2158421   99_Legacy/01_Firmware que funciono ayer ok/.../Controladora_Semaforos.kicad_pcb
2158421   99_Legacy/Entregable_Firmware_y_Manuales_V7/.../Controladora_Semaforos.kicad_pcb
2158421   99_Legacy/Entregable_Firmware_y_Manuales_V8/.../Controladora_Semaforos.kicad_pcb
2158421   99_Legacy/Firmware_Semaforos_v1.0/.../Controladora_Semaforos.kicad_pcb
```

De abrir uno de los de 78 B salió el *«el `.kicad_pcb` está VACÍO»* que sostuvo durante días el
*«no hay medida en el cobre»* — y era falso: el plano bueno trae **185 huellas, 1.447 pistas,
89 vías, 485 pads y 117 redes**. No copiadas: reproducidas el 07/09 sobre el fichero, porque
**KiCad separa los tokens con tabulador**, así que `grep -c '(segment '` —con espacio detrás—
devuelve `0` y un cero se lee como *«no hay»*:

```
grep -oE '\(segment\b'          Controladora_Semaforos.kicad_pcb | wc -l   -> 1447
grep -oE '\(footprint\b'        Controladora_Semaforos.kicad_pcb | wc -l   ->  185
grep -oE '\(net [0-9]+ ' ... | sort -u | wc -l                             ->  117
```

*(Y una advertencia sobre ese último: sin el `sort -u` salen **587** —son las referencias de cada
pad, no las redes—. El patrón que cuenta de más miente igual que el que cuenta de menos.)*

**Hay CINCO copias del plano bueno en el árbol y cuatro están aquí dentro.** La única que se
edita es la de `01_Firmware/`.

---

## 1. Lo que está en el DISCO pero NO en el repositorio

Los `.zip`, `.rar` y `.apk` de esta carpeta —unos **3,9 GB**, dos de ellos de 1 GB y 2 GB— están
**ignorados por `.gitignore` a propósito**: el repositorio ya contiene el fuente del que salen, y
el padre `Controladora_Semaforos` pesa 3,47 GB por exactamente esto, con la historia ya
inempujable a GitHub (límite duro: 100 MB por fichero).

**No los versione.** Se regeneran de su commit con la skill `entregar` o con
`generar_entrega_v9_0.py`; el hash del que salieron va en el propio nombre.

Un `git clone` de este repositorio **no trae nada de esto**, y está bien así.

---

## 2. Lo versionado, y por qué se cayó

| | qué es | por qué se cae | qué lo sustituye |
|---|---|---|---|
| `01_Firmware que funciono ayer ok/` | copia del árbol entero antes de la partición del firmware (`Semaforos`, `RepetidorB`, `Camara`, `deep_sort`) | el firmware se partió en tres puntas | `01_Firmware/Maestro`, `Esclavo`, `Repetidor` |
| `Programa_Semaforos/` | proyecto **STM32CubeIDE** (`Startup/`, `STM32F103C8TX_FLASH.ld`, `Semaforo Debug.launch`) | el proyecto pasó a PlatformIO | los `platformio.ini` de cada punta |
| `Firmware_Semaforos_v1.0/` | la primera versión completa, con su propio árbol dentro | superada por V5…V9 | `01_Firmware/` |
| `Entregable_Firmware_y_Manuales_V7/` · `V8/` | dos entregas **descomprimidas** dentro del árbol | una entrega es un `.zip` que se regenera, no una carpeta que se versiona | el `.zip` de la raíz + `05_Funcional/` |
| `Controladora_Semaforos-backups/` | autoguardados de KiCad del 05/05/2026 | ver §0: **los `.kicad_pcb` están vacíos** | `01_Firmware/Controladora_Semaforos/` |
| `Simulaciones/` (`simulador_all_red.py`, `_crc.py`, `_ruido.py`) · `safety_tests*.py` · `advanced_safety_tests.py` · `simulador_retries.py` · `test_crc8_v7.py` | los arneses sueltos anteriores al banco por packs | **ninguno estaba conectado a la compuerta**, así que un fallo suyo no dejaba rastro | `01_Firmware/Simulaciones/banco/packs/` y `01_Firmware/compuerta.py` |
| `object_detection_tracking.py` · `deep_sort/` | el experimento de YOLO para detectar vehículos | camino **muerto**: hoy las cámaras son **contacto seco** por `J14`/`J16` | `01_Firmware/Maestro/src/modo_inteligente.cpp` |
| `md_to_docx.py` · `md_to_docx_5.py` · `reorg.ps1` | conversores y reorganizador de julio | reemplazados por el empaquetador | skill `entregar` · `generar_entrega_v9_0.py` |
| `MANUAL_FUNCIONAL_DIRECTO/_REPETIDOR.{md,docx}` · `MANUAL_USUARIO.docx` · `2_Manual_Hardware.docx` | manuales de V5–V8.9 | describen un equipo con repetidor y sin app | `04_Manuales/` · `05_Funcional/` |
| `VALIDACION_BANCO_HW.md` · `FORMATO_PRUEBAS_CAMPO.md` · `PLAN_CRECIMIENTO_V2.md` | protocolos de banco y plan de julio | superados | `05_Funcional/3_Protocolo_Pruebas_Rigurosas.md` y la Guía de Cableado |
| `Serial/leer_rs485.py` | lector de RS-485 | el enlace entre puntas es **radio**, no RS-485 | — |
| `Controladora_Semaforos.kicad_sch-bak` | respaldo del esquemático (05/05) | es un `-bak` | `01_Firmware/Controladora_Semaforos/` |

---

## 3. Lo que llegó aquí el 07/09/2026

Tres artefactos **ignorados** que estorbaban en la raíz. Ninguno estaba versionado; moverlos no
tocó el índice de git.

| | por qué se cae |
|---|---|
| `IOT_VIAL_Semaforos_2026-09-04_7d229f0_SIN_BANCO.apk` | **ningún documento la cita.** Su `.zip` gemelo ya vivía aquí |
| `IOT_VIAL_Semaforos_2026-09-04_944c18d_SIN_BANCO.apk` | ídem. `roadmap.md` nombra el **commit** `944c18d`, no este binario |
| `v8_definitiva_changes.patch` | 480 KB del 31/07; **cero citas en todo el árbol**. Su contenido está en el `git log` |

**La APK vigente NO está aquí:** es `IOT_VIAL_Semaforos_2026-09-05_7586c46_SIN_BANCO.apk`, vive
en `05_Funcional/` y la cita `LEEME_PRIMERO.md`. Si instala una de las de esta carpeta, está
instalando una app anterior a la del entregable — y **la del 02/09 y todas las anteriores ni
siquiera abren el socket** (`N-122`, `05_Funcional/14_Manual_App_Movil_IOT_VIAL.md`).
