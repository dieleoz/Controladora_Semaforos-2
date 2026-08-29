---
name: entregar
description: Arma el paquete .zip que se le manda al funcional o al auditor - firmware fuente para PlatformIO y manuales, nunca binarios sueltos. Uso cuando haya que preparar una entrega, un encargo de sesion de banco, o regenerar los .docx de 05_Funcional. Incluye la regla de que nada sale sin banco pasado y como se redacta un LEEME que no se lea como un permiso.
---

# Entregar al funcional

Un semaforo que falla mal mata a alguien. Un paquete es una autorizacion implicita: quien lo
recibe asume que puede instalarlo.

## 1. La regla que decide si se entrega

**Al funcional no sale nada como version instalable hasta que la compuerta este en verde Y el
banco este pasado.** Las dos cosas, no una.

Que la compuerta salga con `0` **no** es permiso: significa que los modelos y los arneses de PC no
encuentran nada. El 05/08/2026 la compuerta dio `11 PASS · 0 FALLA · 0 ABORTADO` **mientras habia
una regresion abierta en la que el Modo Automatico no encendia las luces en la tarjeta**. Las dos
cosas eran ciertas a la vez.

**Pero eso no bloquea todo.** Hay dos paquetes distintos y solo uno esta condicionado:

| paquete | cuando se manda | que es |
|---|---|---|
| **Encargo de banco** | **siempre que falte una medida** | Una PETICION de medida. No entrega nada: pide que alguien ponga la tarjeta delante |
| **Entrega de version** | solo con banco pasado | El firmware para instalar |

Confundirlos es el error. Si falta banco, lo que se manda es el **encargo**, y se dice.

## 2. Los dos comandos

```
python 05_Funcional/convertir_a_word.py     # regenera los .docx desde los .md
python 01_Firmware/compuerta.py             # el acta que acompana al paquete
```

> **Y el `.zip` NO se hace con `Compress-Archive`.** Muere con *"el modulo no pudo cargarse"*
> porque el `PSModulePath` que hereda la sesion mezcla los modulos de PS7 con los de la extension
> del IDE. Es el mismo fallo que ya documenta `CLAUDE.md` §4 para `Get-FileHash`. Se usa
> `zipfile` de Python, que no depende del entorno de quien llama.

Los `.md` de `05_Funcional/` son la **fuente unica**. Los `.docx` se generan; editarlos a mano
crea dos copias que divergen — ya paso. El conversor solo mira el **primer nivel** de la carpeta:
un documento movido a `05_Funcional/historico/` deja de generarse, que es como se retira un
entregable sin borrarlo.

## 2.bis Compilar la APK, que no es obvio y se pierde cada vez

```bash
cd 05_Funcional/App_Semaforo
cp www/app.js www/index.html www/style.css android/app/src/main/assets/public/
printf 'sdk.dir=C:/android-sdk
' > android/local.properties
cd android
export JAVA_HOME="D:/@Proyect/Baliza/7 sw apk/jdk-17/jdk-17.0.12+7"
./gradlew clean assembleDebug --offline
# sale en app/build/outputs/apk/debug/app-debug.apk
```

**Las cuatro trampas, todas medidas el 27/08:**

- **JDK 17, NO 21.** Con el 21 el build muere en `JdkImageTransform ... core-for-system-modules.jar`:
  este AGP no lo soporta. El JDK 21 de la extension del IDE compila Java pero no sirve aqui.
- **El SDK no puede vivir bajo una ruta con espacios.** Esta en `D:\@Proyect\Baliza sw apkndroid-sdk`
  y desde ahi falla con *"El nombre de archivo... no son correctos"*. Se resuelve con una union a
  ruta limpia: `mklink /J C:ndroid-sdk "D:\@Proyect\Baliza sw apkndroid-sdk"`. Es N-44
  otra vez, aplicado al SDK de Android.
- **`local.properties` se escribe con BARRAS NORMALES.** Es un fichero de propiedades de Java: en
  `sdk.dir=C\:ndroid-sdk` el `` se come la barra y queda `C:android-sdk`. Usa `C:/android-sdk`.
- **`local.properties` esta en `.gitignore` y debe seguir estandolo:** lleva una ruta de esta
  maquina. Por eso la receta va aqui.

**Y como se verifica que una APK lleva DENTRO el fuente que dice.** No por tamano ni por confianza:
se abre como zip y se comparan sus `assets/public/{app.js,index.html,style.css}` contra el
repositorio. **Dos APK del mismo contenido NO tienen el mismo md5** —el contenedor cambia por
marcas de tiempo y alineado: 3.806.717 B contra 3.911.388 con contenido identico—, asi que la
comparacion util es **entrada por entrada y por CRC**: 493 entradas, cero nombres distintos, cero
CRC distintos.

