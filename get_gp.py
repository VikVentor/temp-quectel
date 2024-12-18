import serial
import time

# Function to send an AT command and read the response
def send_at_command(ser, command):
    print(f"Sending command: {command}")
    ser.write((command + '\r').encode())  # Send the command with a carriage return
    time.sleep(1)  # Wait for the device to respond
    response = ser.read(ser.in_waiting)  # Read available response
    return response.decode()

# Function to convert GPS coordinates to decimal degrees
def convert_to_decimal(degrees, minutes, direction):
    decimal = degrees + (minutes / 60)
    if direction in ['S', 'W']:  # South and West are negative
        decimal = -decimal
    return decimal

# Open the serial port (update with your serial port, e.g., /dev/ttyUSB0 or COMx)
serial_port = "/dev/ttyUSB2"  # Update to the correct serial port
baud_rate = 115200            # Baud rate for the communication

try:
    # Set up the serial connection
    ser = serial.Serial(serial_port, baud_rate, timeout=1)
    print(f"Connected to {serial_port} at {baud_rate} baud rate.\n")

    # Send AT command to get GPS location
    response = send_at_command(ser, "AT+QGPSLOC=0")  # Get GPS location
    print(f"Response: {response}")

    # Parse the response to extract latitude and longitude
    if "+QGPSLOC:" in response:
        gps_data = response.split(":")[1].strip()
        parts = gps_data.split(",")
        
        # Extract latitude and longitude
        lat_str = parts[1]
        lon_str = parts[2]

        # Latitude (e.g., 1258.8198N)
        lat_degrees = int(lat_str[:2])
        lat_minutes = float(lat_str[2:-1])
        lat_direction = lat_str[-1]

        # Longitude (e.g., 07743.6126E)
        lon_degrees = int(lon_str[:3])
        lon_minutes = float(lon_str[3:-1])
        lon_direction = lon_str[-1]

        # Convert latitude and longitude to decimal degrees
        lat_decimal = convert_to_decimal(lat_degrees, lat_minutes, lat_direction)
        lon_decimal = convert_to_decimal(lon_degrees, lon_minutes, lon_direction)

        # Print the converted values
        print(f"Latitude: {lat_decimal}")
        print(f"Longitude: {lon_decimal}")

except serial.SerialException as e:
    print(f"Error: {e}")
finally:
    if ser.is_open:
        ser.close()  # Close the serial port when done
        print(f"Connection to {serial_port} closed.")
