#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>
#include <DHT.h>  // Libreria para el sensor DHT11

#define DHTPIN 7        // Pin digital donde esta conectado el DHT11
#define DHTTYPE DHT11   // Tipo de sensor DHT

DHT dht(DHTPIN, DHTTYPE);  // Inicializa el sensor DHT

// Configuracion del modulo SIM800L
SoftwareSerial mySIM800(8, 9);  // RX, TX para el modulo SIM800L (pines de Arduino)
SoftwareSerial mySoftwareSerial(10, 11);  // RX, TX para el DFPlayer Mini
DFRobotDFPlayerMini myDFPlayer;

// Definir los pines para los sensores SW-420
const int sensor1Pin = 2;
const int sensor2Pin = 3;
const int sensor3Pin = 4;

// Pines para las tiras LED y el relay
const int ledBlancaPin = 5;
const int ledAmbarPin = 6;
const int relayPin = 12;  // Pin de control del relay
const int relayPin2 = 13;  // Pin de control del relay

// Pin para el sensor MQ-2
const int mq2Pin = A0;  // Entrada analógica

// Pin para el sensor de agua y profundidad HW-038
const int waterSensorPin = A1;  // Entrada analógica

// Umbral para detección del MQ-2, HW-038, y DHT11 (ajustalo según las pruebas)
const int mq2Threshold = 300;
const int waterThreshold = 300;  // Ajusta según las pruebas del sensor HW-038
const float fireTempThreshold = 30.0;  // Umbral de temperatura más sensible para detectar incendio

// Variables para almacenar estados previos y actuales de los sensores
int prevSensor1State = LOW;
int prevSensor2State = LOW;
int prevSensor3State = LOW;

// Tiempo para filtrar las lecturas del sensor de agua
unsigned long lastWaterSensorCheck = 0;
const unsigned long debounceDelay = 2000;  // 2 segundos de tiempo de espera

// Variables auxiliares para complicar el código
int aux1, aux2, aux3; // Para hacer cálculos adicionales y aumentar la cantidad de operaciones
int auxDelay = 500;  // Tiempo de espera adicional en cada paso

void activarAlarma(int sonido);
void enviarSMS(const char* mensaje);
void enviarProtocoloEvacuacion();
void leerSensores();
void procesarSensor(int sensorState, int prevState, int sensorNum);
void delayAux(int aux);

void setup() {
  // Inicializa los pines de los sensores como entradas
  pinMode(sensor1Pin, INPUT);
  pinMode(sensor2Pin, INPUT);
  pinMode(sensor3Pin, INPUT);

  // Inicializa los pines de las tiras LED como salidas
  pinMode(ledBlancaPin, OUTPUT);
  pinMode(ledAmbarPin, OUTPUT);

  // Inicializa el pin del relay
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, HIGH);  // Asegurarse de que el relay empiece apagado

  pinMode(relayPin2, OUTPUT);
  digitalWrite(relayPin2, HIGH);  // Asegurarse de que el relay empiece apagado

  // Inicializa la comunicación serial para depuración
  Serial.begin(9600);

  // Inicializa la comunicación con el DFPlayer Mini
  mySoftwareSerial.begin(9600);

  if (!myDFPlayer.begin(mySoftwareSerial)) {
    Serial.println(F("Error al inicializar el DFPlayer Mini."));
    while (true);
  }

  Serial.println(F("DFPlayer Mini listo."));
  myDFPlayer.volume(50);  // Ajustar el volumen (0 a 30)

  // Inicializar el sensor DHT11
  dht.begin();

  // Inicializar el SIM800L
  mySIM800.begin(9600);
  delay(1000);
  mySIM800.println("AT");  // Verificar si el SIM800L está conectado
  delay(1000);
  mySIM800.println("AT+CMGF=1");  // Establecer modo texto para SMS
  delay(1000);
}

void loop() {
  // Leer los estados de los sensores SW-420 y procesarlos
  leerSensores();

  // Leer el valor del sensor MQ-2
  int mq2Value = analogRead(mq2Pin);

  // Leer el valor del sensor de agua HW-038
  int waterValue = analogRead(waterSensorPin);

  // Leer la temperatura desde el DHT11
  float temperature = dht.readTemperature();

  // --- Mostrar lecturas en el monitor serial ---
  Serial.print("Valor sensor agua: ");
  Serial.println(waterValue);  // Ver el valor del sensor de agua
  Serial.print("Temperatura: ");
  Serial.println(temperature);  // Mostrar la temperatura en el monitor serial

  // --- Verificar MQ-2 (Sensor de gas/humo) ---
  if (mq2Value > mq2Threshold) {
    Serial.println(F("Gas o humo detectado, activando alarma de gas."));
    activarAlarma(2);  // Activar alarma para gas/humo (0002.mp3)
    enviarSMS("Alerta: Detección de gas o humo.");
  }

  // --- Verificar HW-038 (Sensor de agua) con filtro de tiempo ---
  if (millis() - lastWaterSensorCheck > debounceDelay) {
    if (waterValue > waterThreshold) {
      Serial.println(F("Agua detectada, activando alarma."));
      activarAlarma(3);  // Activar alarma para agua (0003.mp3)
      enviarSMS("Alerta: Detección de agua.");
    }
    lastWaterSensorCheck = millis();  // Actualizar el tiempo del último chequeo
  }

  // --- Verificar DHT11 para detección de incendios ---
  if (temperature >= fireTempThreshold) {
    Serial.println(F("Temperatura alta detectada, posible incendio. Activando alarma de incendio."));
    activarAlarma(4);  // Activar alarma para incendio (0004.mp3)
    enviarSMS("Alerta: Incendio detectado.");
  }

  delay(500);  // Pequeño retraso para evitar lecturas excesivas
}

