# shell

### Búsqueda en $PATH

- **Responder**:
	- ¿Cuáles son las diferencias entre la syscall execve(2) y la familia de wrappers proporcionados por la librería estándar de C (libc) exec(3)?

Las diferencias principales son:

- Interfaz: exec(3) proporciona una interfaz mas amigable que execve(2). Ademas esta compuesto por un conjunto de funciones, los cuales son wrappers de execve(2)

- Argumentos y variables: execve(2) requiere que se pasen explicitamente como array de punteros a cadenas. En cambio, exec(3) permite especificar las variables y argumentos de una manera mas sencilla y no tan rigida

- Ejecucion: execve(2) son llamadas al sistema mas a bajo nivel, que ejecutan un nuevo programa en el espacio de memoria del proceso actual. En cambio exec(3) son funciones mas a alto nivel, ya que realizan algunas operaciones de preparacion antes de invocar execve(2). Por ejemplo, manejan la conversión de argumentos de C a un array de punteros a cadenas. Lo cual es requerido al usar execve(2)

- Retorno en caso de error: execve(2) en caso de error devuelve -1 y el error especifico puede consutarse a traves de "errno". En cambio, las funciones exec(3) pueden devolver -1 o pueden no retornar en caso de éxito.

Resumiendo execve(2) es la llamada subyacente que se le hace al sistema, para la ejecucion real del programa. En cambio, exec(3) son una familia de funciones que wrappean y simplifican el uso de execve(2), proporcionando una interfaz mas conveniente para el usuario.

- **Responder**:
	- ¿Puede la llamada a exec(3) fallar? ¿Cómo se comporta la implementación de la shell en ese caso?

Efectivamente la llamada a exec(3), si puede fallar. En tal caso el proceso en ejecucion se termina y sale por error.
Por ejemplo, si ponemos un comando inexistente como "no-existe" en la shell, al momento de ejecutarse execve, el proceso sale por error, mostrando en la shell lo siguiente

Failed to execute execvp.: No such file or directory
	Program: [no-existe] exited, status: 255 

Es decir, la ejecucion sale por status 255, ya que encontro un error durante la ejecucion, en este caso se ingreso un comando inexistente.

---

### Procesos en segundo plano

- **Detallar cuál es el mecanismo utilizado para implementar procesos en segundo plano.**

Para implementar en esta etapa "segundo plano", utilizamos el mecanismo de "Handling Multiple Background Processes"
el cual consiste en usar la syscall waitpid(), junto con el flag WNOHANG
para que el proceso padre, espere de manera no bloqueante a que el proceso hijo termine. Asi de esta manera el usuario puede seguir usando de manera normal la shell.

---

### Flujo estándar

- **Investigar el significado de 2>&1, explicar cómo funciona su forma general.**
- **Mostrar qué sucede con la salida de cat out.txt en el ejemplo.**
- **Luego repetirlo, invirtiendo el orden de las redirecciones (es decir, 2>&1 >out.txt).**
	-  ¿Cambió algo? Compararlo con el comportamiento en bash(1).

En general, la forma 2>&1 redirecciona el flujo de manera que la salida de error estándar (stderr) apunte a la misma descripción de archivo a la que apunta la salida estándar (stdout). Esto significa que, si se redirecciona stdout a un archivo distinto de la pantalla, utilizar 2>&1 redireccionará stderr a dicho archivo.

![Flujo estándar: ejemplo 1](/shell/images/2a_1.png)

Podemos ver que tanto la salida estándar como el error de ls se grabaron en out.txt. Vemos qué ocurre cambiando el orden de los comandos:

![Flujo estándar: ejemplo 2](/shell/images/2a_2.png)

En nuestro caso, al invertir el orden de redirecciones el resultado fue el mismo. Esto difiere del comportamiento de bash, donde la misma inversión provocaría que sólo se redireccione la salida estándar a out.txt, ya que la redirección de stderr al "apuntado" por stdout ocurre antes que la redirección al archivo. La razón de esto es que, por cómo está estructurada nuestra shell, no tenemos la información del orden de redirecciones, mientras que bash ejecuta las redirecciones según aparecen en el comando de izquierda a derecha. Nuestra implementación en particular realiza la redirección de stdout antes que la de stderr sin importar cuál está a la izquierda.

---

### Tuberías múltiples

- **Investigar qué ocurre con el exit code reportado por la shell si se ejecuta un pipe.**
	- ¿Cambia en algo?

