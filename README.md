# SOPG_TP2

| Versión | Descripción | Cumplido |
| :-: | :-: | :-: |
[1.0] | Programa que se comunica con el ttyUSB1 y hace un loop. Si se presiona una tecla se envía a la PC “>SW:X,Y\r\n” Siendo X el número de salida (0,1, o 2) e Y el nuevo estado a setear (0 o 1), y la PC contesta “>OUT:X,Y\r\n” Siendo X el número de salida (0,1, o 2) e Y el estado (0 o 1). El programa contempla señales SIGINT-SIGTERM.| ✔ |
[1.1] | Posee un hilo principal que constantemente lee el RS232(cada 100useg), y si recibe un dato lo envía al socket. Se agrega un hilo(start_tcp) para procesar la función accept. Al procesar SIGINT y/o SIGTERM se cierran el puerto serie, el socket, y el hilo secundario. | ✔ |
[1.2] | Transmite y recibe en forma correcta. Faltan corregir problemas al finalizar el programa.  | ✔ |
[1.3] | Se corrigió errores de reconexión. | ✔ |
[1.5] | Se corrigen errores | ✔ |
[1.5] | Futura versión |   |

# Correcciones de la Versión 1.4 (devolución de profesor Ernesto Gigliotti )

| N° | Descripción del error | Corrección |  Cumplido |
| :-: | :-: | :-: | :-: |
| 1) | No entendi porque haces un read/send (linea 95 a 103) fuera del bucle y luego repetis ese codigo dentro del bucle. Podrias resolverlo para que ese codigo no se repita. | Se reemplaza el bucle while por el do-while() | ✔ |
| 2) | Ojo que usas el newfd en el bucle ppal inclusive cuando no hay una conexion establecida. deberias sincronizar esto con lo que pasa en el thread, podes usar newfd como flag para saber si hay una conexion valida, deberias protegerlo con un mutex cuando lo modificas, y cuando lo lees en ambos threads. | Se usa el newfd como flag. Se protege con mutex en los hilo principal y secundario | ✔ |
| 3) | Al detectar el cierre, deberias usar pthread_cancel para detener el thread, luego cerras los Fds y luego el puerto serie y luego terminas. Lo ideal es que el programa termine por un solo camino, por la salida del main.| Al salir se cierra el hilo secundario con pthread_cancel(), y luedo los demás fd's. | ✔ |
| 4) | Tene en cuenta que la signal puede caer cuando estas bloqueado en el accept tambien!. en ese caso accept se interrumpe y sale con error.deberias detectarlo para hacer que el programa termine por el mismo lugar que en los otros casos. | cuando se produce un error en el accept del hilo secundario se setea el flag de fin de programa. | ✔ |
