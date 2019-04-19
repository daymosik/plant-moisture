#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <Adafruit_Sensor.h>

const char *ssid = "dajmosik";
const char *password = "1a2a3a4a5a";

#define DHTTYPE DHT11

int sense_Pin = 0;
int dht_Pin = 2;
uint8_t pump_Pin = D5;

int dryValue = 0;
int wetValue = 660;
int friendlyDryValue = 0;
int friendlyWetValue = 100;

int lastValues = 0;

DHT dht(dht_Pin, DHTTYPE);

LiquidCrystal_I2C lcd(0x27, 20, 4);

WiFiServer server(80);

void setup()
{
  Serial.begin(9600);

  // pinMode(pump_Pin, OUTPUT);
  // analogWrite(pump_Pin, 1023);
  // delay(2000);
  // analogWrite(pump_Pin, 0);
  // delay(2000);

  // HUMIDITY

  dht.begin();

  // LCD

  lcd.begin(20, 4);
  lcd.init();
  lcd.backlight();

  lcd.setCursor(5, 0);
  lcd.print("I AM");
  lcd.setCursor(2, 1);
  lcd.print("PODOCARPUS :)");

  // WIFI

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");

  // Start the server
  server.begin();
  Serial.println("Server started");

  // Print the IP address
  Serial.print("Use this URL to connect: ");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");

  delay(2000);
}

void loop()
{
  int rawValue = analogRead(sense_Pin);
  int friendlyValue = map(rawValue, dryValue, wetValue, friendlyDryValue, friendlyWetValue);

  int temperature = dht.readTemperature();
  int humidity = dht.readHumidity();

  int allValues = friendlyValue + temperature + humidity;

  if (allValues != lastValues) {
    lcd.init();

    lcd.setCursor(0, 0);
    lcd.print("Moisture: ");
    lcd.print(friendlyValue);
    lcd.print('%');

    lcd.setCursor(0, 1);
    lcd.print("Tmp: ");
    lcd.print(temperature);
    lcd.print(' ');
    lcd.print("Hum: ");
    lcd.print(humidity);
    lcd.print('%');

    lastValues = allValues;
  }

  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client)
  {
    delay(1000);
    return;
  }

  // Wait until the client sends some data
  Serial.println("new client");
  int timewate = 0;
  while (!client.available())
  {
    delay(1);
    timewate = timewate + 1;
    if (timewate > 1800)
    {
      Serial.println(">>> Client Timeout !");
      client.stop();
      return;
    }
  }

  // Read the first line of the request
  String request = client.readStringUntil('\r');
  Serial.println(request);
  client.flush();

  // Return the response
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println(""); //  do not forget this one
  client.println("");
  client.println("");

  if (request.indexOf("/api/getTemperature") != -1) {
    client.println(temperature);
  } else if (request.indexOf("/api/getHumidity") != -1) {
    client.println(humidity);
  } else if (request.indexOf("/api/getMoisture") != -1) {
    client.println(friendlyValue);
  } else {
    client.print("Podocarpus soil moisture is now at: ");
    client.print(friendlyValue);
    client.print("%");
    client.println(" ");

    client.print("Temperature is: ");
    client.print(temperature);
    client.print("C");
    client.println(" ");

    client.print("Humidity: ");
    client.print(humidity);
    client.print('%');
    client.println(" ");
  }

  client.println("");
  client.println("");
  client.println("");
  client.println("");
  client.println("");
  client.println("");
  delay(1);
  Serial.println("Client disonnected");
  Serial.println("");
}
