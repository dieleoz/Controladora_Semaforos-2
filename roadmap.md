# Roadmap — Controladora de Semaforos Moviles de 3 Estados (V9.1)

Lo pendiente, en orden de ejecucion y con quien lo desbloquea. Sin bitacora: una fila que se cierra sale, literal y con
su commit, a un historico nuevo de `05_Funcional/historico/` en la pasada siguiente; derogar es borrar, no tachar. Tope:
1.000 lineas, y `wc -l roadmap.md` da la cuenta. Si el tope aprieta, no se entierra un pendiente: se pregunta.

> **Citas desde fuera.** `§3`, `§6`..`§10` (`§3.16`, `§3.10.bis`, `§3.8`, `§3.4.bis`...) viven en
> [`roadmap_hist.md`](05_Funcional/historico/roadmap_hist.md); la tabla de `§0` del 11/09 («fila 1.x», «fila 2.x») y
> `§2.8`, en [`roadmap_hist_2026-10-14.md`](05_Funcional/historico/roadmap_hist_2026-10-14.md) (o en `roadmap_hist.md`);
> el bloque U (lo cita SPEC_4 §9.9), en
> [`roadmap_hist_2026-11-09.md`](05_Funcional/historico/roadmap_hist_2026-11-09.md); el `§0` y la
> «Version siguiente» hasta el 10/11, literal, en
> [`roadmap_hist_2026-11-10.md`](05_Funcional/historico/roadmap_hist_2026-11-10.md). La antigua «orden, fila 6»
> (`ESTADO.md`) es hoy la fila 10. Las citas a `CLAUDE.md` §x.bis son de antes del 07/09: la regla, por su titulo.

## Que manda sobre este fichero

| | |
|---|---|
| **la decision vigente** | la SPEC (`05_Funcional/SPEC_0..SPEC_8`); [`DECISIONES.md`](DECISIONES.md) es su indice |
| **el hardware medido** | `05_Funcional/17_Arquitectura_28-08_y_Decisiones_Abiertas.md`: gana en cobre, pines y conectores |
| **las reglas permanentes** | [`CLAUDE.md`](CLAUDE.md) |
| **el estado de hoy y que firmware hay en cada equipo** | [`ESTADO.md`](ESTADO.md) |
| **las cifras** (compuerta, banco, flash, arneses) | la ultima acta de `evidencia/`; no se copian aqui |

Instalacion certificada: `V8.4`, `e303485`. Todo lo posterior es `SIN_BANCO` (El Sisga corre `7ff7d12`). Verde no es
entregable: los tres defectos del banco del 03-04/09 pasaron la compuerta entera.

---

## 0. Orden de trabajo (10/11): esto manda; el detalle, en la SPEC y en cada `N-x`/`D-x`

