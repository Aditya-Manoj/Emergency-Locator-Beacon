// EMERGENCY LOCATOR BEACON - Working of POST request from NodeMCU to Custom designed Website

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h> 
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>

#include <TinyGPS++.h>
#include <SoftwareSerial.h>

//JSON FORMAT
//{
//    "LAT": "10_43_43.5_N",
//    "LONG": "79_01_03.6_S",
//    "ALT": 0,
//    "TIME": "01:55 PM", 
//    "DATE": "19/08/2001",
//    "GMAP": "https://www.google.com/maps/place/10%C2%B043'43.5%22N+79%C2%B001'03.6%22E/"
//}
//"{\"LAT\": \"" + latitude + "\", \"LONG\": \"" + longitude + "\", \"ALT\": " + alt + ", \"TIME\": \"" + Timestr + "\", \"DATE\": \"" + Datestr + "\", \"GMAP\": \"" + hlink + "\"}"

//===========================================================================================================
//USER BASED SETTINGS

//POST DATA EVERY - X minutes
const float X = 0.25;
const int postdelay = X * 60 * 1000;

//DEVICE API KEY
String apikey = "myapikey";

/* Set these to your desired credentials. */
const char *ssid = "your_ssid";  //ENTER YOUR WIFI SETTINGS
const char *password = "your_password";

//Link to read data from https://jsonplaceholder.typicode.com/comments?postId=7
//Web/Server address to read/write from 
//const char *host = "adi.pythonanywhere.com";
const char *host = "your_website_domain";

//SHA1 finger print of certificate use web browser to view and copy
//const char fingerprint[] PROGMEM = "09 75 11 B4 A8 F5 5B F2 BA 15 D1 EF 27 56 94 8F A7 26 77 63";
const char fingerprint[] PROGMEM = "YOUR_WEBSITE_FINGERPRINT_HERE";

//===========================================================================================================

int RXPin = 4;
int TXPin = 5;
int GPSBaud = 9600;

String Jsonstr;
int stat = 0;
TinyGPSPlus gps;
SoftwareSerial gpsSerial(RXPin, TXPin);

const int httpsPort = 443;  //HTTPS= 443 and HTTP = 80
//=======================================================================
//                    Power on setup
//=======================================================================

void setup() {
  delay(1000);
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(9600);
//  gpsSerial.begin(GPSBaud);
  WiFi.mode(WIFI_OFF);        //Prevents reconnection issue (taking too long to connect)
  delay(1000);
  WiFi.mode(WIFI_STA);        //Only Station No AP, This line hides the viewing of ESP as wifi hotspot
  
  WiFi.begin(ssid, password);     //Connect to your WiFi router
  Serial.println("");

  Serial.print("Connecting");
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  //If connection successful show IP address in serial monitor
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());  //IP address assigned to your ESP

  gpsSerial.begin(GPSBaud);
  Serial.println("GPS DATA RECEPTION link Succesfull");
}

//=======================================================================
//                    Main Program Loop
//=======================================================================
void loop() {
  Jsonstr = "";
  startgps();
  
  WiFiClientSecure httpsClient;    //Declare object of class WiFiClient
  
  Serial.println(host);

  Serial.printf("Using fingerprint '%s'\n", fingerprint);
  httpsClient.setFingerprint(fingerprint);
  httpsClient.setTimeout(15000); // 15 Seconds
  delay(1000);
  
  Serial.print("HTTPS Connecting");
  int r=0; //retry counter
  while((!httpsClient.connect(host, httpsPort)) && (r < 30)){
      delay(100);
      Serial.print(".");
      r++;
  }
  if(r==30) {
    Serial.println("Connection failed");
  }
  else {
    Serial.println("Connected to web");
  }
  
  String getData, Link;
  
  //POST Data
  Link = "/post/" + String(apikey);

  Serial.print("requesting URL: ");
  Serial.println(host);
  /*
   POST /post HTTP/1.1
   Host: postman-echo.com
   Content-Type: application/json
   Content-Length: 
    
   */
//  int lat, lon, acc;
//  lat = rand() % 50;
//  lon = rand() % 50;
//  acc = rand() % 50;
//  String hlink = "https://www.google.com/maps/place/" + String(lat) + "%C2%B0" + "00'00.0" + "%22N+" + String(lon) + "%C2%B0" + "00'00.0" + "%22E/";
//  String jsonstr = "{\"LAT\": \"" + String(lat) + "N\", \"LONG\": \"" + String(lon) + "S\", \"ALT\": \"" + String(acc) + "\", \"TIME\": \"12:05 PM\", \"DATE\": \"25/5/2022\", \"GMAP\": \"" + hlink + "\"" + "}";

  String jsonstr = Jsonstr;
//  Serial.println("https://www.google.com/maps/place/" + String(lat) + "%C2%B000'00.0%22N+" + String(lon) + "%C2%B000'00.0%22E/");

  Serial.println(jsonstr);
  httpsClient.print(String("POST ") + Link + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Content-Type: application/json"+ "\r\n" +
               "Content-Length: " + String(jsonstr.length()) + "\r\n\r\n" +
               jsonstr + "\r\n" +
               
               "Connection: close\r\n\r\n");

  Serial.println("request sent");
                  
  while (httpsClient.connected()) {
    String line = httpsClient.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("headers received");
      break;
    }
  }

  Serial.println("reply was:");
  Serial.println("==========");
  String line;
  while(httpsClient.available()){        
    line = httpsClient.readStringUntil('\n');  //Read Line by Line
    Serial.println(line); //Print response
  }
  Serial.println("==========");
  Serial.println("closing connection");

  digitalWrite(LED_BUILTIN, LOW);   // turn the LED on (HIGH is the voltage level)
  delay(2000);                       // wait for a second
  digitalWrite(LED_BUILTIN, HIGH); 

  Jsonstr = "";
  stat = 0;

  Serial.println(String(postdelay) + "POSTDELAY");
  Serial.println("*******************************************");
  delay(postdelay);  //POST Data at every 2 seconds
}

