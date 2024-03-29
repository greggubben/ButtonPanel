/*
 * Display a button panel on my desk to control things around the office.
 */

#include <Arduino.h>
#include <LittleFS.h>
#include <WiFiManager.h>          // For managing the Wifi Connection
#include <ESP8266mDNS.h>          // For running OTA and Web Server
#include <WiFiUdp.h>              // For running OTA and Network time
#include <ArduinoOTA.h>           // For running OTA
#include <SPI.h>                  // For display
#include <Adafruit_GFX.h>         // For display
#include <Adafruit_ILI9341.h>     // For display
#include <XPT2046_Touchscreen.h>  // For display
#include <NTPClient.h>            // For getting the network time
#include <vector>
#include "Menu.h"
#include "TouchHandler.h"
#include "onair.h"
#include "headControl.h"
#include "status.h"
//#include <TelnetSerial.h>       // For debugging via Telnet

// Uncomment the following define to debug the screen calibration
//#define CALIBRATE_DEBUG


//
// Device Info
//
const char* devicename = "deskButtonPanel";   // Name of device used by WiFiManager and OTA/mDNS
const char* devicepassword = "paneladmin";    // Password used by WiFiManager and OTA


//
// Configuration to blink built in LED during startup
//
#ifndef LED_BUILTIN
#define LED_BUILTIN 13 // ESP32 DOES NOT DEFINE LED_BUILTIN
#endif
//for using LED as a startup status indicator
#include <Ticker.h>
Ticker ticker;            // Process to blink LED during setup
boolean ledState = LOW;   // Used for blinking LEDs when WifiManager in Connecting and Configuring
const int ledPin =  LED_BUILTIN;  // On board LED used to show status

//
// Config to get the time from an NTP server pool
//
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

//
// TFT Touchscreen definitions
//
#define TFT_CS D0  //for D1 mini or TFT I2C Connector Shield (V1.1.0 or later)
#define TFT_DC D8  //for D1 mini or TFT I2C Connector Shield (V1.1.0 or later)
#define TFT_RST -1 //for D1 mini or TFT I2C Connector Shield (V1.1.0 or later)
#define TS_CS D3   //for D1 mini or TFT I2C Connector Shield (V1.1.0 or later)
#define TFT_LED D2  //for D1 mini to adjust brightness

// #define TFT_CS 14  //for D32 Pro
// #define TFT_DC 27  //for D32 Pro
// #define TFT_RST 33 //for D32 Pro
// #define TS_CS  12 //for D32 Pro

// Visual part of the Display
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);
// Touch response part of the display
XPT2046_Touchscreen ts(TS_CS);

// This is calibration data for the raw touch data to the screen coordinates
#define TS_MINX 700
#define TS_MINY 500
#define TS_MAXX 3500
#define TS_MAXY 3800

// Time to wait to dim or turn off screen
#define SCREEN_DIM_DELAY 60000    // 1 minute
#define SCREEN_OFF_DELAY 600000   // 10 minutes
#define SCREEN_BRIGHTNESS_FULL 255
#define SCREEN_BRIGHTNESS_DIM 128
#define SCREEN_BRIGHTNESS_OFF 0

bool screen_dimmed = false;
bool screen_off = false;
bool screenAlwaysOn = true;

//
// Menu definition
//

MenuPage btlTopMenu = MenuPage("BTL", 1, 1, ILI9341_LIGHTGREY);
//MenuPage lightsTopMenu = MenuPage("Light", 1, 1, ILI9341_YELLOW);

//vector<MenuPage*> topMenuList = {&onairTopMenu, &headTopMenu, &btlTopMenu, &lightsTopMenu, &statusTopMenu};
vector<MenuPage*> topMenuList = {&onairTopMenu, &headTopMenu, &btlTopMenu, &statusTopMenu};
Menu menu = Menu(&tft, &topMenuList);

TouchHandler touchHandler = TouchHandler(&ts);



/*************************************************
 * Callback Utilities during setup
 *************************************************/
 
/*
 * Blink the LED.
 * If on  then turn off
 * If off then turn on
 */
void tick()
{
  //toggle state
  digitalWrite(ledPin, !digitalRead(ledPin));     // set pin to the opposite state
}

/*
 * gets called when WiFiManager enters configuration mode
 */
