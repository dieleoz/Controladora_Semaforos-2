---

## MUDANZA DEL 09/11/2026 - cronica de `roadmap.md` §0 de tareas ya cerradas

Literal, sin editar. Salio de `roadmap.md` §0 sobre `7f5db82`: la tabla «ORDEN DE TRABAJO» (filas 1-5, lineas 88-96)
y «Lo siguiente, en orden» (lineas 102-393: `N-177` V1..V10, `N-178` M1..MF, `N-179` N1..NF, `N-180` G0..G-RF,
`N-181` S..P, bloque U, `N-182`..`N-186`, rama `auditoria/n182`, `A-15`, `N-175`, `N-173` A1..A7.b y los bloques
B, C, D, F, G, H, I, E). Cerrado segun `git log`: `N-177` en `96c3d32`, `N-179` (N4-N6 incluidos) en `8526f84`,
I en `cda33df`, `N-182` en `db74195`, `N-183` en `f41caf8`, `N-185` en `15c5fe6`, auditoria fusionada en `0e7589f`,
B8 con `D-68` (`LIMITE_48H` en el Esclavo), cabecera con `D-82`. Lo que seguia abierto quedo en una linea en
`roadmap.md` §0, «Version siguiente».

| # | que | quien / donde | estado |
|---|---|---|---|
| 1 | `N-186` paquete para Marco `db07446` (3 puntas, APK de hoy, hoja del 10/11 de lo pendiente) | `entregas/` | HECHO, QA APTO |
| 2 | `N-187` construir: NA/NC por camara y retardo de bajada 3..5 s (fabrica = hoy), flash propia, app, simulador | worktrees | CONSTRUIDO sin banco (09/11): compuerta 16/17 convergida, solo `D-22`; falta QA y paquete |
| 3 | Runbook de `N-187`: arquitecto antes, compuerta x2, QA, APK, telefono, UN `.zip` nuevo | orquestador | arquitecto y QA APTO CON CONDICIONES (09/11); hoja Parte 9 (pruebas 26-34); `.zip` |
| 3b | `RTT:` con LoRa MEDIDO (09/11 17:27, `evidencia/091120261729`, 166 tramas): mediana 752 ms, max 952 (incluye los 200 de `RETARDO_RESPUESTA_MS`); ida ~275-375 ms < 500 de `D-34`, sin cambio de tiempos; RF 100 % en 120, 90/80 % en 23 | medido | vigilar con la cobertura |
| 4 | Registros de Marco del 10/11: `RTT:`, cobertura, tiempo del brazo, camaras, C5 -> rangos desde la app | orquestador | esperando |
| 5 | Version siguiente: `D-98` tandas 4 y ultima, BT con clave (`D-158`), §7.5 Alcance, El Sisga tras banco | — | — |

### Lo siguiente, en orden (el estado de cada punto, en `ESTADO.md`)

**PRIMERO (14/10, pedido de Marco en campo): `N-177` verde del Degradado con testigo elegido en la app (`D-59`).**
Marco dejo el paso en Degradado (la radio no le sirvio) y le piden mas de 3 min; hoy el 180 es constante en los dos
postes (`DEG_VERDE_SEG`, `TESTIGO_VERDE_SEG`) y el despachador rechaza otro valor. La trama, la app y el registro de
flash ya llevan el campo `verde`. Riesgo que manda (§11.7): dos postes con verdes distintos dan verdes cruzados; se
cubre como el despeje (el Poste 2 recibe el guardado del Poste 1) y la renovacion exige el mismo verde. El automatico
(`A-15`) sigue en 180. Firmware nuevo en los DOS postes y APK nueva (lleva tambien `38a42ca` y `a16356d`).

| # | paso | sale | estado |
|---|---|---|---|
| V1 | SPEC: `D-59`, SPEC_2 §7.bis y SPEC_4 §3, riesgo aceptado del cambio en caliente | SPEC al dia | hecho (`96c3d32`) |
| V2 | Pruebas vistas en rojo: bloque K del Degradado a dos puntas; app (verde, reenvio, paletero) | arneses | hecho (`96c3d32`) |
| V3 | Firmware Maestro y Esclavo, y app, disjuntos en worktree; integracion por el diff | candidata | hecho (`96c3d32`) |
| V4 | Compuerta completa en el arbol principal hasta dos pasadas iguales; cifras a README/ESTADO | acta | hecho: acta `2026-10-14`, banco 1379/1380 (solo `D-22`); puente 150/150 tras `f4c95e7` |
| V5 | Arquitecto (opus) sobre la candidata | veredicto | APTO sin condiciones sobre `96c3d32` (14/10) |
| V6 | QA sobre el binario y la APK que se entregan | veredicto | APTO CON CONDICIONES (14/10): todos los telefonos con la APK `ba1d595c`; corte de luz con verde 15 min en tarjeta; caso 12 de la app vacuo (D2) |
| V7 | Cerrar condiciones (SPEC -> prueba en rojo -> codigo) y compuerta otra vez si cambia codigo | candidata | hecho: caso 12 visto en rojo y dos condiciones en la hoja (`a0d2121`); acta convergida sobre arbol limpio (`b4501a6`) |
| V8 | Hoja de Marco solo con lo nuevo (verde 4-6 min en los dos postes, cambio en caliente con paletero, `.txt` de `D-55`) | `.html` | hecho y sin enviar; sustituida por `Pruebas_Funcional_2026-10-15.html` (33 casos, con N-178) |
| V9 | Paquete validado `SIN_BANCO` a `entregas/` (firmware de los dos postes, APK de `96c3d32` con `D-55`/`D-56`), `.docx` de SPEC_2 y SPEC_4, ESTADO y README | `.zip` | hecho: `Paquete_Semaforos_2026-10-14_96c3d32_SIN_BANCO.zip`, sha256 `b4390a0d...`; el del 09/10 a `RETIRADOS/` |
| V10 | Banco de Marco con placas reales y la hoja; sus cintas se leen enteras y contando; si pasa, se quita `SIN_BANCO` | cintas | Marco, despues el orquestador |

