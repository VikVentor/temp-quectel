#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <iomanip>
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

    std::cout << "Connected to " << serial_port << " at 115200 baud rate.\n";

    // Send the AT command to get GPS location
    std::string response = send_at_command(fd, "AT+QGPSLOC=0\r");

    // Display raw response for debugging
    std::cout << "Response: " << response << std::endl;

    // Check if we have a valid GPS location response
    std::string gps_data_marker = "+QGPSLOC:";
    size_t pos = response.find(gps_data_marker);
    if (pos != std::string::npos) {
        // Extract GPS data after the marker
        std::string gps_data = response.substr(pos + gps_data_marker.length());

        // Parse latitude and longitude
        std::istringstream gps_stream(gps_data);
        std::string lat_str, lon_str;
        std::getline(gps_stream, lat_str, ',');
        std::getline(gps_stream, lon_str);

        // Parse latitude (e.g., 1258.8198N)
        int lat_degrees = std::stoi(lat_str.substr(0, 2));
        double lat_minutes = std::stod(lat_str.substr(2, lat_str.length() - 3));
        char lat_direction = lat_str[lat_str.length() - 1];

        // Parse longitude (e.g., 07743.6126E)
        int lon_degrees = std::stoi(lon_str.substr(0, 3));
        double lon_minutes = std::stod(lon_str.substr(3, lon_str.length() - 4));
        char lon_direction = lon_str[lon_str.length() - 1];

        // Convert to decimal degrees
        double lat_decimal = convert_to_decimal(lat_degrees, lat_minutes, lat_direction);
        double lon_decimal = convert_to_decimal(lon_degrees, lon_minutes, lon_direction);

        // Print the results
        std::cout << "Latitude: " << std::fixed << std::setprecision(6) << lat_decimal << std::endl;
        std::cout << "Longitude: " << std::fixed << std::setprecision(6) << lon_decimal << std::endl;
    } else {
        std::cout << "GPS data not found in the response.\n";
    }

    // Close the serial port
    close(fd);
    std::cout << "Connection to " << serial_port << " closed.\n";

    return 0;
}
