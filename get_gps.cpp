#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>

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
std::string send_at_command(int fd, const std::string &command) {
    write(fd, command.c_str(), command.length());  // Send the command
    usleep(1000000);  // Wait for the device to respond

    char buffer[1024];
    int n = read(fd, buffer, sizeof(buffer) - 1);
    buffer[n] = '\0';  // Null-terminate the string
    return std::string(buffer);
}

int main() {
    const char *serial_port = "/dev/ttyUSB0";  // Update to your serial port
    int fd = configure_serial(serial_port);
    if (fd == -1) {
        return -1;  // Exit if serial port can't be opened
    }

    std::cout << "Connected to " << serial_port << " at 115200 baud rate.\n";

    // Send AT commands
    std::string response;

    // Send basic AT command
    response = send_at_command(fd, "AT\r");
    std::cout << "Response: " << response << std::endl;

    // Enable GPS
    response = send_at_command(fd, "AT+QGPS=1\r");
    std::cout << "Response: " << response << std::endl;

    // Get GPS location
    response = send_at_command(fd, "AT+QGPSLOC=0\r");
    std::cout << "Response: " << response << std::endl;

    // End GPS session
    response = send_at_command(fd, "AT+QGPSEND\r");
    std::cout << "Response: " << response << std::endl;

    // Close the serial port
    close(fd);
    std::cout << "Connection to " << serial_port << " closed.\n";

    return 0;
}
