#include <Arduino.h>

#include <FastLED.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <EEPROM.h>
#include <ArduinoJson.h>
#include <WiFiUdp.h>
#define UDP_PORT 4210
#define DATA_PIN 2
#define COLOR_ORDER GRB
#define NUM_LEDS 60
#define LED_TYPE WS2812
#define BRIGHTNESS 64
CRGB leds[NUM_LEDS];

WiFiUDP UDP;
String SendHTML(uint8_t P);
void UThere();
void HandleCol();
String ssid = "";
String pass = "";

DynamicJsonDocument doc(512);
String IP = "@@@@";
ESP8266WebServer server(80);
uint8_t program = 1;
uint8_t ID = 24;
String getValue(String data, char separator, int index)
{
	int found = 0;
	int strIndex[] = {0, -1};
	int maxIndex = data.length() - 1;

	for (int i = 0; i <= maxIndex && found <= index; i++)
	{
		if (data.charAt(i) == separator || i == maxIndex)
		{
			found++;
			strIndex[0] = strIndex[1] + 1;
			strIndex[1] = (i == maxIndex) ? i + 1 : i;
		}
	}
	return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}
// Writes Mains IP
void WriteIP(String ip)
{
	int addr = 1;

	String string1 = getValue(ip, '.', 0);
	String string2 = getValue(ip, '.', 1);
	String string3 = getValue(ip, '.', 2);
	String string4 = getValue(ip, '.', 3);

	uint8_t num1 = string1.toInt();
	uint8_t num2 = string2.toInt();
	uint8_t num3 = string3.toInt();
	uint8_t num4 = string4.toInt();

	EEPROM.write(addr, num1);
	addr++;
	EEPROM.write(addr, num2);
	addr++;
	EEPROM.write(addr, num3);
	addr++;
	EEPROM.write(addr, num4);
	IP = (String)num1 + (String)num2 + (String)num3 + (String)num4;
	EEPROM.commit();
}
// gets Mains IP saved
String GetIP()
{
	int addr = 1;

	String string;
	uint8_t num1 = 0;
	uint8_t num2 = 0;
	uint8_t num3 = 0;
	uint8_t num4 = 0;

	num1 = EEPROM.get(addr, num1);
	addr++;
	num2 = EEPROM.get(addr, num2);
	addr++;
	num3 = EEPROM.get(addr, num3);
	addr++;
	num4 = EEPROM.get(addr, num4);
	string = (String)num1 + "." + (String)num2 + "." + (String)num3 + "." + (String)num4;
	return (string);
}

// Writes  ID
void WriteID(uint8_t ip)
{
	EEPROM.write(0, ip);

	EEPROM.commit();
}
void Writessid(String str)
{
	int addr = 5;
	char Char[20];
	str.toCharArray(Char, 20);
	for (int i = 0; i < 20; i++)
		EEPROM.write(i + addr, Char[i]);

	EEPROM.commit();
}
void Writepass(String str)
{
	int addr = 25;
	char Char[20];
	str.toCharArray(Char, 20);
	for (int i = 0; i < 20; i++)
		EEPROM.write(i + addr, Char[i]);

	EEPROM.commit();
}
String Getssid()
{
	int addr = 5;
	char Char[20];
	for (int i = 0; i < 20; i++)
		EEPROM.get(i + addr, Char[i]);
	return (Char);
}
String Getpass()
{
	int addr = 25;
	char Char[20];
	for (int i = 0; i < 20; i++)
		EEPROM.get(i + addr, Char[i]);
	return (Char);
}

