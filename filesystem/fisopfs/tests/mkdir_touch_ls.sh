#!/bin/bash


cd ../prueba

# Crear un directorio
mkdir dir1

# Crear un archivo
touch arch1

# Capturar la salida del comando ls
LS_OUTPUT=$(ls)

# Valores esperados
EXPECTED="dir1 arch1"

# Imprimir los valores esperados y obtenidos
echo "Expected: $EXPECTED"
echo "Got: $LS_OUTPUT"

# Verificar que los archivos y directorios se impriman correctamente
if [[ "$LS_OUTPUT" == *"dir1"* && "$LS_OUTPUT" == *"arch1"* ]]; then
    echo "Test passed: Directory and file are listed correctly."
else
    echo "Test failed: Directory and file are not listed correctly."
fi


cd ..

