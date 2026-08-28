# ===== banco/packs/costura_07_motivos_rechazo.py =====
#
# MOTIVOS DE RECHAZO: SE PUEDE ESCRIBIR UNA SOLA TABLA PARA EL MANUAL
#
# Si cada punta rechaza el Degradado con motivos distintos, el manual necesita dos
# tablas y el operario en el gabinete no sabe cual mirar. Se comprueba que la tabla
# sea UNA.

from banco.modelos.costura import *          # noqa: F401,F403

NOMBRE = "costura_07_motivos_rechazo"
DESCRIPCION = "una sola tabla de motivos para el manual"


def correr(b, fw):
    # Bloque traido LITERAL, solo reindentado.
    verificar = b.verificar

    def hallazgo(reproducido, titulo, detalle, consecuencia):
        """El hallazgo de costura lleva CUATRO argumentos y SI cuenta como
        comprobacion: aqui la comprobacion ES reproducir el desajuste, asi que si el
        modelo no lo reprodujera seria el modelo el que esta mal. En el validador del
        Esclavo la misma palabra significa otra cosa y NO cuenta -alli acompana a una
        propiedad() que ya cuenta por su cuenta-. Dos cosas distintas con el mismo
        nombre: por eso 37/41 y 30/31 no se podian sumar."""
        b.hallazgo(reproducido, titulo, [detalle, f"EN LA CALLE: {consecuencia}"])

    b.titulo("MOTIVOS DE RECHAZO: SE PUEDE ESCRIBIR UNA SOLA TABLA PARA EL MANUAL")


    m_enum = re.search(r"enum MotivoDegradado\s*\{(.*?)\}", T_M_DEG_H, re.S)
    e_enum = re.search(r"enum RechazoDegradado\s*\{(.*?)\}", T_E_DEG_H, re.S)
    m_motivos = [x for x in re.findall(r"(MDG_[A-Z_]+)", m_enum.group(1))] if m_enum else []
    e_motivos = [x for x in re.findall(r"(DEG_[A-Z_]+)", e_enum.group(1))] if e_enum else []
    m_causas = [x for x in m_motivos if x != "MDG_OK"]
    e_causas = [x for x in e_motivos if x != "DEG_ACEPTADO"]

    # El Maestro parte cada motivo en DOS funciones -L1 y L2-, asi que hay que
    # extraerlas por separado: metidas en un solo diccionario, la segunda pisa a la
    # primera y se acabaria comparando media tabla contra la otra entera.
    def cuerpo(fuente, firma):
        m = re.search(re.escape(firma) + r".*?\n\}", fuente, re.S)
        return m.group(0) if m else ""


    m_l1 = dict(re.findall(r"case\s+(MDG_[A-Z_]+):\s*return\s+\"([^\"]*)\";",
                           cuerpo(T_M_DEG_C, "const char* modo_degradado_motivoL1")))
    m_l2 = dict(re.findall(r"case\s+(MDG_[A-Z_]+):\s*return\s+\"([^\"]*)\";",
                           cuerpo(T_M_DEG_C, "const char* modo_degradado_motivoL2")))
    m_textos = {k: (m_l1.get(k, "") + " " + m_l2.get(k, "")).strip() for k in m_causas}
    e_textos_rechazo = dict(re.findall(r"case\s+(DEG_RECHAZO_[A-Z_]+):\s*return\s+\"([^\"]*)\";",
                                       cuerpo(T_E_DEG_C, "const char* degradado_textoRechazo")))


    def proporcion_mayusculas(textos):
        """Fraccion de letras en mayuscula. Se mide asi y no con v == v.upper()
        porque un solo caracter suelto -la 'h' de '>48h'- tumbaria la comprobacion
        sin que el formato haya cambiado en nada."""
        letras = [c for v in textos for c in v if c.isalpha()]
        return (sum(1 for c in letras if c.isupper()) / len(letras)) if letras else 0.0


    # Formato: el Maestro parte el texto en DOS lineas de <=20 caracteres y en
    # minusculas; el Esclavo lo da en UNA linea y en mayusculas.
    m_dos_lineas = bool(m_l1) and bool(m_l2)
    e_una_linea = bool(e_textos_rechazo) and "textoRechazoL2" not in T_E_DEG_C
    frac_e = proporcion_mayusculas(e_textos_rechazo.values())
    frac_m = proporcion_mayusculas(m_textos.values())
    e_mayusculas = frac_e > 0.9
    m_no_mayusculas = frac_m < 0.5

    # Causas que solo existen en una punta.
    solo_maestro = {"MDG_SYNC_VIEJA", "MDG_SIN_DESFASE", "MDG_DESFASE_ALTO"} & set(m_causas)
    solo_esclavo = {"DEG_RECHAZO_SIN_CONFIG", "DEG_RECHAZO_CICLO_NULO",
                    "DEG_RECHAZO_SYNC_VENCIDA"} & set(e_causas)

    hallazgo(len(m_causas) != len(e_causas) and bool(solo_maestro) and bool(solo_esclavo)
             and m_dos_lineas and e_una_linea and e_mayusculas and m_no_mayusculas,
             "los motivos de rechazo NO coinciden entre puntas: ni en conjunto ni en formato",
             f"cada punta tiene causas distintas (Maestro {len(m_causas)}, Esclavo {len(e_causas)}). "
             f"Solo el Maestro: {sorted(solo_maestro)}. Solo el Esclavo: {sorted(solo_esclavo)}. "
             f"Peor que la lista es el umbral: 'MDG_SYNC_VIEJA' del Maestro "
             f"salta a las {M_SYNC_FRESCA_MS/3600000:.0f} h y 'DEG_RECHAZO_SYNC_VENCIDA' del Esclavo "
             f"a las {E_LIMITE_MS/3600000:.0f} h; suenan igual y son cosas distintas. Formato: el "
             f"Maestro parte en dos lineas de 20 caracteres y {100*(1-frac_m):.0f}% en minusculas "
             f"('Falta: la ultima' / 'sync es muy vieja'); el Esclavo da una sola linea "
             f"{100*frac_e:.0f}% en mayusculas ('SYNC CADUCADA >48h').",
             "NO se puede escribir una sola tabla de rechazos para el manual de campo: el operario "
             "que baja del poste del Esclavo con 'FALTA CONFIG CICLO' no encontrara esa fila en la "
             "tabla del Maestro, y el que lee 'sync es muy vieja' en el Maestro creera que le sirve "
             "la fila 'SYNC CADUCADA >48h' del Esclavo, que habla de otro limite. Hacen falta DOS "
             "tablas, o una con la columna 'punta' y el umbral escrito en cada fila")

    # Lo que si esta bien: cada rechazo es distinguible, ninguno se solapa con otro.
    verificar(len(set(m_textos.values())) == len(m_textos) and
              len(set(e_textos_rechazo.values())) == len(e_textos_rechazo),
              "dentro de cada punta, cada motivo tiene un texto propio y distinguible: el operario "
              "sabe QUE le falta, no solo que algo falla",
              "hay motivos que comparten texto: tres averias distintas se convierten en una incognita")

    # Y que los dos limites de las 48 h que se muestran al operario salen del mismo
    # numero derivado, no de dos 48 escritos a mano.
    m_deriva_h = bool(re.search(r"LIMITE_DURO_H\s*=\s*LIMITE_DURO_MS\s*/\s*3600000UL", T_M_DEG_C))
    e_deriva_h = bool(re.search(r"LIMITE_SIN_SYNC_H\s*=\s*LIMITE_SIN_SYNC_MS\s*/\s*3600000UL", T_E_DEG_C))
    verificar(m_deriva_h and e_deriva_h,
              "en las dos puntas el limite en horas se DERIVA del de milisegundos: no hay dos 48 "
              "sueltos que puedan separarse el dia que alguien toque uno",
              "alguna punta escribe el limite en horas por separado: dos numeros que deben ser el "
              "mismo acaban siendo distintos")

    # ===========================================================================
    # VEREDICTO
