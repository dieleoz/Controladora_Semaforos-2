# ROADMAP de la app — lo hecho y lo que falta

La app (APK y web de PC) que HACE hoy esta en `SPEC_4`; aqui solo el estado de cada mejora y lo pendiente. El origen
fue lo que Pasos Peatonales (`D:\@Proyect\IoT\Pasos Peatonales`, HEAD `e968cc7`) rehizo de esta app el 08-09/10; la
comparacion fila a fila esta en git (este fichero en `63e4c63`). Cada fila nueva entra primero en `SPEC_4` y su
prueba se entrega vista en rojo (`CLAUDE.md` §15). «Sin telefono» = sin medir en un celular; «sin banco» = sin tarjeta.

## Hecho

| mejora | `SPEC_4` | commit | falta medir |
|---|---|---|---|
| Hoja de banco con «Debe ver antes de empezar» | — | `Pruebas_Funcional_2026-10-16.html` | — |
| Un color, una accion; gris sin dato; frase por estado; sin jerga (`D-80`, `D-83`) | §9.9 (a)-(c) | `cd437b2` | sol |
| Cabecera: `#rssi-text` <= 16 caracteres, «(sin radio)» (`D-82`) | §9.9 (d) | `cd437b2` | sin red |
| Un solo aviso de paleteros al cambiar el verde en Degradado (`D-81`) | §9.9 (e) | `cd437b2` | — |
| Dialogos propios (0 nativos); grupo nuevo sin valores al azar | §9.1 | `38a42ca` | — |
| Clave de administracion y de soporte (`D-56`, `D-57`) | §9.2 | `a16356d` | — |
| Pastilla «Operario»/«Tecnico», pestana «Ajustes», frase por ajuste (`D-145`) | §9.9 (g) 6, (h) | `4105ba3` | — |
| Mas/Detalle plegados, resumen en una linea, focos en «Mas» (`D-141`, `D-142`) | §9.9 (f), (g) | `4105ba3` | scroll |
| Un solo «Exportar registro» en Eventos; WhatsApp y CSV fuera (`D-143`) | §4, §9.3 | `4105ba3` | compartir |
| Avisos a una frase, «Detalle» plegado, «Emparejar y conectar» (`D-144`) | §9.9 (g) 5 | `4c709fd`, `c94a6cb` | — |
| Guion por tarea del tecnico, bloque «T» de `dom_usabilidad.js` (`D-144`) | §9.9 (i) | `4105ba3` | — |
| Nueve rechazos con frase llana; `RESULT:` desconocido en rojo (`N-183`, `D-156`) | §7.16 | `f41caf8` | — |
| Interruptores en Ajustes > Averias (`D-94`..`D-97`, `D-148`..`D-152`) | §7.10 | `bd16447` | banco |
| La practica contesta los interruptores (`PLUMA:FUERA`, `CAM:NOVETO`) | §9.10 (c) | `666d904`, `6242f8f` | — |
| Ajustes > Instalacion: NA/NC por camara y retardo 3..5 s (`js/instalacion.js`, `js/sim_instalacion.js`; `D-161`..`D-165`) | §9.15 | `3ea00d4`, `c10e60f` | banco |
| Espera de `SET_DEG_AUTO` atada a la conexion (`D-130`) | §3.ter.bis | `6828945` | — |
| Registro de la sesion: cambios de `$STATUS`, un `.txt`, copia fuera de CACHE (`D-55`) | §9.3 | `71241ec` | `copy` |
| Registro en el puente, `CMD:LOG,DESDE:<N>` (`D-72`..`D-75`, `D-100`) | §9.8 | `cd437b2` | banco |
| Reinicios fechados y contador que baja (`D-60`..`D-64`) | §9.4 | `cd437b2` | banco |
| `RESULT:SIN_CAMBIO` en `INTELIGENTE` repetido (`D-65`, `D-66`; `ALCANCE` salio con `D-167`) | §9.5 | `cd437b2` | banco |
| Alarmas del limite repetidas mientras duran (`D-67`..`D-69`) | §9.6 | `cd437b2` | banco |
| SPP del puente que no bloquea, `CONG` contado (`D-70`, `D-71`) | §9.7 | `cd437b2` | banco |
| Practica sin poste (`D-86`..`D-89`), con estado y grupos (`D-117`..) | §9.10, §9.14 | `cd437b2`, `ac844b9` | franja |
| `app.js` en modulos, tandas 1, 2, 3 y 4.a (`D-98`) | §9.12 | `cd437b2`, `8526f84`, `8ac39f4`, tanda 4.a del 10/11 | — |

## Pendiente

1. **Medir en el telefono** (P, por CDP): scroll de Trafico a 360x640 y ancho a 320 (§9.9 (g) 3); la hoja de
   compartir y que WhatsApp reciba el `.txt` (§7.12); `copy` al exportar y cuanto se pierde al matar la app (§9.3);
   la franja SIMULADO sobre el modal del PIN (§9.10); contraste del gris a pleno sol (§9.9 (b)).
2. **`D-98`, tanda ultima (modales y PIN)** (§9.12): solo con `tests/dom_claves.js` delante y `app_01`/`app_05`
   recontados.
3. **Aviso en vivo de un fallo de escritura del fichero de sesion**: hoy solo se declara en el `.txt` (§9.3).
4. **Emparejamiento del puente con clave** (`D-158`): firmware del ESP32, version siguiente; la app no cambia.
5. **Del responsable:** `ESC:AMARILLO` (§6, sin decidir). El Modo Alcance salio con `D-167` (§7.5).
6. **Version siguiente:** el origen del ambar del Poste 2 (§9.11, queda fuera).

## Lo que NO se trae, y por que

- **Claude Design como paso obligatorio.** Alli el brief no quedo en el repo y la app se comparo contra el lienzo a
  mano. No sustituye a la revision en simulado ni al guion por tarea.
- **Reenvio automatico de una orden tras pedir la clave** (alli dio `CLAVE, PRUEBA, HORA`; arreglado en `ca3ab68`).
- **Codigo por delante de la SPEC**: alli lo pagaron dos veces (`ca3ab68`, `93b8977`).
- **Una pestana «Datos tecnicos»**: Eventos queda en la zona tecnica (`D-80`).