| # | que | quien | estado |
|---|---|---|---|
| 1 | Cero discrepancias del telefono: 3 del simulador (`337594e`); A (`D-170`) y B (`D-171`) del firmware | workflow `wf_85b47a98-146` | HECHO `3facd6d` |
| 2 | `D-167` retirar Modo Alcance | orquestador | HECHO `3b70675` |
| 3 | `D-166` + `D-169`: veto por camara y camaras malas (`MALA1`/`MALA2`) | workflow | HECHO `3facd6d`, sin banco |
| 4 | `D-168`: `J15` = BAJAR, barrera arriba en reposo y al arrancar; VERSION dice `J15:` | workflow | HECHO `3facd6d`, sin banco |
| 5 | Candidata: compuerta 16/17 (solo `D-22`, acta de `37f42ce`), arquitecto y QA APTO CON CONDICIONES, auditoria `N-182` integrada, APK y `Paquete_Semaforos_2026-11-11_8a81f1e_SIN_BANCO.zip` (`129a1157...`, APK `c9b82b1a...`); `6ca489f` a `RETIRADOS/` | orquestador | HECHO `8a81f1e` |
| 5b | **`D-173`, ROJO TOTAL es rojo total** (responsable 10/11): desde Automatico e Inteligente tambien pasa a Manual con rojo fijo; la via en verde, 3 s de amarillo (`D-45`, se mide primero) | agente en worktree | HECHO, sin banco |
| 5c | **Menores de la revision, en un workflow (ficheros disjuntos):** (a) `ESC:?` durante la gracia de `tUltimaRespuestaEsclavo` tras ROJO TOTAL, letra de `D-171`; (b) Practica: `CONSULTA_VETO` con las letras del estado simulado y `ESC:?` a los 28 s; (c) el empaquetador aborta con `PLATFORMIO_BUILD_FLAGS` puesto | workflow | HECHO `95246fb` |
| 5d | **Candidata 2:** compuerta hasta dos pasadas iguales, arquitecto y QA, APK y paquete nuevo; `8a81f1e` a `RETIRADOS/` | orquestador | tras 5b y 5c |
| 6 | **Banco de Marco:** hojas del 11/11 (1-12) y del 10/11 (1-40) (Parte 9 `N-187`, Parte 10 barrera arriba) y lo POR VALIDAR de §5. Sus registros se leen enteros y contando (`CLAUDE.md` §7): `RTT:` con LoRa, cobertura en metros, subida y bajada real del brazo, camaras (NA/NC y veto), C5, y lo que deben las cintas (`D1`..`D8` de la mudanza del 09/11: `N-172`, `D-40`, `D-38`, camaras con peatones, centralita, `N-116`). Rangos de `N-187` desde la app, sin firmware (`D-163`). `SIN_BANCO` sale solo con acta `evidencia/<fecha>_campo_<tema>.txt` y el hash cargado | Marco; lee el orquestador | ya |
| 7 | **Brazo lento:** cuando bajar y subir (con el verde, el ambar, tras el rojo, N s antes) con el tiempo medido en 6 (prueba 4) y una simulacion de dos modelos y arbitro a 10, 30 y 60 s. «Sin barrera» ya existe (`D-96`); no crear «abajo fija»: el rojo fijo ya la baja | orquestador mide; decide el responsable | tras 6 |
| 8 | **`D-158`, clave BT comun** al puente, configurable desde la app con PIN, tras medir en banco que clave admiten la libreria BT del ESP32 y los telefonos de Marco | Marco mide; subagente construye | tras 6 |
| 9 | **El Sisga:** respaldo md5 y carga; en la MISMA visita, central y firmware juntos (`D-168`), `Camaras_Sisga_4x.html` corregida (pluma con coche debajo: cierta con `7ff7d12`, falsa desde `d3606be`), el fin de semana medido (`VERSION`, `CONSULTA_NOCHE`, cinta de un sabado) y la instalacion reenviada (`-e all` la devuelve a fabrica) | Marco y orquestador | tras retirar `SIN_BANCO` |
| 10 | **Version siguiente menor**, abajo | -- | no bloquea |

**Version siguiente menor (fila 10):**

- `D-172`: leer un final de carrera de la barrera (pluma abajo con verde = alarma). Primero SPEC_5: que entrada libre,
  su cobre medido en `17_...` y el contacto del final de carrera de la central; despues firmware y app.
- `D-98`, tanda ultima de `app.js` (modales y PIN), con su condicion en
  [`ROADMAP_APP.md`](05_Funcional/App_Semaforo/ROADMAP_APP.md); Pasos Peatonales, alli.
- De arquitecto y QA de `N-187`: `camara_02_j16` no ve el `!= nc`; prueba de la pegada y del aviso de 90 s tras
  re-siembra.
- Hueco `N-182`: tras un corte con la barrera fuera, la pluma sube en la siguiente escritura de la luz, no al arrancar
  (con `D-168` puede cambiar: se re-mide despues de 4).
- Telefono sin medir: scroll a 360x640, 320 px, toques de 48 px (CDP 9333); `N-110` B2, teclado del PIN con el modal
  cerrado (`b033f0b`, ejercido en DOM).
