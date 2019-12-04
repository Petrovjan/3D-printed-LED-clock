#include <Adafruit_NeoPixel.h>
#include <TimeLib.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>

// Which pin on the Arduino is connected to the NeoPixels?
// On a Trinket or Gemma we suggest changing this to 1
#define PIN            D2

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS      24

// When we setup the NeoPixel library, we tell it how many pixels, and which pin to use to send signals.
// Note that for older NeoPixel strips you might need to change the third parameter--see the strandtest
// example for more information on possible values.
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

const char ssid[] = "ssid";  //  your network SSID (name)
const char pass[] = "pswd";       // your network password

const int timeZone = 1;     // Central European Time

static const char ntpServerName[] = "time.ufe.cz";

WiFiUDP Udp;
unsigned int localPort = 8888;  // local port to listen for UDP packets

time_t getNtpTime();
void digitalClockDisplay();
void printDigits(int digits);
void sendNTPpacket(IPAddress &address);

void setup() {
  Serial.begin(9600);
  pixels.begin(); // This initializes the NeoPixel library.
  WiFi.mode(WIFI_STA);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("IP number is ");
  Serial.println(WiFi.localIP());
  Serial.println("Starting UDP");
  Udp.begin(localPort);
  Serial.print("Local port: ");
  Serial.println(Udp.localPort());
  Serial.println("waiting for sync");
  setSyncProvider(getNtpTime);
  setSyncInterval(300);
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_SPIFFS
      type = "filesystem";
    }

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
  Serial.println("Ready");
  stargate();
}

time_t prevDisplay = 0; // when the digital clock was displayed

void loop() {
  ArduinoOTA.handle();

  if (timeStatus() != timeNotSet) {
    if (now() != prevDisplay) { //update the display only if time has changed
      prevDisplay = now();
      for(int i=0;i<24;i++){
        pixels.setPixelColor(i, pixels.Color(0, 0, 0));
      }
      if (hour() == 0 && minute() == 0 && second() == 0){
        stargate();
      }
      display_minutes(minute());
      display_hours(hour());
      pixels.show(); 
    }
  }
}

void display_hours(int curhrs){
  int hrs_led;

  if((curhrs >= 0) && (curhrs < 6)){
      hrs_led = 17 - curhrs;
  }
  else if((curhrs >= 6) && (curhrs < 18)){
      hrs_led = 29 - curhrs;
  }
  else if ((curhrs >= 18) && (curhrs < 24)){
      hrs_led = 41 - curhrs;
  }

  pixels.setPixelColor(hrs_led, pixels.Color(255,0,0));  
  
}

void display_minutes(int mnt){
  int next_mled;   
  int rem = mnt % 5;
  int mled = mnt / 5;

  if((mled >= 0) && (mled <= 5)){
    mled = 6 + mled;
  }
  else{
    mled = mled - 6;
  } 

  if(mled == 11){
    next_mled = 0;
  }
  else{
    next_mled = mled + 1;    
  }
  
  switch (rem){
    case 0:
        pixels.setPixelColor(mled, pixels.Color(255,255,255));
        break;
    case 1:
        pixels.setPixelColor(mled, pixels.Color(255,255,55));
        pixels.setPixelColor(next_mled, pixels.Color(0,0,55));  
        break;
    case 2:
        pixels.setPixelColor(mled, pixels.Color(55,255,55));
        pixels.setPixelColor(next_mled, pixels.Color(55,0,55));  
        break;        
     case 3:
        pixels.setPixelColor(mled, pixels.Color(55,0,55));
        pixels.setPixelColor(next_mled, pixels.Color(55,255,55));  
        break;  
    case 4:
        pixels.setPixelColor(mled, pixels.Color(0,0,55));
        pixels.setPixelColor(next_mled, pixels.Color(255,255,55));  
        break;          
  }
}

void stargate() {
  
        for(int i=0;i<12;i++){
          pixels.setPixelColor(i, pixels.Color(255, 69, 0));
          pixels.show();
          delay(500);
        }

        for(int i=12;i<24;i++){
          pixels.setPixelColor(i, pixels.Color(135, 206, 250));
        }         
        pixels.show();  
        delay(10000);
          
        for(int i=0;i<24;i++){
        pixels.setPixelColor(i, pixels.Color(0, 0, 0));
        }        

}

/*-------- NTP code ----------*/

const int NTP_PACKET_SIZE = 48; // NTP time is in the first 48 bytes of message
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets

time_t getNtpTime()
{
  IPAddress ntpServerIP; // NTP server's ip address

  while (Udp.parsePacket() > 0) ; // discard any previously received packets
  Serial.println("Transmit NTP Request");
  // get a random server from the pool
  WiFi.hostByName(ntpServerName, ntpServerIP);
  Serial.print(ntpServerName);
  Serial.print(": ");
  Serial.println(ntpServerIP);
  sendNTPpacket(ntpServerIP);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = Udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      Serial.println("Receive NTP Response");
      Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
      unsigned long secsSince1900;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      return secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
    }
  }
  Serial.println("No NTP Response :-(");
  return 0; // return 0 if unable to get the time
}

// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress &address)
{
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}

