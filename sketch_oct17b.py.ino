import tkinter as tk
from tkinter import ttk
from tkinter import messagebox
import serial
import matplotlib.pyplot as plt
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
import smtplib
from email.mime.text import MIMEText
from email.mime.multipart import MIMEMultipart

def send_email(subject, message, to_email, smtp_server, smtp_port, sender_email, sender_password):
    try:
        # Create a MIME message object
        msg = MIMEMultipart()
        msg['From'] = sender_email
        msg['To'] = to_email
        msg['Subject'] = subject

        # Attach the email message
        msg.attach(MIMEText(message, 'plain'))

        # Establish an SMTP connection
        server = smtplib.SMTP(smtp_server, smtp_port)
        server.starttls()  # Upgrade the connection to a secure TLS connection

        server.login(sender_email, sender_password)

        server.sendmail(sender_email, to_email, msg.as_string())

        server.quit()

        print("Email sent successfully!")
    except Exception as e:
        print(f"Error: {str(e)}")

# Function to send data over serial
def send_serial_data():
    global running
    while running:
        try:
            # Read values from the edit boxes
            s1 = s1_entry.get()
            s2 = s2_entry.get()
            s3 = s3_entry.get()
            s4 = s4_entry.get()

            # Check if the values are numeric and not empty
            if s1.isdigit() and s2.isdigit() and s3.isdigit() and s4.isdigit():
                data = f"*{s1},{s2},{s3},{s4}#"
                ser.write(data.encode('utf-8'))
                print(f"Sent: {data}")
                break
            else:
                messagebox.showerror("Error", "Threshold values must be numeric and not empty.")

            # Wait for a few seconds
            if running:
                root.after(5000)  # Adjust the delay as needed
        except Exception as e:
            print("Error sending data:", e)

# Function to read and plot data from serial
def read_and_plot_serial_data():
    global running,Flag1, Flag2, Flag3
    Flag1=0
    Flag2=0
    Flag3=0
    while running:
        try:
            data = ser.readline().decode('utf-8').strip()
            if data.startswith('*') and data.endswith('#'):
                data = data[1:-1]
                values = data.split(',')
                if len(values) == 3:
                    s1, s2, s3 = map(float, values)
                    sensor1_data.append(s1)
                    sensor2_data.append(s2)
                    sensor3_data.append(s3)
                    update_graphs()
                    update_sensor_status(s1, s2, s3)
        except Exception as e:
            print("Error reading data:", e)

# Function to update the graphs
def update_graphs():
    if len(sensor1_data) > max_samples:
        del sensor1_data[0]
        del sensor2_data[0]
        del sensor3_data[0]

    ax1.clear()
    ax1.plot(sensor1_data)
    ax1.set_title("Sensor 1 Data")

    ax2.clear()
    ax2.plot(sensor2_data)
    ax2.set_title("Sensor 2 Data")

    ax3.clear()
    ax3.plot(sensor3_data)
    ax3.set_title("Sensor 3 Data")

    canvas.draw()

# Function to update sensor status labels
def update_sensor_status(s1, s2, s3):
    global Flag1, Flag2, Flag3
    if s1 > float(s1_entry.get()):
        s1_status = "High Temperature"
        Mes="Check Aquarium High Temperature detected"
        if Flag1==0:
            print('High temp')
            send_email(subject, Mes, to_email, smtp_server, smtp_port, sender_email, sender_password)
            Flag1=1
            Flag2=0
            Flag3=0
    else:
        s1_status = "Normal Temperature"
        
        
    if s2 <= float(s2_entry.get()):
        s2_status = "Low Light Detected" 
        Mes="Check Aquarium Low Light detected"
        if Flag2==0:
            print('Low Light')
            send_email(subject, Mes, to_email, smtp_server, smtp_port, sender_email, sender_password)
            Flag1=0
            Flag2=1
            Flag3=0
    else:
        s2_status = "Normal Light" 
        
    
    if (s3 <= float(s3_entry.get())-10 and s3>30):
        s3_status = "Good Quality Water" 
    elif (s3 >= float(s3_entry.get())):
        s3_status = "Bad Quality Water" 
        Mes="Check Aquarium Bad Quality Water detected"
        if Flag3==0:
            print('bad Quality')
            send_email(subject, Mes, to_email, smtp_server, smtp_port, sender_email, sender_password)
            Flag1=0
            Flag2=0
            Flag3=1
    else:
        s3_status = "Insert TDS" 
     
    s1_status_label.config(text=f"Sensor 1 Status: {s1_status}")
    s2_status_label.config(text=f"Sensor 2 Status: {s2_status}")
    s3_status_label.config(text=f"Sensor 3 Status: {s3_status}")

# Function to start data collection
def start_collection():
    global running
    running = True
    send_serial_data()
    read_and_plot_serial_data()

# Main Code Starts
# Serial port configuration
ser = serial.Serial('/dev/ttyACM0', 9600)

# Replace these values with your Yahoo email account information
smtp_server = 'smtp.office365.com'
smtp_port = 587
sender_email = 'matanginigupta@outlook.com'
sender_password = 'Matangini@18111'

# Example usage
subject = 'Test Email'
to_email = 'testsetup1811@gmail.com'

# Initialize the GUI
root = tk.Tk()
root.title("Aquarium Health Tracking System")

# Create threshold input fields
s1_label = tk.Label(root, text="Threshhold For Temperature:")
s1_entry = tk.Entry(root)
s2_label = tk.Label(root, text="Threshhold For Lux:")
s2_entry = tk.Entry(root)
s3_label = tk.Label(root, text="Threshhold For Water Quality:")
s3_entry = tk.Entry(root)
s4_label = tk.Label(root, text="Time Delay for Feeder:")
s4_entry = tk.Entry(root)

# Create start and stop buttons
start_button = tk.Button(root, text="Start", command=start_collection, width=30)

# Create figure and axes for graphs
fig, (ax1, ax2, ax3) = plt.subplots(3, 1, figsize=(4, 3))
canvas = FigureCanvasTkAgg(fig, master=root)
canvas.get_tk_widget().pack()

# Initialize data lists
max_samples = 100
sensor1_data = []
sensor2_data = []
sensor3_data = []

# Initialize the running flag
running = False

# Create labels for sensor status
s1_status_label = tk.Label(root, text="Sensor 1 Status: N/A")
s2_status_label = tk.Label(root, text="Sensor 2 Status: N/A")
s3_status_label = tk.Label(root, text="Sensor 3 Status: N/A")

# Place widgets on the GUI
s1_label.pack()
s1_entry.pack()
s2_label.pack()
s2_entry.pack()
s3_label.pack()
s3_entry.pack()
s4_label.pack()
s4_entry.pack()
start_button.pack()

# Place sensor status labels on the GUI
s1_status_label.pack()
s2_status_label.pack()
s3_status_label.pack()

# Start the GUI main loop
root.mainloop()

