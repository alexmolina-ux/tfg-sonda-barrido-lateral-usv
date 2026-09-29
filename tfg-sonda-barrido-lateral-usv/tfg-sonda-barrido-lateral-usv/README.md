# tfg-sonda-barrido-lateral-usv

Código fuente del Trabajo de Fin de Grado **«Integración y optimización de un sistema de sonda de barrido lateral en USV con montaje móvil para mejorar la calidad de adquisición»** (Grado en Ciencias y Tecnologías del Mar, Universitat Politècnica de Catalunya, convocatoria de octubre de 2026).

*Source code of a BSc thesis (UPC, 2026): firmware of an ESP32 tow-fish sensor node, a Python UDP receiver and the Unity scripts of its digital twin.*

- **Autor:** Àlex Molina López
- **Dirección:** Daniel Mihai Toma y Marc Nogueras
- **Versión asociada a la memoria:** etiqueta [`v1.0`](https://github.com/alexmolina-ux/tfg-sonda-barrido-lateral-usv/tree/v1.0). Contiene los códigos de los Anexos A y B de la memoria y los programas de banco de su Tabla 11; no incluye `CODIGOFINAL.ino` ni la versión original de `PRUEBA_IMU_TORPEDO.ino`, que la memoria no transcribe.

El nodo del cuerpo remolcado (*tow-fish*) combina un WT32-ETH01 (ESP32 con Ethernet), una unidad inercial MPU-6050 y un sensor de presión MS5837-30BA (Bar30) en un bus I2C compartido (SDA GPIO 4, SCL GPIO 14). Envía por UDP una trama de texto `roll,pitch,yaw,profundidad,temperatura` a 20 Hz de diseño, desde 192.168.2.50 hasta el ordenador del operador (192.168.2.1, puerto 8080), en la red del USV BlueBoat. En los ensayos de campo, con `PRUEBA_IMU_TORPEDO`, la cadencia medida fue de 19,23 Hz y la registró `receptor.py`. El Gemelo Digital en Unity escucha en el mismo puerto; su recepción se ha validado con emisores virtuales, y con el nodo físico solo por diseño y por inspección del código.

## Estructura

```
firmware/                 Programas Arduino del nodo (Anexo A y Tabla 11 de la memoria)
  CODIGOFINAL_v2/           versión definitiva propuesta (Fase 18)
  PRUEBA_IMU_TORPEDO/       firmware desplegado en piscina y mar (Fases 12 y 14 a 17), versión de la Fase 16
  banco/                    versiones de banco de las Fases 01 a 11
python/                   Receptor de campo y emisores virtuales (Anexo A)
unity/SCRIPTS/            19 scripts C# del Gemelo Digital (Anexo B)
```

Cada programa de Arduino está en una carpeta con su mismo nombre, para que se abra directamente en el Arduino IDE. Los archivos conservan el nombre y el contenido con que se citan en la memoria.

### firmware/

| Programa | Fases | Descripción |
|---|---|---|
| `CODIGOFINAL_v2` | 18 | Trama de cinco campos reales, con tara de presión (media de diez lecturas), densidad seleccionable (`DENSIDAD_FLUIDO`: 997 kg/m³ en banco y piscina, 1025 kg/m³ en el mar) y cadencia compensada. Versión definitiva propuesta, verificada por revisión de diseño (Fase 18) y pendiente de validación experimental: no se ha compilado ni ejecutado en ningún ensayo documentado. |
| `PRUEBA_IMU_TORPEDO` | 12 y 14 a 17 | Único firmware desplegado en agua. Trama de cinco campos con la profundidad y la temperatura fijas a `0.00`, porque no lee el Bar30. Se incluye la versión conservada con los ejes corregidos (Fase 16: pitch = `getAngleX()`, roll = `getAngleY()`); la versión original invertía roll y pitch y no se incluye, y no consta cuál se cargó en las Fases 14, 15 y 17. |
| `banco/FASE1codigo_COMPROBACIONIMU_CON_ESP32_ENTRADA_TIPO_C` | 01 | Lectura en bruto del MPU-6050 con el ESP32 de entrada USB-C (GPIO 21/22); puerto serie; 10 Hz. |
| `banco/FASE2codigo_COMPROBACIONIMU_CON_ESP32C_Y_ESP32_ETH01` | 02 | La misma lectura sobre el WT32-ETH01 (GPIO 14/15 provisionales), con el ESP32 USB-C como puente serie. |
| `banco/FASE3codigo_ESP_32MB_CON_ESP32_ETH01_Y_MPU6050` | 03 | Bus I2C definitivo (GPIO 4/14); 20 Hz. |
| `banco/FASE4sensorpresion` | 04 | MPU-6050 y MS5837-30BA en el bus compartido; 10 Hz. |
| `banco/FASE5sensorpresionyimuethernet` | 05 | Ethernet por eventos (192.168.1.200), librería MPU6050_light y tara de presión; envío UDP binario preparado, pero comentado; 10 Hz. |
| `banco/FASE6CODIGO_CORRIENTE_PLACA_CON_ENTRDA_ETHERNET` | 06 | Solo la pila de red (192.168.1.10), para la prueba de latencia con ping. |
| `banco/FASE7Prueba_mangueradepresion` | 07 y 08 | Estrés UDP a ≈ 1 kHz con contador de paquetes hacia 192.168.1.11:8888 (su uso en la Fase 08 se asume). |
| `banco/FASE11CODIGO_PARA_PROBAR_EN_UNITY_LOS_DATOS` | 11 | Trama de tres campos a 20 Hz hacia 192.168.1.100:8080 (*Hardware-in-the-Loop* por diseño, sin ensayo). Es el archivo original: asigna roll a `getAngleX()` y pitch a `getAngleY()`, al revés que la convención adoptada. |

### python/

| Programa | Descripción |
|---|---|
| `receptor.py` | Receptor de campo. Escucha en el puerto UDP 8080, guarda cada trama de cinco campos con su hora de llegada en un CSV y, al pulsar Ctrl + C, genera el XLSX; ambos quedan en `Datos_Misiones/`. Requiere `pandas` y `openpyxl`. |
| `simulador_torpedo.py` | Emisor virtual de tres campos (roll y pitch senoidales, yaw en rampa) hacia 127.0.0.1:8080 a 20 Hz de diseño, para la validación *Software-in-the-Loop* (Fases 09 y 10). |
| `simulador_torpedo_5variables.py` | Emisor virtual de cinco campos, con profundidad y temperatura sintéticas (Fase 09). |

### unity/SCRIPTS/

Los 19 scripts C# del Gemelo Digital (Unity 6000.5.5f1), en las carpetas del proyecto original: 15 en `Communication`, `Controllers`, `Navigation`, `Physics`, `Sensors`, `Simulation` y `UI`, y 4 en la raíz. `TorpedoIMU.cs` recibe la telemetría en un hilo secundario (puerto 8080) y acepta tramas de tres o cinco campos; `SimulationManager.cs` alterna el Modo Simulación y el Modo Real con la tecla M. Corresponden a la versión del 22-09-2026 (CÓDIGOS UNITY.pdf), con la sangría restituida, y coinciden con el Anexo B de la memoria. El repositorio no incluye la escena ni los recursos del proyecto de Unity.

## Uso

1. **Firmware.** Arduino IDE 2 con el núcleo ESP32 3.x y las librerías `MPU6050_light` y BlueRobotics `MS5837`. Antes de compilar `CODIGOFINAL_v2`, se elige `DENSIDAD_FLUIDO` según el medio. No consta que compile: su primera prueba es el protocolo de banco de la Fase 18 (apartado 9.3 de la memoria).
2. **Receptor.** Con el ordenador del operador en 192.168.2.1/24: `python3 receptor.py`.
3. **Gemelo Digital.** Se copia `unity/SCRIPTS` en la carpeta `Assets` de un proyecto de Unity. Sin hardware, puede alimentarse con uno de los emisores de `python/`.

## Cómo citar

Molina López, À. (2026). *tfg-sonda-barrido-lateral-usv* (Versión 1.0) [Repositorio de código fuente con licencia MIT]. GitHub. https://github.com/alexmolina-ux/tfg-sonda-barrido-lateral-usv/tree/v1.0

## Licencia

MIT (véase `LICENSE`). El diseño mecánico e hidrodinámico del cuerpo remolcado corresponde al TFG paralelo de Max Solà Fernandez (Solà, 2026) y no forma parte de este repositorio.