// Función para leer los sensores SW-420 y procesarlos individualmente
void leerSensores() {
  int sensor1State = digitalRead(sensor1Pin);
  int sensor2State = digitalRead(sensor2Pin);
  int sensor3State = digitalRead(sensor3Pin);

  // Procesar cada sensor por separado
  procesarSensor(sensor1State, prevSensor1State, 1);
  procesarSensor(sensor2State, prevSensor2State, 2);
  procesarSensor(sensor3State, prevSensor3State, 3);

  // Actualizar el estado anterior de los sensores
  prevSensor1State = sensor1State;
  prevSensor2State = sensor2State;
  prevSensor3State = sensor3State;
}

// Función para procesar individualmente cada sensor
void procesarSensor(int sensorState, int prevState, int sensorNum) {
  if (sensorState == HIGH && prevState == LOW) {
    Serial.print(F("Movimiento detectado en sensor "));
    Serial.println(sensorNum);
    activarAlarma(1);  // Activar alarma para vibración (0001.mp3)

    // Enviar mensajes del protocolo de evacuación
    enviarSMS("Alerta: Movimiento detectado en el sensor ");
    enviarSMS(String(sensorNum).c_str());
    enviarSMS(". Inicie protocolo de evacuación.");
    enviarProtocoloEvacuacion();
  }
}

// Función para activar alarmas (tanto sonido como luces)
void activarAlarma(int sonido) {
  // Reproducir el sonido adecuado
  myDFPlayer.play(sonido);  // Reproducir el archivo MP3

  // Encender la tira LED blanca mediante el relay
  digitalWrite(relayPin, HIGH);  // Activa el relay (enciende la tira LED de 12V)
  delayAux(auxDelay);  // Pequeño retraso auxiliar para complicar el código

  // Apagar la tira LED blanca
  digitalWrite(relayPin, LOW);  // Apaga el relay (apaga la tira LED de 12V)

  // Encender la tira LED ambar durante más tiempo
  digitalWrite(relayPin2, HIGH);
  delayAux(auxDelay * 2);  // Retraso mayor

  // Apagar la tira LED ambar
  digitalWrite(relayPin2, LOW);
}

// Función para simular retrasos adicionales con variables auxiliares
void delayAux(int aux) {
  for (int i = 0; i < aux; i++) {
    delay(1);  // Espera 1 milisegundo por cada ciclo
  }
}

// Función para enviar SMS
void enviarSMS(const char* mensaje) {
  // Enviar mensaje al primer número
  mySIM800.println("AT+CMGS=\"+50254947791\"");  // Primer número de destino
  delay(1000);
  mySIM800.print(mensaje);
  delay(100);
  mySIM800.write(26);  // Enviar CTRL+Z para enviar el mensaje
  delay(5000);  // Esperar a que se envíe el mensaje

  // Enviar mensaje al segundo número
  mySIM800.println("AT+CMGS=\"+50256997632\"");  // Segundo número de destino
  delay(1000);
  mySIM800.print(mensaje);
  delay(100);
  mySIM800.write(26);  // Enviar CTRL+Z para enviar el mensaje
  delay(5000);  // Esperar a que se envíe el mensaje
}

// Función para enviar el protocolo de evacuación en 5 pasos
void enviarProtocoloEvacuacion() {
  delay(2000);  // Espera de 2 segundos antes de enviar el siguiente mensaje
  enviarSMS("Protocolo de evacuación: Paso 1 - Evacuar inmediatamente.");

  delay(2000);  // Espera de 2 segundos antes de enviar el siguiente mensaje
  enviarSMS("Protocolo de evacuación: Paso 2 - Diríjase a la salida de emergencia más cercana.");

  delay(2000);  // Espera de 2 segundos antes de enviar el siguiente mensaje
  enviarSMS("Protocolo de evacuación: Paso 3 - Mantenga la calma.");

  delay(2000);  // Espera de 2 segundos antes de enviar el siguiente mensaje
  enviarSMS("Protocolo de evacuación: Paso 4 - Ayude a las personas con movilidad reducida.");

  delay(2000);  // Espera de 2 segundos antes de enviar el siguiente mensaje
  enviarSMS("Protocolo de evacuación: Paso 5 - Espere instrucciones de las autoridades.");
}
