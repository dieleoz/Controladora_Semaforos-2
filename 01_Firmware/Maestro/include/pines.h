// ===== include/pines.h =====
#pragma once

// --- Semáforo 1 (vehicular principal) ---
#define ROJO1       PA0   // S1 -> J3
#define AMARILLO1   PA1   // S2 -> J4
#define VERDE1      PA2   // S3 -> J5

// --- Semáforo 2 (vehicular secundario) ---
#define ROJO2       PA3   // S4 -> J6
#define AMARILLO2   PA4   // S5 -> J7
#define VERDE2      PA5   // S6 -> J8

// --- Semáforo peatonal ---
#define ROJO_PEATON   PA6 // S7 -> J11
#define VERDE_PEATON  PA7 // S8 -> J9

// D-14 (13/09, decidido por el responsable en D-32 (4)): LA ENTRADA DE ALARMA DE LA
// CAMARA -el controlador cierra un contacto y la camara graba- NO SE INSTALA EN ESTE
// DESPLIEGUE. Medido sobre la guia que sigue el instalador
// (05_Funcional/Camaras_Sisga_4x.html): cablea J16 (48 menciones) y J15 (15), nombra
// J14 (9), y de J9, J11 y J13 -los tres candidatos de D-d, roadmap.md 2.4- CERO. Y solo
// cablea la SALIDA de alarma de la camara (1A/1B, hacia J16); nunca su entrada, que es
// lo que D-14 pedia.
//
// EL ANCLA VIVE AQUI PORQUE J9/J11/J13 SON LOS TRES CANDIDATOS: J9 (VERDE_PEATON,
// arriba) y J11 (ROJO_PEATON, arriba) siguen sin firmware detras (D-d), y J13 (BUZZER,
// mas abajo) tambien. NINGUNO SE GASTA PARA D-14: no hay salida de la tarjeta cableada
// hacia la entrada de alarma de la camara, y quien llegue con el destornillador no
// cablea nada ahi para esto. Si D-14 se instala algun dia, primero se elige canal
// (roadmap.md 2.4) y se escribe el codigo; hasta entonces no hay nada que construir.

// --- Actuadores ---
#define LORA_DE_RE        PB12 // Control DE/~RE del MAX3485 (bus OUT) -> E90-DTU LoRa
#define BUZZER             PB1  // -> opto U13 -> MOSFET Q8 -> bornera J13
// J15 SI EXISTE, y esta bornera es la correcta. La nota anterior decia lo contrario
// porque se midio sobre 03_Hardware_Tarjeta/KiCad/*.kicad_sch, que esta INCOMPLETO
// -sin LCD, sin botones y sin este canal-. En el esquematico bueno
// (01_Firmware/Controladora_Semaforos/.../*.kicad_sch, 649 KB) la red se llama
// "Motor" y tiene canal de potencia propio, igual que las ocho luces:
//
//     PB2 --> R70 220 / R69 10K --> opto TLP127 (U15) --> MOSFET IRLZ44N (Q10) --> J15
//
// Son DIEZ MOSFET y DIEZ optos en la placa (Q1..Q10, U6..U15), no nueve: el buzzer
// tiene el suyo (U13/Q8/J13) y la talanquera el suyo. Ver roadmap.md N-64.
#define MOTOR_TALANQUERA   PB2  // -> opto U15 -> MOSFET Q10 -> bornera J15

// SFTY-28: niveles de la pluma. En LOW el MOSFET no conduce, el motor queda SIN
// energia y la pluma se queda ABAJO. Ese es el fallo seguro y por eso el nivel de
// reposo es el de cerrar: si el equipo se muere, la barrera no queda abierta.
// La especificacion de compra pide ademas actuador con retorno por muelle o gravedad,
// porque eso el software no lo puede garantizar.
#define TALANQUERA_ABRIR   HIGH
#define TALANQUERA_CERRAR  LOW

