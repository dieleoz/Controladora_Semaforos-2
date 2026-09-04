package com.iotvial.semaforos;

import android.Manifest;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Bundle;

import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;

import com.getcapacitor.BridgeActivity;

/**
 * N-125: LOS PERMISOS DE BLUETOOTH SE PIDEN AQUI, PORQUE NADIE MAS LOS PEDIA.
 *
 * El sintoma que lo destapo, del banco del 04/09: el tecnico empareja el ESP32 en Ajustes
 * de Android, la app dice "sin enlace", y al pulsar "Buscar Modulos Bluetooth" contesta
 * "el escaneo fallo".
 *
 * LA CAUSA, leida del plugin. La accion `list` va directa a listBondedDevices()
 * -BluetoothSerial.java:107-109- SIN comprobar un solo permiso, y el unico
 * requestPermission() que hay en todo el plugin es de ACCESS_COARSE_LOCATION -:220-, que
 * es el permiso de Android 6 a 11 y ademas cuelga de la accion de DESCUBRIR, no de listar.
 *
 * Esta app apunta a targetSdk 34. Desde Android 12 (API 31) getBondedDevices() exige
 * BLUETOOTH_CONNECT CONCEDIDO EN RUNTIME y lanza SecurityException si no lo esta. Esa
 * excepcion cae en el callback de error del plugin y sale como "el escaneo fallo".
 *
 * Los permisos SI estaban declarados en AndroidManifest.xml desde siempre. Declarar no es
 * pedir: en runtime, un permiso peligroso no concedido se comporta igual que uno que no
 * existe. Es el mismo error de forma que un pinMode() sin digitalRead().
 *
 * POR QUE AQUI Y NO EN EL PLUGIN. El plugin vive en capacitor-cordova-android-plugins, un
 * directorio que Capacitor REGENERA: un parche ahi se pierde en la siguiente sincronizacion
 * sin que nadie se entere, que es como vuelve un defecto ya arreglado. MainActivity es
 * nuestra y sobrevive.
 *
 * NO SE BLOQUEA NADA SI EL USUARIO DICE QUE NO. Se pide y se sigue: la app ya sabe declarar
 * que no pudo buscar -"no encontrar y no poder buscar son cosas distintas"-, y esa
 * distincion se conserva. Un permiso denegado tiene que verse como lo que es, no como un
 * equipo ausente.
 */
public class MainActivity extends BridgeActivity {

    private static final int PERMISOS_BLUETOOTH = 1001;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        pedirPermisosBluetooth();
    }

    private void pedirPermisosBluetooth() {
        // Por debajo de Android 12 estos permisos no existen: pedirlos ahi lanza excepcion.
        // Alli el modelo viejo -BLUETOOTH y BLUETOOTH_ADMIN, que no son de runtime- ya basta,
        // y el manifest los declara con maxSdkVersion=30.
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.S) {
            return;
        }

        // BLUETOOTH_CONNECT es el que necesita listBondedDevices() y el que abre el socket.
        // BLUETOOTH_SCAN va con el porque el plugin tambien ofrece descubrir; pedir los dos
        // de una vez ahorra una segunda ventana al tecnico, que esta subido a un poste.
        final String[] queremos = {
            Manifest.permission.BLUETOOTH_CONNECT,
            Manifest.permission.BLUETOOTH_SCAN
        };

        boolean falta = false;
        for (String permiso : queremos) {
            if (ContextCompat.checkSelfPermission(this, permiso)
                    != PackageManager.PERMISSION_GRANTED) {
                falta = true;
                break;
            }
        }

        // Solo se pregunta si falta alguno: volver a pedir lo ya concedido no abre ventana,
        // pero deja ruido en el log y confunde al que lo lee buscando el fallo.
        if (falta) {
            ActivityCompat.requestPermissions(this, queremos, PERMISOS_BLUETOOTH);
        }
    }
}
