#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ArduinoJson.h>
#include <LittleFS.h>

#define ESP_SSID "MyConfig"
#define ESP_PASSWORD "12345678"
#define CONFIG_FILE_PATH "/config.json"
#define DEFAULT_ERROR_MESSAGE "There was an error configuring the module, try again!"

ESP8266WebServer server(80);

// Create own soft network
void create_network (String ssid, String password, String dns="") {
  WiFi.mode(WIFI_OFF); 
  delay(500);
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  server.begin();  
  if (dns != "") {
    MDNS.begin(dns);
  }
}

// Join network
void join_network (String ssid, String password) {
    WiFi.mode(WIFI_OFF); 
    delay(500);
    WiFi.mode(WIFI_STA);      
    WiFi.begin(ssid, password);
    WiFi.hostname(ssid);   
    delay(5000);
}

// Configuration page
void config_page() {
  String webpage = "<!DOCTYPE html><html lang=en><title>Configure Network</title><meta charset=utf-8><meta content='width=device-width,initial-scale=1'name=viewport><style>*{box-sizing:border-box}body{margin:0}.header{background-color:#f1f1f1;padding:20px;text-align:center}.column{float:left;width:100%;padding:15px;text-align:center}.row:after{content:'';display:table;clear:both}.btn{border:line;background-color:#1e90ff;padding:2% 4%;font-size:20px;cursor:pointer;display:inline-block}.server{color:#fff}.server:hover{background-color:#fff;color:#2196F3}.client{background-color:#ff9800;color:#fff}.client:hover{background-color:#fff;color:#ff9800}@media screen and (max-width:600px){.column{width:100%}}</style><div class=header><h1><b><font color=blue size=10>Configure Network</font></b></h1></div><div class=row><div class=column><form action=/finish><p>Now select your network and fill in the password field.<br>These credentials are needed for the module to connect to your network.<p><i> SSID or password must not be longer than 100 characters! </i></p><p><i>If your network is open, just leave password field in blank!</i><h3>Name/SSID:<select name=ssid>";
  int number_networks = WiFi.scanNetworks();
  for(int i = 0; i < number_networks; i++) {
    String ssid = WiFi.SSID(i);
      webpage += "<option value=" + ssid + ">" + ssid + "</option>";
  }
  webpage += "</select><a href=/ ><button type=button>Refresh networks</button></a><h3>Password: <input type=password name=password></h3><br><input type=Submit class='btn server'style='display:block;margin:0 auto'value=Finish!>";
  server.send(200, "text/html", webpage); 
}

// Finish of configuration
void finish_page () {
  
  String webpage = "";
  String ssid = "";
  String password = "";
  String error_message = DEFAULT_ERROR_MESSAGE;
  
  // Try extracting SSID and password from arguments
  if (server.args() > 0) {
  
    for (int i = 0; i < server.args(); i++ ) { 
      if (server.argName(i) == "ssid") {
        ssid = server.arg(i);
      }
      else if (server.argName(i) == "password") {
        password = server.arg(i);
      }
    }
  }

  // If it was possible to succesfully extract SSID and Password
  if (ssid != "") {

    // If SSID or Password are longer than 100 characters
    if (ssid.length() > 100 || password.length() > 100) {
          error_message = "SSID or password must not be longer than 100 characters!";
          goto error;
    }
     
    webpage = "<!DOCTYPE html><html lang=en><title>Configure Network</title><meta charset=utf-8><meta content='width=device-width,initial-scale=1'name=viewport><style>*{box-sizing:border-box}body{margin:0}.header{background-color:#f1f1f1;padding:20px;text-align:center}.column{float:left;width:100%;padding:15px;text-align:center}.row:after{content:'';display:table;clear:both}.btn{border:line;background-color:#1e90ff;padding:2% 4%;font-size:20px;cursor:pointer;display:inline-block}.server{color:#fff}.server:hover{background-color:#fff;color:#2196F3}.client{background-color:#ff9800;color:#fff}.client:hover{background-color:#fff;color:#ff9800}@media screen and (max-width:600px){.column{width:100%}}</style><div class=header><h1><b><font color=blue size=10>Finished!</font></b></h1></div><div class=row><div class=column><p>Configuration is finished, your module will reset and start working soon.</p>";
    webpage += "<p>SSID: " + ssid + "</p>";
    webpage += "<p>Password: " + password + "</p>";
    webpage += "</div>";
    
    // Save network credentials
    LittleFS.format(); // Format filesystem (Reset)
    StaticJsonDocument<256> doc; // Create static JSON document with 256 bytes
    String json_text = "";
    doc["ssid"] = ssid;
    doc["password"] = password;
    serializeJson(doc, json_text); // Convert JSON document to string
    File file = LittleFS.open(CONFIG_FILE_PATH, "w"); // Open file for writing
    if (!file) {
      error_message = "Failed to open " + String(CONFIG_FILE_PATH) + " for writing";
      goto error;
    }
    if (!file.print(json_text)) {
      error_message = "Failed to write in " + String(CONFIG_FILE_PATH);
      goto error;
    }
    file.close(); // Close file
    Serial.println("Module configured! SSID=" + ssid + " Password=" + password);
    // --------------------------------------------
  }
  else {
    error:
      Serial.println(error_message);
      webpage = "<!DOCTYPE html><html lang=en><title>Configure Network</title><meta charset=utf-8><meta content='width=device-width,initial-scale=1'name=viewport><style>*{box-sizing:border-box}body{margin:0}.header{background-color:#f1f1f1;padding:20px;text-align:center}.column{float:left;width:100%;padding:15px;text-align:center}.row:after{content:'';display:table;clear:both}.btn{border:line;background-color:#1e90ff;padding:2% 4%;font-size:20px;cursor:pointer;display:inline-block}.server{color:#fff}.server:hover{background-color:#fff;color:#2196F3}.client{background-color:#ff9800;color:#fff}.client:hover{background-color:#fff;color:#ff9800}@media screen and (max-width:600px){.column{width:100%}}</style><div class=header><h1><b><font color=red size=10>Error!</font></b></h1></div><div class=row><div class=column><p>" + error_message + "</div>";
  }
  server.send(200, "text/html", webpage);
}


void setup() {
  
  Serial.begin(115200);

  create_network(ESP_SSID, ESP_PASSWORD, "config");

  // Configure server pages
  server.on("/", config_page);
  server.on("/finish", finish_page);
  server.onNotFound( []() { server.send(200, "text/html", "Page not found!"); } );

  // Mounts the filesystem
  if (!LittleFS.begin()) {
    Serial.println("LittleFS mount failed");
    return;
  }

  Serial.println("Config server started!");
  
}

void loop() {
  server.handleClient();
  MDNS.update();
}


