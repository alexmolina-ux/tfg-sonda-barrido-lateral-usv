#include <ETH.h>
#include <WiFiUdp.h> // Librería para inyectar paquetes UDP

// Configuración de los pines exactos para el hardware de la WT32-ETH01
#define ETH_ADDR        1
#define ETH_POWER_PIN   16
#define ETH_MDC_PIN     23
#define ETH_MDIO_PIN    18
#define ETH_TYPE        ETH_PHY_LAN8720
#define ETH_CLK_MODE    ETH_CLOCK_GPIO0_IN

WiFiUDP udp;

// La IP de tu Mac y el puerto por donde entrarán los datos
const char* mac_ip = "192.168.1.11"; 
const int udp_port = 8888;          

unsigned long contador_paquetes = 0;

void setup() {
  // IPs manuales
  IPAddress ip(192, 168, 1, 10);
  IPAddress gateway(192, 168, 1, 1);
  IPAddress subnet(255, 255, 255, 0);

  // Inicializar Ethernet (con el orden corregido para tu versión)
  ETH.begin(ETH_TYPE, ETH_ADDR, ETH_MDC_PIN, ETH_MDIO_PIN, ETH_POWER_PIN, ETH_CLK_MODE);
  ETH.config(ip, gateway, subnet);

  // Le damos 2 segundos al switch para negociar el enlace físico
  delay(2000); 
  
  udp.begin(udp_port);
}

void loop() {
  // Simulamos un paquete pesado del sonar. 
  // Cada ciclo sumará un número al contador para saber cuántos han llegado.
  String carga_datos = "SONAR_DATA_PACKET_" + String(contador_paquetes) + "_[RAW_BYTES:1101001010101110101010101010]\n";
  
  // Abrimos conexión, inyectamos el paquete y disparamos
  udp.beginPacket(mac_ip, udp_port);
  udp.print(carga_datos);
  udp.endPacket();

  contador_paquetes++;

  // Pausa de 1 milisegundo = Dispara unos 1000 paquetes por segundo
  delay(1); 
}