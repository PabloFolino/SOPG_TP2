# SOPG_TP2

| Versión | Descripción | Cumplido |
| :-: | :-: | :-: |
[1.0] | Programa que se comunica con el ttyUSB1 y hace un loop. Si se presiona una tecla se envía a la PC “>SW:X,Y\r\n” Siendo X el número de salida (0,1, o 2) e Y el nuevo estado a setear (0 o 1), y la PC contesta “>OUT:X,Y\r\n” Siendo X el número de salida (0,1, o 2) e Y el estado (0 o 1). El programa contempla señales SIGINT-SIGTERM.| ✔ |
[1.1] | Posee un hilo principal que constantemente lee el RS232(cada 100useg), y si recibe un dato lo envía al socket. Se agrega un hilo(start_tcp) para procesar la función accept. Al procesar SIGINT y/o SIGTERM se cierran el puerto serie, el socket, y el hilo secundario. | ✔ |
[1.2] | Transmite y recibe en forma correcta. Faltan corregir problemas al finalizar el programa.  | ✔ |
[1.3] | Futura versión |   |