**El nombre lleva el commit: `IOT_VIAL_Semaforos_<fecha>_<hash>_SIN_BANCO.apk`.** El 27/08 habia
tres ficheros: dos eran el mismo binario con dos nombres y el tercero otro binario con el mismo
nombre que el segundo. *"Instala la v8.9"* era una instruccion ambigua. **Y el hash no se estampa
sin haber verificado el contenido**: escribir un commit que no se ha medido es lo que la regla del
instrumento prohibe.

## 2.ter El `.zip`, y por que se hace con Python

```python
import zipfile
with zipfile.ZipFile(destino, "w", zipfile.ZIP_DEFLATED) as z:
    for origen, dentro in contenido:
        z.write(origen, dentro)
```

**Nunca `Compress-Archive`**: muere por el `PSModulePath` heredado, el mismo fallo que `CLAUDE.md`
§4 documenta para `Get-FileHash`. Despues del `.zip`, **tres comprobaciones sobre el propio zip**,
no sobre la intencion:

- recuento de artefactos de compilacion **= 0** (`.pio/`, `build/`, `__pycache__`, `node_modules`);
- el **md5 del binario que va dentro** contra el del repositorio: que sea el mismo fichero, no uno
  parecido;
- que el LEEME de dentro **cite el nombre exacto del binario que le acompana**. Un paquete cuyo
  LEEME nombra otra APK es como se instala la version equivocada.

**El nombre del `.zip` lleva el commit**, igual que la APK: `Paquete_..._<fecha>_<hash>_SIN_BANCO.zip`.

> ⚠️ **Y cada vez que cambia un fichero web, la APK anterior queda obsoleta.** Paso tres veces en
> una tarde: se toca `style.css`, la APK del disco ya no lleva el fuente que dice, y su nombre
> sigue apuntando a un commit que ya no la describe. **Recompilar y renombrar es parte del cambio,
> no un paso posterior** — y volver a verificar el contenido, porque el nombre no lo garantiza.

## 3. Que va dentro, y que NO## 3. Que va dentro, y que NO

```
LEEME_PRIMERO.md        lo primero que se lee, y lo primero que se escribe
ACTA_verificacion.txt   el acta de la corrida: fecha, hash de HEAD, toolchain
01_Firmware/            FUENTE para PlatformIO: Maestro, Esclavo, Repetidor,
                        Simulaciones (packs incluidos) y los cuatro arneses
02_Manuales/            .docx y .md
```

**Sin binarios `.bin`.** Se compilan del fuente, y asi lo que se carga se corresponde con el
codigo que se revisa. La excepcion es el paquete de biseccion, donde los binarios etiquetados
**son** el objeto de la prueba: ahi van, con sus MD5 al lado.

**Sin artefactos de compilacion.** La forma fiable de garantizarlo no es una lista de exclusiones
—siempre se escapa algo— sino **partir de los ficheros versionados**:

```powershell
$tracked = git ls-files 01_Firmware | Where-Object { $_ -notmatch '^01_Firmware/(Camara|Semaforos|Diagnostico_LCD)/' }
```

`.pio/`, `build/` y `__pycache__` estan en `.gitignore`, asi que no pueden colarse. Comprobar
despues que el recuento de artefactos es **0**, no suponerlo.

Y fuera las carpetas que el funcional no necesita (`Camara`, `Semaforos`, `Diagnostico_LCD`): 56
ficheros de ruido en los que se pierde lo que si importa.

## 3.bis Lo que el 28/08 cambio en `05_Funcional/`, y que no puede salir mal

**Hay un documento nuevo y es el que ordena a los demas:
`05_Funcional/17_Arquitectura_28-08_y_Decisiones_Abiertas.md`.** Recoge la arquitectura decidida en
obra ese dia y, sobre todo, **lo que sigue abierto**, con su escala de tres niveles —MEDIDO /
ESCRITO / SIN VERIFICAR—. Va en todo paquete, y **antes** que los manuales que corrige: los demas
documentos todavia no incorporan sus correcciones, y el propio 17 lo dice en su cabecera. Cuidado
con un detalle mecanico: nacio **sin `.docx`**, asi que el conversor tiene que correr antes de
armar el `.zip` o se entrega la carpeta con un `.md` suelto que nadie abre.

**La Lista de Compras cambio, y no es una correccion menor.**
`15_Lista_de_Compras_Hardware.md` va por su **3.ª revision del 28/08**: el **`ESP32` SUSTITUYE al
modulo Bluetooth SPP** —ya no se compran `HC-05` ni `JDY-30`—, el **`DS3231` deja la placa STM32 y
se cuelga del `ESP32`** por I2C con pila propia, y **se retiran la pantalla LCD, los cuatro
pulsadores y el mando de reles**. Lo descartado esta **tachado con su motivo, no borrado**, y esa
forma se respeta: una linea borrada vuelve a proponerse dentro de un mes y nadie recuerda que se
descarto. **Una lista de compras vieja se ejecuta con dinero**, asi que si sale una copia impresa
anterior, sale con la cabecera de la revision o no sale.