A carretera solo tras banco (`CLAUDE.md` §0.2): el paquete lo dice en su primera linea.

**Version siguiente de `N-177` (bajos del arquitecto, no bloquean):** `testigoFlash_leer` no recomprueba 180..900 x60;
el Esclavo lleva 180/900 en literal, sin atarlos a `VERDE_MIN_MIN/MAX`; `_cambioEnCaliente` de la app compara con el
testigo guardado, no con el ciclo real del Poste 1 (lo cubre el aviso de paleteros); SPEC_6 A.2.bis sin «Verde fuera de
rango», `EN_VERDE` ni `NO_GUARDADO`; `ciclo_degradado_restante` con 900 s, coste sin medir.

**DESPUES (14/10, orden del responsable): `N-178` cadena de mejoras sin parar.** Subagentes en cadena, uno detras de
otro en el arbol principal (nunca dos a la vez). Lo que el responsable no definio lo decide Fable y queda en la SPEC y
en una fila de `DECISIONES.md` («decidido por Fable por delegacion, 14/10»). Sin preguntar al responsable, sin commit ni
push (lo decide el responsable). Cada paso: SPEC -> prueba vista en rojo -> codigo -> compuerta; dos intentos fallidos,
se revierte el paso y sigue el siguiente. Simulador congelado (§4): pruebas en arneses existentes. No toca la candidata
`96c3d32` de Marco: sale una candidata nueva.

| # | paso | fuente | estado |
|---|---|---|---|
| M1 | Reinicios fechados en `$STATUS` (REIN, ULT_REIN, UP_ANT, HEAP_MIN) y alarma en la app si un contador baja sin reinicio | `ROADMAP_APP.md` 11 | hecho (D-60..D-64) |
| M2 | Todo `$ACK` con `RES`/`APLICA` o `$ERR`; `MOTIVO:SIN_CAMBIO` | `ROADMAP_APP.md` 12 | hecho (D-65, D-66) |
| M3 | Alarmas que se ven aunque nadie estuviera conectado | `ROADMAP_APP.md` 9 | hecho (D-67, D-68; D-69 no) |
| M4 | SPP del puente que no bloquea | `ROADMAP_APP.md` 13 | hecho (D-70, D-71) |
| M5 | Registro en el equipo (ESP32 del puente, `LOG,DESDE:<N>`) | `ROADMAP_APP.md` 10, `D-55` | hecho (D-72..D-75, D-100) |
| M6 | Bajos del arquitecto de `N-177` | arriba | hecho (D-76..D-79) |
| M7 | App: un color y una frase por estado, cabecera a 360 px, un solo aviso al cambiar el verde, prueba contra jerga | `ROADMAP_APP.md` 2-3 | hecho en ronda 2 (D-80..D-83, D-101) |
| M8 | `empaquetar_funcional.py` con commit de la candidata | sesion 14/10 | hecho (D-84, D-85) |
| M9 | Modo practica sin poste | `D-58`, `ROADMAP_APP.md` 14 | hecho (D-86..D-89) |
| M10 | Bajo trafico por dias y franja | `N-175`, SFTY-20 | derogado por D-103 |
| M11 | Pantalla informativa durante el ambar | SFTY-22 | hecho (D-91..D-93) |
| M12 | Interruptores de SPEC_8 §6: solo SPEC, sin placa no hay codigo | SPEC_8 §6 | solo SPEC (D-94..D-97), rojo honesto |
| M13 | `app.js` en modulos y fase F | `ROADMAP_APP.md` 8 | tanda 1 y fase F hechas |
| MF | Compuerta completa convergida, arquitecto y QA una vez sobre la candidata | runbook | acta 2026-10-15 convergida; condiciones cerradas salvo orden «APK primero» en la hoja del proximo paquete (D-102) |
**DESPUES (15/10, orden del responsable): `N-179` cadena sin parar hasta el runbook entero.** Mismo molde que
`N-178`: subagentes en cadena en el arbol principal, nunca dos a la vez; SPEC primero, prueba vista en rojo, codigo,
compuerta; dos fallos en un punto, se revierte el paso. Lo no definido lo decide Fable (SPEC y fila de `DECISIONES.md`).
El paquete `5288c3d` de Marco no se toca: sale otra candidata.

