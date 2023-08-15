
#include <LittleFS.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

#define NETNAME "<<insert wifi-network ssid>>"
#define NETPASS "<<insert wifi-network password>>"
#define SERIALBAUD 921600

//
// MyServer Class
//
//  This webserver class is based on the
//  ESP8266WebServer class, and provides
//  an /upload function.
//
//  It is used to demonstrate the use of the
//  on() function with the uploading of simple
//  and complex posts / files.
//

class MyServer : public ESP8266WebServer {

private:
  File fupload;

public:

  //////////////////////////////////////////
  //
  // MyServer() - Constructor
  //
  //
  MyServer(int port)
    : ESP8266WebServer(port) {

    on(
      "/upload",
      HTTP_ANY,
      [&]() {
        switch (method()) {

          case HTTP_GET:

            Serial.println("HTTP_GET\n");
            serveFileUpload();
            break;

          case HTTP_PUT:
          case HTTP_POST:
            send(400, "text/plain", "Requires multipart/* Content-Type");

            break;

          default:

            send(405, "text/plain", "Method not supported");
            break;
        }
      },
      [&]() {
        Serial.print("Upload Handler Callback: ");
        // Handle multi-part form uploads
        handleUpload();
      });

    on(
      "/jsfetch",
      HTTP_POST,
      [&]() {
        Serial.print("JsFetch Handler Callback: ") ;
        handleJsFetch();
      });

    onNotFound(
      [&]() {
        // Very much simplified handler.  This handler does
        // not check the method() - it assumes everything is
        // a HTTP_GET.
        
        Serial.print("Serving File Callback:\n") ;
        
        String path = "/data/" ;
        if (uri().compareTo("/")==0) { path = path + "index.html" ; }
        else { path = path + uri() ; }
        const String &contentType = mime::getContentType(path) ;

        File file ;
        file = LittleFS.open(path, "r") ;
        Serial.println("Reading file: " + path + " (uri " + uri() + ")") ;
        if (file.isFile()) {
          streamFile(file, contentType) ; 
        } else {
          send(404) ;
        }
        file.close() ;

      }) ;
  }

  //////////////////////////////////////////
  //
  // serveFileUpload()
  //
  // Server web page for upload page
  //
  void serveFileUpload() {

    const char *response =
      "<html>\n"
      "  <head>\n"
      "    <title>MyServer Upload</title>\n"
      "  </head>\n"
      "  <body>\n"
      "    <heading>MyServer Upload</heading>\n"
      "    <h2>Upload file</h2>\n"
      "    <form action=\"/upload\" method=\"POST\" enctype=\"multipart/form-data\">\n"
      "      <p>File: <input type=\"file\" name=\"inputlabel\"></p>\n"
      "      <p><input type=\"submit\" value=\"Upload\"></p>\n"
      "    </form>\n"
      "  </body>\n"
      "</html>\n";
    send(200, "text/html", response);
  }

  //////////////////////////////////////////
  //
  // handleUpload() - Method 1
  //
  // Upload file, and store in /data folder
  // used for multipart/* forms, and capable
  // of saving / uploading binary files.
  //
  void handleUpload() {

    // upload().status - progress state
    // upload().filename - name of file
    // upload().type - should be content-type of entry (doesn't work @14/08/2023)
    // upload().name - label of form field

    String path = String("/data/") + upload().filename;

    switch (upload().status) {

      case UPLOAD_FILE_START:

        Serial.printf("Upload File Start: name = %s, type = %s, filename = %s\n",
                      upload().name, upload().type, upload().filename);

        LittleFS.mkdir("/data");
        LittleFS.remove(path);
        fupload = LittleFS.open(path, "w");
        break;

      case UPLOAD_FILE_WRITE:

        Serial.println("Upload File Write");
        fupload.write(upload().buf, upload().currentSize);
        break;

      case UPLOAD_FILE_END:

        Serial.printf("Upload File End: name = %s, type = %s, filename = %s\n",
                      upload().name, upload().type, upload().filename);

        Serial.println("Writing file: " + path) ;
        fupload.close();

        if (!LittleFS.exists(path)) {
          send(500, "text/plain", "error saving upload file");
        } else {
          uint64_t len = upload().totalSize;
          send(200, "text/plain", "upload OK");
        }
        break;

      default:
        send(500, "text/plain", "unknown upload error");
        break;
    }
  }


  //////////////////////////////////////////
  //
  // handleJsFetch() - Method 2
  //
  // Upload file, and store in /data folder
  // used for simple uploads.  Data is stored
  // in memory in a string and is accessed with
  // the arg("plain") function.
  //
  void handleJsFetch() {

    const String &plain = arg("plain");
    const String &filename = arg("file");
    String path = String("/data/") + filename;

    Serial.println("Uploading file to " + path) ;
    
    if (plain.length() <= 0 || filename.length() <= 0) {

      send(400, "text/plain", "No data / invalid filename provided");

    } else {

      LittleFS.mkdir("/data");
      LittleFS.remove(path);
      fupload = LittleFS.open(path, "w");
      Serial.println("Writing file: " + path) ;
      fupload.write(plain.c_str());
      fupload.close();
      send(200, "upload OK");
    }
  }

};


// Create WebServer object
MyServer WebServer(80);
int wifistatus;

void setup() {

  // Start Serial Interface
  Serial.begin(SERIALBAUD);
  Serial.println("\n\n");
  delay(3000);

  // Start filesystem
  LittleFS.begin();

  // Initialise WiFi
  Serial.print("Connecting ... \n");
  WiFi.begin(NETNAME, NETPASS);

  wifistatus = WiFi.status();
  // Start webserver
  WebServer.begin();
}



void loop() {

  if (wifistatus != WL_CONNECTED && WiFi.status() == WL_CONNECTED) {
    wifistatus = WiFi.status();
    Serial.print("WiFi Connected: ");
    Serial.println(WiFi.localIP());
  }

  // Process webserver transactions
  WebServer.handleClient();
}
