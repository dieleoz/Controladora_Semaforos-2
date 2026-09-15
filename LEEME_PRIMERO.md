# LEEME PRIMERO — paquete de banco del 15/09/2026

**Esto es un ENCARGO DE BANCO: una peticion de medida, no una version para instalar.**

## 1. Que corre en campo hoy

- La instalacion certificada es la **V8.4, commit `e303485`** (31/07/2026).
- El **Maestro del Sisga** (`SERIE:179DB0`) corrio el 10/09 con **V9 `SIN_BANCO`**: se cargo `7ff7d12` y
  despues se probo `b354fe9`. Cual quedo dentro no consta. El firmware del Esclavo del Sisga, SIN VERIFICAR.
- **Este paquete no es nada de eso.** Es el firmware del commit **`ca2de3d`**, candidato a primer estable.

## 2. ¿Ha pasado banco? **NO.**

Nada de lo construido desde el 05/09 ha visto una tarjeta. **No se instala en ningun equipo de campo, no se
carga en el equipo del Sisga y el sufijo `SIN_BANCO` no se quita.** Lo quita quien lo pruebe en un equipo.

## 3. ALTO — lo que hay que hacer ANTES de enchufar nada

1. **Cargar por SWD con `mode=UR` y `-e all`.** Si falla, se reintenta; no se cambia el modo.
2. **Verificar el hash de lo CARGADO**: `sha256sum` del fichero antes, y de la flash leida de vuelta despues.
   Tienen que coincidir con los de §6. Nunca por el tamano.
3. **`J16` p1 lleva 12 V crudos directos al micro: se retira el pin del conector volante.** Siempre.
4. **`J14` es una ENTRADA del micro: ahi no se conecta nada.** La talanquera va en `J15`.
5. **`J16` p5 y p8 no se cablean ni se puentean.** Solo la prueba C del encargo, en la mesa, y **nunca en el
   equipo del Sisga**.
6. Camaras de prueba: `p10` contra `p9`, `p12` contra `p11`. Nunca contra masa.
7. Ninguna radio se energiza sin su antena.

## 4. Lo que sigue abierto, antes de las novedades

- La compuerta (`evidencia/2026-09-15_compuerta.txt`) da **18 PASS · 1 FALLA · 0 ABORTADO**. El FALLA es
  correcto: acusa a `D-22` (el cristal `Y1`), decidida y sin construir. En el banco `Y1` no se toca.
- **El caso exacto de `D-34` —la orden de verde llega y su acuse no vuelve— no se puede reproducir con dos
  radios en una mesa.** El encargo lo dice y pide lo que si se puede medir.
- **El puente ESP32 tambien se carga, en los dos postes**: sin el de este commit la hora no llega al STM32 y dos
  pruebas no se pueden hacer.
- **Un verde de la compuerta no dice que el firmware funcione en la tarjeta.** Por eso existe el encargo.

## 5. Que trae, por lo que puede herir

- **`D-34`**: el verde de un poste ya no sobrevive al ambar intermitente del otro cuando se cae la radio. El
  poste 2 suelta su verde antes de su silencio y lo avisa; el poste 1 no le entrega el verde al poste 2 sin
  haberlo oido en el ultimo latido.
- **`D-33`**: la pluma baja unos segundos despues del rojo, y cualquier camara del poste retiene la bajada
  mientras vea algo. Si la retencion se alarga, la app abre un cartel de **barrera retenida**.
- **El mando esta fuera del firmware**: un puente en `J16` p5/p8 ya no compone secuencias **con este
  firmware**. Con el viejo, si. Por eso esos bornes siguen sin cablearse.
- **El cristal que arranca y no cuenta**: el equipo deja de creerse una sincronizacion que no puede fechar, no
  reanuda el Degradado sin veredicto del cristal, `REINICIAR_RELOJ` ya no contesta OK sin comprobar que cuenta,
  y el Maestro alarma la causa cuando cae a ambar —sin confundir «no hay cristal» con «el cristal no
  cuenta»— (`1.49` a/b).
- **Una camara pidiendo paso ya no sostiene la radio**: el poste 1 cuenta su silencio desde lo que el poste 2
  le CONTESTA, no desde cualquier trama (`1.49` c). Antes, con la radio del poste 1 al 2 cortada, el poste 1
  podia seguir en verde toda su fase frente al ambar del otro.
- **La app**: traduce el aviso de barrera retenida y la respuesta nueva del reloj.

## 6. Que hay dentro, y que documento se ejecuta

**El documento que se ejecuta es `05_Funcional/ENCARGO_BANCO_15-09.md`.** Tiene las pruebas, los huecos que
se rellenan y como se devuelve. Este LEEME no sustituye a ninguna de sus casillas.

- Paquete: `Paquete_Banco_2026-09-15_ca2de3d_SIN_BANCO.zip`
- Maestro: `Maestro_2026-09-15_ca2de3d_SIN_BANCO.bin` · 42228 B · sha256
  `317f8e463cb461413594148102e2f670e12f922d029a23ce12de66f1593157f4`
- Esclavo: `Esclavo_2026-09-15_ca2de3d_SIN_BANCO.bin` · 36532 B · sha256
  `3884e6afaf75d38338f9be183332f1aa8673c24788928d22069e47afd3773434`
- App: `IOT_VIAL_Semaforos_2026-09-15_622a20b_SIN_BANCO.apk` · 4.020.773 B · sha256
  `811f6399555f02fa0e53fe8bd74038013be5a7d0fefbfff5fd58593eb3f03688`. Se verifica ESTE fichero: una
  recompilacion del mismo commit en otro PC puede dar otro sha256.
- Puente ESP32: `ESP32_Expansion_2026-09-15_ca2de3d_SIN_BANCO.bin` · 1130096 B · sha256
  `171ceb3cf5c1db897c34a9cb0c04456cd509e79a0cb0c1198e886b90d5534437`
- **Los `.bin` NO van dentro: va el FUENTE para PlatformIO y va la APK.** Los sha256 de arriba son los de esta
  maquina (compilados desde cero dos veces, mismo hash); si tu compilador da otro, anota el que cargues.
- El acta de la compuerta no va en el paquete: vive en `evidencia/` del repositorio.

**Nota mecanica.** Este LEEME se comitea despues de `ca2de3d`: un fichero no puede contener el hash de su
propio commit. El firmware es el de `ca2de3d`, y se comprueba con
`git diff --name-only ca2de3d..HEAD -- 01_Firmware`, que tiene que salir **vacio**.
