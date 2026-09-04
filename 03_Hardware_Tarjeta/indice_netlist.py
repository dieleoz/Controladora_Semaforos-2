import re, sys, json, collections
sys.stdout.reconfigure(encoding='utf-8')
P = '01_Firmware/Controladora_Semaforos/Controladora_Semaforos/Controladora_Semaforos.kicad_pcb'
s = open(P, encoding='utf-8', errors='ignore').read()

ESC = chr(92)   # backslash, sin escribirlo en un literal

def bloques(txt, tok):
    """Bloques (tok ...) con parentesis balanceados, saltando cadenas."""
    out = []
    i = 0
    pat = '(' + tok
    while True:
        i = txt.find(pat, i)
        if i < 0:
            break
        d = 0
        j = i
        while j < len(txt):
            c = txt[j]
            if c == '"':
                j += 1
                while j < len(txt) and txt[j] != '"':
                    j += 2 if txt[j] == ESC else 1
            elif c == '(':
                d += 1
            elif c == ')':
                d -= 1
                if d == 0:
                    break
            j += 1
        out.append(txt[i:j + 1])
        i = j + 1
    return out

fps = bloques(s, 'footprint')
red = collections.defaultdict(list)   # net -> [(ref, pad)]
comp = {}                             # ref -> value

for fp in fps:
    r = re.search(r'\(property\s+"Reference"\s+"([^"]*)"', fp)
    v = re.search(r'\(property\s+"Value"\s+"([^"]*)"', fp)
    ref = r.group(1) if r else '?'
    comp[ref] = v.group(1) if v else ''
    for pad in bloques(fp, 'pad'):
        pn = re.match(r'\(pad\s+"([^"]*)"', pad)
        nt = re.search(r'\(net\s+(\d+)\s+"([^"]*)"\)', pad)
        if pn and nt:
            red[nt.group(2)].append((ref, pn.group(1)))

print("footprints: %d   nets con pad: %d" % (len(fps), len(red)))
json.dump({'red': dict(red), 'comp': comp}, open(sys.argv[1], 'w', encoding='utf-8'))
