#include <Wire.h>

// Pines correctos para tu ESP32-ETH01
#define I2C_SDA_PIN 4
#define I2C_SCL_PIN 14
#define MPU_ADDR 0x68 

int16_t AcX, AcY, AcZ;
float pitch, roll;

void setup() {
  Serial.begin(115200);
  
  // 1. Iniciamos I2C en los pines que sabemos que funcionan
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  
  // 2. Despertamos al sensor
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B); // Registro de energía
  Wire.write(0);    // 0 = Despierto
  Wire.endTransmission(true);
  
  Serial.println("\n--- SISTEMA DE NAVEGACIÓN TOWFISH ACTIVO ---");
  delay(1000);
}

void loop() {
  // 3. Pedimos los datos al acelerómetro
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B); // Registro inicial
  Wire.endTransmission(false);
  Wire.requestFrom((uint16_t)MPU_ADDR, (uint8_t)6, true);
  
  if (Wire.available() >= 6) {
    // 4. Reconstruimos los datos crudos
    AcX = Wire.read() << 8 | Wire.read();
    AcY = Wire.read() << 8 | Wire.read();
    AcZ = Wire.read() << 8 | Wire.read();
    
    // 5. Matemáticas del TFG: Conversión a Grados (Trigonometría)
    pitch = -(atan2(AcX, sqrt(AcY * AcY + AcZ * AcZ)) * 180.0) / PI;
    roll  = (atan2(AcY, AcZ) * 180.0) / PI;
    
    // 6. Imprimimos en formato compatible con el Serial Plotter
    Serial.print("Cabeceo_Pitch:");
    Serial.print(pitch);
    Serial.print(",");
    Serial.print("Balanceo_Roll:");
    Serial.println(roll);
  }
  
  delay(50); // Muestreo rápido para que la gráfica sea muy fluida
}