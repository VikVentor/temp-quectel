import serial
import time

# Function to send an AT command and read the response
def send_at_command(ser, command):
    print(f"Sending command: {command}")
    ser.write((command + '\r').encode())  # Send the command with a carriage return
    time.sleep(1)  # Wait for the device to respond
    response = ser.read(ser.in_waiting)  # Read available response
    print(f"Response: {response.decode()}\n")

# Open the serial port (update with your serial port, e.g., /dev/ttyUSB0 or COMx)
serial_port = "/dev/ttyUSB0"  # Update to the correct serial port
baud_rate = 115200            # Baud rate for the communication

try:
    # Set up the serial connection
    ser = serial.Serial(serial_port, baud_rate, timeout=1)
    print(f"Connected to {serial_port} at {baud_rate} baud rate.\n")

    # Send AT commands
    send_at_command(ser, "AT")           # Send basic AT command
    send_at_command(ser, "AT+QGPS=1")    # Enable GPS
    send_at_command(ser, "AT+QGPSLOC=0") # Get GPS location
    send_at_command(ser, "AT+QGPSEND")   # End GPS session

except serial.SerialException as e:
    print(f"Error: {e}")
finally:
    if ser.is_open:
        ser.close()  # Close the serial port when done
        print(f"Connection to {serial_port} closed.")