void configModeCallback (WiFiManager *myWiFiManager) {
  //Serial.println("Entered config mode");
  //Serial.println(WiFi.softAPIP());
  //if you used auto generated SSID, print it
  //Serial.println(myWiFiManager->getConfigPortalSSID());
  //entered config mode, make led toggle faster
  ticker.attach(0.2, tick);
}


/*************************************************
 * Callback Utility for determining if screen should always be on
 *************************************************/
 
/*
 * Determine if screen should always be on.
 * Workday between 7:00am and 5pm
 */
void alwaysOnTick()
{
  int day = timeClient.getDay();
  int hour = timeClient.getHours();
  if ((1 <= day && day <= 5) && (7 <= hour && hour <= 17)) {
    screenAlwaysOn = true;
  }
  else {
    screenAlwaysOn = false;
  }
}


/**********************************
 * Global variables and functions
 **********************************/

// Global Variables
int16_t screenWidth;
int16_t screenHeight;
bool wasTouched = false;
unsigned long lastPressTime = 0;  // the last time the screen touch state changed


/*
 * Calibrate the raw input to the screen pixels
 */
void calibrateTouch(int16_t *pressX, int16_t *pressY) {

#ifdef CALIBRATE_DEBUG
  tft.fillRect(0, screenHeight-80, screenWidth, 80, ILI9341_BLACK);
  tft.setTextColor(ILI9341_YELLOW);
  tft.setTextSize(2);
  tft.setCursor(0,screenHeight-80);
  tft.printf("raw x:%4d y:%4d\n", *pressX, *pressY);

  Serial.print("raw x = ");
  Serial.println(*pressX);
  Serial.print("raw y = ");
  Serial.println(*pressY);
#endif

  // Scale using the calibration #'s
  // and rotate coordinate system
  //TODO: Resolve map() ambiguity with mDNS
  //tsPoint->y = map(tsPoint->y, TS_MINY, TS_MAXY, 0, screenHeight);
  *pressY = (*pressY - TS_MINY) * (screenHeight) / (TS_MAXY - TS_MINY);
  //tsPoint->x = map(tsPoint->x, TS_MINX, TS_MAXX, 0, screenWidth);
  *pressX = (*pressX - TS_MINX) * (screenWidth) / (TS_MAXX - TS_MINX);

#ifdef CALIBRATE_DEBUG
  tft.printf("map x:%4d y:%4d\n", *pressX, *pressY);
  Serial.print("map x = ");
  Serial.println(*pressX);
  Serial.print("map y = ");
  Serial.println(*pressY);
#endif

  *pressY = screenHeight - *pressY;
  //*pressX = *pressX;

#ifdef CALIBRATE_DEBUG
  tft.printf("ali x:%4d y:%4d\n", *pressX, *pressY);
  Serial.print("aligned X = ");
  Serial.println(*pressX);
  Serial.print("aligned Y = ");
  Serial.println(*pressY);
#endif

  if(*pressX > screenWidth) {
    *pressX = screenWidth;
  }
  if(*pressX < 0) {
    *pressX = 0;
  }
  if(*pressY > screenHeight) {
    *pressY = screenHeight;
  }
  if(*pressY < 0) {
    *pressY = 0;
  }

#ifdef CALIBRATE_DEBUG
  tft.fillCircle(*pressX, *pressY, 3, ILI9341_YELLOW);
  tft.printf("bnd x:%4d y:%4d\n", *pressX, *pressY);
  Serial.print("bounded X = ");
  Serial.println(*pressX);
  Serial.print("bounded Y = ");
  Serial.println(*pressY);
  //delay(5000);
#endif

}

/*
 * Call back function when a Touch Event is triggered
 */
void touchEventCallback(Event *event) {
  menu.eventHandler(event);
}


/*
 * Set up the device and menus
 */
