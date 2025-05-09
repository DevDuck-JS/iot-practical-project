import requests
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

# Fetch initial threshold from database
cursor.execute("SELECT distance FROM trigger_threshold ORDER BY update_at DESC LIMIT 1")
result = cursor.fetchone()
if result:
    initial_threshold = result[0]
    print("Sending threshold to Arduino:", initial_threshold)
    ser.write(f"THRESHOLD:{initial_threshold}\n".encode())


while True:
    line = ser.readline().decode('utf-8', errors='ignore').strip()

    if line.startswith("PASSWORD:"):
        entered_password = line.split(":")[1]
        print("Received password:", entered_password)

        cursor.execute("SELECT user_type FROM user WHERE password = %s LIMIT 1", (entered_password,))
        result = cursor.fetchone()

        if result:

            user_type = result[0]
            print("Matched user_type:", user_type)


            timestamp = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
            cursor.execute(
                "INSERT INTO access_log (user_type, access_time) VALUES (%s, %s)",
                (user_type, timestamp)
            )
            db.commit()

        else:
            user_type = "Denied"
            print("Password not correct.")

        # Send response to Arduino
        ser.write((user_type + "\n").encode())

    elif line.startswith("CHANGE:"):
        try:
            _, user_type, new_password = line.split(":")
            now = datetime.now().strftime('%Y-%m-%d %H:%M:%S')

            cursor.execute(
                "UPDATE user SET password = %s, update_at = %s WHERE user_type = %s",
                (new_password, now, user_type)
            )
            db.commit()

            # Optional: log the change in access_log table
            cursor.execute(
                "INSERT INTO access_log (user_type, access_time) VALUES (%s, %s)",
                (f"{user_type} (changed password)", now)
            )
            db.commit()

            print(f"{user_type}'s password updated to {new_password} at {now}")

        except Exception as e:
            print("Error processing change request:", e)

    elif line.startswith("DISTANCE:"):
        try:
            distance_val = line.split(":")[1]
            print("Distance: " + distance_val + " cm.")
        except IndexError:
            print("Malformed distance data.")
