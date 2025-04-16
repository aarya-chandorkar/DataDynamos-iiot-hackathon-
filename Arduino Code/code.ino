#include <WiFi.h>
#include <HTTPClient.h>

// Define pin connections
#define SOIL_SENSOR_AO_PIN 34  // Soil moisture sensor analog output connected to GPIO 34
#define LDR_PIN 35             // LDR connected to GPIO 35 (with voltage divider)

// WiFi credentials
const char *ssid = "SagarK";
const char *password = "Sagar_123";

// ThingSpeak details
const char *thingSpeakAPIKey = "GGQFKSK9TDDNO1JX";
unsigned long thingSpeakChannelID = 2921362;

// Thresholds
const int LIGHT_THRESHOLD = 70;         // Light % above which it's considered too bright
const int MOISTURE_THRESHOLD = 80;      // Moisture % above which it's overwatering

void wifiConnect() {
    WiFi.begin(ssid, password);
    Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(1000);
    }
    Serial.println("\nConnected to WiFi network");
    Serial.print("Local ESP32 IP: ");
    Serial.println(WiFi.localIP());
}

void readSensorData(float &soilMoisturePercent, float &lightPercent) {
    int soilMoisture = analogRead(SOIL_SENSOR_AO_PIN);
    int lightLevel = analogRead(LDR_PIN);

    soilMoisturePercent = map(soilMoisture, 4095, 0, 0, 100); // 0% dry, 100% wet
    lightPercent = map(lightLevel, 3000, 300, 0, 100);         // 100% bright, 0% dark
    lightPercent = constrain(lightPercent, 0, 100);

    Serial.print("Soil Moisture: ");
    Serial.print(soilMoisturePercent);
    Serial.println("%");
    Serial.print("Light Level: ");
    Serial.print(lightPercent);
    Serial.println("%");
}

void sendToThingSpeak(float soilMoisturePercent, float lightPercent) {
    HTTPClient http;

    int overwaterWarning = (soilMoisturePercent > MOISTURE_THRESHOLD) ? 1 : 0;
    int lightWarning = (lightPercent > LIGHT_THRESHOLD) ? 1 : 0;

    String url = "https://api.thingspeak.com/update?api_key=" + String(thingSpeakAPIKey) +
                 "&field1=" + String(soilMoisturePercent) +
                 "&field2=" + String(lightPercent) +
                 "&field3=" + String(lightWarning) +
                 "&field4=" + String(overwaterWarning);

    http.begin(url);
    int httpCode = http.GET();

    if (httpCode == HTTP_CODE_OK) {
        Serial.println("Data sent to ThingSpeak successfully!");
    } else {
        Serial.println("Failed to send data to ThingSpeak. HTTP Code: " + String(httpCode));
    }

    http.end();
}

void setup() {
    Serial.begin(115200);
    wifiConnect();
    Serial.println("Starting the ESP32");
    delay(5000);
}

void loop() {
    if (WiFi.status() != WL_CONNECTED) {
        wifiConnect();
    }

    float soilMoisturePercent, lightPercent;
    readSensorData(soilMoisturePercent, lightPercent);
    sendToThingSpeak(soilMoisturePercent, lightPercent);

    delay(20000);  // 20 seconds delay between readings
}
