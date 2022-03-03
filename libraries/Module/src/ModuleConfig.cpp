#include "Arduino.h"
#include "ModuleUtils.h"
#include "ModuleConfig.h"
#include <WiFiClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPClient.h>
#include <LittleFS.h>
#include <ArduinoJson.h>

ESP8266WebServer server(80);

// Class Constructor
ModuleConfig::ModuleConfig() 
{
    this->_config_path = CONFIG_PATH;
    this->_ap_SSID = DEFAULT_SSID;
    this->_ap_password = DEFAULT_PASSWORD;
    this->_ap_DNS = DEFAULT_DNS;
    LittleFS.begin();
}

// Class Constructor
ModuleConfig::ModuleConfig(const char * ap_SSID, const char * ap_password, const char * ap_DNS, const char * config_path) 
{
    this->_config_path = config_path;
    this->_ap_SSID = ap_SSID;
    this->_ap_password = ap_password;
    this->_ap_DNS = ap_DNS;
    LittleFS.begin();
}


// Start configuration
void ModuleConfig::begin()
{
    module_utils.create_network(_ap_SSID, _ap_password, _ap_DNS);

    // Configure server pages
    std::function<void(void)> config = [this]() { this->config_page(); };
    server.on("/", config);
    std::function<void(void)> finish = [this]() { this->finish_page(); };
    server.on("/finish", finish);
    server.onNotFound( []() { server.send(200, "text/html", "Page not found!"); } );
    server.begin(); // Start webserver
}

// Reset configuration
bool ModuleConfig::reset()
{
    LittleFS.format(); // Clear all filesystem!
}


// Check if module is configured or not
bool ModuleConfig::check()
{
    if (load_SSID() == "") {
        return false;
    } 
    return true;
}


// Save configuration 
bool ModuleConfig::save(const char * ssid, const char * password)
{
    StaticJsonDocument<CONFIG_SIZE> doc;
    doc["ssid"] = ssid;
    doc["password"] = password;

    String content_json = "";
    serializeJson(doc, content_json);
    // Success
    if (!module_utils.save_file(this->_config_path, content_json.c_str())) {
        return false;
    }
    return true;
}


// Load/read SSID fromc config file
const char * ModuleConfig::load_SSID()
{
    const char * config = module_utils.read_file(this->_config_path);
    StaticJsonDocument<CONFIG_SIZE> doc;
    auto error = deserializeJson(doc, config);
    // In case of deserialization error
    if (error) {
        return "";
    }
    return doc["ssid"];
}


// Load/read password from config file
const char * ModuleConfig::load_password()
{
    const char * config = module_utils.read_file(this->_config_path);
    StaticJsonDocument<CONFIG_SIZE> doc;
    auto error = deserializeJson(doc, config);
    // In case of deserialization error
    if (error) {
        return "";
    }
    return doc["password"];
}


// Listen
void ModuleConfig::listen()
{
    server.handleClient();
    MDNS.update();
}


// Configuration page
void ModuleConfig::config_page() {
    String webpage = "<!DOCTYPE html><html lang=en><title>Configure Network</title><meta charset=utf-8><meta content='width=device-width,initial-scale=1'name=viewport><style>*{box-sizing:border-box}body{margin:0}.header{background-color:#f1f1f1;padding:20px;text-align:center}.column{float:left;width:100%;padding:15px;text-align:center}.row:after{content:'';display:table;clear:both}.btn{border:line;background-color:#1e90ff;padding:2% 4%;font-size:20px;cursor:pointer;display:inline-block}.server{color:#fff}.server:hover{background-color:#fff;color:#2196F3}.client{background-color:#ff9800;color:#fff}.client:hover{background-color:#fff;color:#ff9800}@media screen and (max-width:600px){.column{width:100%}}</style><div class=header><h1><b><font color=blue size=10>Configure Network</font></b></h1></div><div class=row><div class=column><form action=/finish><p>Now select your network and fill in the password field.<br>These credentials are needed for the module to connect to your network.<p><i> SSID or password must not be longer than 100 characters! </i></p><p><i>If your network is open, just leave password field in blank!</i><h3>Name/SSID:<select name=ssid>";
    int number_networks = WiFi.scanNetworks();
    for (int i = 0; i < number_networks; i++) {
        String ssid = WiFi.SSID(i);
        webpage += "<option value=" + ssid + ">" + ssid + "</option>";
    }
    webpage += "</select><a href=/ ><button type=button>Refresh networks</button></a><h3>Password: <input type=password name=password></h3><br><input type=Submit class='btn server'style='display:block;margin:0 auto'value=Finish!>";
    server.send(200, "text/html", webpage); 
}


// Finish of configuration
void ModuleConfig::finish_page () {
  
    String webpage = "";
    String ssid = "";
    String password = "";
    String error_message = "";
    
    // Try extracting SSID and password from arguments
    if (server.args() > 0) {
    
        for (int i = 0; i < server.args(); i++) { 
            if (server.argName(i) == "ssid") {
                ssid = server.arg(i);
            } else if (server.argName(i) == "password") {
                password = server.arg(i);
            }
        }
    }

    // If it was not possible to extract SSID/password
    if (ssid == "") {
        error_message = "There was an error configuring the module, try again!";
        goto error;
    }

    // If SSID or Password are longer than 100 characters
    if (ssid.length() > 100 || password.length() > 100) {
        error_message = "SSID or password must not be longer than 100 characters!";
        goto error;
    }

    // Save network credentials
    if (!this->save(ssid.c_str(), password.c_str())) {
        error_message = "Failed to save configuration!";
        goto error;
    }
        
    webpage = "<!DOCTYPE html><html lang=en><title>Configure Network</title><meta charset=utf-8><meta content='width=device-width,initial-scale=1'name=viewport><style>*{box-sizing:border-box}body{margin:0}.header{background-color:#f1f1f1;padding:20px;text-align:center}.column{float:left;width:100%;padding:15px;text-align:center}.row:after{content:'';display:table;clear:both}.btn{border:line;background-color:#1e90ff;padding:2% 4%;font-size:20px;cursor:pointer;display:inline-block}.server{color:#fff}.server:hover{background-color:#fff;color:#2196F3}.client{background-color:#ff9800;color:#fff}.client:hover{background-color:#fff;color:#ff9800}@media screen and (max-width:600px){.column{width:100%}}</style><div class=header><h1><b><font color=blue size=10>Finished!</font></b></h1></div><div class=row><div class=column><p>Configuration is finished, your module will reset and start working soon.</p>";
    webpage += "<p>SSID: " + ssid + "</p>";
    webpage += "<p>Password: " + password + "</p>";
    webpage += "</div>";
    server.send(200, "text/html", webpage);
    return;

    error:
        webpage = "<!DOCTYPE html><html lang=en><title>Configure Network</title><meta charset=utf-8><meta content='width=device-width,initial-scale=1'name=viewport><style>*{box-sizing:border-box}body{margin:0}.header{background-color:#f1f1f1;padding:20px;text-align:center}.column{float:left;width:100%;padding:15px;text-align:center}.row:after{content:'';display:table;clear:both}.btn{border:line;background-color:#1e90ff;padding:2% 4%;font-size:20px;cursor:pointer;display:inline-block}.server{color:#fff}.server:hover{background-color:#fff;color:#2196F3}.client{background-color:#ff9800;color:#fff}.client:hover{background-color:#fff;color:#ff9800}@media screen and (max-width:600px){.column{width:100%}}</style><div class=header><h1><b><font color=red size=10>Error!</font></b></h1></div><div class=row><div class=column><p>" + error_message + "</div>";
        server.send(200, "text/html", webpage);
}