| # | paso | fuente | estado |
|---|---|---|---|
| N1 | Modo noche: varias franjas por dia; salida por hora o por la app con PIN, siempre por un todo-rojo configurable desde la app (180 s de fabrica, el administrador lo cambia; la app lo propone por distancia entre postes); sin radio, lo que diga su Degradado | `D-103`, responsable 15/10 | construido, sin banco; QA: la salida por minuto exacto de fin se perdia (24 h de ambar), cerrado con `D-115` y N11 del arnes de dos puntas |
| N2 | Cabecera de `MENU` que dice que hacen las luces | `D-105` | hecho (`D-112`): `js/luces_tablas.js`, `tests/dom_cabecera_fase.js` |
| N3 | Sello de firmware: un hash que empieza por 0 pierde el cero (`FW:479942` por `0479942`, medido con `pio run -v`) | paquete 15/10 | hecho (`D-113`): `strings` del `.bin` da `FW:6115e59+SUCIO`; un hash con 0 inicial, en la tarjeta |
| N4 | Menores de QA de `N-178`: `registro.cpp` primero inexistente tras fallo; `respaldo_borrar()` y `DR9`/`DR10`; duracion del registro en SPEC_4 | QA 15/10 | |
| N5 | `app.js` en modulos, tanda 2 | `N-178` M13 | |
| N6 | Lint del 14/10: SPEC_1 filas de la compuerta, SFTY sin spec, huerfanos, packs sin historico; documentos vivos al tope | cierre 14/10 | |
| NF | Compuerta convergida, arquitecto y QA, condiciones, hoja, APK y paquete | runbook | HECHO: acta sobre `1b67107` (1480/1485, solo `D-22` y `D-94`..`D-97`); paquete `2026-10-16_ad74390` zip `c68f11f0...`, APK `a8214ed5...` |

Sin teclado (no entra): hardware `N-108`, `N-120`, `C-6`, `N-116`, `Y2`; `D-94`..`D-97` (placa con barrera y pila).

**DESPUES (15/10, orden del responsable): `N-180` via -> grupos -> pareja por MAC, y simulador con estado.** SPEC
primero (SPEC_4 §9.14, SPEC_6 B.1 (4), `D-117`..`D-121`, commit `9b4a36f`), revision del arquitecto ANTES de construir
(`D-120` retira la barrera «ninguna trama de un literal a mano»), prueba vista en rojo, codigo, compuerta. Sin firmware:
el filtro de pareja en radio (SFTY-27) espera la medida con dos parejas en banco (`D-118`).

| # | paso | fuente | estado |
|---|---|---|---|
| G0 | Arquitecto valida §9.14 y `D-117`..`D-121`; Fable ajusta la SPEC si pide condiciones | `D-120` | hecho: NO APTO con 5 condiciones, cerradas en §9.14 |
| G1 | Grupos: `js/grupos.js`, lista de equipos por via y grupo, matricula por MAC, retirar `site_manager.js` | `D-117`..`D-119` | hecho, `ac844b9` |
| G2 | Simulador con estado y flota de 2 vias y 5 grupos mas un poste virgen; cintas como referencia | `D-120`, `D-121` | hecho, `ac844b9` |
| GF | Compuerta convergida, arquitecto y QA, condiciones, casos en la hoja, APK y paquete | runbook | en curso: condiciones de arquitecto y QA cerradas (`D-122`, `D-126`); acta `2026-10-16` convergida (solo `D-22`, `D-94`..`D-97`); casos 42-45 en la hoja sin comitear; faltan APK, telefono y paquete |
| G-RF | Dos parejas en banco: mismo canal y canales distintos; acta `_campo_dos_parejas` | SPEC_6 B.1 (4) | sin teclado |

**AHORA (16/10, orden del responsable): `N-181` UN SOLO paquete que no deba nada de teclado.** Vacia la version
siguiente de `N-180` (revision del 15/10 y telefono a 320 px CSS) y el rango de `SET_TIEMPOS`. Un workflow con los
constructores en paralelo, cada uno en su worktree y con ficheros disjuntos (`CLAUDE.md` §11); el orquestador integra
por el diff y corre en el arbol principal compuerta, APK, telefono y paquete. Numeros reservados: `D-127`..`D-140`.
Lo ya definido no se pregunta (SPEC_4 §9.14 (c): un `NODE:` distinto da aviso rojo y NO cambia la matricula).

| # | quien | ficheros (solo esos) | que | estado |
|---|---|---|---|---|
| S | 1 agente, antes que nadie | SPEC_4, SPEC_2, `DECISIONES.md` | Lo no escrito, con valor por defecto y fila `D-127`..: tope del PR, confirmar «Quitar del grupo», avisos una vez por conexion, rango 3..15 min con `$ERR` || hecho, `8f46354` |
| A | worktree | `js/grupos.js` y `www/js/grupos.js`, y sus pruebas | MAC en minusculas; `grupos:[null]`; orden numerico; «Quitar del grupo»; `NODE:` distinto hacia §9.14 (c); `anotarFw`; `preguntarNuevo` || hecho, `b5bad6e` |
| B | worktree | `app.js` (las dos copias) y sus pruebas | ACK diferido entre sesiones; PR con tope; avisos por MAC al reenlazar || hecho, `6828945` |
| C | worktree | `style.css`, `index.html` y sus copias de `www/` | 320 px: franja sobre el modal, «emparejado», «(radio N %)», «MANDOS DE CAMPO», `.card-sub`, buscador; rotulo «Equipos de cintas» || hecho, `f1e88c2` |
| D | worktree | `bluetooth.cpp` de Maestro y Esclavo, arnes de `SET_TIEMPOS` | `SET_TIEMPOS:259` rechazado con `$ERR` antes del `(uint8_t)`; mutantes vivos `SET_TIEMPOS` sin rango y `parar()` vacio || hecho, `9ffc049` |
| E | worktree | `tests/dom_claves.js` | reloj inyectable en «Practica» (4) || hecho, `dab890d` |
| U | tras integrar A-E | ver bloque U debajo | Simplificar la app, en el mismo paquete || en curso en `N-182` (09/11, workflow U-S y U-B) |
| X | worktree, tras D | SPEC_8 §6, `respaldo.cpp`, despachador, app | `D-94`..`D-97`: arquitecto ANTES (§11.7), en cadena tras D (comparten despachador) | hecho en `N-182` (`bd16447`) |
| F | orquestador | arbol principal | integrar por el diff; compuerta completa hasta dos pasadas iguales; cifras a ESTADO | |
| Q | 2 agentes a la vez | solo lectura | arquitecto (opus) y QA una vez sobre la candidata; condiciones por SPEC -> rojo -> codigo | |
| P | orquestador | `entregas/`, telefono | APK y CRC; telefono CDP 9333 a 320 px; `.bin` con sello limpio; hoja con lo nuevo y lo que mide Marco (`D-22`, `D-124`, pila de `D-94`..`D-97`); UN paquete; `0fe782f` a `RETIRADOS/` | |

