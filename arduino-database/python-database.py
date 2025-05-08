import serial
import mysql.connector
from datetime import datetime

# Open serial connection (adjust to your port)
ser = serial.Serial('/dev/cu.usbmodem1201', 9600, timeout=1)  # Use COMx for Windows

# Connect to MySQL
db = mysql.connector.connect(
    host="localhost",
    user="root",
    password="",
    database="arduino"
)

cursor = db.cursor()

while True:
    line = ser.readline().decode('utf-8').strip()
    if line:
        print("Received:", line)

        if line.startswith("ACCESS:"):
            access_type = line.split(":")[1]
            timestamp = datetime.now().strftime('%Y-%m-%d %H:%M:%S')

            sql = "INSERT INTO access_log (user_type, access_time) VALUES (%s, %s)"
            val = (access_type, timestamp)

            cursor.execute(sql, val)
            db.commit()
            print("Logged to DB:", val)
