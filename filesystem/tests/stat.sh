#!/bin/bash


cd ../prueba

# Crear un directorio
mkdir dir1

# Crear un archivo
touch arch1

# Verificar permisos 

FILE_PERMS=$(stat -c%A arch1)
EXPECTED_PERMS="-rw-r--r--"
if [[ "$FILE_PERMS" == "$EXPECTED_PERMS" ]]; then
    echo "Test passed: File permissions are correct."
else
    echo "Test failed: File permissions are incorrect. Expected $EXPECTED_PERMS, got $FILE_PERMS."
fi


DIR_PERMS=$(stat -c%A dir1)
EXPECTED_PERMS="drwxr-xr-x"
if [[ "$DIR_PERMS" == "$EXPECTED_PERMS" ]]; then
    echo "Test passed: Directory permissions are correct."
else
    echo "Test failed: Directory permissions are incorrect. Expected $EXPECTED_PERMS, got $DIR_PERMS."
fi


cd ..

