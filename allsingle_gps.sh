#!/bin/bash

# Define the device port and baud rate
PORT="/dev/ttyUSB2"
BAUD=115200

# Send the commands to the device
(echo -e "AT+QGPS=1\r"; sleep 1; echo -e "AT+QGPSLOC=0\r") | \
  screen $PORT $BAUD 2>/dev/null | \
  awk -v RS="\r" '/\+QGPSLOC:/ {
      # Extract latitude and longitude from the output
      split($2, data, ",");
      lat_raw = data[2];
      lon_raw = data[3];

      # Convert latitude to decimal degrees
      lat_deg = substr(lat_raw, 1, 2);
      lat_min = substr(lat_raw, 3) / 60;
      lat_dd = lat_deg + lat_min;
 # Extract latitude and longitude from the output
      split($2, data, ",");
      lat_raw = data[2];
      lon_raw = data[3];

      # Convert latitude to decimal degrees
      lat_deg = substr(lat_raw, 1, 2);
      lat_min = substr(lat_raw, 3) / 60;
      lat_dd = lat_deg + lat_min;
      if (substr(lat_raw, length(lat_raw), 1) == "S") lat_dd = -lat_dd;

      # Convert longitude to decimal degrees
      lon_deg = substr(lon_raw, 1, 3);
      lon_min = substr(lon_raw, 4) / 60;
      lon_dd = lon_deg + lon_min;
      if (substr(lon_raw, length(lon_raw), 1) == "W") lon_dd = -lon_dd;

      # Print the output
      printf "lat=%.6f\nlon=%.6f\n", lat_dd, lon_dd;
   # Convert longitude to decimal degrees
      lon_deg = substr(lon_raw, 1, 3);
      lon_min = substr(lon_raw, 4) / 60;
      lon_dd = lon_deg + lon_min;
      if (substr(lon_raw, length(lon_raw), 1) == "W") lon_dd = -lon_dd;

      # Print the output
      printf "lat=%.6f\nlon=%.6f\n", lat_dd, lon_dd;
      exit;
  }'

