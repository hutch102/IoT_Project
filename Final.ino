#include <WiFi.h>
#include <LiquidCrystal.h>
#include <ArduinoHttpClient.h>

// Initialize LCD with pins
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

// WiFi credentials
const char *ssid = "Kyles phone";
const char *password = "ChristIsKing";

// ThingSpeak channel details
const String channelID = "2802930";
const String writeAPIKey = "QZI0HK84TB6LNTS6";

// WiFi and HTTP client setup
WiFiClient wifiClient;
HttpClient client = HttpClient(wifiClient, "api.thingspeak.com", 80);
WiFiServer server(80);

const int trigPin = 9;
const int echoPin = 10;
const int buzzerPin = 6;

// Timers and thresholds
unsigned long lastThingSpeakUpdate = 0;
const unsigned long thingSpeakInterval = 15000; // 15 seconds for ThingSpeak
unsigned long lastWiFiCheck = 0;
const unsigned long wifiCheckInterval = 5000;
unsigned long lastValidOperation = 0;
unsigned long stationaryStart = 0;

const float distanceChangeThreshold = 3.0;
const unsigned long stationaryDuration = 5000;

float lastDistance = 0.0;
bool isParked = false;

void (*resetFunc)(void) = 0;

// Variables to track min and max distance for the current session
float minDistance = 1000.0; // Arbitrarily large number
float maxDistance = 0.0;    // Arbitrarily small number

void setup()
{
    Serial.begin(115200);
    lcd.begin(16, 2);
    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);
    pinMode(buzzerPin, OUTPUT);

    connectWiFi();
    server.begin(); // Start the server
    lastValidOperation = millis();
}

void connectWiFi()
{
    lcd.clear();
    lcd.print("Connecting WiFi...");

    WiFi.begin(ssid, password);
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20)
    {
        delay(500);
        lcd.setCursor(0, 1);
        lcd.print("Attempt: " + String(attempts + 1));
        attempts++;
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        lcd.clear();
        lcd.print(WiFi.localIP());
        Serial.println("Connected: " + WiFi.localIP().toString());
    }
    else
    {
        lcd.clear();
        lcd.print("WiFi Failed");
        delay(2000);
        resetFunc();
    }
}

void checkWiFiConnection()
{
    if (millis() - lastWiFiCheck >= wifiCheckInterval)
    {
        if (WiFi.status() != WL_CONNECTED)
        {
            Serial.println("WiFi disconnected. Reconnecting...");
            connectWiFi();
        }
        lastWiFiCheck = millis();
    }
}

void handleWebRequest(WiFiClient client)
{
    String request = client.readStringUntil('\r'); // Read HTTP request
    Serial.println("Request Received: " + request);

    if (request.indexOf("GET /get_distance") >= 0)
    {
        client.println("HTTP/1.1 200 OK");
        client.println("Content-Type: text/plain");
        client.println("Connection: close");
        client.println();
        client.print(String(lastDistance, 1) + " cm");
    }
    else if (request.indexOf("GET /") >= 0)
    {
        client.println("HTTP/1.1 200 OK");
        client.println("Content-Type: text/html");
        client.println("Connection: close");
        client.println();
        client.println("<!DOCTYPE HTML>");
        client.println("<html>");
        client.println("<head>");
        client.println("<title>Parking Sensor</title>");
        client.println("<meta name='viewport' content='width=device-width, initial-scale=1'>");
        client.println("<script>");
        client.println("function fetchDistance() {");
        client.println("  fetch('/get_distance')");
        client.println("    .then(response => response.text())");
        client.println("    .then(data => {");
        client.println("      document.getElementById('distance').innerText = data;");
        client.println("    });");
        client.println("  setTimeout(fetchDistance, 1000);"); // Update every second
        client.println("}");
        client.println("</script>");
        client.println("</head>");
        client.println("<body onload='fetchDistance()'>");
        client.println("<h1>Parking Sensor</h1>");
        client.println("<p>Distance: <span id='distance'>Waiting for data...</span></p>");
        client.println("</body>");
        client.println("</html>");
    }

    client.flush();
    client.stop(); // Close connection
    Serial.println("Response Sent and Connection Closed.");
}

void loop()
{
    checkWiFiConnection();

    float distance = measureDistance();
    if (distance > 0 && distance < 400)
    {
        lastValidOperation = millis();

        // Update min and max distance
        if (distance < minDistance)
        {
            minDistance = distance;
        }
        if (distance > maxDistance)
        {
            maxDistance = distance;
        }

        if (abs(distance - lastDistance) < distanceChangeThreshold)
        {
            if (!isParked && (millis() - stationaryStart >= stationaryDuration))
            {
                isParked = true;
                handleParked(distance);
            }
        }
        else
        {
            stationaryStart = millis();
            if (isParked)
            {
                isParked = false;
                handleMoving();
            }
        }

        lastDistance = distance;

        if (!isParked)
        {
            displayDistance(distance);
            handleBuzzer(distance);
            if (millis() - lastThingSpeakUpdate >= thingSpeakInterval)
            {
                sendToThingSpeak(distance);
                lastThingSpeakUpdate = millis();
            }
        }
    }

    // Accept incoming client connections
    WiFiClient client = server.available();
    if (client)
    {
        Serial.println("New Client Connected.");
        handleWebRequest(client);
    }
}

float measureDistance()
{
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    long duration = pulseIn(echoPin, HIGH);
    return (duration * 0.034 / 2);
}

void displayDistance(float distance)
{
    lcd.setCursor(0, 1);
    lcd.print("                ");
    lcd.setCursor(0, 1);
    lcd.print(distance, 1);
    lcd.print(" cm");
}

void handleBuzzer(float distance)
{
    if (distance < 30)
    {
        tone(buzzerPin, 1000);
    }
    else if (distance >= 30 && distance < 90)
    {
        tone(buzzerPin, 1000);
        delay(50);
        noTone(buzzerPin);
        delay(50);
    }
    else if (distance >= 90 && distance < 150)
    {
        tone(buzzerPin, 1000);
        delay(200);
        noTone(buzzerPin);
        delay(200);
    }
    else
    {
        noTone(buzzerPin);
    }
}

void sendToThingSpeak(float distance)
{
    if (WiFi.status() == WL_CONNECTED)
    {
        String url = "/update?api_key=" + writeAPIKey +
                     "&field1=" + String(distance, 1) +
                     "&field2=" + String(minDistance, 1) +
                     "&field3=" + String(maxDistance, 1);

        client.get(url);
        int statusCode = client.responseStatusCode();
        String response = client.responseBody();

        Serial.print("ThingSpeak Status: ");
        Serial.println(statusCode);
        Serial.print("Response: ");
        Serial.println(response);
    }
}

void handleParked(float distance)
{
    lcd.clear();
    lcd.print("Parked at:");
    lcd.setCursor(0, 1);
    lcd.print(distance, 1);
    lcd.print(" cm");

    unsigned long startTime = millis();
    while (millis() - startTime < 15000)
    {
        if (WiFi.status() == WL_CONNECTED)
        {
            String url = "/update?api_key=" + writeAPIKey + "&field4=" + String(distance, 1);
            client.get(url);
            int statusCode = client.responseStatusCode();
            String response = client.responseBody();
            Serial.println("Parked Status: " + String(statusCode));
            Serial.println("Response: " + response);
        }
        delay(1000);
    }
}

void handleMoving()
{
    lcd.clear();
    lcd.print("Vehicle Moving");
    Serial.println("Vehicle started moving again.");
}