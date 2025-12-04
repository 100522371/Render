#!/usr/bin/env python3
# Este script busca y ejecuta ejecutables de prueba unitaria (utcommon, utaos, utsoa)
# en una estructura de carpetas de compilación específica (out/build/<variant>/<...>/<mode>)).
# variant = clang-tidy o default 
# mode = debug o release
# Uso: /workspace/scripts/utest.py -v <variant> -m <mode>
import argparse
import os
import shutil
import subprocess
import sys

EXEC_NAMES = ["utcommon", "utaos", "utsoa"]

# Devuelve la ruta raíz del script (un nivel arriba de este archivo).
def script_root():
    return os.path.normpath(os.path.join(os.path.dirname(os.path.realpath(__file__)), ".."))

# Construye la ruta base "out/build/<variant>" donde buscamos binarios.
def out_base(variant):
    root = script_root()
    return os.path.join(root, "out", "build", variant)

# Normaliza el modo de compilación a "Debug"/"Release" esperado por la estructura.
def normalize_mode(mode):
    m = mode.lower()
    if m == "debug":
        return "Debug"
    if m == "release":
        return "Release"
    return mode  # allow passthrough if caller already provided proper case

# Comprueba si una ruta es un archivo ejecutable (existe y tiene permiso de ejecución).
def is_executable_file(p):
    return os.path.isfile(p) and os.access(p, os.X_OK)

# Busca el ejecutable dado en varias ubicaciones comunes dentro de out/build/<variant>
# y como último recurso en el PATH del sistema.
def find_executable(name, variant, mode):
    # Normalizamos el modo y preparamos la base donde buscar.
    mode = normalize_mode(mode)
    base = out_base(variant)

    # Patrones candidatos según layouts comunes de builds.
    candidates = [
        os.path.join(base, name, mode, name),
        os.path.join(base, name, mode, "bin", name),
        os.path.join(base, name, mode),                  # directory (maybe named same as bin)
        os.path.join(base, name, name),
        os.path.join(base, name),
        os.path.join(base, mode, name),
        os.path.join(base, mode, "bin", name),
        os.path.join(base, name, mode, name + ".exe"),
        os.path.join(base, mode, name + ".exe"),
    ]

    # Recorremos candidatos y devolvemos el primero que sea ejecutable.
    for p in candidates:
        if p and is_executable_file(p):
            return p

    # Búsqueda recursiva poco profunda dentro de la carpeta base.
    if os.path.isdir(base):
        for root_dir, dirs, files in os.walk(base):
            # computa la profundidad relativa a base
            rel = os.path.relpath(root_dir, base)
            depth = 0 if rel == "." else rel.count(os.sep) + 1
            if name in files:
                p = os.path.join(root_dir, name)
                if is_executable_file(p):
                    return p
            # Evita escanear demasiado profundo para ahorrar tiempo.
            if depth >= 3:
                dirs.clear()

    # Fallback: buscar en PATH del sistema.
    which = shutil.which(name)
    if which:
        return which

    # Si no se encontró, devolvemos None.
    return None

# Ejecuta el binario y captura su salida, mostrando encabezado informativo.
def run_exec(path):
    print(f"--- Running: {path} ---")
    try:
        proc = subprocess.run([path], stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
    except Exception as e:
        print(f"ERROR launching {path}: {e}")
        return 2, ""
    print(proc.stdout)
    return proc.returncode, proc.stdout

# Punto de entrada: parsea argumentos, busca los ejecutables, los ejecuta y gestiona errores.
def main():
    parser = argparse.ArgumentParser(
        description="Run unit test executables utcommon, utaos and utsoa (search only in out/build/<variant>/<...>/<mode>)"
    )
    parser.add_argument("--variant", "-v", required=True, choices=["clang-tidy", "default"],
                        help="Which out/build variant to search: clang-tidy or default (required)")
    parser.add_argument("--mode", "-m", required=True, choices=["debug", "release"],
                        help="Build mode: debug or release (required)")
    args = parser.parse_args()

    missing = []
    failures = {}

    # Para cada ejecutable, intentar localizar y ejecutar.
    for name in EXEC_NAMES:
        exe = find_executable(name, args.variant, args.mode)
        if not exe:
            missing.append(name)
            print(f"Not found: {name} (looked in out/build/{args.variant} and subpaths for mode {args.mode}, then PATH)")
            continue
        code, _ = run_exec(exe)
        if code != 0:
            failures[name] = code

    # Mostrar resumen de faltantes y fallos.
    if missing:
        print(f"Missing executables: {', '.join(missing)}")
    if failures:
        print("Failures:")
        for n, c in failures.items():
            print(f"  {n}: exit {c}")

    # Salida con código no cero si hubo problemas.
    if missing or failures:
        sys.exit(1)

    print("All tests ran and exited 0.")
    sys.exit(0)

if __name__ == "__main__":
    main()
