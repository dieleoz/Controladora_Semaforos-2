# LÉEME PRIMERO — paquete del 04/09/2026

## 1. Qué corre hoy en la calle, y que esto NO es eso

**En campo corre `V8.4`, commit `e303485`, del 31/07/2026.**

Este paquete es `2e24f97`. Todo lo que hay dentro es posterior a lo que está instalado.

## 2. ¿Ha pasado banco? **NO.**

Con esas palabras, porque es lo único que importa antes de decidir instalar nada.

La **sesión 2 de banco (3–4/09)** paró con defectos. Lo que va en este paquete son los
arreglos **posteriores a esa sesión**, verificados en el PC y **sin volver a la tarjeta**.

La compuerta sale en **`20 PASS · 0 FALLA · 0 ABORTADO`** y el banco por packs en
**`985/985` sobre 68 packs**. Eso significa exactamente esto: *los modelos y los arneses de
PC no encuentran nada*. **No dice que el firmware funcione en la tarjeta.**

La prueba está en este mismo paquete: los tres defectos que pararon el banco del 3–4/09
pasaron esas mismas 20 comprobaciones sin despeinarlas.

## 3. 🔴 Lo que está roto o abierto, y hay que leer antes de tocar

### 3.1 · El Modo Automático no mueve las luces en la tarjeta (`N-42`) — SIN CERRAR

Sigue abierto y **no está diagnosticado**. Una lectura del fuente de esta sesión apunta a
que `modoAutomatico_setup()` deja el modo en fase `CONFIG_ROJO`, que sólo avanza con
`botonAceptar()`, y ése devuelve `false` desde el 31/08. En esa fase
`coordinador_actualizar()` no se llama. **Es una lectura de código, no un diagnóstico:**
nadie lo ha ejercido en tarjeta. No se cambia nada sobre esa base.

### 3.2 · 🔴 No hay puente H en la PCB, y sólo sale UNA línea de control a la pluma

Censado sobre el `.kicad_pcb`: de `L298`, `L293`, `DRV8x`, `BTS7x`, `TB6612`, `IBT` y
`BTN8x` hay **cero**. La talanquera es un MOSFET de lado bajo —`PB2` → opto `U15` →
`Q10` → `J15`— y `J15` es un `Conn_01x02`: **dos bornes, un sentido**.

**Está decidido poner un `L298N` por barrera, y ESO NO SE CABLEA TODAVÍA.** Un puente H
necesita dos entradas de control y de esta placa sale una. Además `J15` conmuta **masa**,
no entrega 5 V. Y con un motor de 12 V el `L298N` cae entre 2 y 5 V, que en par es más de
la mitad. **Pendiente de decidir el esquema y de conocer la corriente del motorreductor.**

### 3.3 · No se cablean tres cámaras: **son DOS, una por poste**

El manual anterior listaba tres entradas y alguien podía leer tres cámaras. Los tres
bornes —`J14`/`PB0`, `J16` p10/`PB14`, `J16` p12/`PB15`— acaban en la **misma** petición de
paso. **La cámara va a `J14`**, que es el único con antirrebote por hardware
(`R64` 10K + `C25` 100nF). Corregido en los manuales de este paquete.

### 3.4 · El rótulo Bluetooth no es fiable el primer día

El ESP32 aprende su nombre (`SEM-<serie>-M` / `-E`) del `$STATUS` y **lo guarda para la
arrancada siguiente**. Un módulo virgen anuncia `SEM-SIN-MATRICULA`, y **los dos postes se
llaman igual** hasta que se les da una vuelta de energía después de que el STM32 hable.

## 4. Qué trae de nuevo

- **El ciclo no baja de 3 minutos.** Decisión vial del responsable: *«es la mínima
  distancia de seguridad»*. La guarda vive en el firmware, no en la app.
  **Coste aceptado a sabiendas: ya no se puede probar en mesa con ciclos de un minuto.**
- **`N-131`** — esa guarda era media guarda: había cinco sitios más con el mínimo viejo
  escrito a mano, incluido un despeje que por pantalla bajaba a 5 s. Cerrado.
- **`N-130`** — el equipo ya no dice que sí a lo que no va a hacer. Pedir paso desde el
  Esclavo con el cruce fuera de Modo Inteligente ahora **se rechaza y se avisa**, en vez de
  contestar «pedido al Maestro» y no mover nada.
- **App:** todo lo que se pulsa llega a 44×44 px, ningún par de objetivos pegado, y la
  pantalla dice **a qué poste estás conectado y qué mandos tiene** antes de pulsar.
- Manuales y guía de cableado corregidos: ámbar de emergencia del Maestro (no existe),
  parar el Esclavo con `FORZAR_ROJO` (está rechazado), despeje de «5 a 999 s» (imposible).

## 5. Qué hay en el paquete

| | |
|---|---|
| `IOT_VIAL_Semaforos_2026-09-04_2e24f97_SIN_BANCO.apk` | la app, verificada byte a byte contra el fuente |
| `ACTA_verificacion.txt` | el acta de la corrida: fecha, `HEAD`, toolchain |
| `01_Firmware/` | **fuente** para PlatformIO. Sin binarios: se compilan del código que se revisa |
| `02_Manuales/` | `.docx` y `.md` |

**El orden de trabajo es: firmware primero, cargado y verificado en la tarjeta; el cableado
después.** Nunca al revés. Un commit no protege de un destornillador.

## 6. Carga por SWD

`mode=UR` con `-e all`, y no se cambia. Si falla, **se reintenta** — enganchar es cuestión
de *timing* y puede fallar varias veces con `Unable to get core ID`. Eso no es falta de
cableado. `HOTPLUG` con un firmware que se cuelga al arrancar deja
`failed to erase memory`.