void setup(void)
{
  Serial.begin(115200);
  Serial.println(ESP.getFullVersion());

  // Set builtin LED as Output
  pinMode(ledPin, OUTPUT);
  pinMode(TFT_LED, OUTPUT);
  digitalWrite(TFT_LED, SCREEN_BRIGHTNESS_FULL);
  screen_dimmed = false;
  screen_off = false;

  tft.begin();
  if (!ts.begin()) { 
    Serial.println("Unable to start touchscreen.");
  } 
  else { 
    Serial.println("Touchscreen started."); 
  }

  tft.fillScreen(ILI9341_BLACK);
  // origin = left,top landscape (USB left upper)
  tft.setRotation(1);
  ts.setRotation(3);

  screenWidth = tft.width();
  screenHeight = tft.height();

  tft.println(ESP.getFullVersion());
  tft.println();

  // start ticker to slow blink LED strip during Setup
  ticker.attach(0.6, tick);

  //
  // Set up the Wifi Connection
  //
  WiFi.hostname(devicename);
  WiFi.mode(WIFI_STA);      // explicitly set mode, esp defaults to STA+AP
  WiFiManager wm;

  // wm.resetSettings();    // reset settings - for testing
  wm.setAPCallback(configModeCallback); //set callback that gets called when connecting to previous WiFi fails, and enters Access Point mode
  //if it does not connect it starts an access point with the specified name here  "AutoConnectAP"
  if (!wm.autoConnect(devicename,devicepassword)) {
    //Serial.println("failed to connect and hit timeout");
    //reset and try again, or maybe put it to deep sleep
    ESP.restart();
    delay(1000);
  }
  tft.println("Connected");
  tft.print("IP: ");
  tft.println(WiFi.localIP());


  //
  // Set up the Multicast DNS
  // needed for OTA
  //
  MDNS.begin(devicename);
  tft.println("mDNS Started");

  //
  // Set up OTA
  //
  // ArduinoOTA.setPort(8266);
  ArduinoOTA.setHostname(devicename);
  ArduinoOTA.setPassword(devicepassword);
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_FS
      type = "filesystem";
    }
    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    //Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    //Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    //Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    //Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      //Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      //Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      //Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      //Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      //Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
  tft.println("OTA Started");

  // Set up the time
  timeClient.setTimeOffset(-4*3600);
  timeClient.setUpdateInterval(3600000);
  timeClient.begin();
  timeClient.update();
  tft.println("Time Started");

  touchHandler.start(&touchEventCallback);

  // Set up On Air
  onairSetup(&tft);
  // Set up On Air
  headControlSetup(&tft);
  // Set up Status
  statusSetup(&tft, &timeClient);

  //
  // Keep startup info on the screen for a bit
  //
  delay(1000);

  if (menu.setup()) {
    Serial.println("MENU started");
    tft.println("Menu Started");
  }
  else {
    Serial.println("MENU not started");
    tft.println("Menu not Started");
  }
  menu.setActiveTopMenu(&topMenuList, &onairTopMenu);

  // Hold one more time incase there is some info from menu setup
  delay(1000);

  // refesh all and display menu
  menu.draw();


  //
  // Done with Setup
  //
  ticker.detach();            // Stop blinking the LED
  digitalWrite(ledPin, HIGH); // set pin to the opposite state
  ticker.attach(5*60, alwaysOnTick);  // Check if screen should always be on

}


/*
 * Handle Touch Events along with any housekeeping tasks
 * like MDNS and OTA events.
 */
void loop() {
  // Handle any requests
  ArduinoOTA.handle();
  MDNS.update();
  timeClient.update();

  bool isTouched = touchHandler.detectEvent(&calibrateTouch);

    // There is a change in touch state
  if ((isTouched != wasTouched) || (screenAlwaysOn && (screen_dimmed || screen_off))) {
    // Change in touch state
    digitalWrite(TFT_LED, SCREEN_BRIGHTNESS_FULL);
    screen_dimmed = false;
    screen_off = false;
    lastPressTime = millis();
  }

  // Is it time to dim the screen?
  if (!screenAlwaysOn && ((!screen_dimmed) && ((millis() - lastPressTime) > SCREEN_DIM_DELAY))) {
    digitalWrite(TFT_LED, SCREEN_BRIGHTNESS_DIM);
    screen_dimmed = true;
  }

  // Is it time to turn off the screen?
  if (!screenAlwaysOn && ((!screen_off) && ((millis() - lastPressTime) > SCREEN_OFF_DELAY))) {
    digitalWrite(TFT_LED, SCREEN_BRIGHTNESS_OFF);
    screen_off = true;
  }

  wasTouched = isTouched;
}
