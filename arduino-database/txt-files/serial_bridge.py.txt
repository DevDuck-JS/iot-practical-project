import serial
import time

def send_command(cmd):
    try:
        ser = serial.Serial('/dev/cu.usbmodem1201', 9600, timeout=1)
        time.sleep(2)  # Wait for Arduino reset
        ser.write((cmd + "\n").encode())
        ser.close()
    except Exception as e:
        print("Serial write error:", e)