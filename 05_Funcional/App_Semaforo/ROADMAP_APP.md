# ROADMAP de la app — mejoras aprendidas de Pasos Peatonales

Version siguiente de la app (APK y web de PC), no de este banco. Fuente: lo que la sesion de Pasos Peatonales
(`D:\@Proyect\IoT\Pasos Peatonales`, HEAD `e968cc7`) respondio el 08-09/10 sobre como rehizo la app heredada de esta.
Nada de aqui esta construido en Semaforos 2; alli, «sin probar» = sin placa. Cada fila entra primero en `SPEC_4` (o la
SPEC que toque) y su prueba se entrega vista en rojo (`CLAUDE.md` §15). Lo que es del responsable va marcado DECIDE.

## Lo que tomaron de nosotros, y como lo mejoraron

| tomado de aqui | mejora alli | fichero alli (`03_App_Movil/web/` = W) |
|---|---|---|
| formato de trama y XOR-8 | verificado en cada linea, en un modulo | `W/js/trama.js` |
| `serial_pc.js` (Web Serial) | una interfaz para USB y SPP; probada con placa (USB) | `W/js/serial_pc.js` |
| exportar (filesystem + share) | PDF, .txt y .csv en un toque, hoja de compartir | `W/js/compartir.js` |
| empaquetador con CRC APK contra `www/` | `www/` regenerado desde `web/` al compilar | `apk/scripts/generar_www.js` |
| L-06 (gradle por java) | `compilar.sh` con `java -classpath` por los espacios de la ruta | `apk/compilar.sh` |
| L-12, L-13, `$ACK` solo con el resultado | escritos en su SPEC desde el principio | `SPEC_8:24-26` |
| app.js monolitico, PIN en claro | NO heredados: 27 modulos; clave como huella | — |

## Lo que se trae, en orden

Orden por dano en campo, no por facilidad. Cada fase cierra con sus pruebas en verde y una APK `_SIN_BANCO`.

### Fase 1 — Lo que hoy confunde al tecnico en el poste (pide poco firmware o ninguno)

1. **Hoja de banco con «Por que» y «Debe ver» (estado de partida) en cada caso.** El 06/10 Marco probo la (b) en el
   modo equivocado y la cinta no sirvio. Solo `.html`; entra en la siguiente hoja.
2. **Un color, una accion** (rojo ir, ambar mirar, verde, gris sin datos) y **una frase por estado**, sin LED ni
   jerga; tramas y contadores a una pestana «Datos tecnicos». Su revision UX encontro 18 problemas, contraste
   3,15:1 y toques de 13 px: aqui no se ha medido. Primero se mide (punto 4), despues se cambia.
3. **Cabecera en dos lineas; el estado del enlace fuera de ella**, en su franja. Cierra la fila de version
   siguiente del 06/10 de `roadmap.md` (rotulo recortado).
4. **Revision UX en simulado**: un agente con Chrome sin cabeza a 360x640 y 1280x800, capturas y lista de problemas
   con contraste calculado. Es una medida, no un entregable: de ella sale la lista de la fase 2.
5. **Dialogos propios en vez de `prompt`/`confirm`/`alert`** nativos (alli salian en ingles). Hoy hay 5 en `app.js`.

### Fase 2 — Modo administracion y estructura

6. **Lo que configura, solo con clave.** Hoy el rol se cambia con PIN (`btn-toggle-role`, pestanas `.admin-tab`
   ocultas). Alli: clase `.solo-admin` ocultada por CSS sin `admin` en `body`; sin clave, el tecnico ve franja,
   luces de los dos postes, probar, enviar acta y radios en palabras. DECIDE: que funciones son de administracion
   aqui (tiempos, Degradado, depuracion, hora). Prueba alli: `test_v46_dom.js` (10/13 en rojo antes).
7. **Clave cambiable y recuperacion**: 4-8 cifras, en `localStorage`, dura hasta cerrar la app; clave de soporte
   guardada como huella con sal, fuera de manuales. Es barrera de uso, no seguridad del equipo: la del equipo
   sigue siendo el PIN del firmware (`SPEC_4` §4). DECIDE: si se quiere clave de soporte.
