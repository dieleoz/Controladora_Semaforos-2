// ===== include/version_fw.h =====
//
// EL SELLO DE FIRMWARE: QUE COMMIT LLEVA ESTE BINARIO DENTRO.
//
// ESTE FICHERO ES EL MISMO, BYTE A BYTE, EN LAS TRES PUNTAS -Maestro, Esclavo y
// ESP32_Expansion-, y no por comodidad: dos sellos con formatos distintos serian dos
// convenciones que el tecnico tiene que aprender en el mismo poste, y la comparacion de
// una foto de campo contra un commit dejaria de ser mecanica. Que sigan siendo iguales no
// se confia a este comentario -un comentario no falla cuando alguien toca un fichero-: lo
// compara byte a byte version_01_sello_de_firmware en cada corrida, como costura_01 hace
// con los contratos de radio.
//
// POR QUE EXISTE. CLAUDE.md 0.2 exige que una foto de campo traiga el hash de lo que habia
// dentro del equipo. Hasta hoy ese hash NO ESTABA EN NINGUNA PARTE DEL EQUIPO: vivia en la
// memoria de quien cargo la tarjeta, y la cinta del Sisga del 16/09 se pudo atribuir solo
// porque el responsable se acordaba. Una medida de campo sin sujeto no es una medida.
//
// EL VALOR SE INYECTA AL COMPILAR Y NO SE ESCRIBE AQUI. Lo saca de git el '!' de
// platformio.ini. Un numero escrito a mano en un fuente nace caducado y nadie puede
// recalcularlo: es el caso exacto de CLAUDE.md 14 -"un numero en un sitio que no puede
// recalcularlo no se sincroniza: se retira"-, y un sello a mano ademas MIENTE con
// autoridad, porque lo que afirma es precisamente de que commit sale el binario.
//
// ---------------------------------------------------------------------------
// LA MARCA DE ARBOL SUCIO NO ES UN ADORNO: ES LA MITAD QUE IMPORTA.
//
// Un binario compilado sobre trabajo sin comitear NO contiene el commit que su hash
// nombra: contiene ese commit MAS lo que hubiera encima, que no esta en ningun sitio y no
// se puede recuperar. Sin la marca, ese binario declararia un arbol limpio que no lleva
// dentro, y la foto de campo tomada sobre el se atribuiria al commit equivocado -o sea el
// defecto que este sello viene a cerrar, con una capa de autoridad encima-.
//
// SE ESCRIBE EN PALABRA Y NO EN UN SIGNO. Un '+' suelto es el convenio de git y no dice
// nada al tecnico del poste; el precedente del repositorio es el sufijo _SIN_BANCO de
// CLAUDE.md 13, que esta en palabra por el mismo motivo: "su ausencia se lee como
// permiso", y para eso hay que poder leerla.
//
// Y SE MARCA SUCIO TAMBIEN POR LO NO SEGUIDO, no solo por lo modificado: un .cpp nuevo sin
// anadir a git ENTRA en el binario igual que uno modificado. El censo del arbol es el de
// 'status --porcelain', que los cuenta a los dos. Equivocarse hacia "sucio" cuesta una
// palabra de mas en una trama; equivocarse hacia "limpio" cuesta la atribucion de una
// cinta.
// ---------------------------------------------------------------------------
//
// SI EL SELLO NO ENTRO, NO SE INVENTA NINGUNO. Falta cuando git no esta en el PATH, cuando
// el arbol no es un repositorio, y -medido- cuando estos fuentes los compila un arnes de
// banco con g++ de host: Simulaciones/puente_esp32/compilar.ps1 compila los dos
// bluetooth.cpp REALES y no pasa por platformio.ini. En ese caso FW_SELLADO vale 0, el
// despachador NO contesta OK y la respuesta lleva la marca de "todavia no lo se".
//
// POR QUE '--' Y NO '!'. CLAUDE.md 10 separa las dos marcas y aqui toca la primera: '!'
// significa "el valor existe y esta FUERA DE COTA", y '--' significa "todavia no lo se".
// Un binario sin sellar no tiene un hash fuera de rango: NO TIENE HASH. Es el mismo caso
// que BAT:-- del $STATUS y que el CNT:-- de reportarBitsDelReloj(), y se marca igual para
// que el tecnico no tenga que aprender una segunda convencion en la misma pantalla.

#ifndef VERSION_FW_H
#define VERSION_FW_H

// EL SELLO LLEGA COMO DOS MACROS SIN COMILLAS, Y ESO NO ES ESTILO.
//
// Pasar -DFW_HASH=\"abc1234\" obliga a que las comillas sobrevivan al shell que ejecuta el
// '!' -cmd.exe en Windows, sh en otra maquina- y a SCons detras. Ese escape es justo lo que
// se rompe en silencio, y cuando se rompe el binario sale SIN SELLAR sin que nada lo diga.
// Un token desnudo -un hash corto es una pp-number valida- no necesita escapar nada, y la
// comilla la pone el preprocesador aqui, donde no hay shell que la toque.
//
// SE EXIGEN LAS DOS. Con el hash puesto y la suciedad ausente, el binario declararia un
// arbol limpio que NADIE MIDIO, que es la unica mentira que este fichero no puede
// permitirse. Si falta cualquiera de las dos, el firmware dice que no lo sabe.
#if defined(FW_HASH) && defined(FW_SUCIO)
  #define FW_SELLADO 1
#else
  #define FW_SELLADO 0
#endif

// La vuelta de tuerca obligatoria: sin el nivel intermedio, '#' congela el NOMBRE de la
// macro -"FW_HASH"- en vez de su valor.
#define FW_TEXTUAL2(x) #x
#define FW_TEXTUAL(x)  FW_TEXTUAL2(x)

// Ninguna de las dos lleva ',' '*' '$' CR ni LF: la coma separa campos de la trama y el
// asterisco abre el checksum, asi que cualquiera de los dos partiria la trama por dentro
// con el checksum casando igual. Lo exige el pack, que es donde eso no envejece.
#define FW_MARCA_SUCIO  "+SUCIO"
#define FW_SIN_SELLO    "--"

#if FW_SELLADO
  #if FW_SUCIO
    #define FW_TEXTO  FW_TEXTUAL(FW_HASH) FW_MARCA_SUCIO
  #else
    #define FW_TEXTO  FW_TEXTUAL(FW_HASH)
  #endif
#else
  #define FW_TEXTO  FW_SIN_SELLO
#endif

// POR QUE ESTO ES EL PREPROCESADOR Y NO UN if DE VERDAD.
//
// El sello es un hecho del TIEMPO DE COMPILACION: no hay estado, no hay medida y no hay
// nada que pueda cambiar mientras el equipo esta encendido. Un `if (FW_SELLADO)` compilaria
// a la guarda de un solo valor que CLAUDE.md 6.2 persigue -el enum que sale como
// `movs r0,#1`-, y encima gastaria flash en una rama que el enlazador ya sabe muerta.
//
// Lo que eso cuesta, dicho: en ESTE binario existe UNA sola de las dos respuestas, asi que
// ningun arnes en ejecucion puede ver las dos. Quien tiene que poder verlas es el
// instrumento, y por eso el pack lee los DOS literales del fuente y los exige los dos
// presentes; y el sello se comprueba ademas sobre el binario, buscando la cadena dentro del
// .elf, que es la medida y no la lectura.

#endif // VERSION_FW_H
