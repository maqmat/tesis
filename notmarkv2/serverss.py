import cv2
from http.server import BaseHTTPRequestHandler, HTTPServer
from socketserver import ThreadingMixIn
import threading
import os
import serial
import socket
import time

# === CONFIGURACIÓN =
HOST = "0.0.0.0"
PORT = 8000

# === CAPTURA DE VIDEO ===
cap = cv2.VideoCapture(1)
cap.set(cv2.CAP_PROP_FRAME_WIDTH, 320)
cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 240)

last_frame = None

def capture_frames():
    global last_frame
    while True:
        ret, frame = cap.read()
        if not ret:
            continue
        _, jpeg = cv2.imencode('.jpg', frame, [int(cv2.IMWRITE_JPEG_QUALITY), 50])
        if _:
            last_frame = jpeg.tobytes()
        # No delay aquí para máxima velocidad

# === CONEXIÓN SERIAL DIRECTA CON ARDUINO ===
try:
    arduino = serial.Serial('COM3', 9600, timeout=1)  # Cambia por tu puerto COM
    print("[✓] Arduino conectado")
except Exception as e:
    print(f"[!] Error al conectar con Arduino: {e}")
    arduino = None

# === LECTURA RÁPIDA DE GAS ===
last_gas_value = "0"

def arduino_gas_reader():
    global last_gas_value
    while True:
        if arduino and arduino.in_waiting > 0:
            line = arduino.readline().decode('utf-8').strip()
            if line.startswith("gas,"):
                gas_value = line.split(",")[1]
                last_gas_value = gas_value
        else:
            time.sleep(0.01)  # Pequeño respiro para la CPU

# === HANDLER DE PETICIONES ===
class RequestHandler(BaseHTTPRequestHandler):
    def do_GET(self):
        print(f"[DEBUG] Ruta solicitada: {self.path}")

        # === Página principal ===
        if self.path == "/":
            self.send_response(200)
            self.send_header("Content-type", "text/html")
            self.end_headers()
            with open("index.html", "rb") as f:
                self.wfile.write(f.read())

        # === Transmisión MJPEG ===
        elif self.path.startswith("/camara"):
            self.send_response(200)
            self.send_header("Content-type", "multipart/x-mixed-replace; boundary=frame")
            self.end_headers()

            try:
                while True:
                    if last_frame:
                        self.wfile.write(b"--frame\r\n")
                        self.send_header("Content-type", "image/jpeg")
                        self.send_header("Content-length", str(len(last_frame)))
                        self.end_headers()
                        self.wfile.write(last_frame)
                        self.wfile.write(b"\r\n")
                        self.wfile.flush()
                    time.sleep(0.02)  
            except:
                print("[!] Cliente desconectado")

        # === Comandos para los servos ===
        elif self.path.startswith("/command/"):
            command = self.path.split("/")[-1]
            print(f"[+] Comando recibido: {command}")

            if arduino and arduino.is_open:
                try:
                    arduino.write((command + '\n').encode())
                    arduino.flush()
                    print(f"[✓] Comando '{command}' enviado al Arduino")
                except Exception as e:
                    print(f"[!] Error al escribir en Arduino: {e}")
            else:
                print("[!] Arduino no disponible")

            self.send_response(200)
            self.send_header("Content-type", "text/plain")
            self.send_header("Content-Length", "2")
            self.send_header("Connection", "close")
            self.end_headers()
            self.wfile.write(b"OK")
            self.wfile.flush()

        # === Archivos estáticos ===
        elif self.path.startswith("/static/"):
            file_path = self.path[1:]
            if os.path.exists(file_path):
                with open(file_path, 'rb') as f:
                    self.send_response(200)
                    if file_path.endswith(".css"):
                        self.send_header("Content-type", "text/css")
                    elif file_path.endswith(".jpg"):
                        self.send_header("Content-type", "image/jpeg")
                    self.end_headers()
                    self.wfile.write(f.read())
            else:
                self.send_error(404, "Archivo no encontrado")
        
        elif self.path == "/gas":
            self.send_response(200)
            self.send_header("Content-type", "text/plain")
            self.end_headers()
            self.wfile.write(f"{last_gas_value}\n".encode())

# === SERVIDOR MULTIHILADO ===
class ThreadedHTTPServer(ThreadingMixIn, HTTPServer):
    daemon_threads = True

def run_server():
    server_address = (HOST, PORT)
    httpd = ThreadedHTTPServer(server_address, RequestHandler)
    print(f"[Servidor web] Escuchando en http://{HOST}:{PORT}")
    httpd.serve_forever()

if __name__ == "__main__":
    # Hilo para captura de video
    capture_thread = threading.Thread(target=capture_frames, daemon=True)
    capture_thread.start()

    # Hilo para lectura de gas
    if arduino:
        gas_thread = threading.Thread(target=arduino_gas_reader, daemon=True)
        gas_thread.start()

    # Iniciar servidor web
    run_server()