**Bloque U (27/10, orden del responsable: «no es usable, tantas funciones que no se entienden»).** Va despues de
integrar A-E (toca los mismos ficheros que B y C) y antes de Q, que revisa una sola vez todo. Lo de Pasos Peatonales
(su sesion, 27/10): ocultar tras la clave NO basto alli (queja de Marco abierta en su V6); la regla es «antes de un
texto, aviso o modo: ¿lo necesita para operar? Si no, plegado o fuera». Ya decidido aqui y sin construir: dos zonas y
`JERGA` (`D-80`, `D-83`, SPEC_4 §9.9). Numeros reservados: `D-141`..`D-147`.

| # | quien | ficheros (solo esos) | que | estado |
|---|---|---|---|---|
| U-S | 1 agente, arbol principal | SPEC_4 §9.9 y §9.2, `DECISIONES.md` | Inventario de lo visible hoy en cada pantalla sin clave; por cada elemento: queda, plegado (`<details>`), tras la clave (`D-56`) o fuera, con el criterio de arriba. Principal sin scroll a 360x640 y sin scroll lateral a 320; un nombre llano y una frase de ayuda por ajuste; la franja: sintoma y que hacer en una frase; `SIN_PAREJA` ofrece emparejar ahi mismo; rotulos «Operario»/«Tecnico» | |
| U-B | 1 agente, arbol principal, tras U-S | `app.js`, `index.html`, `style.css` y sus copias de `www/` (md5 iguales), `tests/dom_usabilidad.js` | Construir U-S: prueba en rojo primero (lo visible sin clave y con ella, `JERGA`, cuenta de botones y palabras de la principal contra la cifra de U-S) | |

Lo que no mide jsdom (scroll, toques de 48 px, 320 px) lo mide P en el telefono por CDP. Tiempo: U-S 45 min, U-B
1,5-2 h.

**`N-182` (09/11, responsable): paquete para Marco con radios, camaras y barrera; sustituye a P de `N-181`.** Marco
pide version para probar radios y camaras; el responsable le pide barrera (puente H) conectada, subida y bajada sin
final de carrera, tiempos, las 4 camaras en sus escenarios y, al final, prueba en calle con mensula (tiempo de
respuesta por incidencia y cuanto se sostiene el rele). Lo que sale: firmware, APK y `.zip` con UN `.html` de lo que
debe probar y lo que falta. U (simplificar la app) queda para el paquete siguiente. Numeros reservados:
`D-148`..`D-155`.

Hallazgo de fuente, sin medir (09/11): «en modo local va bien hasta el viernes; el fin de semana alguien va a
programarlo». (1) El Sisga (`7ff7d12`/`b354fe9`) es anterior al modo noche (`8526f84`): no tiene horario. (2) Con HEAD
tampoco cabe un fin de semana continuo: franja < 24 h (`inicio == fin` = apagada, `D-108`) y la franja que empieza
al acabar otra se salta (`FRANJA_SALTADA`, `D-115`, puesto a proposito contra «24 h de ambar de dia»). (3) `-e all`
borra las franjas en cada carga. Responsable, 09/11: el fin de semana sigue como esta, sin cambio de firmware; cree
que no se marcaron sabado y domingo. Pero `7ff7d12` no tiene dias (solo `reloj_ajustarFranjaNocturna` por horas):
se MIDE en la visita (equipo, `VERSION`, `CONSULTA_NOCHE` y cinta de un sabado), en la hoja H.