// --- Camaras IA AcuSense (entradas de contacto seco) ---
//
// La camara de DEMANDA entra por PB0, y la placa la ayuda: esa linea lleva R64 10K y
// C25 100nF hasta la bornera J14, o sea un RC de 1 ms. Es un ANTIRREBOTE POR HARDWARE
// -la bornera estaba pensada como entrada, y por eso el esquematico la llama "Puerta"-.
// 🛑 D-27 (2) (11/09, decision del responsable): J14 QUEDA LIBRE, SIN CABLEAR. El fin de carrera
// NO se instala en este despliegue, y eso cierra el conflicto de A-2 SIN TOCAR EL FIRMWARE.
// MIENTRAS EL FIRMWARE LEA PB0 COMO DEMANDA, EN J14 NO SE CONECTA NADA: J14 es una ENTRADA del
// micro -3,3 V, sin opto ni diodo- y la salida de talanquera es J15. Un rele cableado aqui se
// desconecta ANTES de energizar. El ancla vive en este fichero a proposito: es donde mira quien
// llega con el destornillador (CLAUDE.md 3).
#define CAM_DEMANDA_PIN    PB0  // -> R64 10K + C25 100nF -> bornera J14 (antirrebote 1 ms)

// LA CAMARA DE UMBRAL NO TIENE ENTRADA FISICA, Y ESTE PIN NO ES ELLA.
//
// Medido el 27/08 sobre el esquematico bueno: PB8 va por R16 1K a un LED (D5). Es un
// TESTIGO en la placa, no una bornera y no una entrada optoacoplada. Durante meses
// cuatro manuales lo describieron como "umbral de tramo" (N-59) y el firmware le hacia
// un pinMode que no servia para nada.
//
// Se deja en alta impedancia a proposito: sea cual sea el sentido del LED -no esta
// trazado-, un pin flotante no puede encenderlo. Con INPUT_PULLUP se le colarian unos
// 40 uA y quedaria un testigo encendido a medias que nadie sabria explicar.
//
// Si algun dia se quiere la camara de umbral hacen falta DOS cosas, y ninguna es este
// pin: un hilo -pad de PB8 retirando R16/D5, o uno de los cuatro pines sin cablear
// (PA11, PA12, PA15, PC13)- y un comando de radio que lleve la cuenta del tramo al
// Maestro, que es quien decide. Ver roadmap.md N-59 y N-64.
#define LED_TESTIGO        PB8  // -> R16 1K -> LED D5. NO es entrada de camara

// --- LCD ST7920 (modo serial, 3 hilos + PSB + RST) ---
//
// LOS DATOS SON PB3/PB4/PB5 Y NADA MAS. PB6 es un nivel estatico que se escribe una sola
// vez en el arranque y PB7 es el reset: ninguno de los dos transporta datos del display,
// aunque su sitio en este bloque lo sugiera. Importa al depurar -si la pantalla sale mal,
// mover PB6 o PB7 no cambia lo que se dibuja- y al repartir pines.
//
// Y AMBOS ESTAN MULTIPLEXADOS CON PERIFERICO, cosa que no estaba escrita en ningun sitio
// del fuente y que cambia decisiones de hardware: PB6 y PB7 son USART1 REMAPEADO y ademas
// I2C1 por hardware. Quien pida un segundo puerto serie o un bus I2C tiene que saber que
// esos dos pines ya se los quedo la pantalla, y que soltarlos obliga a recablear el LCD.
//
// PENDIENTE DE CONFIRMAR EN LA PLACA -- el nombre de PB6 no cuadra:
// aqui se llama LCD_PSB, pero la ETIQUETA DE RED del esquematico para ese mismo hilo es
// "RS(A0)". Los dos nombres no pueden ser ciertos a la vez, y menos cuando en este mismo
// bloque el nombre "RS" ya esta dado a PB4. Lo que hace el firmware -pinMode y un
// digitalWrite(LOW) una sola vez en lcd.cpp, y nunca mas- es propio de un PSB, el selector
// de modo serie/paralelo; un RS se conmuta en cada byte. Eso INCLINA la respuesta, no la
// cierra: se resuelve siguiendo el hilo hasta la pata rotulada del modulo del display, no
// leyendo mas codigo. Queda anotado sin darlo por resuelto en ninguna de las dos direcciones.
#define LCD_SCLK    PB3   // -> E del LCD  (clock serial)
#define LCD_CS      PB4   // -> RS del LCD (chip select)
#define LCD_SID     PB5   // -> RW del LCD (dato serial)
#define LCD_PSB     PB6   // -> PSB del LCD (fijo LOW). Nombre SIN CONFIRMAR: la red se rotula "RS(A0)"
#define LCD_RST     PB7   // -> RST del LCD (reset)

