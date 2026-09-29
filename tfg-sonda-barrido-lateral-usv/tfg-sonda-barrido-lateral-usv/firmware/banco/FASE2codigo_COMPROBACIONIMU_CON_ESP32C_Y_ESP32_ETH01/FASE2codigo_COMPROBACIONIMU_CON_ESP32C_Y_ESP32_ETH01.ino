#include <Wire.h>

const int MPU_addr = 0x68; // Dirección I2C por defecto del MPU-6050

// Pines I2C reales del ESP32-ETH01 ("Cerebro"), según el diagrama de interconexión:
// GPIO14 = SDA, GPIO15 = SCL
#define I2C_SDA_PIN 14
#define I2C_SCL_PIN 15

int16_t AcX, AcY, AcZ;
float pitch, roll;

void setup() {
  // La telemetría viaja por puerto serie a través del ESP32 puente (Tipo C),
  // que actúa como conversor USB-Serial hacia el ordenador
  Serial.begin(115200);

  // Configuramos el bus I2C con los pines definitivos del ESP32-ETH01
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);

  // Despertar al sensor MPU-6050 (viene dormido por defecto)
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x6B); // Registro de gestión de energía
  Wire.write(0);    // Escribimos 0 para despertarlo
  Wire.endTransmission(true);

  Serial.println("\n--- SISTEMA DE TELEMETRÍA TOWFISH INICIADO ---");
  Serial.println("ESP32-ETH01 operativo. IMU inicializada en GPIO14 (SDA) / GPIO15 (SCL).");
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

  // Convertimos el vector de gravedad en ángulos de inclinación (trigonometría básica)
  pitch = -(atan2(AcX, sqrt(AcY * AcY + AcZ * AcZ)) * 180.0) / PI;
  roll  = (atan2(AcY, AcZ) * 180.0) / PI;

  // Formato preparado para el "Serial Plotter" de Arduino
  Serial.print("Cabeceo_Pitch:");
  Serial.print(pitch);
  Serial.print(",");
  Serial.print("Balanceo_Roll:");
  Serial.println(roll);

  delay(100); // Muestreo a 10 Hz
}