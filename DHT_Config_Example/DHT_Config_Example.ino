/* 
Code example of DHT sensor that sends data to a webserver

The DHT sensor is controlled by ESP8266 chip. You can configure the SSID and password into the device using the ModuleConfig library.
*/

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include <ModuleUtils.h>
#include <ModuleConfig.h>
#include <DHT.h>

#define ESP_SSID "ConfigureMe!"
#define ESP_PASSWORD "12345678"
#define ESP_DNS "config"
#define CONFIG_PATH "/config.json"
#define HOSTNAME "hostname"

#define DHT_TYPE DHT11
#define DHT_PIN 0
#define SAMPLING_TIME 60000
#define MEAN_SAMPLES 3

ModuleConfig cfg = ModuleConfig(ESP_SSID, ESP_PASSWORD, ESP_DNS, CONFIG_PATH);

DHT dht(DHT_PIN, DHT_TYPE);

const char * ssid;
const char * password;

float ar_temp[MEAN_SAMPLES];
float ar_humid[MEAN_SAMPLES];
int sample_counter = 0;
float last_temp = 0;
float last_humid = 0;

// Configuration
void configure() {
    while (!cfg.check()) {
        cfg.listen();
    }
    // Send message when module was configured
    ssid = cfg.load_SSID();
    password = cfg.load_password();
    Serial.print("Module was just configured! ");
    Serial.print("SSID='");
    Serial.print(ssid);
    Serial.print("' Password='");
    Serial.print(password);
    Serial.println("'");
    delay(3000); // Delay so the client can still receive finish page
}


// Send data
void send_data (float temperature, float humidity) {
    String address = "http://api.thingspeak.com/update?api_key=PK9TEU4WSN0QDYSA&field1=" + String(temperature) + "&field2=" + String(humidity);
    String payload = module_utils.get_http_request(address);
}


// Compute Mean
float mean (float values[], int size) {
    float summ = 0;
    for (int i=0; i < size; i++) {
        summ += values[i];
    }
    return summ/size;
}


// Setup
void setup ()
{
    delay(5000);
    LittleFS.begin(); // Begin LittleFS
    Serial.begin(115200); // Begin Serial
    dht.begin(); // Begin DHT

    if (!cfg.check()) {
        Serial.println("Module not configured. Starting configuration...");
        cfg.begin();
        configure(); // Loop until configuration is finished
    }

    ssid = cfg.load_SSID(); // Get SSID from config file
    password = cfg.load_password(); // Get password from config file

    Serial.println("Module is configured");
    Serial.print("SSID='");
    Serial.print(ssid);
    Serial.print("' Password='");
    Serial.print(password);
    Serial.println("'");

    module_utils.join_network(ssid, password, HOSTNAME); // Connect to AP (if ESP disconnects, it will automatically handle reconnection, only call this ONCE, otherwise ESP might crash. Also, this method resets ssid and password variables)
    
    // Wait until connected to AP
    while (WiFi.status() != WL_CONNECTED) {
        Serial.println("Trying to connect to network..."); 
        ssid = cfg.load_SSID();
        password = cfg.load_password();
        module_utils.join_network(ssid, password, HOSTNAME);
        delay(5000);
    }
    Serial.println("Connected to network");

}


// Main loop
void loop ()
{

    // Read and validate data from sensor
    while (sample_counter < MEAN_SAMPLES) {
    
        float tempC = dht.readTemperature(); // Read temperature
        float humid = dht.readHumidity(); // Read humidity
        delay(100); // Without this delay ESP will crash, because it will get stuck in loop, being unable to process background tasks

        // Validate readings
        if (!isnan(tempC) && !isnan(humid)) { // If temperature and humidity are not NaN
            if ( (abs(last_temp - tempC) <= 10 && abs(last_humid - humid) <= 50) || (last_temp == 0 && last_humid == 0) ) { // If temperature and humidity are not variating a lot
                ar_temp[sample_counter] = tempC;
                ar_humid[sample_counter] = humid;
                last_temp = tempC;
                last_humid = humid;
                sample_counter++;
                delay(SAMPLING_TIME); // Wait
            }
        }
    }
    sample_counter = 0; // Reset counter
    
    // Compute mean of readings
    float avg_temp = mean(ar_temp, MEAN_SAMPLES);
    float avg_humid =  mean(ar_humid, MEAN_SAMPLES);

    send_data(avg_temp, avg_humid); // Send data to webserver

    // Print data in terminal
    Serial.print("Temperature (°C): ");
    Serial.println(avg_temp);
    Serial.print("Humidity (%): ");
    Serial.println(avg_humid);
    Serial.println("=================");
}