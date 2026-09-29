#include <ETH.h>

// Configuración de los pines exactos para el hardware de la WT32-ETH01
#define ETH_ADDR        1
#define ETH_POWER_PIN   16
#define ETH_MDC_PIN     23
#define ETH_MDIO_PIN    18
#define ETH_TYPE        ETH_PHY_LAN8720
#define ETH_CLK_MODE    ETH_CLOCK_GPIO0_IN

void setup() {
  Serial.begin(115200);

  // Definir la IP fija
  IPAddress ip(192, 168, 1, 10);
  IPAddress gateway(192, 168, 1, 1);
  IPAddress subnet(255, 255, 255, 0);

  // LÍNEA CORREGIDA: Nuevo orden de parámetros para ESP32 Core 3.x
  ETH.begin(ETH_TYPE, ETH_ADDR, ETH_MDC_PIN, ETH_MDIO_PIN, ETH_POWER_PIN, ETH_CLK_MODE);
  
  // Aplicar la configuración de la IP
  ETH.config(ip, gateway, subnet);

  Serial.println("Puerto Ethernet encendido y configurado.");
}

void loop() {
  // Vacío para la prueba de ping
}