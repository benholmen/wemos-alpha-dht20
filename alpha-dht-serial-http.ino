#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <DHT.h>
#include <DHT_U.h>

// Temp probe setup
#define DHTTYPE DHT22 // DHT 22 (AM2302)
#define DHTPIN_SHIELD D4
#define DHTPIN_OTHER D5
DHT dht_shield(DHTPIN_SHIELD, DHTTYPE);
DHT dht_other(DHTPIN_OTHER, DHTTYPE);

// Wifi
char ssid[] = "Tonto";
char password[] = "Orion's Belt";

// Refresh interval in millisec
int dht_refresh_interval = 30*1000;

float humidity, temperature;
int temperature_f;
unsigned long ts_last_reading = 0;
WiFiServer server(80); // HTTP server

void setup() {
  Serial.begin(115200);
  delay(10);

  // WiFi
  Serial.print("[WIFI] Connecting to " + String(ssid) + "...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    Serial.print(".");
  }
  Serial.println("connected.");
  
  Serial.print("[HTTP] Web server started at http://");
  server.begin(); // Start listening for connections
  Serial.print(WiFi.localIP());
  Serial.println("/");
}

void loop() {
  if (ts_last_reading == 0 || millis() - ts_last_reading > dht_refresh_interval) {
    // Only read sensor every n seconds
    ts_last_reading = millis();

    // Two sensors right now:
    float h_shield = dht_shield.readHumidity();
    float t_shield = dht_shield.readTemperature();
    Serial.println("[SHIELD] " + String(t_shield) + "C " + String(h_shield) + "%");
    write_data("wemos-shield", t_shield, h_shield);
    
    float h_other = dht_other.readHumidity();
    float t_other = dht_other.readTemperature();
    Serial.println("[OTHER]  " + String(t_other) + "C " + String(h_other) + "%");
    write_data("wemos-breadboard", t_other, h_other);
    
/*    humidity = dht_shield.readHumidity();
    temperature = dht_shield.readTemperature();

    // Check if any reads failed and exit early (to try again).
    if (isnan(humidity) || isnan(temperature)) {
      Serial.println("[DHT]  Failed to read from sensor");
    }
    else {
      temperature_f = (temperature * 9 / 5) + 32;
    }
    Serial.println(String(temperature_f) + "F " + String(humidity) + "%");
    write_data("Shield", t_shield, h_shield);
*/
  }

  // listen for incoming connections
  WiFiClient server_cxn = server.available();
  if (server_cxn) {
    String html = "<html><pre>" + String(temperature_f) + "F " + String(humidity) + "%</pre><pre>Last reading "+String((millis() - ts_last_reading) / 1000)+"s ago</pre></html>";

    // Read the first line of the request
    String request = server_cxn.readStringUntil('\r');
    server_cxn.flush();
  
    // Return the response
    server_cxn.println("HTTP/1.1 200 OK");
    server_cxn.println("Content-Type: text/html");
    server_cxn.println(""); //  do not forget this one
    server_cxn.println(html);
    Serial.println("Sent HTTP content");
  }

  digitalWrite(BUILTIN_LED, HIGH);
  delay(200);
  digitalWrite(BUILTIN_LED, LOW);
  delay(800);

  delay(10);
}

void write_data(String sensor, float temperature, float humidity) {
  HTTPClient http;  // HTTP client
  http.begin("http://wx.benholmen.com/save?sensor=" + sensor + "&temperature=" + String(temperature) + "&humidity=" + String(humidity));
  int http_response_code = http.GET();
  Serial.println("HTTP response: "+http_response_code);
  http.end();  
}

