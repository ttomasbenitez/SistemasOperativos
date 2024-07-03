#!/bin/bash

cd ../prueba

# Crear un directorio
mkdir dir1

# Crear un archivo
touch arch1

# Escribir en el archivo arch1
echo "a" > arch1
echo "b" >> arch1

# Capturar la salida del comando cat
CAT_OUTPUT=$(cat arch1)

# Valores esperados para el contenido de arch1
EXPECTED_CONTENT="a
b"

# Imprimir los valores esperados y obtenidos del contenido de arch1
echo "Expected content: $EXPECTED_CONTENT"
echo "Got content: $CAT_OUTPUT"

# Verificar que el contenido del archivo sea el esperado
if [[ "$CAT_OUTPUT" == "$EXPECTED_CONTENT" ]]; then
    echo "Test passed: File content is correct."
else
    echo "Test failed: File content is incorrect."
fi

cd ..


