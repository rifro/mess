import os
import re
from pathlib import Path

# --- CONFIGURATIE ---
TARGET_DIR = './rdv'
SOURCE_EXT = ('.cpp', '.h', '.cu', '.cuh', '.hpp')

# Anti-patterns (RegEx)
ANTIPATTERN_DUBBELE_PREFIX = re.compile(r'\b(m_m|g_g|d_d|h_h|k_k|s_s|g_m|m_g)\w+', re.IGNORECASE)
ANTIPATTERN_TYPE_LEAK = re.compile(r'\b\w+(U32|I32|F32|U64|I64|F64|float33|u32)\w+\b')
ANTIPATTERN_VERMINKT_KEYWORD = re.compile(r'\b(StaticCas|ReinterpretCas|ConstCas|DynamicCas)\b', re.IGNORECASE)
ANTIPATTERN_HOOFDLETTER_SOEP = re.compile(r'\b[a-z]+[A-Z]{2,}[a-z]*\b') # bijv. calculateNORmals
ANTIPATTERN_DUBBELE_UNDERSCORE = re.compile(r'\w+__\w+')

def check_line(line, line_num, file_path):
    # Strip comments en strings voor de check
    clean_line = re.sub(r'//.*$|/\*.*?\*/|".*?"|\'.*?\'', '', line)
    
    issues = []
    
    if ANTIPATTERN_DUBBELE_PREFIX.search(clean_line):
        issues.append("DUBBELE PREFIX (bijv. m_m)")
    if ANTIPATTERN_TYPE_LEAK.search(clean_line):
        issues.append("TYPE LEAK (bijv. U32 midden in naam)")
    if ANTIPATTERN_VERMINKT_KEYWORD.search(clean_line):
        issues.append("VERMINKT KEYWORD (bijv. StaticCas)")
    if ANTIPATTERN_HOOFDLETTER_SOEP.search(clean_line):
        # We negeren bekende afkortingen zoals PCA, CUDA, RDV
        if not any(x in clean_line for x in ["PCA", "CUDA", "RDV", "GPU", "CPU"]):
            issues.append("HOOFDLETTER SOEP (onnatuurlijk CamelCase)")
    if ANTIPATTERN_DUBBELE_UNDERSCORE.search(clean_line):
        issues.append("DUBBELE UNDERSCORE")

    if issues:
        print(f"[{file_path}:{line_num}]")
        print(f"  > Code: {line.strip()}")
        print(f"  > Probleem: {', '.join(issues)}\n")
        return 1
    return 0

def main():
    print(f"=== Bagger-Check scan gestart in {TARGET_DIR} ===\n")
    
    total_issues = 0
    file_count = 0

    if not os.path.exists(TARGET_DIR):
        print(f"Fout: Map {TARGET_DIR} niet gevonden.")
        return

    for root, _, files in os.walk(TARGET_DIR):
        for f in files:
            if f.endswith(SOURCE_EXT):
                file_count += 1
                p = Path(root) / f
                try:
                    with open(p, 'r', encoding='utf-8', errors='ignore') as src:
                        for idx, line in enumerate(src, 1):
                            total_issues += check_line(line, idx, p)
                except Exception as e:
                    print(f"Kon {p} niet lezen: {e}")

    print(f"Scan voltooid. {file_count} bestanden gecontroleerd.")
    print(f"Totaal aantal verdachte regels gevonden: {total_issues}")

if __name__ == "__main__":
    main()
