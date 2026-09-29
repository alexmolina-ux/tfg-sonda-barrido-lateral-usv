import socket, time, math

UDP_IP = "127.0.0.1"
UDP_PORT = 8080

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

t = 0
print("Enviando datos simulados a Unity. Pulsa Ctrl+C para detener.")
while True:
    roll  = 15 * math.sin(t * 0.15)
    pitch = 8  * math.sin(t * 0.10)
    yaw   = (t * 5) % 360

    mensaje = f"{roll:.2f},{pitch:.2f},{yaw:.2f}"
    sock.sendto(mensaje.encode(), (UDP_IP, UDP_PORT))
    print(mensaje)

    t += 0.05
    time.sleep(0.05)