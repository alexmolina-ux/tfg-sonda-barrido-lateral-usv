#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Mon Sep 21 09:26:28 2026

@author: alexmolinalopez
"""

import socket, time, math

UDP_IP = "127.0.0.1"
UDP_PORT = 8080

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

t = 0
print("Enviando datos simulados (5 campos) a Unity. Pulsa Ctrl+C para detener.")
while True:
    roll  = 15 * math.sin(t * 0.15)
    pitch = 8  * math.sin(t * 0.10)
    yaw   = (t * 5) % 360

    # NUEVO: profundidad y temperatura sintéticas, para ejercitar
    # también SetDepth() y SetTemperature() en TorpedoIMU.cs
    profundidad = 2.0 + 1.5 * math.sin(t * 0.05)   # oscila entre 0.5 y 3.5 m
    temperatura = 18.0 + 0.5 * math.sin(t * 0.02)  # oscila alrededor de 18 °C

    mensaje = f"{roll:.2f},{pitch:.2f},{yaw:.2f},{profundidad:.2f},{temperatura:.2f}"
    sock.sendto(mensaje.encode(), (UDP_IP, UDP_PORT))
    print(mensaje)

    t += 0.05
    time.sleep(0.05)