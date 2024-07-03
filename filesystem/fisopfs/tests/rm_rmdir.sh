#!/bin/bash


cd ../prueba

# Crear un directorio
mkdir dir1

# Crear un archivo
touch arch1

rm arch1
rmdir dir1

LS_OUTPUT=$(ls)

EXPECTED=""

# Imprimir los valores esperados y obtenidos
echo "Expected: $EXPECTED"
echo "Got: $LS_OUTPUT"

cd ..