- `G-RF`: dos parejas en banco (mismo canal y distintos, `D-118`) antes del filtro de pareja (SFTY-27).
- `N-166`, colision de nombre `A-15` (sin medir si sigue).
- Gobernanza, con tope semanal: dictamen de SPEC y manuales (`6f38ba4`, fase F); grafo fuera del repo (traerlo a
  `entregas/` o rehacerlo); repo ordenado (F1/F2); tras banco, lo duplicado Maestro/Esclavo a `lib/`.

Cerrado desde el 09/11: `N-187` (`3ea00d4`, `c10e60f`; docs `c404ad6`), bloque I del Esclavo y cortes de luz
(`2049501`), `D-98` tanda 4.a (`6b9f058`), particion de botones/semaforo/bluetooth (`8867292`), hoja del 10/11 con la
Parte 10 (`7534e53`), paquete `6ca489f` (`52b4acc`), spec `D-168`/`D-169` (`edb2462`).

## 1. Lo que necesita una TARJETA

| | que | detalle |
|---|---|---|
| **T-1** | `N-116`: la escalera de 4 peldanos sobre la tarjeta Maestro de la sesion 1 (la `179DB0` es otra placa, `N-126`). El primero, desenchufar los cinco conectores y remedir el riel, son cinco minutos y puede cerrar el caso | `roadmap_hist.md` §6.1 |
| **T-2** | Subir `SFTY6_SILENCIO_MS` sobre `e303485` (V8.4 sigue en 12 s; la V9 lleva 28 s, `D-48`). Decae si la V8.4 se sustituye por la V9 tras banco | `roadmap_hist.md` §6.3 |
| **T-3** | Lo POR VALIDAR de §5, en el banco de la fila 6 | §5 |
| **T-6** | Las `SIN VERIFICAR` de `05_Funcional/17_...md` (`grep -c` da lineas, no items) | ese fichero |
| **T-7** | `N-110` B2 en el telefono (construido `b033f0b`, ejercido en DOM) | fila 10 |

## 2. Lo que necesita una DECISION del responsable

| | que hay que decidir | donde vive |
|---|---|---|
| **D-a** | La firma del funcional sobre el manual del «doble» (`TECHO_POR_SUELO = 2`, ya en el paquete); el recuadro de `05_Funcional/historico/1_Manual_Usuario.md` esta vacio y `evidencia/` no tiene documento firmado | `D-19` |
| **D-d** | `J9` `VERDE_PEATON`, `J11` `ROJO_PEATON`, `J13` `BUZZER`: tres canales de potencia fabricados y cero firmware (16 B de flash cada uno). ¿Cabeza peatonal, zumbador, o no poblados? | SPEC_5, tabla de conectores |
| **D-f** | `N-129`: un modulo virgen anuncia `SEM-SIN-MATRICULA` y las dos puntas se llaman igual hasta la siguiente arrancada. ¿Paso de puesta en marcha o se cambia el momento del rotulado? (el rotulo aprendido si se vio, `N-126`) | `MATRIC` |
| **D-h** | `N-113`: el aviso remoto es coste recurrente (SIM o WiFi en el cruce), no una linea de firmware | `roadmap_hist.md` §6.5 |
| **D-i** | `MATRIC`: `RF_Packet` son 4 bytes sin direccion; direccionar por ID de Bluetooth cambia el contrato de radio en las dos puntas | `MATRIC` |
| **D-j** | `A-8` (los dos `Arming Schedule`; sin fila en `DECISIONES.md`) y `A-10` (LED `D21` de `VERDE2`: comprobacion de banco) | `DECISIONES.md` |

## 4. Lo que necesita una COMPRA o un SOLDADOR