// --- J16: mitad botonera, mitad camaras (decision del 31/08/2026) ---
//
// D-2: LAS DOS POSICIONES DE ABAJO SON LAS CAMARAS, Y ESA ES LA DECISION QUE GOBIERNA
// TODO LO QUE SIGUE. Se eligieron esas dos y no otras por descarte: son las unicas del
// conector que el reconocedor de secuencias del mando no necesita, asi que darselas a
// la camara no le quita su puerta a nadie.
//
// J16 llevaba los CUATRO pulsadores. Desde el 31/08 se queda con dos -A y B, que son
// los que alimentan las secuencias del mando de reles (SFTY-21)- y sus otras dos
// posiciones pasan a ser entradas de camara. No es un cambio de nombre: cambia el modo
// del pin, la polaridad con la que se lee y quien lo lee.
//
//   J16 p5   PB9    BOTON1      Arriba / mando A   INPUT pelado,  activo en ALTO
//   J16 p8   PB13   BOTON2      Abajo  / mando B   INPUT pelado,  activo en ALTO
//
//   ^ ESAS DOS FILAS DECIAN "INPUT_PULLUP, activo en BAJO" Y ERA FALSO (05/09). El
//     fuente hace pinMode(BOTON1, INPUT) pelado -botones.cpp:160-161- y lee
//     digitalRead(b.pin) == HIGH -botones.cpp:40-, o sea justo lo contrario. Y lo
//     contradecia el texto de N-118 TREINTA LINEAS MAS ABAJO EN ESTE MISMO FICHERO,
//     mas la medida en cobre del 03/09: pull-down real de 10 kOhm en las cuatro
//     posiciones (R65-R68), con 3,3 V en la de al lado. El gesto es cerrar contra los
//     3,3 V, y es el MISMO para los cuatro pines de J16, sin excepcion.
//
//     Se anota en vez de corregirse en silencio porque esta es la cabecera que todo el
//     mundo lee primero, y una frase falsa aqui gobierna lo que alguien cablea. Lo
//     encontro un agente revisando las spec, no un test: los comentarios no compilan.
//   J16 p10  PB14   CAM_C_PIN   camara             INPUT pelado,  activo en ALTO
//   J16 p12  PB15   CAM_D_PIN   camara             INPUT pelado,  activo en ALTO
//
// POR QUE ACTIVO EN ALTO, Y POR QUE ESO NO ES UNA PREFERENCIA. Es la cuenta de N-67,
// con la misma resistencia y el mismo valor: R67 y R68 son 10K A MASA sobre las redes
// /Boton3 y /Boton4, y J16 saca 3,3 V en p9 y p11 -las posiciones de al lado-. Eso es
// un pull-DOWN con la tension a un pin de distancia: el gesto previsto es cerrar el
// contacto seco de la camara contra los 3,3 V del propio conector. Con INPUT_PULLUP el
// pull-up interno (~40 kOhm) contra ese 10K deja el pin en 3,3 x 10/50 = 0,66 V, que el
// micro lee LOW: demanda permanente sin camara conectada, e invertida al cerrarla.
//
// 🟢 M3 CERRADA EN BANCO EL 03/09, Y AQUI HABIA UNA FRASE FALSA QUE SE TACHA EN VEZ DE
// BORRARSE. Ponia: "si la placa soldada fuera la del netlist, PB9 y PB13 en INPUT_PULLUP
// estarian en LOW permanente y el menu no se podria navegar -Y HAY EVIDENCIA DE BANCO DE
// QUE SE NAVEGA-". Sobre esa ultima frase se sostenia que A y B eran un caso distinto de
// C y D, y por eso se quedaron en INPUT_PULLUP cuando C y D pasaron a INPUT.
//
// NO EXISTE ESA EVIDENCIA. Lo que 17_Arquitectura citaba como tal es un PROTOCOLO -un
// plan de pruebas, no un resultado-. La unica observacion real de banco sobre estos pines
// dice lo contrario: N-26 (01/08) apunta que la tarjeta "se plantaba sola en la pantalla
// de configuracion del Modo Manual sin que nadie tocara la botonera", que es la firma de
// unos pines en BAJO al arrancar con J16 vacio.
//
// Y la medida: paso 20 del 03/09 -p5 9,92 kOhm y 0,6 V; p8 9,92 y 0,6; p10 9,93 y 0 V;
// p12 9,94 y 0 V-. Los CUATRO pines son identicos. La placa soldada SI es la del netlist,
// asi que A y B se leen como C y D: INPUT pelado y activo en ALTO (N-118). Ya se puede
// cablear camara a J16.
//
// D-3: Y ES ESA MEDIDA LA QUE CIERRA LA DECISION, no un razonamiento. Lo que estuvo
// bloqueado no era el firmware -que ya leia como el cobre pide- sino ENCHUFAR: la duda
// era si el reposo lo fijaba de verdad la placa, y eso solo lo contesta el conector
// vacio con un instrumento delante. Mientras estuvo abierta, lo prohibido era el
// destornillador.
//
// El firmware sigue yendo primero (CLAUDE.md 9.bis): un pin en INPUT no ejecuta nada,
// mientras que con el firmware viejo dentro PB14 seguia siendo botonAceptar() activo en
// BAJO y cualquier hilo enchufado en p10 lo pulsaba.
//
// Y p1 de J16 lleva 12 V CRUDOS -sin opto, sin serie, sin clamp- a nueve posiciones de
// p10 y once de p12. Se tapa fisicamente antes de enchufar nada (17_...:2.1).
//
// D-4: TAPARLO ES OBLIGATORIO EN CADA EQUIPO QUE SE MONTE, no una cautela de banco. Y
// va escrito en este fichero porque es el sitio donde alguien mira antes de cablear: el
// firmware NO PUEDE protegerse de esto -esa posicion no llega a ningun pin del micro,
// asi que ninguna linea de codigo se entera de que la puentearon contra una de senal-.
// La barrera es fisica o no hay barrera.
#define BOTON1      PB9   // J16 p5  - Arriba / mando A
#define BOTON2      PB13  // J16 p8  - Abajo  / mando B
#define CAM_C_PIN   PB14  // J16 p10 - camara de contacto seco (era BOTON3, "Aceptar")
// D-25 (11/09, decision del responsable): DOS CAMARAS POR POSTE, y por eso p12 DEJA DE ESTAR
// VACIO. Deroga de D-13 "una camara por poste" y "p12 vacio a proposito". El codigo que lo
// cumple ya existe -CAM_J16[2] recorrido en el mismo bucle de camaras_actualizar()-, asi que
// esta marca dice DONDE vive la decision, no que quede algo por construir.
// ⚠️ LO QUE D-25 SI DEJA PENDIENTE Y ESTA MARCA NO CUBRE: al ir p12 cableado, la exencion de
// CAM_CIEGA para una camara que nunca dio flanco PIERDE SU MOTIVO -una segunda camara muerta
// desde la instalacion no se detecta sola-. Vive en botones.cpp, SIN numero de fila a proposito
// para que no se cuente como construida, y en roadmap.md fila 1.9.
#define CAM_D_PIN   PB15  // J16 p12 - camara de contacto seco (era BOTON4, "Cancelar")

