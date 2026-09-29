#include <WiFi.h>
#include <ETH.h>
#include <Wire.h>
#include "MS5837.h"
#include <MPU6050_light.h>

// ── Configuración del PHY Ethernet (WT32-ETH01) ───────────────────────────────
#define ETH_PHY_MDC      23
#define ETH_PHY_MDIO     18
#define ETH_PHY_POWER    16
#define ETH_CLK_MODE     ETH_CLOCK_GPIO0_IN

// ── Bus I2C ───────────────────────────────────────────────────────────────────
#define I2C_SDA   4    // IO04 del WT32-ETH01
#define I2C_SCL   14   // IO14 del WT32-ETH01

// ── IP estática del ESP32 ─────────────────────────────────────────────────────
// Cambia estos valores si tu red usa un rango diferente
IPAddress ip_esp32   (192, 168,   1, 200);  // IP que tendrá el ESP32
IPAddress ip_gateway (192, 168,   1,   1);  // Gateway (tu router o el Mac)
IPAddress ip_mascara (255, 255, 255,   0);  // Máscara de subred
IPAddress ip_dns     (  8,   8,   8,   8);  // DNS — Google, no es crítico aquí

// ── Estado del sistema ────────────────────────────────────────────────────────
static bool eth_connected       = false;
static bool sensor_ok           = false;
static float presion_referencia = 0.0f;

// ── Objetos de sensor ─────────────────────────────────────────────────────────
MS5837   bar02;
MPU6050  mpu(Wire);

// ── Estructura de telemetría ──────────────────────────────────────────────────
struct TelemetryPacket {
  float    depth_m;
  float    pressure_mbar;
  float    temperature_c;
  float    accel_x,  accel_y,  accel_z;   // m/s²
  float    gyro_x,   gyro_y,   gyro_z;    // °/s
  float    pitch_deg, roll_deg;
  uint32_t timestamp_ms;
};

// ── Manejador de eventos Ethernet ─────────────────────────────────────────────
void WiFiEvent(WiFiEvent_t event) {
  switch (event) {

    case ARDUINO_EVENT_ETH_START:
      Serial.println("[ETH] Interfaz iniciada");
      ETH.setHostname("torpedo-telemetry");
      // IP estática — se configura aquí, en el momento en que la interfaz arranca
      ETH.config(ip_esp32, ip_gateway, ip_mascara, ip_dns);
      break;

    case ARDUINO_EVENT_ETH_CONNECTED:
      Serial.println("[ETH] Cable conectado físicamente");
      break;

    case ARDUINO_EVENT_ETH_GOT_IP:
      Serial.printf("[ETH] Enlace OK\n");
      Serial.printf("      IP:        %s\n", ETH.localIP().toString().c_str());
      Serial.printf("      MAC:       %s\n", ETH.macAddress().c_str());
      Serial.printf("      Velocidad: %d Mbps\n", ETH.linkSpeed());
      eth_connected = true;
      break;

    case ARDUINO_EVENT_ETH_DISCONNECTED:
      Serial.println("[ETH] Cable desconectado");
      eth_connected = false;
      break;

    case ARDUINO_EVENT_ETH_STOP:
      Serial.println("[ETH] Interfaz detenida");
      eth_connected = false;
      break;

    default:
      break;
  }
}

// ── Inicialización del bus I2C y sensores ─────────────────────────────────────
bool initSensors() {

  Wire.begin(I2C_SDA, I2C_SCL);
  Wire.setClock(400000);

  // ── Bar02 ─────────────────────────────────────────────────────────────────
  bar02.setModel(MS5837::MS5837_30BA);  // CORREGIDO: el sensor físico instalado es un Bar30

  if (!bar02.init()) {
    Serial.println("[BAR02] FALLO — comprueba SDA/SCL y que VCC sea 3.3V");
    return false;
  }

  // 1025 kg/m³ agua de mar | 997 kg/m³ agua dulce
  bar02.setFluidDensity(1025);

  // 10 lecturas de estabilización antes de tomar la referencia
  Serial.println("[BAR02] Tomando presión de referencia en superficie...");
  Serial.println("        Asegúrate de que el sensor está en el AIRE.");
  delay(1000);

  for (int i = 0; i < 10; i++) {
    bar02.read();
    delay(100);
  }

  presion_referencia = bar02.pressure();
  Serial.printf("[BAR02] Presión de referencia: %.2f mbar\n", presion_referencia);
  Serial.println("[BAR02] OK");

  // ── MPU6050 ───────────────────────────────────────────────────────────────
  // 0x68 si AD0 está a GND | cambia a mpu.begin(0x69) si AD0 está a VCC
  byte status = mpu.begin();

  if (status != 0) {
    Serial.printf("[MPU6050] FALLO — código de error: %d\n", status);
    Serial.println("          Si el error es 1, prueba mpu.begin(0x69)");
    return false;
  }

  // La placa debe estar HORIZONTAL y COMPLETAMENTE INMÓVIL durante estos 3 s
  Serial.println("[MPU6050] Calculando offsets de calibración...");
  Serial.println("          Coloca la placa HORIZONTAL e INMÓVIL durante 3 segundos.");
  delay(3000);
  mpu.calcOffsets(true, true);
  Serial.println("[MPU6050] OK — calibración completada");

  return true;
}

