#include <Wire.h>
#include "MS5837.h" // Librería del sensor de presión

// Pines correctos para tu ESP32-ETH01
#define I2C_SDA_PIN 4
#define I2C_SCL_PIN 14
#define MPU_ADDR 0x68 

// Variables del IMU
int16_t AcX, AcY, AcZ;
float pitch, roll;

// Objeto del sensor de presión
MS5837 sensorPresion;

void setup() {
  Serial.begin(115200);
  
  // 1. Iniciamos I2C en los pines compartidos por ambos sensores
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  
  // 2. Despertamos al sensor de actitud (MPU-6050)
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B); // Registro de energía
  Wire.write(0);    // 0 = Despierto
  Wire.endTransmission(true);
  
  // 3. Iniciamos el sensor de presión (Bar)
  while (!sensorPresion.init()) {
    Serial.println("Error: No se encuentra el sensor de Presión. Revisa las conexiones.");
    delay(2000);
  }
  
  // Configuramos el modelo del sensor de presión (cambiar a 02BA si es el Bar02)
  sensorPresion.setModel(MS5837::MS5837_30BA); 
  sensorPresion.setFluidDensity(997); // Agua dulce
  
  Serial.println("\n--- SISTEMA TOWFISH COMPLETO ACTIVO ---");
  delay(1000);
}

void loop() {
  // --- LECTURA DEL IMU (Actitud) ---
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B); 
  Wire.endTransmission(false);
  Wire.requestFrom((uint16_t)MPU_ADDR, (uint8_t)6, true);
  
  if (Wire.available() >= 6) {
    AcX = Wire.read() << 8 | Wire.read();
    AcY = Wire.read() << 8 | Wire.read();
    AcZ = Wire.read() << 8 | Wire.read();
    
    pitch = -(atan2(AcX, sqrt(AcY * AcY + AcZ * AcZ)) * 180.0) / PI;
    roll  = (atan2(AcY, AcZ) * 180.0) / PI;
  }

  // --- LECTURA DEL SENSOR DE PRESIÓN ---
  sensorPresion.read();

  // --- IMPRESIÓN DE TODOS LOS DATOS ---
  Serial.print("Cabeceo:");
  Serial.print(pitch);
  Serial.print(", Balanceo:");
  Serial.print(roll);
  
  Serial.print(" | Presion_mbar:");
  Serial.print(sensorPresion.pressure());
  
  Serial.print(" | Profundidad_m:");
  Serial.print(sensorPresion.depth());
  
  Serial.print(" | Temp_C:");
  Serial.println(sensorPresion.temperature());
  
  delay(100); // Pequeña pausa para no saturar el Monitor Serie
}