// --- RS485 "IN" / Telemetría Bluetooth (USART1) ---
#define RS485_IN_RX     PA10
#define RS485_IN_TX     PA9
// PA8 gobierna ~RE (pin 2) y DE (pin 3) de U2 -el MAX3485 del USART1-, NO de U3: U3 es el
// del USART3, el de la radio LoRa (PB11/PB12/PB10, par A/B por J12). Aqui ponia "U3" y
// estaba invertido; queda escrito para que no se vuelva a escribir al reves.
//
// Y el "Hi-Z" solo vale para el RECEPTOR. Con PA8 en HIGH el TRANSMISOR de U2 sigue
// encendido, asi que J10 emite la telemetria de forma permanente y no puede recibir nunca.
// Inofensivo mientras J10 este vacio; el porque completo, en bluetooth_setup() de
// bluetooth.cpp y en 01_Firmware/TROUBLESHOOTING.md (DE/RE clavado = linea bloqueada en
// ambos sentidos, repetidor del 31/07/2026).
#define RS485_IN_DE_RE  PA8  // HIGH: apaga el receptor de U2 y libera PA10 (el TX de U2 queda activo)

// --- RS485 "OUT" (USART3) ---
#define RS485_OUT_RX    PB11
#define RS485_OUT_TX    PB10
// DE/RE is controlled by LORA_DE_RE (PB12) as defined above.