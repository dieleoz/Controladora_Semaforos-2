# PreToolUse (Edit|Write): al tocar DECISIONES.md, recuerda que es un indice (04/10, tercera recaida).
import json
import sys

try:
    datos = json.load(sys.stdin)
except Exception:
    sys.exit(0)
ruta = (datos.get("tool_input") or {}).get("file_path", "")
if ruta.replace("\\", "/").endswith("DECISIONES.md"):
    print(json.dumps({"hookSpecificOutput": {
        "hookEventName": "PreToolUse",
        "additionalContext": ("DECISIONES.md es un indice, no el producto: el comportamiento va a la SPEC; "
                              "la fila es de una linea y va en el mismo commit que su SPEC; no se pregunta al "
                              "responsable lo que ya ordeno; un ESTADO no lista filas de DECISIONES como tarea.")}}))