Según bash(1), en bash el exit code de un pipeline equivale al exit code del último comando del mismo. Sin embargo, en nuestra shell, si bien devolvemos recursivamente el exit code del comando derecho, evaluar la variable $? luego de ejecutar un pipe devuelve siempre 0.

![Tuberías múltiples: ejemplo 1](/shell/images/2b_1.png)

- **¿Qué ocurre si, en un pipe, alguno de los comandos falla? Mostrar evidencia (e.g. salidas de terminal) de este comportamiento usando bash. Comparar con su implementación.**

Si alguno de los comandos falla en un pipeline, se imprimen por pantalla las salidas correspondientes de stderr (y, si hubiera, la de stdout del último comando), ya que no implementamos redirección de stderr. No obstante, el orden en que se imprime cada salida no es determinístico porque depende del orden de ejecución de los procesos luego de llamar a fork(2).

Comportamiento en nuestra shell:

![Tuberías múltiples: ejemplo 2](/shell/images/2b_2.png)


Comportamiento en bash:

![Tuberías múltiples: ejemplo 3](/shell/images/2b_3.png)

---

### Variables de entorno temporarias

- **Responder:**
	- ¿Por qué es necesario hacerlo luego de la llamada a fork(2)?

Es necesario hacerlo luego de la llamada a fork(2) porque al ser temporales, buscamos que sean relativas al proceso hijo. Si se hiciese antes del fork(2) serían relativas al proceso padre, y en este caso, a todo el programa. 

- **Responder:**
	-  En algunos de los wrappers de la familia de funciones de exec(3) (las que finalizan con la letra e), se les puede pasar un tercer argumento (o una lista de argumentos dependiendo del caso), con nuevas variables de entorno para la ejecución de ese proceso. Supongamos, entonces, que en vez de utilizar setenv(3) por cada una de las variables, se guardan en un arreglo y se lo coloca en el tercer argumento de una de las funciones de exec(3).
		- ¿El comportamiento resultante es el mismo que en el primer caso? Explicar qué sucede y por qué.
		- Describir brevemente (sin implementar) una posible implementación para que el comportamiento sea el mismo.

El comportamiento resultante no sería el mismo porque al pasar un tercer argumento (o una lista de argumentos dependiendo del caso), con nuevas variables de entorno para la ejecución de ese proceso,  se está reemplazando por completo el entorno del proceso hijo con las variables proporcionadas en el arreglo. Esto significa que cualquier variable de entorno previamente definida sería inexitente y solo se tendrían disponibles las variables presentes en el arreglo.
Para que el comportamiento sea el mismo se podría enviar en el tercer argumento, junto con las nuevas variables, las variables previas que se encuentran en 'environ'.


---

### Pseudo-variables

- **Investigar al menos otras tres variables mágicas estándar, y describir su propósito.**
	- Incluir un ejemplo de su uso en bash (u otra terminal similar).

Definiendo:

	#!/bin/bash

	# Script name: ejemplo.sh

	# $0: Filename del script actual.
	echo "Nombre: $0"

	# $n: Argumentos pasados al script.
	echo "Primer argumento: $1"
	echo "Segundo argumento: $2"

	# $#: Cantidad de argumentos pasados.
	echo "Cantidad: $#"

	# $$: Número de proceso de la shell.
	echo "Shell PID: $$"

	# $$/$@ Todos los argumentos tomando por elementos o la linea entera.
	echo "usando @:"
	for arg in "$@"; do
	    echo "$arg"
	done

	echo "usando *:"
	for arg in "$*"; do
	    echo "$arg"
	done

	# Proceso en segundo plano para demostrar
	sleep 10 &
	# $!: PID del último proceso en segundo plano.
	echo "PID background: $!"


Al ejecutar:

	./ejemplo.sh Hola como estas

Se obtiene lo siguiente:

	Nombre: ./ejemplo.sh
	Primer argumento: Hola
	Segundo argumento: como
	Cantidad: 3
	Shell PID: 19515
	usando @:
	Hola
	como
	estas
	usando *:
	Hola como estas
	PID background: 19516

---

### Comandos built-in

- **Responder:** ¿Entre cd y pwd, alguno de los dos se podría implementar sin necesidad de ser built-in? ¿Por qué? ¿Si la respuesta es sí, cuál es el motivo, entonces, de hacerlo como built-in? (para esta última pregunta pensar en los built-in como true y false)

