
This demo provides a simple / minimal Web Server which runs on an ESP8266 Arduino board.
The webserver supports Get, Post and Put.

This is essentially an example showing the different ways Post and Put can be implemented
to support file / data upload.

To Build:

  1. Edit the ino file, and include your Wifi network and password.
  2. Compile the ino file using the Arduino IDE (I/m using version 2.1.1).
  3. Download the application to your Arduino module.
  4. Look at the serial output (make sure you've set the right baud rate).
  5. Note the IP address, and connect to http://ip.address/upload
  6. Use the upload form to upload 'index.html'
  7. Now open http://ip.address/ to show index.html
  8. Try the other upload methods

2023/08/15

