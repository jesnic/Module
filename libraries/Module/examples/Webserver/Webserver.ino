/* 
Code example of ESP8266 webserver

In this example, ESP8266 will start its own wifi network and host a webpage
at 192.164.4.1/ or test/ (DNS).
*/

#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <Module.h> // Import Library

ESP8266WebServer server(80); // Webserver instance

void webpage ()
{
    server.send(200, "text/html",  "This is a test webpage and it is working!");
}

void setup()
{  
    module.create_network("Test", "12345678", "test"); // Create network with name "Test", password "12345678" and DNS "test"

    server.begin(); // Start webserver
    server.on("/", webpage); // Link url to function

    delay(5000);    
}

void loop()
{
    server.handleClient(); // Listen to client requests's
    MDNS.update(); // Update DNS
}
