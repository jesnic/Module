#include "Arduino.h"
#include "ModuleUtils.h"
#include <WiFiClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPClient.h>
#include <LittleFS.h>
#include <ArduinoJson.h>

// ESP8266WebServer server(80);

// Class Constructor
ModuleUtils::ModuleUtils() 
{
    LittleFS.begin();
}

// Create own soft network
void ModuleUtils::create_network (const char * ssid, const char * password, const char * dns)
{
    WiFi.mode(WIFI_AP); // Set wifi mode to Access Point
    WiFi.softAP(ssid, password); // Configure wifi
    IPAddress IP = WiFi.softAPIP(); 
    if (dns != "") {
        MDNS.begin(dns);
    }
    // server.begin(); // Start webserver
}

// Join a network
void ModuleUtils::join_network (const char * ssid, const char * password, const char * hostname)
{
    WiFi.mode(WIFI_STA); // Set wifi mode to Station 
    WiFi.begin(ssid, password); // Connect to network
    if (hostname != "") {
        WiFi.setHostname(hostname); //define hostname
    }
}

// POST HTTP Request
const char * ModuleUtils::post_http_request (const char * address, const char * data)
{
    WiFiClient client;
    HTTPClient request;
	request.begin(client, address); // Request destination
	int response_code = request.POST(data); // Send the request
	String payload = request.getString(); // Get the payload
	request.end(); // Close connection
	return payload.c_str();
}

// GET HTTP Request
String ModuleUtils::get_http_request (String address)
{
    WiFiClient client;
    HTTPClient request;
	request.begin(client, address); // Request destination
	int response_code = request.GET(); // Send the request
	String payload = request.getString(); // Get the payload
	request.end(); // Close connection
	return payload;
}

// Save file
bool ModuleUtils::save_file(const char * path, const char * content)
{
    File file = LittleFS.open(path, "w"); // Open file for writing

    if (!file) {
        return false; // Failed to open file
    }
    if (file.print(content)) {
        file.close();
        return true; // File written
    } 
    else {
        return false; // Write failed
    }
}

// Read file
const char * ModuleUtils::read_file(const char * path) {

    File file = LittleFS.open(path, "r");

    if (!file) {
        return ""; // Failed to open file
    }
    
    String text = file.readString();
    file.close();
    return text.c_str();
}

// Delete file
bool ModuleUtils::delete_file(const char * path) {

    if (LittleFS.remove(path)) {
        return true; // File deleted
    } 
    else {
        return false; // Delete failed
    }
}

// Rename file
bool ModuleUtils::rename_file(const char * path1, const char * path2) {
  if (LittleFS.rename(path1, path2)) {
    return true; // File renamed
  } 
  else {
    return false; // Rename failed
  }
}

// Append file
bool ModuleUtils::append_file(const char * path, const char * content) {

    File file = LittleFS.open(path, "a");

    if (!file) {
        return false; // Failed to open file
    }
    if (file.print(content)) {
        file.close();
        return true; // Append done
    } 
    else {
        return false; // Append failed
    }
}

ModuleUtils module_utils;