#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>

// Function to configure the serial port
int configure_serial(const char *port) {
    int fd = open(port, O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd == -1) {
        perror("Unable to open serial port");
        return -1;
    }

    struct termios options;
    tcgetattr(fd, &options);
    options.c_cflag = B115200 | CS8 | CLOCAL | CREAD;  // Set baud rate, 8 data bits, no parity
    options.c_iflag = IGNPAR;                          // Ignore parity errors
    options.c_oflag = 0;                               // No output processing
    options.c_lflag = ICANON;                          // Canonical mode

    tcflush(fd, TCIFLUSH);
    tcsetattr(fd, TCSANOW, &options);

    return fd;
}

// Function to send an AT command and read the response
void send_at_command(int fd, const char *command) {
    write(fd, command, strlen(command));  // Send the command
    usleep(1000000);  // Wait for the device to respond

    char buffer[1024];
    int n = read(fd, buffer, sizeof(buffer) - 1);
    if (n > 0) {
        buffer[n] = '\0';  // Null-terminate the string
        printf("Response: %s\n", buffer);
    }
}

// Function to convert GPS coordinates to decimal degrees
double convert_to_decimal(int degrees, double minutes, char direction) {
    double decimal = degrees + (minutes / 60.0);
    if (direction == 'S' || direction == 'W') {  // South and West are negative
        decimal = -decimal;
    }
    return decimal;
}

int main() {
    const char *serial_port = "/dev/ttyUSB2";  // Update to your serial port
    int fd = configure_serial(serial_port);
    if (fd == -1) {
        return -1;  // Exit if serial port can't be opened
    }

    printf("Connected to %s at 115200 baud rate.\n", serial_port);

    // Send the AT command to get GPS location
    send_at_command(fd, "AT+QGPSLOC=0\r");

    // Read the response and parse it
    char buffer[1024];
    int n = read(fd, buffer, sizeof(buffer) - 1);
    if (n > 0) {
        buffer[n] = '\0';  // Null-terminate the string
        printf("Response: %s\n", buffer);

        if (strstr(buffer, "+QGPSLOC:")) {
            // Extract GPS data string after "+QGPSLOC:"
            char *gps_data = strchr(buffer, ':') + 1;
            gps_data = strtok(gps_data, "\r\n"); // Remove any trailing newline/carriage return
            printf("Parsed GPS data: %s\n", gps_data);

            double lat_decimal = 0.0, lon_decimal = 0.0;
            int lat_degrees, lon_degrees;
            double lat_minutes, lon_minutes;
            char lat_direction, lon_direction;

            // Parse latitude
            int parsed = sscanf(gps_data, "%2d%lf%c", &lat_degrees, &lat_minutes, &lat_direction);
            if (parsed == 3) {
                lat_decimal = convert_to_decimal(lat_degrees, lat_minutes, lat_direction);
                printf("Latitude: %.6f\n", lat_decimal);
            } else {
                printf("Failed to parse latitude correctly.\n");
            }

            // Move to longitude (assuming it's the second part of the response)
            char *lon_data = strchr(gps_data, ',') + 1;  // Move past the comma separator
            parsed = sscanf(lon_data, "%3d%lf%c", &lon_degrees, &lon_minutes, &lon_direction);
            if (parsed == 3) {
                lon_decimal = convert_to_decimal(lon_degrees, lon_minutes, lon_direction);
                printf("Longitude: %.6f\n", lon_decimal);
            } else {
                printf("Failed to parse longitude correctly.\n");
            }
        } else {
            printf("GPS data not found in response.\n");
        }
    } else {
        printf("Failed to get GPS data\n");
    }

    // Close the serial port
    close(fd);
    printf("Connection to %s closed.\n", serial_port);

    return 0;
}