8. **`app.js` (338 KB) partido en modulos y `www/` regenerado en cada compilacion.** Se hace por tandas al tocar
   cada zona, no de golpe: los instrumentos leen el fuente por RUTA (`CLAUDE.md` §5) y cada movimiento lleva su
   ruta en el mismo commit.

### Fase 3 — Lo que pide firmware (ESP32 del puente o STM32) y una decision

9. **Alarmas que se ven aunque nadie estuviera conectado**: el equipo repite la alarma mientras dure y, al conectar,
   la app pide `ALARMAS` y el equipo lista las activas con `FIN,<n>`; el cierre repite la CAUSA de la apertura;
   toda frase sale de una tabla. Cierra las tres `DEGRADADO` de limite que salen una sola vez (`SPEC_6` parte C).
10. **Registro en el equipo, no en la cinta de la app**: anillo en RAM y FIFO en flash con CRC; la app pide
    `LOG,DESDE:<N>` y recuerda el ultimo N. Cabria en el ESP32 del puente, no en el STM32 (flash en la ultima acta,
    `CLAUDE.md` §10). DECIDE: esto o las cuatro mejoras del registro de la app aprobadas el 06/10 (`ESTADO.md`).
11. **Reinicios fechados**: `$STATUS` con REIN, ULT_REIN, UP_ANT y HEAP_MIN; la app alarma si un contador baja sin que
    cambie REIN. Habria explicado la caida de 1789 a 91 del 06/10 a las 10:43:30.
12. **Todo `$ACK` lleva `RES`/`APLICA` o es `$ERR`**; la orden que no cambia nada da `MOTIVO:SIN_CAMBIO` y la app
    dice «ya estaba asi» (`SPEC_4` §7, el cambio bajo `if (modo != modoAnterior)`). Alli tambien quedo a medias.
13. **SPP que no bloquea en el puente**: un `write` SPP espera hasta 1 s; si no hay sitio se descarta y se cuenta.
    Antes, un arnes que compile `transporte_app.cpp` y lo vea fallar con la cola llena (`SPEC_4` §7: hoy nadie).

## Lo que NO se trae, y por que

- **Simulador dentro de la app con flota de cruces.** Alli sale activo en la APK de campo (sin filtrar en
  `generar_www.js`) y cada campo nuevo del firmware obliga a tocarlo a mano. Esta app ya quito un «SIMULADOR DE
  PRUEBAS - DEMO EN VIVO» que pintaba fases inventadas sobre los mismos semaforos que la telemetria (`app.js`,
  comentario de la pestana de depuracion), y el simulador de este repo esta CONGELADO (`CLAUDE.md` §4). Si se quiere
  probar la UI sin placa, el camino es el de sus pruebas: tramas capturadas del firmware compilado e inyectadas en
  jsdom/Chrome sin cabeza, que es lo que ya hacen `tests/dom_*.js`. DECIDE si se quiere ademas un modo de practica.
- **Claude Design como paso obligatorio.** Alli el brief no quedo en el repo y la app se comparo contra el lienzo a
  mano. Sirve para la fase 1 si se guarda el brief y el lienzo en el repo; no sustituye a la revision del punto 4.
- **Reenvio automatico de una orden tras pedir la clave** (alli dio `CLAVE, PRUEBA, HORA`; arreglado en `ca3ab68`).
- **Codigo por delante de la SPEC**: alli lo pagaron dos veces (`ca3ab68`, `93b8977`).

## Validar como un usuario

Su caso modelo es `W/pruebas/test_qa45_dom_clave.js`: sonda dentro del `index.html` real, puerto falso con checksum
real, `prompt` respondido, cada orden registrada; Chrome sin cabeza con `--dump-dom`. Sin navegador, OMITIDA; sonda
que no corre, ABORTADO; ninguna cuenta como PASS. Aqui `tests/dom_*.js` ya corre `app.js` en jsdom: lo que falta es
el guion por TAREA del tecnico (conectar, leer estado, exportar, entrar en Degradado) y el contraste calculado.
Sin nuevo instrumento si la razon instrumentos/producto no lo permite (`CLAUDE.md` §4, trinquete).
