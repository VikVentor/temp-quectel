#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>

// Function to configure the serial port
int configure_serial_port(const char *device) {
    struct termios options;
    int fd = open(device, O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd == -1) {
        perror("Unable to open serial port");
        return -1;
    }

    // Get current serial port settings
    if (tcgetattr(fd, &options) < 0) {
        perror("Unable to get serial port settings");
        close(fd);
        return -1;
    }

    // Set baud rate
    cfsetispeed(&options, B115200);   // Input speed (baud rate)
    cfsetospeed(&options, B115200);   // Output speed (baud rate)

    // Set 8 data bits, no parity, 1 stop bit
    options.c_cflag &= ~PARENB;       // No parity
    options.c_cflag &= ~CSTOPB;       // 1 stop bit
    options.c_cflag &= ~CSIZE;        // Clear current data size
    options.c_cflag |= CS8;           // 8 data bits

    // Disable hardware flow control
    options.c_cflag &= ~CRTSCTS;
    // Disable software flow control
    options.c_iflag &= ~(IXON | IXOFF | IXANY);

    // Enable receiver, disable modem control lines
    options.c_cflag |= CREAD | CLOCAL;

    // Set the new options
    if (tcsetattr(fd, TCSANOW, &options) < 0) {
        perror("Unable to set serial port settings");
        close(fd);
        return -1;
    }

    return fd;
}

// Function to send an AT command and get the response
void send_at_command(int fd, const char *command) {
    char response[256];

    // Send the AT command
    write(fd, command, strlen(command));
    write(fd, "\r", 1); // Send carriage return

    // Wait for response
    usleep(1000000); // Sleep for 1 second to allow the device to respond

    // Read the response
    int bytes_read = read(fd, response, sizeof(response) - 1);
    if (bytes_read > 0) {
        response[bytes_read] = '\0'; // Null-terminate the string
        printf("Response: %s\n", response);
    } else {
        printf("No response or error reading the serial port\n");
    }
}

int main() {
    const char *serial_device = "/dev/ttyUSB0"; // Update with your serial device path

    // Configure the serial port
    int fd = configure_serial_port(serial_device);
    if (fd == -1) {
        return 1; // Exit if the serial port configuration failed
    }

    // Send some AT commands and get the response
    send_at_command(fd, "AT");  // Test AT command
    send_at_command(fd, "AT+QGPS=1");  // Enable GPS
    send_at_command(fd, "AT+QGPSLOC=0");  // Get GPS location
    send_at_command(fd, "AT+QGPSEND");  // End GPS session

    // Close the serial port
    close(fd);

    return 0;
}
