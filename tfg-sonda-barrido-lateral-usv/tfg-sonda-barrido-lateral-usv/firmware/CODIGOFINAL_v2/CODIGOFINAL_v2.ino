/*
 * CODIGOFINAL_v2.ino — Firmware definitivo del nodo sensor del tow-fish
 * Nodo: WT32-ETH01 (ESP32-ETH01, PHY LAN8720) + MPU-6050 (GY-521) + MS5837-30BA (Bar30)
 * Trama UDP de texto: "roll,pitch,yaw,profundidad,temperatura" (5 campos, 2 decimales) a 20 Hz
 * Compatible con receptor.py (exige 5 campos) y con TorpedoIMU.cs del Gemelo Digital (acepta 3 o 5).
 *
 * Trazabilidad de cada decisión (cuaderno del TFG):
 *   - Pines I2C 4/14 y placa auxiliar de alimentación/flasheo: Fase 03.
 *   - Pines del PHY LAN8720 y firma de ETH.begin() del core 3.x: Fases 05, 06, 11 y 12.
 *   - Convención de ejes pitch = getAngleX(), roll = getAngleY(): Fases 05, 11 y 16 (E1).
 *   - Modelo 30BA y densidad según el medio (997 dulce / 1025 mar): Fases 04, 05, 11 y 13 (E2).
 *   - Tara de presión en superficie: Fase 05 (E3).
 *   - Comprobación de arranque de sensores con aviso por puerto serie: Fase 04 (E4).
 *   - Cadencia de 20 Hz compensando el tiempo de lectura: Fases 03 y 11; 19,23 Hz medidos en Fases 14-17 (E5).
 *   - udp.begin(8888) solo fija el puerto de origen; el nodo no recibe: Fases 07, 08, 11 y 12 (E6).
 *   - Eco de la trama por puerto serie para supervisión en banco: Fase 04 (E7).
 *   - Direccionamiento 192.168.2.x: Fases 12 y 14-17 y guía "BlueBoat Software Setup" (E8):
 *       .1 ordenador del operador (destino UDP), .2 BlueOS, .3 router de la BaseStation, .4 router de a bordo.
 *
 * Estado: PENDIENTE DE VERIFICAR EN BANCO (Fase 18). No ha sido ejecutado en ningún ensayo documentado.
 */

#include <WiFi.h>
#include <ETH.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <MPU6050_light.h>
#include "MS5837.h"

// --- PHY Ethernet WT32-ETH01 (LAN8720): Fases 05, 06, 11 y 12 ---
#define ETH_PHY_ADDR  1
#define ETH_PHY_POWER 16
#define ETH_PHY_MDC   23
#define ETH_PHY_MDIO  18
#define ETH_PHY_TYPE  ETH_PHY_LAN8720
#define ETH_CLK_MODE  ETH_CLOCK_GPIO0_IN

// --- Bus I2C compartido: Fases 03, 04, 05 y 11 ---
#define I2C_SDA_PIN 4
#define I2C_SCL_PIN 14

// --- E2: densidad del fluido según el medio. ELEGIR ANTES DE COMPILAR ---
#define DENSIDAD_AGUA_MAR   1025.0f   // kg/m3 (Fases 05, 11 y 13)
#define DENSIDAD_AGUA_DULCE  997.0f   // kg/m3 (Fases 04 y 11)
#define DENSIDAD_FLUIDO DENSIDAD_AGUA_DULCE   // banco y piscina: DULCE; mar: MAR

// --- Red nativa del BlueBoat: Fases 12 y 14 a 17; "BlueBoat Software Setup" (E8) ---
IPAddress ipLocal(192, 168, 2, 50);     // nodo (fuera del rango reservado .1-.4)
IPAddress gateway(192, 168, 2, 4);      // router de a bordo del BlueBoat
IPAddress subnet(255, 255, 255, 0);
IPAddress ipDestino(192, 168, 2, 1);    // ordenador del operador (receptor.py / Unity)
const int puertoDestino = 8080;         // Fases 11, 12 y 14-17
const unsigned long PERIODO_MS = 50;    // cadencia de diseño: 20 Hz (E5)
const float G_MS2 = 9.80665f;

WiFiUDP udp;
MPU6050 mpu(Wire);
MS5837 sensorPresion;
float presionReferencia_mbar = 0.0f;    // E3: tara en superficie
bool presionOK = false;
bool imuOK = false;

