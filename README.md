# Detector de Secuencias!!!

Programa desarrollado a finales de 2024 como herramienta para verificar los resultados de ejercicios de sistemas digitales de forma rápida y precisa.

## ⚙️ Funcionamiento
El programa entrega al usuario los resultados de los procesos principales en el diseño de un detector de secuencias, facilitando la resolución de este tipo de ejercicios.

### Datos de Entrada:
*   **Secuencia a detectar:** Ejemplo: `1011`.
*   **Modelo de máquina:** Opción entre **Mealy** o **Moore**.
*   **Traslape:** Configurable (Con traslape / Sin traslape).
*   **Tipo de Flip-Flop:** Soporta implementaciones con **D, T y JK**.

### Proceso que realiza:
1.  **Diagrama de estados:** Genera la lógica de transición.
2.  **Tablas de estado:** Construye la tabla según el modelo elegido.
3.  **Simplificación lógica:** Realiza la simplificación automática mediante **Mapas de Karnaugh**.
4.  **Ecuaciones resultantes:** Entrega las funciones lógicas finales para la implementación del circuito.

## 🛠️ Tecnologías
*   **Lenguaje:** C++
*   **Entorno:** Consola (Standard C++)
