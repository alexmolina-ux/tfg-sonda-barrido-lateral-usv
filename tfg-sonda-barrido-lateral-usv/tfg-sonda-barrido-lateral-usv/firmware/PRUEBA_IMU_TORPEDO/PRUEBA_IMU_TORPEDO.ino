#include <WiFi.h>
#include <ETH.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <MPU6050_light.h> // Incorporamos la librería para el Yaw

// Pines Ethernet WT32-ETH01
#define ETH_PHY_ADDR  1
#define ETH_PHY_POWER 16
#define ETH_PHY_MDC   23
#define ETH_PHY_MDIO  18
#define ETH_PHY_TYPE  ETH_PHY_LAN8720
#define ETH_CLK_MODE  ETH_CLOCK_GPIO0_IN

// Pines I2C 
#define I2C_SDA_PIN 4
#define I2C_SCL_PIN 14

// Configuración de Red
IPAddress ipLocal(192, 168, 2, 50);       
IPAddress gateway(192, 168, 2, 4);        
IPAddress subnet(255, 255, 255, 0);
IPAddress ipDestino(192, 168, 2, 1);      
const int puertoDestino = 8080;

WiFiUDP udp;
MPU6050 mpu(Wire); // Creamos el objeto IMU

void WiFiEvent(WiFiEvent_t event) {
  if (event == ARDUINO_EVENT_ETH_GOT_IP) {
    Serial.println("\n[+] Ethernet Conectado");
    Serial.print("    IP del ESP32: ");
    Serial.println(ETH.localIP());
    udp.begin(8888);
  }
}

void setup() {
  Serial.begin(115200);
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);

  // Iniciamos red
  WiFi.onEvent(WiFiEvent);
  ETH.begin(ETH_PHY_TYPE, ETH_PHY_ADDR, ETH_PHY_MDC, ETH_PHY_MDIO, ETH_PHY_POWER, ETH_CLK_MODE);
  ETH.config(ipLocal, gateway, subnet);
  
  Serial.println("\n--- SISTEMA DE PRUEBA HIL ACTIVO ---");
  Serial.println("Calibrando IMU... ¡NO MOVER durante 3 segundos!");
  
  // Encendemos y calibramos el sensor completo
  mpu.begin();
  delay(1000);
  mpu.calcOffsets(); // Esta línea es magia: fija el "cero" del giroscopio
  
  Serial.println("¡Calibración completada! Enviando telemetría...");
}

void loop() {
  mpu.update(); // Realiza los cálculos matemáticos de integración
  
  // CORREGIDO: se iguala la asignación de ejes a la convención de la Fase 5
  float pitch = mpu.getAngleX();
  float roll = mpu.getAngleY();
  float yaw = mpu.getAngleZ(); // ¡Ahora extraemos el Yaw en tiempo real!
  
  // Formato exacto para tu Unity/Python: Roll, Pitch, Yaw, Prof (0), Temp (0)
  String mensaje = String(roll, 2) + "," + String(pitch, 2) + "," + String(yaw, 2) + ",0.00,0.00";
  
  // Mostramos en Monitor Serie
  Serial.println(mensaje);

  // Enviamos por Ethernet UDP al Mac
  udp.beginPacket(ipDestino, puertoDestino);
  udp.print(mensaje);
  udp.endPacket();
  
  delay(50); // Muestreo a 20 Hz
}