void WiFiEvent(WiFiEvent_t event) {
  if (event == ARDUINO_EVENT_ETH_GOT_IP) {
    Serial.print("[ETH] Enlace activo. IP: ");
    Serial.println(ETH.localIP());
    udp.begin(8888);   // E6: fija el puerto UDP de origen; el nodo no recibe datagramas
  } else if (event == ARDUINO_EVENT_ETH_DISCONNECTED) {
    Serial.println("[ETH] Cable desconectado (Fase 08: el bucle sigue sin bloquearse)");
  }
}

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("\n[NODO] CODIGOFINAL_v2 — arranque");
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);

  // --- Ethernet (Fases 05, 06, 11 y 12) ---
  WiFi.onEvent(WiFiEvent);
  ETH.begin(ETH_PHY_TYPE, ETH_PHY_ADDR, ETH_PHY_MDC, ETH_PHY_MDIO, ETH_PHY_POWER, ETH_CLK_MODE);
  ETH.config(ipLocal, gateway, subnet);

  // --- E4 y E7: IMU con comprobación de arranque y calibración avisada ---
  byte estadoIMU = mpu.begin();          // MPU6050_light: 0 = OK (verificar en la librería instalada)
  imuOK = (estadoIMU == 0);
  Serial.print("[IMU] mpu.begin() = ");
  Serial.print(estadoIMU);
  Serial.println(imuOK ? " (OK)" : " (ERROR: revisar bus I2C 4/14)");
  Serial.println("[IMU] Calibrando offsets: NO MOVER el cilindro durante 3 s");
  delay(1000);
  mpu.calcOffsets();                     // giroscopio y acelerometro (Fases 01 y 05)
  Serial.println("[IMU] Calibracion completada");

  // --- E4: sensor de presion con reintento (Fase 04) ---
  sensorPresion.setModel(MS5837::MS5837_30BA);   // Bar30 (Fases 05 y 11)
  for (int i = 0; i < 5 && !presionOK; i++) {
    presionOK = sensorPresion.init();
    if (!presionOK) {
      Serial.println("[BAR30] No detectado en el bus I2C, reintentando en 2 s...");
      delay(2000);
    }
  }
  sensorPresion.setFluidDensity(DENSIDAD_FLUIDO);   // E2

  // --- E3: tara en superficie, media de diez lecturas (Fase 05) ---
  if (presionOK) {
    float suma = 0.0f;
    for (int i = 0; i < 10; i++) {
      sensorPresion.read();
      suma += sensorPresion.pressure();   // mbar
      delay(100);
    }
    presionReferencia_mbar = suma / 10.0f;
    Serial.print("[BAR30] Presion de referencia (mbar): ");
    Serial.println(presionReferencia_mbar, 2);   // en aire, a nivel del mar, debe ser ~1013 mbar (cabo 7)
  } else {
    Serial.println("[BAR30] Sin sensor: profundidad y temperatura se enviaran a 0.00");
  }
  Serial.print("[NODO] Densidad configurada (kg/m3): ");
  Serial.println(DENSIDAD_FLUIDO, 0);
}

void loop() {
  unsigned long t0 = millis();

  // --- IMU: convencion definitiva de ejes (E1) ---
  mpu.update();
  float pitch = mpu.getAngleX();   // cabeceo (Fases 05, 11 y 16)
  float roll  = mpu.getAngleY();   // balanceo
  float yaw   = mpu.getAngleZ();   // sin referencia magnetica: deriva no acotada (Fases 11, 16 y 17)

  // --- Presion: profundidad relativa a la tara (E3) con la densidad del medio (E2) ---
  float profundidad = 0.0f;
  float temperatura = 0.0f;
  if (presionOK) {
    sensorPresion.read();                                                          // lectura bloqueante (a cronometrar en la Fase 18)
    float deltaPa = (sensorPresion.pressure() - presionReferencia_mbar) * 100.0f;  // mbar -> Pa
    profundidad = deltaPa / (DENSIDAD_FLUIDO * G_MS2);                             // h = (P - P0) / (rho * g)
    temperatura = sensorPresion.temperature();                                     // grados C
  }

  // --- Trama de cinco campos (Fase 11) ---
  String mensaje = String(roll, 2) + "," + String(pitch, 2) + "," + String(yaw, 2) + ","
                 + String(profundidad, 2) + "," + String(temperatura, 2);
  Serial.println(mensaje);           // E7: supervision local por puerto serie
  udp.beginPacket(ipDestino, puertoDestino);
  udp.print(mensaje);
  udp.endPacket();

  // --- E5: cadencia fija de 20 Hz descontando el tiempo de lectura y envio ---
  unsigned long transcurrido = millis() - t0;
  if (transcurrido < PERIODO_MS) delay(PERIODO_MS - transcurrido);
}
