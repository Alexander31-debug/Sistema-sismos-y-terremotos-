# Sistema de Detección y Alarma de Emergencias

Este proyecto es un sistema de detección de emergencias basado en sensores como SW-420, MQ-2, HW-038 y DHT11, con la capacidad de enviar alertas SMS utilizando un módulo SIM800L. Además, controla luces y alarmas sonoras a través del módulo DFPlayer Mini.

## Características

- Detección de vibraciones, gas, agua e incendios.
- Alertas de audio con diferentes sonidos dependiendo del tipo de emergencia.
- Control de luces LED mediante relés.
- Envío de mensajes SMS automáticos con el protocolo de evacuación.
  
## Instalación

1. **Hardware necesario:**
   - Arduino Uno o compatible.
   - Módulo DFPlayer Mini.
   - Módulo SIM800L.
   - Sensores SW-420, MQ-2, HW-038, DHT11.
   - Relés para controlar tiras LED.

2. **Librerías necesarias:**
   - `DFRobotDFPlayerMini`: [Enlace a la librería](https://github.com/DFRobot/DFRobotDFPlayerMini)
   - `DHT`: Para el sensor DHT11.
   - `SoftwareSerial`: Incluida en el entorno de desarrollo de Arduino.

3. **Instalación del código:**
   - Clona este repositorio o descarga los archivos en tu computadora.
   - Carga el código en tu placa Arduino usando el IDE de Arduino.
   - Conecta los módulos y sensores de acuerdo con las conexiones especificadas en el código.

## Uso

1. Una vez instalado y cargado el código en el Arduino, los sensores comenzarán a monitorear los diferentes parámetros (vibración, humo, agua, temperatura).
2. Cuando uno de los sensores detecta una condición de emergencia, el sistema:
   - Enciende las luces LED correspondientes.
   - Reproduce un sonido de alarma específico.
   - Envía un mensaje SMS con instrucciones de evacuación.
   
## Problemas resueltos

Este sistema está diseñado para entornos donde se requiere una respuesta rápida a emergencias como sismos, inundaciones, incendios o fugas de gas. Puede implementarse en hogares, centros de atención a adultos mayores, oficinas, etc.