Pwd tranquilamente podria implementarse como un programa independiente (es decir que lo ejecute otro proceso y no la shell), porque se podria usar la syscall getcwd() para lograr la misma funcionalidad que cumple pwd.
El motivo por el cual pwd se lo hace como built-in es por un tema de eficiencia. Es decir, al ser built-in puede acceder directamente al estado interno de la shell para obtener el directorio actual sin necesidad de iniciar un nuevo proceso, lo cual es más rápido y eficiente en comparación con invocar un programa externo.

---

### Segundo plano avanzado

- **Explicar detalladamente el mecanismo completo utilizado.**

Utilizando sigaltstack(2) y sigaction(2), seteamos un handler exclusivo para la señal SIGCHLD que cambia el comportamiento por defecto frente a dicha señal (Ign) para hacer que se escriba a stdout la información del proceso hijo terminado. De esta manera, logramos que se notifique por pantalla la terminación de un proceso en segundo plano en el momento en que ocurre, y además se liberen los recursos del proceso finalizado, para lo que fue necesario transferir al handler la responsabilidad de ejecutar waitpid(2) y "cosechar" a los procesos background que se encontraba en run_cmd().

Para evitar que el handler ejecute waitpid() sobre los procesos foreground, utilizamos setpgid(2) para cambiar el process group ID (PGID) de todos los comandos que no sean del tipo BACK, y setearlo para que sea igual a su propio PID. Según los estándares de POSIX, esto significa que cada proceso que se ejecuta en primer plano se transforma en el líder de su nuevo process group y tiene un PGID distinto al de la shell. Al mismo tiempo, la llamada a waitpid(2) dentro del handler tiene como primer argumento 0, lo que le indica al kernel que notifique únicamente los cambios de estado de los procesos hijos que compartan el PGID del proceso llamador. De esta forma nos aseguramos de que siempre que se intercepte SIGCHLD y se "coseche" al proceso finalizado, la señal provenga de un proceso en segundo plano, pues es el único caso donde el mantenemos el PGID igual entre padre e hijo luego de llamar a fork(2). En el caso de los pipelines, donde llamamos dos veces a fork(2) por cada pipe, también cambiamos el PGID de cada hijo para que el handler no lo capture.

Las dos flags que usamos en sigaction(2) son fundamentales: SA_ONSTACK le indica al kernel que instale la stack alternativa que creamos para el manejo de señales y la use en lugar de la actual, lo que es útil para evitar el agotamiento de la pila, mientras que SA_RESTART permite reanudar la ejecución de una syscall reentrante (e.g. waitpid(2), read(2), etc.) en caso de haber sido interrumpida por la llamada al handler, en lugar de que ésta lance un error del tipo EINTR.

La propia función handler también debe ser async-signal-safe, y en particular reentrante. Según signal-safety(7), esto implica que todas las funciones que se llamen dentro del handler deben ser a su vez async-signal-safe, lo cual es especialmente engorroso luego de darse cuenta de que ninguna función de la familia de printf(3) es asincrónicamente segura y que debemos imprimir por pantalla números (PID y estado del proceso). Nuestra solución fue usar write(2) en lugar de printf(3) y armar manualmente el buffer de escritura utilizando la función itoa_safe() (Santilli, C (2018). https://stackoverflow.com/questions/14573000/print-int-from-signal-handler-using-write-or-async-safe-functions/52111436#52111436) que realiza la conversión de entero a string sin alocar memoria estáticamente para los buffers utilizados. Esto elimina el riesgo de operar con datos inconsistentes en caso de que se modificaran los punteros en subsecuentes llamadas al handler.

- **Responder:**
	- ¿Por qué es necesario el uso de señales?

El uso de señales es necesario en este caso porque nuestra shell funciona con un ciclo que itera cada vez que se lee una línea de la terminal, lo que significa que si queremos notificar la información del proceso sin usar un handler no tenemos otra opción que hacerlo de forma sincrónica y oportunística, evaluando e imprimiendo los datos en un momento específico de cada iteración. El handler permite que la evaluación se realice asincrónicamente, interrumpiendo el proceso que se esté ejecutando en el momento para ejecutar el comportamiento deseado. Las señales también son importantes porque sirven para que los procesos se comuniquen entre sí, y no estamos limitados a evaluar el estado de un hijo en un proceso específico ya que la información de señales recibidas es compartida por todos los procesos.

---

### Historial

(Aún no se resolvieron los desafíos)

---