// ── Lectura de sensores → paquete de telemetría ───────────────────────────────
TelemetryPacket readSensors() {

  TelemetryPacket pkt;
  pkt.timestamp_ms = millis();

  // ── Bar02 ─────────────────────────────────────────────────────────────────
  bar02.read();
  pkt.pressure_mbar = bar02.pressure();
  pkt.temperature_c = bar02.temperature();

  // Profundidad respecto a la presión de referencia tomada en superficie
  // delta en Pa = delta en mbar × 100
  // profundidad (m) = delta_Pa / (densidad × gravedad)
  float delta_pa = (pkt.pressure_mbar - presion_referencia) * 100.0f;
  pkt.depth_m    = delta_pa / (1025.0f * 9.80665f);

  // ── MPU6050 ───────────────────────────────────────────────────────────────
  mpu.update();
  pkt.accel_x   = mpu.getAccX();
  pkt.accel_y   = mpu.getAccY();
  pkt.accel_z   = mpu.getAccZ();
  pkt.gyro_x    = mpu.getGyroX();
  pkt.gyro_y    = mpu.getGyroY();
  pkt.gyro_z    = mpu.getGyroZ();
  pkt.pitch_deg = mpu.getAngleX();
  pkt.roll_deg  = mpu.getAngleY();

  return pkt;
}

// ─────────────────────────────────────────────────────────────────────────────
void setup() {

  Serial.begin(115200);
  delay(500);

  // Ethernet primero — reserva GPIO0 y pines RMII antes que cualquier otra cosa
  WiFi.onEvent(WiFiEvent);

  // Si con phy_addr=1 no hay enlace, cambia el 1 por 0
  ETH.begin(ETH_PHY_LAN8720,
            1,
            ETH_PHY_MDC,
            ETH_PHY_MDIO,
            ETH_PHY_POWER,
            ETH_CLK_MODE);

  // Sensores I2C después de que Ethernet haya reservado sus pines
  sensor_ok = initSensors();

  if (!sensor_ok) {
    Serial.println("[SISTEMA] Sensores no disponibles — solo Ethernet activo");
  }

  Serial.println("[SISTEMA] Setup completado");
  Serial.printf ("[SISTEMA] IP estática configurada: %s\n", ip_esp32.toString().c_str());
  Serial.println("[SISTEMA] Conecta el cable RJ45 si no lo has hecho ya");
}

// ─────────────────────────────────────────────────────────────────────────────
void loop() {

  static uint32_t last_ms = 0;
  const uint32_t  INTERVAL = 100;  // 10 Hz

  if (millis() - last_ms >= INTERVAL) {
    last_ms = millis();

    if (sensor_ok) {
      TelemetryPacket pkt = readSensors();

      Serial.printf(
        "[%6.2f s] "
        "Prof: %6.3f m | "
        "P: %7.2f mbar | "
        "T: %5.1f C | "
        "Pitch: %5.1f deg | "
        "Roll: %5.1f deg | "
        "ETH: %s\n",
        pkt.timestamp_ms / 1000.0f,
        pkt.depth_m,
        pkt.pressure_mbar,
        pkt.temperature_c,
        pkt.pitch_deg,
        pkt.roll_deg,
        eth_connected ? "OK" : "sin enlace"
      );

      // ── Envío UDP al Blueboat (descomenta cuando Ethernet esté activo) ───
      // if (eth_connected) {
      //   udp.beginPacket(IPAddress(192,168,1,100), 5005);
      //   udp.write((uint8_t*)&pkt, sizeof(pkt));
      //   udp.endPacket();
      // }
    }
  }
}