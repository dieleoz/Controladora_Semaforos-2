<!-- movido de roadmap.md el 08/10/2026 (cerrado), literal -->
### `N-167` El paquete validado de `a0d605b`: el runbook entero — ✅ HECHO el 29-30/09, sale `a505fa2` `SIN_BANCO`

El 29/09 salio `a0d605b` `SIN_BANCO` sin pruebas propias ni compuerta, por decision del responsable, para que el
funcional lo pruebe en banco. Este es el camino para que salga validado. **Un rojo en cualquier paso para el paquete**
(salvo `D-22`, que pide tarjeta). Aun validado, el binario del paso 4 lleva `_SIN_BANCO` hasta que se mida en
una tarjeta (`CLAUDE.md` §13). Arquitecto 29/09: NO APTO sobre `a0d605b`, hallazgos en el paso 4.

| paso | que | quien | sale cuando |
|---|---|---|---|
| 0 | Leer ENTEROS los exportes de Marco de cada caso de la guia, contando tramas por poste y hora (`CLAUDE.md` §7) | orquestador | acta `evidencia/2026-09-30_campo_deg_auto/` |
| 1 | SPEC al dia con lo construido: los desvios del agente (`#define protocolo_enviarPaquete`, un intento por corte, `ACCION` de `$ALARM`, sin acuse con el Maestro en MENU) se deciden y se escriben; lo que diga el log de Marco manda | orquestador + responsable | `SPEC_2` §7.ter y `SPEC_4` sin frases falsas |
| 2 | Pruebas PRIMERO, vistas en rojo (contra `a0d605b` el rojo sale de inyectar defecto en el `.cpp` real y de los hallazgos abiertos); censo de las pruebas que CELEBRAN el vencimiento (`CLAUDE.md` §9): arnes de dos puntas del automatico (fichero nuevo del arnes de Degradado) con los escenarios de §7.ter (h) y la asercion "nunca verde contra ambar ni verde contra verde" con inyeccion de defecto; pruebas jsdom de la tarjeta y del aviso de paleteros; `app_01` lee `js/deg_auto.js`; dobles del simulador del puente | subagente | linea roja copiada de cada una |
| 3 | Arquitecto (`orquestador:arquitecto-iot`) sobre SPEC y pruebas | subagente | APTO |
| 4 | Codigo: quitar el vencimiento de 31 dias (decidido 29/09); la rendicion del Esclavo se persiste y bloquea la reentrada automatica tras reinicio (hallazgo 2); salir a mano deja `intentoHecho` (hallazgo 3); literal `SIGUE_AMBAR` exacto (5); y lo que caiga en el paso 2. **Hallazgo 1** (una punta se rinde a ambar por hora no fiable y la otra sigue en verde): decide el responsable | subagente en worktree | pruebas en verde |
| 4b | Arquitecto sobre el codigo FINAL, con md5 del binario | subagente | APTO |
| 5 | Compuerta completa dos veces con el arbol quieto; flash por objeto en `firmware.map`; cifras al acta | orquestador | dos pasadas iguales, solo `D-22` en rojo |
| 6 | QA (`orquestador:qa-istqb`) sobre el binario y la APK que se entregan | subagente | APTO |
| 7 | Guia de campo `.html` al dia; los manuales NO (`CLAUDE.md` §15) | subagente | revisada contra lo entregado |
| 8 | Paquete con la skill `entregar`, sha256, `ESTADO.md` y commit + push | orquestador | `.zip` en `entregas/` |

Fases de organizacion del repo: 0 paquetes a `entregas/` · 1 inventarios · 2 `CLAUDE.md` a 200 y `HISTORIA.md` ·
3 `orquestador@diego`, pre-commit y skills a `.claude/particularidades/` · 4 poda B y trinquete en el pre-commit ·
5 `lib/`, despues de banco. De 0 a 4, hechas.

