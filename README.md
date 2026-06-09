# Detector de Secuencias

Programa hecho a finales de 2024 para verificar ejercicios de sistemas digitales. Dada una secuencia, arma todo el diseño del detector y entrega las ecuaciones finales.

## Funcionamiento

### Datos de entrada
* **Secuencia a detectar:** ejemplo `1011`.
* **Modelo de máquina:** Mealy o Moore.
* **Traslape:** con o sin traslape.
* **Tipo de Flip-Flop:** D, T o JK.

### Proceso que realiza
1. **Diagrama de estados:** genera la lógica de transición.
2. **Tabla de estados:** la construye según el modelo elegido.
3. **Simplificación:** resuelve los Mapas de Karnaugh automáticamente.
4. **Ecuaciones finales:** arma las funciones lógicas listas para implementar el circuito.

### Salida
Todo se muestra en un menú de consola navegable, con cuatro vistas: el diagrama de estados, la tabla de estados completa (con las columnas de flip-flops y la salida Z), los mapas de Karnaugh ya resueltos y las ecuaciones finales. Las funciones salen en notación booleana, por ejemplo `D0: Q1 Q0* + E`, donde `*` indica negado, `E` es la entrada y `Z` la salida.

## Tecnologías
* **Lenguaje:** C++
* **Entorno:** consola (C++ estándar)

## Cómo resuelve el Karnaugh (para N variables)

La idea clave fue ver cada agrupación como un reflejo por un eje de simetría. Partiendo de eso, a cada celda le precalculo sus *reflejos*: con qué celdas se empareja al "doblar" el mapa por cada eje (horizontal y vertical). Y como los ejes crecen solos con el número de variables, la misma lógica sirve para cualquier N.

El flujo es:

1. Armo el mapa en **código Gray** y reparto las `(nFF + 1)` variables entre filas y columnas (`var_izq`, `var_der`).
2. A cada celda le **precalculo sus reflejos** por cada eje (`asignarReflejos`).
3. Para agrupar, arranco en un `1` sin marcar y voy saltando a un reflejo válido. Prioridad: `1` antes que `X`, y horizontal antes que vertical.
4. **Obligo a que el grupo sea potencia de 2.** Si al sumar una celda se rompe el rectángulo, retrocedo (`retrocederCelda`).
5. El término de cada grupo son los **bits que se mantienen constantes** en todas sus celdas; los que cambian se eliminan.

## Notas
* **Falta validación experta.** Funciona en todos los casos que probé, pero no está verificado formalmente, así que puede tener casos borde. Si alguien que domine bien el tema lo revisa, bienvenido.
* **El solver de Karnaugh saldrá en un repo aparte**, enfocado solo en eso. Este repo es el detector de secuencias (Mealy/Moore); resolver el Karnaugh es solo uno de sus pasos.