### Los documentos que llevan aviso de NO EJECUTAR salen CON el aviso, o no salen

Hay manuales que hoy **describen algo que no se debe hacer todavia**, y el aviso es la parte util
del documento — no el preambulo que se recorta para que quepa:

| documento | el aviso que NO se puede separar de el |
|---|---|
| `13_Manual_Modulo_Expansion_I2C_y_Compras.md` | 🛑 **DISENO / PRE-IMPLEMENTACION (V9.0): NO ESTA EN EL FIRMWARE V8.9**, y la fe de erratas de riesgo electrico: **la talanquera NO va en `J14`** —`J14` es una ENTRADA del micro (`PB0`, 3,3 V, sin opto ni diodo); la salida es `J15`—. *"Si ya cableo un rele a `J14`: DESCONECTELO ANTES DE ENERGIZAR"* |
| `15_Lista_de_Compras_Hardware.md` | la misma fe de erratas en su fila **A4**, mas el bloque de cambio de arquitectura que se lee **antes que el resto del documento** |
| `17_Arquitectura_28-08_y_Decisiones_Abiertas.md` | **mientras la polaridad de los cuatro pines de boton no se mida con multimetro (medida `M3`), NO se cablea camara a `J16`** — al reves da demanda permanente o demanda que nunca llega, y las dos son de calle. Y `J16` p1 lleva **12 V crudos** a un conector de senal directa al micro |

**Regla practica, que es la misma que la del LEEME:** si un documento entra en el paquete, entra
**entero y con su cabecera de estado**. Extraer una tabla de conexiones de un manual marcado
*"pre-implementacion"* la convierte en una instruccion de taller, y quien la recibe la ejecuta con
un destornillador. **Y el orden de trabajo se dice en el LEEME: el firmware primero, cargado y
verificado en la tarjeta, y el cableado despues** — nunca al reves (`CLAUDE.md` §9.bis).

## 4. El LEEME es la pieza critica

Es lo unico que garantiza que se lea la mitad de arriba. Orden obligatorio:

1. **Que corre en campo hoy**, y que esto no es eso.
2. **Si ha pasado banco o no.** Con esas palabras.
3. **Que esta roto y sin causa**, si lo hay, en un bloque que no se pueda saltar.
4. Solo despues, las novedades.

> **No abrir con "100% PASS".** Un LEEME que empieza con la cifra en verde se lee como un permiso,
> y quien lo recibe no llega a la linea que dice que no ha pasado banco.

**Las cifras se copian del acta, nunca se escriben a mano.** Un borrador de este paquete decia
*"elimina las 7 transposiciones ciegas"* cuando el banco media **8**. Nadie lo habria notado.

**Y una hipotesis nunca se redacta como una reparacion.** Un manual de pruebas llego a decir *"se
sustituyen C1 y C2 por capacitores de 6 a 10 pF"* para un fallo **no diagnosticado**. Ese es el
error que ya se pago una vez: una pantalla acusaba a *"Y2, pila y R5"* sin haber medido ninguno de
los tres y mando a cambiar componentes sanos. **Primero la lectura, despues la pieza** — y si es
hardware, se le pide al funcional, no se dictamina aqui.

## 5. Antes de comprimir, cuatro comprobaciones

- [ ] `python 01_Firmware/compuerta.py` corrido **ahora** —completo, nunca `--rapido`—, y el acta
      metida en el paquete. **Si la corrida anterior fue `--rapido`, hacen falta DOS pasadas
      completas**: el banco lee el acta ANTERIOR, y la de un `--rapido` no trae las filas
      `compila *`. El acta que va al paquete tiene que traerlas.
- [ ] `python 05_Funcional/convertir_a_word.py` corrido **despues** de tocar cualquier `.md`, para
      que el `17_Arquitectura_28-08...` y las revisiones de la lista de compras salgan en `.docx`.
- [ ] Recuento de artefactos de compilacion en el `stage` = **0**.
- [ ] El LEEME dice si ha pasado banco, en la primera pantalla.
- [ ] **Un solo encargo vigente.** Si hay dos documentos que dicen cosas distintas sobre lo mismo,
      el viejo se archiva en `historico/` con una cabecera que explique por que se cae. No se
      borra: una causa que desaparece en silencio vuelve a proponerse.

## 6. Los `.zip` no se versionan

`.gitignore` los excluye, y con razon: el repositorio ya contiene el contenido. El `.zip` se
regenera de un commit concreto — por eso el LEEME lleva el hash de `HEAD`.

Estado de hoy en `ESTADO.md`. Reglas permanentes en `CLAUDE.md`. Para interpretar la compuerta,
la skill `verificar`.
