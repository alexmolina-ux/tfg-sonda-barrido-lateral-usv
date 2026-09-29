#include <Wire.h>

const int MPU_addr = 0x68; // Dirección I2C por defecto del MPU-6050
int16_t AcX, AcY, AcZ;
float pitch, roll;

void setup() {
  // Iniciamos la comunicación con tu Mac a máxima velocidad estable
  Serial.begin(115200);
  
  // Configuramos los pines I2C para tu ESP32-ETH01
  // SDA = 21, SCL = 22
  Wire.begin(21, 22); 
  
  // Despertar al sensor MPU-6050 (viene dormido por defecto)
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x6B); // Registro de gestión de energía
  Wire.write(0);    // Escribimos 0 para despertarlo
  Wire.endTransmission(true);
  
  Serial.println("\n--- SISTEMA DE TELEMETRÍA TOWFISH INICIADO ---");
  Serial.println("Calibrando IMU... OK");
  delay(2000);
}

void loop() {
  // Solicitamos los datos en bruto del acelerómetro
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x3B); // Empezamos a leer desde el registro 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_addr, 6, true); // Pedimos 6 bytes (X, Y, Z)
  
  // Reconstruimos los valores de 16 bits
  AcX = Wire.read() << 8 | Wire.read(); 
  AcY = Wire.read() << 8 | Wire.read(); 
  AcZ = Wire.read() << 8 | Wire.read(); 
  
  // Matemáticas para el TFG: 
  // Convertimos la fuerza G en grados de inclinación (Trigonometría básica)
  pitch = -(atan2(AcX, sqrt(AcY * AcY + AcZ * AcZ)) * 180.0) / PI;
  roll  = (atan2(AcY, AcZ) * 180.0) / PI;
  
  // Formato preparado para el "Serial Plotter" de Arduino
  Serial.print("Cabeceo_Pitch:");
  Serial.print(pitch);
  Serial.print(",");
  Serial.print("Balanceo_Roll:");
  Serial.println(roll);
  
  // Tomamos 10 muestras por segundo (100ms)
  delay(100); 
}