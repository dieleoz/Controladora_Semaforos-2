#!/usr/bin/env bash
# Trinquete del simulador (CLAUDE.md §4): la razon lineas de instrumentos / lineas de producto no sube.
# Lo llama .topes, que el hook del metodo carga. Techo en .topes (RAZON_TECHO); se baja, nunca se sube.
# Cuenta el INDICE (lo que se va a comitear), no el disco. Carpetas historico/ no cuentan.
contar() {  # $1 = regex de rutas; cuenta lineas de los ficheros del indice que casan
  git ls-files -s | awk '{print $4}' | grep -E "$1" | grep -v '/historico/' |
    while read -r f; do git show ":$f" 2>/dev/null; done | wc -l
}
ins=$(contar '^01_Firmware/(Simulaciones|Validacion_[^/]+)/.*\.(py|cpp|h|js)$|^01_Firmware/compuerta\.py$|^05_Funcional/App_Semaforo/tests/.*\.js$')
pro=$(contar '^01_Firmware/(Maestro|Esclavo|Repetidor|ESP32_Expansion)/(src|include)/|^05_Funcional/App_Semaforo/www/(app\.js|js/.*\.js)$')
razon=$(awk -v i="$ins" -v p="$pro" 'BEGIN{ if (p>0) printf "%.3f", i/p; else print "999" }')
if awk -v r="$razon" -v t="${RAZON_TECHO:-999}" 'BEGIN{ exit !(r > t + 0.0005) }'; then
  echo "RECHAZO  trinquete: instrumentos/producto = $razon ($ins/$pro), techo $RAZON_TECHO. Retira pruebas o" \
       "espera a que un acta de campo archive un tema (CLAUDE.md §4)." >&2
  exit 1
fi
echo "trinquete: $razon ($ins/$pro), techo $RAZON_TECHO"