| # | quien | ficheros (solo esos) | que | estado |
|---|---|---|---|---|
| M | 2 agentes, solo lectura, mismo encargo | firmware de camaras, pluma y respaldo | Que hace HOY el equipo con una camara caida, las dos, la barrera sin final de carrera; se comparan: donde difieren se abre el fuente || hecho: 8/11 coinciden; B2, B3 y R1 conciliados en el fuente |
| R | 2 revisores, solo lectura, mismo encargo | `D-94`..`D-97`, `D-115`, SPEC_8 §6, SPEC_0 §5.10 | `CLAUDE.md` §11.7 ANTES de construir: modo de fallo de cada barrera que se retira (veto de camara, `FRANJA_SALTADA`); se comparan || hecho: A NO APTO, B APTO CON CONDICIONES; 7 condiciones a la spec (`D-148`..`D-152`), `268e90b` |
| S2 | 1 agente, tras M y R | SPEC_8 §6, `DECISIONES.md` | Condiciones de R a la spec antes de construir X (`D-148`..) || hecho, `268e90b`; `D-96` ARRIBA confirmado por el responsable (09/11) con su modo de fallo |
| X | worktree | `respaldo.cpp`, despachador `bluetooth.cpp`, pluma y camaras de Maestro y Esclavo, su arnes | `D-94`..`D-97` || hecho, `bd16447`: 99/99, 184/184, 158/158; flash 80,2 % / 66,8 % |
| AP | worktree | `app.js` (dos copias), `index.html`, sus pruebas | Interruptores de `D-94` || hecho, `bd16447`: DOM 842/842 (aviso de arranque con literal inventado: rojo visto y corregido) |
| H | arbol principal | `05_Funcional/Hoja_Marco_<fecha>.html` | Hoja: barrera, 4 camaras, camara caida, interruptores, fin de semana, radios, calle con mensula; `D-22`, `D-124` || hecho: `Pruebas_Funcional_2026-11-09.html`, 24 pruebas, `acd7f6e` |
| F, Q, P | orquestador | como `N-181` | Integrar, compuerta, arquitecto y QA una vez, APK, `.bin`, UN `.zip`; `0fe782f` a `RETIRADOS/` || hecho: paquete `db74195` (zip `20d12dbe...`, APK `3bc6dfc8...`), compuerta sobre `c94a6cb` 1518/1519 (solo `D-22`); arquitecto y QA con condiciones cerradas; hoja corregida por QA y alineada con U; sin telefono |

**`N-182` cerrado en teclado (09/11):** U construido (`4105ba3`, `4c709fd`, `c94a6cb`; D-141..D-145), interruptores
(`bd16447`), `NO_QUEDO_PUESTO` traducido. Sin medir: el telefono (scroll a 360x640, 320 px, 48 px). Hueco medido: tras
un corte con la barrera fuera, la pluma sube en la siguiente escritura de la luz, no al arrancar. Riesgo medido en el
fuente (C5): la camara no frena una bajada ya ordenada, y Marco cuenta ~60 s de recorrido con final de carrera; se
cronometra en banco antes de decidir. Auditoria aparte en la rama `auditoria/n182` (otra sesion).

**`N-183` (09/11): SPEC_4 contra la app.** Hecho en `f41caf8`: dos lectores (52 apartados, 11 conciliados), 24 marcas
caducadas al construido, nueve rechazos con frase llana, `D-156`; QA con condiciones cerradas (jsdom 921/921). Fuera:
tandas 3+ de `D-98` (traslado que rompe instrumentos por ruta) y lo de firmware (§7.2, §7.7, §7.8, §7.11 (b)).
Para el responsable: §7.1 (cuatro ordenes sin PIN; propuesto aceptarlas y cerrar el emparejamiento abierto del
puente, `spp.begin` sin clave, como tarea de firmware) y §7.5 (Modo Alcance sin medida propia: retirar o medir).

**En curso (09/11, tras el simulador `666d904`):** (1) `app_07` en rojo por la forma de `interruptores()` del
simulador, con SPEC_4 §9.10 en el mismo cambio; (2) pasada de coherencia de todas las SPEC, DECISIONES,
`ARQUITECTURA.map` y `OPTIMIZACIONES.md` contra lo de hoy (solo lectura, ediciones exactas); (3) despues: cifras,
compuerta, APK y paquete que sustituye a `db74195`. Regla desde hoy: cada cambio de codigo lleva su spec en el mismo
commit y un grep de lo nuevo en SPEC y DECISIONES antes de comitear.
Para la rama `auditoria/n182`: el recorrido del menu de `dom_simulador.js` empareja mal respuesta y orden
(`SET_DEG_AUTO:1`, `CONSULTA_NOCHE`, `SET_TIEMPOS:5,5,30` saldrian «sin respuesta»), y el juez de vocabulario no ve
las tramas que el firmware arma con `snprintf` (interruptores).

**`N-184` cerrado: radio.** El enlace es RS485 a 9600 transparente con la E90-DTU (433L37)-V8 de fabrica y M0/M1 en
ON (`D-160`, SPEC_6 B.1-B.3; programa: RF_Setting 4.6). Medido: 100 % de latidos y `RTT:` mediana 752 ms, max 952
(`evidencia/091120261729`); la ida cabe en los 500 ms de `D-34`, sin cambio de tiempos. La cronica, en `HISTORIA.md`
§14. Falta: cobertura en metros (hoja del 10/11).

**`N-186` hecho: paquete `db07446` para Marco** (firmware de tres puntas sin Repetidor ni 1200, APK de hoy, hoja de
lo pendiente; Degradado con testigo probado 72 h en el arnes; QA APTO).

**Rama `auditoria/n182` (otra sesion, 09/11):** 7 commits de instrumentos sobre `7f4a502` (rondas.json ignorado,
lista cerrada en `barrera_03`, ABORTADO en `Validacion_Respaldo`, D-94 por el despachador real del Maestro en el
arnes del puente, intermitente D-78, `app_01` y `dom_simulador` con los interruptores). Merge cuando su arquitecto,
QA y compuerta salgan; condicion: devolver el «NO CUBRE» de la cabecera de `escenario_p_salida.py`.

**`N-185` (responsable, 09/11): el Repetidor sale de esta arquitectura.** No toca la caida de radio de Marco (dos
postes directos). Alcance a recontar con grep antes (`CLAUDE.md` §14): filas de la compuerta (compila repetidor,
simulador de repetidor), `01_Firmware/Repetidor` y su sitio en el paquete, SPEC_0/2/6, `ARQUITECTURA.map`, README;
fila en DECISIONES al ejecutarlo; los instrumentos que leen sus rutas se retiran en el MISMO commit (`CLAUDE.md` §5).

