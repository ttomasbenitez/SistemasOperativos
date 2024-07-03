# fisop-fs

Documentación del TP3: Filesystem - Grupo 8 [Benítez, Brocca, Villa Jiménez]

## Estructura y acceso

Para la representación en memoria del sistema de archivos, definimos un `struct block` que consta de los siguientes elementos:
- `enum block_type type`: el tipo de bloque, según el `enum block_type` que puede tomar los valores `EMPTY`, `FILE_TYPE` y `DIR_TYPE`
- `int in_use`: indica si el bloque está ocupado
- `mode_t mode`: modo de protección según elemento `st_mode` de stat(2) (asumimos que el usuario tiene permisos de lectura y escritura)
- `char name[]`: nombre del bloque, obtenido del último componente del path sin incluir "/". La excepción es el directorio raíz, cuyo nombre es "/"
- `char parent_path[]`: path del bloque padre, terminado en "/". Para el directorio raíz, lo representamos como un string vacío
- `char data[]`: contenido del bloque, si se trata de un archivo
- `uid_t uid`: user ID del owner según elemento `st_uid` de stat(2) (se asume el del usuario actual)
- `gid_t gid`: group ID del owner según elemento `st_gid` de stat(2) (se asume el del usuario actual)
- `time_t access_time`: fecha y hora de último acceso según elemento `st_atime` de stat(2)
- `time_t modification_time`: fecha y hora de la última modificación según elemento `st_mtime` de stat(2)

Para evitar redundancia, el tamaño del archivo está representado implícitamente por la longitud del elemento `data`, y cada función que hace uso del mismo lo hace mediante `strlen()`.

A partir de esta estructura de bloques, representamos la totalidad del sistema de archivos como un array de `struct block` llamado `filesystem`.

Durante la inicialización, en caso de no leer un archivo persistido en disco, reservamos el primer bloque libre del array para el directorio raíz, y seguido de eso inicializamos los demás bloques como libres/vacíos y con valores por defecto.

Definimos valores máximos para la longitud de los paths, la longitud los nombres de bloques, el tamaño del contenido de cada bloque y la cantidad total de bloques, validando que estos no se superen.

Incluimos funciones auxiliares en `fs.c`, que asisten a las operaciones de FUSE en la creación, liberación, búsqueda y modificación de bloques.

La forma que implementamos de obtener un bloque a partir de un path es tomar dos partes separadas del path según la última ocurrencia de "/" en este, y recorrer `filesystem` hasta hallar el primer bloque que cumpla que su `parent_path` y `name` son respectivamente iguales a ambas partes del path. Se podría implementar de forma menos costosa, pero al ser una implementación reducida la diferencia de performance no es muy grande.

## Persistencia

Implementamos la persistencia de datos en disco grabando en el directorio actual un archivo con extensión ".fisopfs" (por defecto "fs_data.fisopfs") cada vez que se desmonta el sistema de archivos o ocurre un flush. En la siguiente inicialización, el filesystem intenta levantar el archivo guardado. En caso de no existir, inicializa los bloques por defecto como se describió anteriormente.

## Pruebas
Incorporamos pruebas sobre las distintas funcionalidades en forma de scripts de bash en la carpeta `tests`. A continuación se muestran sus salidas:

#### ls_parentdir.sh
![ls_parentdir](/fisopfs/images/ls_parentdir.png)

#### mkdir_touch_ls.sh
![ls_parentdir](/fisopfs/images/mkdir_touch_ls.png)

#### rm_rmdir.sh
![ls_parentdir](/fisopfs/images/rm_rmdir.png)

#### stat.sh
![ls_parentdir](/fisopfs/images/stat.png)

#### touch_inside_subdir.sh
![ls_parentdir](/fisopfs/images/touch_inside_subdir.png)

#### write_append_cat.sh
![ls_parentdir](/fisopfs/images/write_append_cat.png)

## Comentarios

Sólo logramos que funcione la persistencia de datos invocando al filesystem con la opción -f.

No implementamos flags de apertura de archivo, se asume que todos los archivos se abren para lectura y escritura.

Las pruebas no resetean los datos persistidos, es necesario vaciar el directorio raíz antes de ejecutar cada prueba para obtener los resultados esperados.