//-------------------------EXTERNAL FUNCTIONS-------------------------------------
void startgps()
{
  // This sketch displays information every time a new sentence is correctly encoded.
  while (stat == 0) {
    while (gpsSerial.available() > 0) {
      if (gps.encode(gpsSerial.read())) {
        genjson();
        if (stat == 1) {
          Serial.println("JSON TO BE PUBLISHED: " + Jsonstr);
          Serial.println();
          break;
        }
      }
    }
  }
  
  //  delay(1000);
      

  // If 5000 milliseconds pass and there are no characters coming in
  // over the software serial port, show a "No GPS detected" error
  Serial.println("-----------JSON STRING CREATION SUCCESSFULL-------------");
  delay(1000);
  if (millis() > 5000 && gps.charsProcessed() < 10)
  {
    Serial.println("No GPS detected");
    while(true);
  }
}

void genjson() {
  String hlink, Datestr, Timestr, alt, latitude, longitude;
  stat = 1;

  if (gps.location.isValid()) {
    Serial.print("Latitude: ");
    Serial.println(gps.location.lat(), 6);
    Serial.print("Longitude: ");
    Serial.println(gps.location.lng(), 6);
    Serial.print("Altitude: ");
    Serial.println(gps.altitude.meters());
    latitude = String(gps.location.lat(), 6);
    longitude = String(gps.location.lng(), 6);
    alt = String(gps.altitude.meters());
    hlink = "https://www.google.com/maps/place/" + latitude + "+" + longitude;
    Serial.println(hlink);
  }
  else
  {
    Serial.println("Location: Not Available");
    stat = 0;
  }

  if (gps.date.isValid() && gps.time.isValid())
  {
    Datestr = String(gps.date.day()) + "/" + String(gps.date.month()) + "/" + String(gps.date.year());
    Serial.println(Datestr);

    Timestr = "";
    if (gps.time.hour() < 10) Timestr += "0";
    Timestr += (String(gps.time.hour()) + ":");
    if (gps.time.minute() < 10) Timestr += "0";
    Timestr += (String(gps.time.minute()) + ":");
    if (gps.time.second() < 10) Timestr += "0";
    Timestr += (String(gps.time.second()) + ".");
    if (gps.time.centisecond() < 10) Timestr += "0";
    Timestr += String(gps.time.centisecond());

    Serial.println(Timestr);
  
  }
  else
  {
    Serial.println("DATE & TIME Not Available");
    stat = 0;
  }

  String jsonstr = ""; 
//= "{\"LAT\": \"" + latitude + "\", \"LONG\": \"" + longitude + "\", \"ALT\": \"" + alt + "\", \"TIME\": \"" + Timestr + "\", \"DATE\": \"" + Datestr + "\", \"GMAP\": \"" + hlink + "\"}";

  if (stat) {
    jsonstr = "{\"LAT\": \"" + latitude + "\", \"LONG\": \"" + longitude + "\", \"ALT\": \"" + alt + "\", \"TIME\": \"" + Timestr + "\", \"DATE\": \"" + Datestr + "\", \"GMAP\": \"" + hlink + "\"}";
    Serial.println(String(stat) + "JSON STR: " + jsonstr);
    Jsonstr = jsonstr;   
  }
//  else if (jsonstr.length() > 0) {
//    Serial.println("JSON STRING VALID TO BE PUBLISHED" + jsonstr);
//  }
  delay(2000);
}
//=======================================================================
