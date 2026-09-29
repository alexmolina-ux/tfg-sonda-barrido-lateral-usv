import socket
import pandas as pd
from datetime import datetime
import csv
import os  # Añadimos esta librería para poder gestionar carpetas en el Mac

# Configuración de red
UDP_IP = "0.0.0.0"
UDP_PORT = 8080

# Inicialización del Socket UDP
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((UDP_IP, UDP_PORT))

print(f"Escuchando telemetría en el puerto {UDP_PORT}...")
print("Pulsa Ctrl+C en la terminal en cualquier momento para finalizar la misión y generar el Excel.")

# ---------------- NUEVO: GESTIÓN DE CARPETAS ---------------- #
carpeta_guardado = "Datos_Misiones" # Nombre de la subcarpeta que crearemos

# Comprobamos si la carpeta existe. Si no existe, Python la crea automáticamente.
if not os.path.exists(carpeta_guardado):
    os.makedirs(carpeta_guardado)
    print(f"-> Se ha creado la subcarpeta '{carpeta_guardado}' para guardar los datos.")

# Generamos un nombre base único con la fecha y hora de la prueba
nombre_base = f"Mision_Towfish_{datetime.now().strftime('%Y%m%d_%H%M%S')}"

# Unimos el nombre de la carpeta con el nombre de los archivos
archivo_csv = os.path.join(carpeta_guardado, f"{nombre_base}.csv")
archivo_excel = os.path.join(carpeta_guardado, f"{nombre_base}.xlsx")
# ------------------------------------------------------------- #

# Abrimos el CSV de seguridad en modo escritura continua
with open(archivo_csv, mode='w', newline='') as file:
    writer = csv.writer(file)
    # Escribimos la cabecera de las columnas
    writer.writerow(["Tiempo", "Roll", "Pitch", "Yaw", "Profundidad", "Temperatura"])
    
    try:
        while True:
            # Recibimos el datagrama UDP desde el ESP32
            data, addr = sock.recvfrom(1024)
            mensaje = data.decode('utf-8').strip()
            
            # Generamos la marca de tiempo exacta (Hora:Minuto:Segundo.Milisegundo)
            timestamp = datetime.now().strftime("%H:%M:%S.%f")[:-3]
            
            print(f"[{timestamp}] Datos recibidos: {mensaje}")
            
            # Separamos los valores que vienen del torpedo por comas
            valores = mensaje.split(',')
            
            # Comprobamos que llegaron los 5 datos correctamente
            if len(valores) == 5:
                fila = [timestamp] + valores
                writer.writerow(fila)
                file.flush() # Forzamos el guardado en el disco duro al instante por seguridad
                
    except KeyboardInterrupt:
        # Esto es lo que ocurre cuando pulsas Ctrl + C
        print("\n\nDeteniendo la captura de datos...")

# Al pulsar Ctrl+C, se sale del bucle infinito y pasamos a generar el Excel
print(f"Generando archivo Excel final...")
try:
    # Leemos el archivo CSV seguro que hemos ido creando
    df = pd.read_csv(archivo_csv)
    # Lo convertimos a Excel
    df.to_excel(archivo_excel, index=False)
    print("¡Éxito! Misión guardada y formateada correctamente.")
    print(f"Tienes tus archivos dentro de la carpeta: Pruebas_Towfish/{carpeta_guardado}/")
except Exception as e:
    print(f"Error al generar el Excel: {e}")