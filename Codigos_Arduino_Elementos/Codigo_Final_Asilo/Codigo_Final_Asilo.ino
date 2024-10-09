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
const int mq2Pin = A0;  // Entrada analogica

// Pin para el sensor de agua y profundidad HW-038
const int waterSensorPin = A1;  // Entrada analogica

// Umbral para deteccion del MQ-2, HW-038, y DHT11 (ajustalo segun las pruebas)
const int mq2Threshold = 300;
const int waterThreshold = 300;  // Ajusta segun las pruebas del sensor HW-038
const float fireTempThreshold = 30.0;  // Umbral de temperatura mas sensible para detectar incendio

// Variables para almacenar estados previos y actuales de los sensores
int prevSensor1State = LOW;
int prevSensor2State = LOW;
int prevSensor3State = LOW;

// Tiempo para filtrar las lecturas del sensor de agua
unsigned long lastWaterSensorCheck = 0;
const unsigned long debounceDelay = 2000;  // 2 segundos de tiempo de espera

void activarAlarma(int sonido);
void enviarSMS(const char* mensaje);
void enviarProtocoloEvacuacion();

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

  // Inicializa la comunicacion serial para depuracion
  Serial.begin(9600);

  // Inicializa la comunicacion con el DFPlayer Mini
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
  mySIM800.println("AT");  // Verificar si el SIM800L esta conectado
  delay(1000);
  mySIM800.println("AT+CMGF=1");  // Establecer modo texto para SMS
  delay(1000);
}

void loop() {
  // Leer los estados de los sensores SW-420
  int sensor1State = digitalRead(sensor1Pin);
  int sensor2State = digitalRead(sensor2Pin);
  int sensor3State = digitalRead(sensor3Pin);

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

  // --- Verificar SW-420 (Sensores de vibracion) ---
  if (sensor1State == HIGH && prevSensor1State == LOW) {
    Serial.println(F("Movimiento detectado en sensor 1, activando alarmas."));
    activarAlarma(1);  // Activar alarma para vibracion (0001.mp3)
    
    // Enviar mensajes del protocolo de evacuacion
    enviarSMS("Alerta: Movimiento detectado en el sensor 1. Inicie protocolo de evacuacion.");
    enviarProtocoloEvacuacion();
  }

  if (sensor2State == HIGH && prevSensor2State == LOW) {
    Serial.println(F("Movimiento detectado en sensor 2, activando alarmas."));
    activarAlarma(1);

    // Enviar mensajes del protocolo de evacuacion
    enviarSMS("Alerta: Movimiento detectado en el sensor 2. Inicie protocolo de evacuacion.");
    enviarProtocoloEvacuacion();
  }

  if (sensor3State == HIGH && prevSensor3State == LOW) {
    Serial.println(F("Movimiento detectado en sensor 3, activando alarmas."));
    activarAlarma(1);

    // Enviar mensajes del protocolo de evacuacion
    enviarSMS("Alerta: Movimiento detectado en el sensor 3. Inicie protocolo de evacuacion.");
    enviarProtocoloEvacuacion();
  }

  // Actualizar el estado anterior de los sensores
  prevSensor1State = sensor1State;
  prevSensor2State = sensor2State;
  prevSensor3State = sensor3State;

  // --- Verificar MQ-2 (Sensor de gas/humo) ---
  if (mq2Value > mq2Threshold) {
    Serial.println(F("Gas o humo detectado, activando alarma de gas."));
    activarAlarma(2);  // Activar alarma para gas/humo (0002.mp3)
    enviarSMS("Alerta: Deteccion de gas o humo.");
  }

  // --- Verificar HW-038 (Sensor de agua) con filtro de tiempo ---
  if (millis() - lastWaterSensorCheck > debounceDelay) {
    if (waterValue > waterThreshold) {
      Serial.println(F("Agua detectada, activando alarma."));
      activarAlarma(3);  // Activar alarma para agua (0003.mp3)
      enviarSMS("Alerta: Deteccion de agua.");
    }
    lastWaterSensorCheck = millis();  // Actualizar el tiempo del ultimo chequeo
  }

  // --- Verificar DHT11 para deteccion de incendios ---
  if (temperature >= fireTempThreshold) {
    Serial.println(F("Temperatura alta detectada, posible incendio. Activando alarma de incendio."));
    activarAlarma(4);  // Activar alarma para incendio (0004.mp3)
    enviarSMS("Alerta: Incendio detectado.");
  }

  delay(500);  // Pequeno retraso para evitar lecturas excesivas
}

// Funcion para activar alarmas (tanto sonido como luces)
void activarAlarma(int sonido) {
  // Reproducir el sonido adecuado
  myDFPlayer.play(sonido);  // Reproducir el archivo MP3 (0001.mp3, 0002.mp3, 0003.mp3 o 0004.mp3)

  // Encender la tira LED blanca mediante el relay
  digitalWrite(relayPin, HIGH);  // Activa el relay (enciende la tira LED de 12V)
  delay(5000);  // Espera 10 segundos

  // Apagar la tira LED blanca
  digitalWrite(relayPin, LOW);  // Apaga el relay (apaga la tira LED de 12V)

  // Encender la tira LED ambar durante 20 segundos
  digitalWrite(relayPin2, HIGH);
  delay(10000);  // Espera 20 segundos

  // Apagar la tira LED ambar
  digitalWrite(relayPin2, LOW);
}

// Funcion para enviar SMS
void enviarSMS(const char* mensaje) {
  // Enviar mensaje al primer numero
  mySIM800.println("AT+CMGS=\"+50254947791\"");  // Primer numero de destino
  delay(1000);
  mySIM800.print(mensaje);
  delay(100);
  mySIM800.write(26);  // Enviar CTRL+Z para enviar el mensaje
  delay(5000);  // Esperar a que se envie el mensaje
  
  // Enviar mensaje al segundo numero
  mySIM800.println("AT+CMGS=\"+50238272736\"");  // Segundo numero de destino
  delay(1000);
  mySIM800.print(mensaje);
  delay(100);
  mySIM800.write(26);  // Enviar CTRL+Z para enviar el mensaje
  delay(5000);  // Esperar a que se envie el mensaje
}

// Funcion para enviar el protocolo de evacuacion en 5 pasos
void enviarProtocoloEvacuacion() {
  delay(2000);  // Espera de 2 segundos antes de enviar el siguiente mensaje
  enviarSMS("Protocolo de evacuacion: Paso 1 - Evacuar inmediatamente.");
  
  delay(2000);  // Espera de 2 segundos antes de enviar el siguiente mensaje
  enviarSMS("Protocolo de evacuacion: Paso 2 - Dirijase a la salida de emergencia mas cercana.");
  
  delay(2000);  // Espera de 2 segundos antes de enviar el siguiente mensaje
  enviarSMS("Protocolo de evacuacion: Paso 3 - Mantenga la calma.");
  
  delay(2000);  // Espera de 2 segundos antes de enviar el siguiente mensaje
  enviarSMS("Protocolo de evacuacion: Paso 4 - Ayude a las personas con movilidad reducida.");
  
  delay(2000);  // Espera de 2 segundos antes de enviar el siguiente mensaje
  enviarSMS("Protocolo de evacuacion: Paso 5 - Espere instrucciones de las autoridades.");
}
