#include <ETH.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <MPU6050_light.h>
#include "MS5837.h"

// --- PINES DEL CHIP ETHERNET (Específicos para ESP32-ETH01) ---
#define ETH_PHY_ADDR  1
#define ETH_PHY_POWER 16
#define ETH_PHY_MDC   23
#define ETH_PHY_MDIO  18
#define ETH_PHY_TYPE  ETH_PHY_LAN8720
#define ETH_CLK_MODE  ETH_CLOCK_GPIO0_IN

// --- CONFIGURACIÓN DE RED (IP ESTÁTICA) ---
IPAddress ipLocal(192, 168, 1, 50);       // La IP de este ESP32
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

IPAddress ipDestino(192, 168, 1, 100);    // La IP que le pondremos a tu Mac
const int puertoDestino = 8080;

WiFiUDP udp;
MPU6050 mpu(Wire);
MS5837 sensorPresion;

void setup() {
  Serial.begin(115200);
  Wire.begin(4, 14); // SDA = IO4, SCL = IO14

  Serial.println("Iniciando Ethernet...");
  
  // NUEVO ORDEN DE PARÁMETROS PARA LA LIBRERÍA ACTUALIZADA:
  ETH.begin(ETH_PHY_TYPE, ETH_PHY_ADDR, ETH_PHY_MDC, ETH_PHY_MDIO, ETH_PHY_POWER, ETH_CLK_MODE);
  ETH.config(ipLocal, gateway, subnet); // Forzar IP estática

  // Esperar a que el cable RJ45 esté conectado al switch
  Serial.print("Esperando conexión de red (conecta el cable)...");
  while(!ETH.linkUp()) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nETH Conectado!");
  Serial.print("IP del ESP32: "); Serial.println(ETH.localIP());

  udp.begin(8888);

  // Inicializar IMU (GY-521)
  if(mpu.begin() != 0) {
    Serial.println("Error conectando MPU6050");
  } else {
    Serial.println("Calibrando IMU, no lo muevas...");
    delay(1000);
    mpu.calcOffsets(); 
    Serial.println("IMU Calibrado.");
  }

  // Inicializar Sensor Bar
  sensorPresion.setModel(MS5837::MS5837_30BA); 
  if (!sensorPresion.init()) { 
    Serial.println("Error conectando el sensor Bar."); 
  }
  sensorPresion.setFluidDensity(997); 
}

void loop() {
  mpu.update();
  sensorPresion.read(); 

  float roll = mpu.getAngleX();
  float pitch = mpu.getAngleY();
  float yaw = mpu.getAngleZ();
  float profundidad = sensorPresion.depth(); 
  float temperatura = sensorPresion.temperature(); 

  Serial.print("Profundidad: "); Serial.print(profundidad);
  Serial.print(" m | Temp: "); Serial.print(temperatura); Serial.println(" C");

  // Enviar Roll, Pitch y Yaw por UDP a Unity
  String mensaje = String(roll, 2) + "," + String(pitch, 2) + "," + String(yaw, 2);
  
  udp.beginPacket(ipDestino, puertoDestino);
  udp.print(mensaje);
  udp.endPacket();

  delay(50); 
}