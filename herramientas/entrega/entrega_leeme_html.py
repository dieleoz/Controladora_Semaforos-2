# ===== entrega_leeme_html.py =====
#
# El LEEME del paquete en HTML, para generar_entrega_v9_0.py. Sale de ese fichero el
# 28/09 en un commit de CORTE, sin cambio de comportamiento: el texto de abajo es el
# mismo, linea por linea. Solo usa `html` y `re`; no sabe nada del paquete.

import html
import re


# ---------------------------------------------------------------------------
# El LEEME en HTML. Subconjunto de Markdown, y las dos reglas que el conversor a
# Word incumple (skill `entregar` seccion 2): se respeta `\|` dentro de una celda, y un
# bloque `>` NO se aplasta en un solo parrafo.

_CELDA = re.compile(r"(?<!\\)\|")


def _celdas(linea):
    partes = _CELDA.split(linea.strip())
    if partes and partes[0] == "":
        partes = partes[1:]
    if partes and partes[-1] == "":
        partes = partes[:-1]
    return [p.strip().replace("\\|", "|") for p in partes]


def _en_linea(texto):
    """Escapa HTML PRIMERO y aplica el marcado despues: si se hace al reves, un
    `<` del texto se come la etiqueta que se acaba de generar."""
    t = html.escape(texto, quote=False)
    t = re.sub(r"`([^`]+)`", r"<code>\1</code>", t)
    t = re.sub(r"\*\*([^*]+)\*\*", r"<strong>\1</strong>", t)
    t = re.sub(r"(?<![\w*])\*([^*\n]+)\*(?![\w*])", r"<em>\1</em>", t)
    return t


def _md_a_html(md, titulo):
    """Renderiza el LEEME. No es un motor de Markdown: cubre lo que el LEEME usa
    -titulos, tablas, citas, listas y parrafos- y nada mas. Un motor completo aqui
    seria una dependencia que el funcional tendria que tener instalada."""
    lineas = md.split("\n")
    out, i = [], 0
    while i < len(lineas):
        linea = lineas[i]

        if not linea.strip():
            i += 1
            continue

        m = re.match(r"^(#{1,4})\s+(.*)$", linea)
        if m:
            n = len(m.group(1))
            out.append("<h%d>%s</h%d>" % (n, _en_linea(m.group(2)), n))
            i += 1
            continue

        # Tabla: cabecera, separador de guiones y cuerpo.
        if linea.lstrip().startswith("|") and i + 1 < len(lineas) \
                and re.fullmatch(r"\|[\s:|-]+\|", lineas[i + 1].strip()):
            cab = _celdas(linea)
            i += 2
            cuerpo = []
            while i < len(lineas) and lineas[i].lstrip().startswith("|"):
                cuerpo.append(_celdas(lineas[i]))
                i += 1
            out.append("<table>")
            out.append("<thead><tr>%s</tr></thead>"
                       % "".join("<th>%s</th>" % _en_linea(c) for c in cab))
            out.append("<tbody>")
            for fila in cuerpo:
                fila = (fila + [""] * len(cab))[:len(cab)]
                out.append("<tr>%s</tr>" % "".join("<td>%s</td>" % _en_linea(c) for c in fila))
            out.append("</tbody></table>")
            continue

        # Cita. Las lineas se conservan con <br>: aplastarlas en un parrafo es el
        # defecto medido del conversor a Word, y se machaca justo lo que lleva pasos
        # numerados o huecos de respuesta.
        if linea.lstrip().startswith(">"):
            dentro = []
            while i < len(lineas) and lineas[i].lstrip().startswith(">"):
                dentro.append(re.sub(r"^\s*>\s?", "", lineas[i]))
                i += 1
            interno = _md_a_html("\n".join(dentro), None)
            out.append("<blockquote>%s</blockquote>" % interno)
            continue

        if re.match(r"^\s*[-*]\s+", linea):
            items = []
            while i < len(lineas) and re.match(r"^\s*[-*]\s+", lineas[i]):
                item = re.sub(r"^\s*[-*]\s+", "", lineas[i])
                i += 1
                # Continuacion indentada de la misma vinieta.
                while i < len(lineas) and lineas[i].startswith("  ") and lineas[i].strip() \
                        and not re.match(r"^\s*[-*]\s+", lineas[i]):
                    item += " " + lineas[i].strip()
                    i += 1
                items.append(item)
            out.append("<ul>%s</ul>" % "".join("<li>%s</li>" % _en_linea(x) for x in items))
            continue

        # Parrafo: hasta la linea en blanco o hasta que empiece otra cosa.
        parr = []
        while i < len(lineas) and lineas[i].strip() \
                and not lineas[i].lstrip().startswith(("|", ">", "#")) \
                and not re.match(r"^\s*[-*]\s+", lineas[i]):
            parr.append(lineas[i].strip())
            i += 1
        out.append("<p>%s</p>" % _en_linea(" ".join(parr)))

    cuerpo = "\n".join(out)
    if titulo is None:
        return cuerpo

    return """<!DOCTYPE html>
<html lang="es">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>%s</title>
<style>
  :root { color-scheme: light; }
  body { margin: 0 auto; padding: 2rem 1.25rem 4rem; max-width: 52rem;
         font: 16px/1.6 -apple-system, "Segoe UI", Roboto, Helvetica, Arial, sans-serif;
         color: #1a1a1a; background: #fdfdfc; }
  h1 { font-size: 1.7rem; line-height: 1.25; margin: 2rem 0 .75rem; }
  h2 { font-size: 1.25rem; margin: 2.25rem 0 .5rem; padding-bottom: .3rem;
       border-bottom: 2px solid #e5e2dc; }
  h3 { font-size: 1.05rem; margin: 1.5rem 0 .4rem; }
  p, li { margin: .55rem 0; }
  code { font: .875em/1.4 "Cascadia Mono", Consolas, "SF Mono", monospace;
         background: #f0eeea; padding: .1em .35em; border-radius: 3px; }
  strong { font-weight: 650; }
  blockquote { margin: 1rem 0; padding: .6rem 1rem; border-left: 4px solid #c9c4bb;
               background: #f7f5f2; }
  blockquote p:first-child { margin-top: 0; }
  blockquote p:last-child { margin-bottom: 0; }
  table { border-collapse: collapse; width: 100%%; margin: 1rem 0; font-size: .92rem;
          display: block; overflow-x: auto; }
  th, td { border: 1px solid #ddd9d2; padding: .45rem .6rem; text-align: left;
           vertical-align: top; }
  th { background: #f0eeea; font-weight: 650; }
  tr:nth-child(even) td { background: #faf9f7; }
  @media print { body { max-width: none; padding: 0; } h2 { break-after: avoid; }
                 table { break-inside: avoid; } }
</style>
</head>
<body>
%s
</body>
</html>
""" % (html.escape(titulo), cuerpo)