struct Cola
{
	int r, g, b;
};
Cola Interpolate(int r, int g, int b, int rOld, int gOld, int bOld, float t)
{
	int R = int(rOld + ((r - rOld) * t));
	int G = int(gOld + ((g - gOld) * t));
	int B = int(bOld + ((b - bOld) * t));
	Cola what;
	what.r = R;
	what.g = G;
	what.b = B;
	return (what);
}
// gets ID saved
uint8_t GetID()
{
	uint8_t num1 = 0;

	num1 = EEPROM.get(0, num1);
	return (num1);
}
void CheckInputs()
{
	Serial.setTimeout(100);
	String op = Serial.readString();
	if (op.startsWith("ip"))
	{
		op.replace("ip", "");
		WriteIP(op);
		IP = GetIP();
		delay(200);
		Serial.print("Ip set to:" + op);
	}
	if (op.startsWith("id"))
	{
		op.replace("id", "");
		WriteID(op.toInt());
		ID = op.toInt();
		delay(200);
		Serial.print("ID set to:" + op);
	}
	if (op.startsWith("ssid"))
	{
		op.replace("ssid", "");
		Writessid(op);
		ssid = op;
		delay(200);
		Serial.print("ssid set to:" + op);
	}
	if (op.startsWith("pass"))
	{
		op.replace("pass", "");
		Writepass(op);
		pass = op;
		delay(200);
		Serial.print("pass set to:" + op);
	}
}
int blynk = 100;
int blynkMilis = 100;
bool blinky = false;
void setup()
{
	Serial.begin(115200);
	
	Serial.println("Started");
	Serial.println("Started");
	EEPROM.begin(512);
	ID = GetID();
	IP = GetIP();
	ssid = Getssid(); 
	pass = Getpass();
	Serial.println("ssid: " + ssid);
	Serial.println("pass: " + pass);
	Serial.println("Got ID: " + String(int(ID)));
	// Led strip setup
	FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS);
	FastLED.setCorrection(TypicalLEDStrip);
	//FastLED.setMaxPowerInMilliWatts(10);

	// Just blinkeys
	for (int i = 0; i < 6; i++)
	{

		leds[0] = CRGB::White;
		FastLED.show();
		CheckInputs();
		delay(50);
		leds[0] = CRGB::Black;
		FastLED.show();
		CheckInputs();
		delay(50);
	}
	ID = GetID();
	IP = GetIP();
	ssid = Getssid();
	pass = Getpass();
	FastLED.clear();

	WiFi.hostname("esp01");
	Serial.begin(115200);
	// Connect to wifi
	WiFi.begin(ssid, pass);
	while (WiFi.status() != WL_CONNECTED)
	{
		CheckInputs();
		leds[0] = CRGB::Red;
		FastLED.show();
		delay(100);
		leds[0] = CRGB::Black;
		FastLED.show();
		delay(100);
	}
	Serial.println("");
	Serial.println("Got main IP: " + IP);
	Serial.println("Got Mac: " + WiFi.macAddress());
	Serial.println("Got IP: " + WiFi.localIP().toString());
	Serial.println("Got ID: " + String(int(ID)));
	// Server propeties
	server.on("/espinator/SetCol", HandleCol);
	server.on("/espinator/UThere", UThere);
	server.begin();

	// Get id and send ip from Mac
	WiFiClient client;
	HTTPClient http;
	String asd = "espMain.local";
	String serverPath = "http://" + IP + "/espinator/MyData?Mac=" + WiFi.macAddress() + "&ID=" + ID + "&IP=" + WiFi.localIP().toString();
	http.setTimeout(400);
	http.begin(client, serverPath.c_str());
	int httpCode = http.GET();

	bool ledy = false;
	bool searching = true;
	// search for main server
	while (searching)
	{
		CheckInputs();
		if (httpCode != -1)
		{
			deserializeJson(doc, http.getString());
			// ID = doc["id"].as<int>();
			searching = false;
			Serial.println("Found server" + (String)httpCode);
		}
		else
		{
			http.begin(client, serverPath.c_str());
			httpCode = http.GET();
			if (ledy)
				leds[0] = CRGB::Green;
			else
				leds[0] = CRGB::Black;
			FastLED.show();
			ledy = !ledy;
			Serial.println("can't find server" + (String)httpCode);
		}
		delay(10);
	}

	leds[0] = CRGB::Black;
	FastLED.show();
	Serial.println("Got ID: " + (String)ID);
	blinky = false;
	// digitalWrite(2, true);
	UDP.begin(UDP_PORT);

	for (int i = 0; i < 2; i++)
	{
		leds[0] = CRGB::Yellow;
		FastLED.show();
		delay(100);
		leds[0] = CRGB::Black;
		FastLED.show();
		delay(100);
		CheckInputs();
	}
}
uint8_t packet[32];
// int col = 0;
int OldColR = 0;
int OldColG = 0;
int OldColB = 0;
int NewColR = 0;
int NewColG = 0;
int NewColB = 0;
int OldFrame = 0;
int NewFrame = 0;
long lastMilis = 0;
int blendering = 100;
void loop()
{
	while (WiFi.status() != WL_CONNECTED)
	{
		leds[0] = CRGB::Red;
		FastLED.show();
		delay(100);
		leds[0] = CRGB::Black;
		FastLED.show();
		delay(100);
	}
	server.handleClient();
	int packetSize = UDP.parsePacket();
	if (packetSize)
	{
		int len = UDP.read(packet, 32);
		// bool led = true;
		if (len > 0)
		{
			packet[len] = '\0';
		}
		lastMilis = millis();
		NewFrame = packet[3];

		UDP.beginPacket(UDP.remoteIP(), UDP.remotePort());
		UDP.write('G');
		UDP.write('G');
		UDP.write(ID);
		UDP.write(0);
		UDP.write(0);
		UDP.endPacket();

		if (OldFrame != NewFrame)
		{
			OldFrame = NewFrame;
			OldColR = NewColR;
			OldColG = NewColG;
			OldColB = NewColB;
			NewColR = packet[0];
			NewColG = packet[1];
			NewColB = packet[2];
			//blendering = int(packet[4])*10;
			lastMilis = millis();
			
			// Serial.println(String(NewColR) + " " + String(NewColG) + " " + String(NewColB)+" " + String(NewFrame)+" " + String(OldFrame));
		}
	}
	float a = float(millis() - lastMilis) / 100;
	if (a > 1)
		a = 1;
	if (a < 0)
		a = 0;
	Cola color = Interpolate(NewColR, NewColG, NewColB, OldColR, OldColG, OldColB, a);
	FastLED.showColor(CRGB(color.r, color.g, color.b));
	Serial.println(String(color.r) + " " + String(color.g) + " " + String(color.b) + " " + String(NewFrame) + " " + String(OldFrame));
	delay(1);
	if (millis() - lastMilis > 10000)
	{
		OldColR = NewColR;
		OldColG = NewColG;
		OldColB = NewColB;
		NewColR = 0;
		NewColG = 0;
		NewColB = 0;
		blendering=1000;
		WiFiClient client;
		HTTPClient http;
		String serverPath = "http://" + IP + "/espinator/MyData?Mac=" + WiFi.macAddress() + "&ID=" + ID + "&IP=" + WiFi.localIP().toString();
		http.setTimeout(150);
		http.begin(client, serverPath.c_str());
		int httpCode = http.GET();
		CheckInputs();
		lastMilis = millis();
	}
}

// Server Asks
void UThere()
{
	server.send(200, "application/json", "{ \"status\":\"ok\"}");
	Serial.println("U There");
}
// Sent Col
void HandleCol()
{
	if (server.arg("Col") != "")
	{
		int r, g, b;
		char const *hexColor = server.arg("Col").c_str();
		std::sscanf(hexColor, "#%02x%02x%02x", &r, &g, &b);
		FastLED.showColor(CRGB(r, g, b));

		server.send(200, "application/json", "{ \"status\":\"ok\"}");
		blinky = false;
	}
}