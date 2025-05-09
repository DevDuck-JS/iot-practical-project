from flask import Flask, render_template, request, redirect, make_response, jsonify
import mysql.connector
from serial_bridge import send_command
from datetime import datetime

app = Flask(__name__)

# MySQL connection
db = mysql.connector.connect(
    host="localhost",
    user="root",
    password="",
    database="arduino"
)
cursor = db.cursor(dictionary=True)

@app.route('/')
def root():
    return redirect('/dashboard')

@app.route('/dashboard')
def dashboard():
    # Reset unlock state (optional)
    cursor.execute("UPDATE system_state SET unlocked = FALSE WHERE id = 1")
    db.commit()

    cursor.execute("SELECT * FROM access_log ORDER BY access_time DESC LIMIT 10")
    logs = cursor.fetchall()

    cursor.execute("SELECT distance FROM trigger_threshold WHERE id = 1")
    threshold_row = cursor.fetchone()
    current_threshold = threshold_row['distance'] if threshold_row else 'N/A'

    rendered = render_template('dashboard.html', logs=logs, threshold=current_threshold)

    response = make_response(rendered)
    response.headers["Cache-Control"] = "no-store, no-cache, must-revalidate, max-age=0"
    response.headers["Pragma"] = "no-cache"
    response.headers["Expires"] = "0"
    return response

@app.route('/check-unlocked')
def check_unlocked():
    cursor.execute("SELECT unlocked FROM system_state WHERE id = 1")
    result = cursor.fetchone()
    is_unlocked = result['unlocked'] if result else False
    return jsonify({'unlocked': is_unlocked})

@app.route('/trigger-buzzer', methods=['POST'])
def trigger_buzzer():
    send_command("BUZZER:ON")
    return redirect('/dashboard')

@app.route('/set-threshold', methods=['POST'])
def set_threshold():
    threshold = int(request.form.get("threshold"))
    now = datetime.now().strftime('%Y-%m-%d %H:%M:%S')

    cursor.execute("UPDATE trigger_threshold SET distance = %s, update_at = %s WHERE id = 1", (threshold, now))
    db.commit()

    send_command(f"THRESHOLD:{threshold}")
    return redirect('/dashboard')

@app.route('/refresh-log', methods=['POST'])
def refresh_log():
    return redirect('/dashboard')

if __name__ == '__main__':
    app.run(debug=True)