| | que | cuando |
|---|---|---|
| **C-1** | `2K2` en serie en las 5 entradas de campo (`N-120`), `SIN VERIFICAR` en tarjeta | al soldar la placa nueva: es el momento y no vuelve |
| **C-2** | No poblar los 12 V de `J16` p1; mientras exista, taparlo en cada equipo (`D-4`) | al montar |
| **C-3** | `A5`: reductor DC-DC conmutado 12 -> 5 V, >= 1 A, para el ESP32; ninguna referencia elegida. El ESP32 del Sisga se reinicio con 2 `SUBIDA_DE_TENSION` el 10/09 (causa: hipotesis) | no esta pedido |
| **C-4** | Quien disena y quien fabrica la placa portadora | bloquea desplegar, no probar |
| **C-6** | El `Y2` de 32,768 kHz del Maestro `179DB0` no oscila (16/09, `LSERDY` no sube); soldador: `Y2`, sus dos condensadores y las pistas a `PC14`/`PC15`. Medidas, a `05_Funcional/17_...` §12 | antes del banco de esa placa |

## 5. POR VALIDAR: el codigo esta escrito y ninguna tarjeta lo ha ejercido con cinta

Nada de aqui se cierra sin cinta o acta de banco. El Sisga (10/09, `7ff7d12`) ejercio del lado del Maestro y por
telemetria `N-150`, `N-153` y el ciclo 3/3/15; `N-147` a medias. Nada de eso ve las luces ni el Esclavo.

| | commit | como se ejerce |
|---|---|---|
| **N-142** | `6274acc` | el Esclavo avisa por radio de su ambar de emergencia; con las dos puntas |
| **N-147** | `8e9e8a9` | entrar en Manual y esperar 15 s sin tocar: no cicla |
| **N-150** | `414b962` | aplicar tiempos desde el telefono: el ciclo arranca |
| **N-151** | `273b315` | DAR PASO fuera de Automatico no traba el cruce |
| **N-152** | `d6ce67e` | cancelar el ambar desde el Poste 2: el Maestro lo oye en `MODO_AMBAR` |
| **N-153** | `79ef5a6` | la pantalla dibuja la pluma arriba y abajo |
| **N-157**, `D-24` | `4b90f98`, `ee957ef`, `9550c57` | la camara se vigila a si misma; `CAM_CIEGA` (24 h) no es ejecutable en una sesion |
| **`D-20`**, **`N-160`** | `9dd8bbf`; `f151674`, `3a91973`, `6c90ff0` | reloj de base sembrada y `reloj_enHora()` (autoriza el Degradado); el sembrador que acusa y el Diario del Maestro |
| **`N-154`** | `1d92bfb` | un `$ALARM` con su hora |
| **`N-162`** | `c51cc85`, `141f191`, `63d6964`, `d68eb36` | getters de hora, `ESC:` sin `VERDE` con el Esclavo en rojo, `GO_GREEN` repetido sin reiniciar el ambar (dos puntas), sello del puente |
| **`D-26`** | `68dd2c5` | las tres tarjetas: `HORA_ESP32_SEMBRADA` en el diario del poste 1 (solo en el cambio); cortar la radio 25 s y ver `FALLO_RF` y luego `HORA_ESP32_SEMBRADA` en el poste 2; `J17` fuera 15 min y `CAUSA:J17_MUDO` |
| **la app** | varios | `disconnect()`, `padding-bottom: 90px`, `discoverUnpaired`, `validarTrama()`: arreglados en el fuente, ninguno en un telefono |
| **`0x68`** del `DS3231` | -- | verificado en el Maestro `179DB0` (Sisga); falta el del Esclavo |

**NO PROBADO en una sesion, y se anota asi, no como fallo:** `CAM_CIEGA` en su tiempo real (solo con una compilacion
de umbral reducido, que no es la de campo); `D-14`, que la camara grabe al cerrar el contacto (cero anclas en el
firmware); la guarda de hora del Degradado ya dentro (inalcanzable, `roadmap_hist.md` §3.10.bis). El Degradado del
poste 2 si se prueba desde `9dd8bbf`: antes, hora al Esclavo.