Abierto, para el responsable (no se construye en `N-182`): «inhibir UNA camara o dos» (hoy `D-94` es un solo
interruptor y SPEC_8 §2 dice que las dos entradas hacen lo mismo) y «barrera ABAJO» desde la app (`D-96` solo da
fuera de servicio = arriba; abajo forzada en verde no tiene fila).

Cada constructor: prueba vista en rojo antes del codigo, duda del encargo, no comitea. Dos correcciones fallidas en un
punto: se revierte ese punto y sigue el resto (`CLAUDE.md` §11). Tiempo estimado: S 45 min; A-E en paralelo 2-2,5 h;
F 1-1,5 h; Q y condiciones 1-1,5 h; P 1-1,5 h. Total 6-7 h sin X; con X, 8-10 h (firmware de seguridad en cadena).

**Version siguiente (06/10, visto en el telefono):** la cabecera recorta el rotulo del enlace hasta no leerse
(«(sin ...») en cuanto deja de pisar los iconos: dos intentos de CSS fallidos, congelado en `42e7746`, donde el
texto se lee pero se monta sobre los iconos. Arreglo propuesto: el rotulo en su propia linea, no recortado.

**Version siguiente, app (09/10):** lo aprendido de Pasos Peatonales, ordenado por fases y con lo que DECIDE el
responsable, en [`05_Funcional/App_Semaforo/ROADMAP_APP.md`](05_Funcional/App_Semaforo/ROADMAP_APP.md).

**Degradado automatico (`A-15`), sin cambio de firmware.** El 05/10 Marco lo intento sin radio y el firmware
rechazo las 7 ordenes (`SIN_ENLACE_CON_EL_OTRO_POSTE`, `evidencia/051020261239/`): hizo lo que dice la spec, pero la
app no lo explica. Decidido por el responsable: es una ELECCION DE MONTAJE («si se cae la radio: ambar y voy con el
testigo» por defecto, `D-43`, o «entran solos a los 5 min»), que la app pregunta al conectar, solo deja cambiar
con radio, muestra siempre en la pantalla principal y anuncia al caer la radio (con cuenta atras aproximada).

Reordenado el 02/10. Se cierra por fases: ninguna empieza sin cerrar la anterior. **Orden de ejecucion (02/10):
A1-A2 y H hechos -> I (en curso) -> G (legacy, D-44/D-46, y el ancla de D-24) -> UN solo runbook A3-A6
sobre la candidata con I y G -> F.** El pack de anclas da rojo legitimo mientras haya decisiones sin construir. Al
acabar solo quedan C
(decisiones) y D (cintas de campo).
Contestado pieza a pieza por el responsable el 02/10: `FORZAR_ROJO` tambien pasa por amarillo (`D-45`); la
tanda 2 de G retira `SOLICITAR_PASO` y `SET_MODO:DEGRADADO`; `J14`, SFTY-20 y los pines de peaton se quedan (`D-46`).
Todo cambio de firmware respeta las E/S y pines que fijan las guias de campo `.html` entregadas a Marco.
**`N-175` Modo de bajo trafico (SFTY-20, diseno en `OPTIMIZACIONES.md`), optimizacion, version siguiente.**
Responsable, 06/10: lo programa el usuario cuando quiera, por DIAS DE LA SEMANA y franja horaria (p.ej. este mes
martes a jueves; fines de semana no, hay mas trafico). Luces del diseno: Maestro ambar intermitente, Esclavo rojo
intermitente (norma 4.4.3). Sustituido el 15/10 por el modo noche de `D-103` (SPEC_0 §5.10), decidido por el
responsable (las puntas se ven; 20-30 vehiculos en la franja): se construye en `N-179`, version siguiente.

**A. `N-173` Runbook de la candidata** (se salto para mandar a Marco; ahora entero, para dejar `main` ok). La
candidata crecio con I (`cda33df`) y G (`f9cad1f`): un solo runbook sobre todo.

| # | paso | sale | estado |
|---|---|---|---|
| A1 | SPEC al comportamiento construido (`D-38`, `D-40`, aviso de radio, DR6/DR7, carteles) | SPEC al dia | hecho (`709c0c3`) |
| A2 | Pruebas que celebraban lo viejo (§9), inyeccion vista en rojo | arneses al dia | hecho (`7603afc`) |
| A3 | Compuerta completa en el arbol principal | acta | hecho: converge en 4 pasadas, 18 PASS y 1 FALLA = `D-22` (pide tarjeta) (`ae9a018`) |
| A4 | Arquitecto (opus) | veredicto | **NO APTO** (`3d16b39`): amarillo del Esclavo contra el ambar del Maestro; `FORZAR_ROJO` en Degradado acusa y el verde vuelve; `D-47` sin prueba; la libreria RTC pisa DR6/DR7 en marcha (causa probable de `N-172`, coincide con DeepSeek y Nemotron) |
| A4.bis | Correccion de las cuatro, SPEC -> prueba en rojo -> codigo | candidata nueva | hecho y en `main` (`f6ef03d`): `D-50` suelta a 21,5 s, `D-51` `FORZAR_ROJO` en Degradado, pruebas de `D-47`, `N-172` sin libreria RTC; app con los tres acuses de `FORZAR_ROJO` |
| A3.bis | Compuerta otra vez | acta | hecho: `evidencia/2026-10-04_compuerta.txt` sobre `f6ef03d`, 18 PASS y banco 1360/1361 (solo `D-22`) |
| A4.ter | Arquitecto otra vez (opus) | veredicto | APTO CON CONDICIONES sobre `f6ef03d`: (1) `FORZAR_ROJO` en Degradado sin radio deja al Poste 1 en ambar contra el verde por reloj del Poste 2; (2) con el permiso esperando la siembra, el verde vuelve tras un rojo total; (3) el `setup()` del Maestro se prueba sobre una copia; (4) nadie ejecuta las tres respuestas de `FORZAR_ROJO` |
| A4.quater | SPEC: salida del Degradado programada (hora configurable en cada poste, aviso de paleteros, salen a la vez por reloj; decidido 04/10); `FORZAR_ROJO` en Degradado = rojo fijo inmediato; condiciones 2-4 | SPEC_2, SPEC_4, fila de `D-51` y la nueva | hecho: `c0ab59c`, `b97405e`, firmas `716296b`, `fb80ded` |
| A4.quater.b | Arquitecto sobre la SPEC nueva, antes del codigo | veredicto | APTO CON CONDICIONES; condiciones cerradas en la SPEC (`b97405e`) |
| A4.quater.c | Pruebas vistas en rojo con el valor de la SPEC: arnes Degradado dos puntas, arnes del puente (respuestas BT), test de la app | arneses en rojo | hecho (`bf6f024`): Degradado 123/128, puente 131/147, jsdom 22 FALLAS, vistas en rojo |
| A4.quater.d | Firmware Maestro y Esclavo (orden de salida, rojo fijo, permiso pendiente, relectura de CNTL) y app (formulario, aviso, textos), disjuntos; lo que la SPEC no diga para el trabajo y vuelve a la SPEC | candidata | hecho: firmware `a4545bb`, app `7cd3042`, literales `e94a052` |
| A4.quater.e | Integracion por el diff y compuerta completa en el arbol principal hasta que dos pasadas den lo mismo; cifras a README/ESTADO | acta | hecho: acta del 04/10 sobre `39a8a1f`, 18 PASS, banco 1368/1369 (solo `D-22`); cifras `c61e428` |
| A4.quinquies | Arquitecto sobre el binario | veredicto | APTO CON CONDICIONES (B1, B2, precedencia, SPEC); cerradas en `5db6eb9` |
| A5 | QA sobre el binario que se entrega | veredicto | APTO CON CONDICIONES, mismas condiciones; cerradas en `5db6eb9`; compuerta convergida en `7aeb327` |
| A4.quinto | Hoja de Marco solo con lo que falta (`Pruebas_Funcional_2026-10-04.html`, nueve casos); la del 02/10 a historico | `.html` | hecho (`a5df8ec`); `PAQUETE_PENDIENTE` se rellena en A6 |
| A6 | Paquete validado `SIN_BANCO` a `entregas/`, `PAQUETE_PENDIENTE` de la hoja de Marco rellenado, `.docx` de las SPEC tocadas, ESTADO y README al dia | `.zip` | hecho: `Paquete_Semaforos_2026-10-04_e831249_SIN_BANCO` en `entregas/` (hoja `ef708f6`) |
| A7 | Banco de Marco con `22fd0d8` y la hoja del 04/10 (14 casos); sus cintas se leen enteras y contando; si pasa, se quita `SIN_BANCO` | cintas | Marco, despues el orquestador |
| A7.b | Segunda pasada de arquitecto y QA sobre las correcciones B1/B2 (cerradas con prueba en rojo, sin revision propia) | veredicto | si el responsable la pide |

**B. Lo que se cierra en paralelo a A, con ficheros disjuntos**

| # | tarea | quien | sale |
|---|---|---|---|
| ~~B1~~ | Hecho (`8378a84`): lista de montaje del lunes 05/10 en `.html` que da PDF: respaldo md5 del firmware del Sisga, grabar, `J16` p1 tapado, `J14`/p5/p8 vacios, radios 2,4 kbps M0/M1 OFF, camaras `D-39`, sin Degradado desatendido | agente campo | `.html` |
| B2 | `N-166` colision de nombre `A-15` | orquestador | una renombrada |
| B3 | Metodo atrasado (`4e8e898` -> `cd3c71b`): actualizar plugin y primera linea de `CLAUDE.md` | orquestador | metodo al dia |
| B4 | Hecho (`5baa997`): Manual 10 (HC-05 en `J17`, caducado) a historico: lo lee `documentos_03` como contrato de tramas; mover con la tupla del pack en el mismo commit, banco antes y despues | orquestador | uno menos |
| B5 | Hecho: hook de proyecto `.claude/hooks/decisiones_recordatorio.py` al editar `DECISIONES.md` | orquestador | hook |
| B6 | Dictamen de SPEC y manuales (`revision/opinion-spec-manuales-2026-10-04`, `6f38ba4`): SPEC_5 con dos numeraciones; tachados en README y SPEC_1/2/4/5; `HUECOS MEDIDOS` con otro nombre en SPEC_5/8 y ausente en SPEC_0; `17_...` sobre 1000 lineas; `.docx` viejos; hoja de pruebas que remite a hojas en `historico/`; tres hojas cortas de operador, APK y conexionado cuando haya banco | orquestador | fase F |
| B7 | `Camaras_Sisga_4x.html` dice que la pluma baja con un coche debajo: cierto con `7ff7d12` (Sisga, sin veto) y falso desde `d3606be` (veto de `D-33`); se corrige cuando se sepa que firmware queda en campo | orquestador | tras decision del responsable |
| B8 | Firmware: el Esclavo se rinde por las 48 h sin `$ALARM` (solo el Maestro emite `LIMITE_48H`; comentario de `Esclavo/src/bluetooth.cpp`). La app ya no promete una alarma; falta la del Esclavo y el aviso del `$EVENT` con 28 d y 48 h en el mismo campo (SPEC_6 hueco 2) | orquestador | SPEC -> prueba -> codigo, tras banco |

**C. Decisiones del responsable** (no las cierra nadie escribiendo)

| # | pregunta | depende de |
|---|---|---|
| ~~C1~~ | Contestada: `D-47` (el rojo fijo sobrevive al corte) | -- |
| ~~C2~~ | Se cierra sola: `D-46` retira `SET_MODO:DEGRADADO`, al Degradado se entra solo por el testigo | -- |
| C3 | `Validacion_LCD` se llevo 4 packs sin `banco/historico/` (`D-32`); renombrar toca 42 ficheros | -- |
| C4 | El grafo vive hoy FUERA del repo, en `D:/@Proyect/Entregas_Semaforos/grafo/` (foto del 28/09 sobre `925c71e`); `entregas/grafo/` no existe. Pendiente: traerlo a `entregas/` o rehacerlo (el metodo aun no lo define). Los historicos ya se MUEVEN a `05_Funcional/historico/` sin `CRUDAS` (02/10) | el responsable |

**D. A la espera de las cintas de campo** (se leen enteras y contando, `CLAUDE.md` §7)

| # | que dira la cinta | prueba de la hoja `2026-10-01` |
|---|---|---|
| D1 | `N-172`: 02/10 sobre `d3606be` el Maestro volvio a quedar en ambar tras salir del Degradado sin orden (08:21-08:31): la reposicion DR6/DR7 no basta en una prueba corta (H-D no era la causa aqui). La cinta no cubre el corte: hace falta exportar en menos de 5 min tras volver la luz, y el diario del Esclavo | 1 y 2 |
| D2 | `D-40` en tarjeta: corte en Automatico -> ambar; Esclavo solo -> lo recupera el Maestro | 3 |
| D3 | `D-38` en tarjeta: J17 fuera -> rojo fijo y alarma cada 60 s | 4 |
| D4 | ~~Aviso de radio en Degradado y salida solo desde el Poste 1~~: medido el 02/10 (`evidencia/021020260914/`), funciona | 5 |
| D5 | Camaras con peatones: retraso de la alarma, veto con carro y con persona | 6 |
| D6 | Centralita de la talanquera: contacto mantenido o pulso | 7 |
| D7 | ESP32 del Esclavo con ultimo arranque por watchdog (`OTRO_PERRO`, 01/10): ¿se repite? (`N-116`) | cualquier cinta |
| D8 | Lo viejo de Marco: los 17 s del Esclavo, focos con el Maestro solo (1.54); `N-164`/`N-165` en banco | -- |

**F. Repo ordenado** (pedido el 02/10): F1 inventario de historia/legacy, grafo y `.map` contra lo que piden las
skills, con quien lee cada fichero por ruta (agente de solo lectura, en curso); F2 mover/archivar en commits
pequenos, rutas y compuerta en el MISMO commit (`CLAUDE.md` §5), SOLO despues de A6 y con el arbol quieto.

**G. Retirar del firmware lo que queda del LCD y la botonera** (responsable, 02/10; choca con `D-32`, que dejo el
mando): G1 censo de solo lectura (piezas, llamadores, vetos que cuelgan, instrumentos, flash), en curso; G2 fila
`D-43`; G3 candidata propia con runbook entero, DESPUES de A6.

**H. `DECISIONES.md`** (responsable, 02/10: «marea»): consulta al arquitecto de plataforma, en curso; la forma
que salga se aplica en la fase F.

**I. Ambar antes del rojo (norma)** (Marco, 02/10): el firmware pasa de VERDE a ROJO sin amarillo (SPEC_1 §3.1) y
enciende ambar solo antes del verde. El Manual de Senalizacion Vial 2024 (`fuentes/md/`, 4.4.3, pp. 394-395): «En ningun
caso se podra cambiar de luz verde a luz roja [...] sin que antes aparezca el amarillo»; secuencias admitidas en
4.4.2 (p. 394), p. ej. Rojo-Verde-Amarillo-Rojo; amarillo 3 s a 50 km/h (rotulo de la Fig. 4-9). I1 decision del
responsable (`D-45`, secuencia); I2 candidata propia con runbook: toca luces, coordinacion, pluma y despeje.

**E. Despues de banco pasado:** fase 5, lo duplicado Maestro/Esclavo a `lib/`.

### La cuenta de la mudanza del 09/11

```
roadmap.md ANTES (7f5db82)                           578 lineas

roadmap.md, lineas de original conservadas           276   1-87, 97-100 (decision de N-187 y regla), 394-578
aqui, lineas de original                             301   88-96 y 102-393
suprimida                                              1   la 101, en blanco
                                                    -----
                                                    578

roadmap.md DESPUES                                   310 lineas (neto -268)
  anadido alli: la tabla de orden nueva, la linea de cierre y la lista de version siguiente (34 lineas)
```
