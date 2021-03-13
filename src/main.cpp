/*
 * Display a button panel on my desk to control things around the office.
 */

#include <Arduino.h>
#include <LittleFS.h>
#include <WiFiManager.h>        // For managing the Wifi Connection
#include <ESP8266mDNS.h>        // For running OTA and Web Server
#include <WiFiUdp.h>            // For running OTA
#include <ArduinoOTA.h>         // For running OTA
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <XPT2046_Touchscreen.h>
#include <vector>
#include "Menu.h"
#include "onair.h"
#include "headControl.h"
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
// TFT Touchscreen definitions
//
#define TFT_CS D0  //for D1 mini or TFT I2C Connector Shield (V1.1.0 or later)
#define TFT_DC D8  //for D1 mini or TFT I2C Connector Shield (V1.1.0 or later)
#define TFT_RST -1 //for D1 mini or TFT I2C Connector Shield (V1.1.0 or later)
#define TS_CS D3   //for D1 mini or TFT I2C Connector Shield (V1.1.0 or later)

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

// Define minimum delays for when screen is touched
#define DEBOUNCE_DELAY 50
#define LONGPRESS_DELAY 250


//
// Menu definition
//

vector<MenuTop*> topMenuList = {&onairTopMenu, &headTopMenu};
Menu menu = Menu(&tft, &ts, &topMenuList);



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


/**********************************
 * Global variables and functions
 **********************************/

// Global Variables
int16_t screenWidth;
int16_t screenHeight;
bool wasTouched = false;
bool shortPress = false;
bool longPress = false;
unsigned long lastPressTime = 0;  // the last time the output pin was toggled
MenuItem *pressedButton = nullptr;


/*
 * Calibrate the raw input to the screen pixels
 */
void calibrateTouch(TS_Point *tsPoint, int16_t *pressX, int16_t *pressY) {

#ifdef CALIBRATE_DEBUG
  tft.fillRect(0, screenHeight-80, screenWidth, 80, ILI9341_BLACK);
  tft.setTextColor(ILI9341_YELLOW);
  tft.setTextSize(2);
  tft.setCursor(0,screenHeight-80);
  tft.printf("raw x:%4d y:%4d\n", tsPoint->x, tsPoint->y);

  Serial.print("raw tsPoint.x = ");
  Serial.println(tsPoint->x);
  Serial.print("raw tsPoint.y = ");
  Serial.println(tsPoint->y);
#endif

  // Scale using the calibration #'s
  // and rotate coordinate system
  //TODO: Resolve map() ambiguity with mDNS
  //tsPoint->y = map(tsPoint->y, TS_MINY, TS_MAXY, 0, screenHeight);
  tsPoint->y = (tsPoint->y - TS_MINY) * (screenHeight) / (TS_MAXY - TS_MINY);
  //tsPoint->x = map(tsPoint->x, TS_MINX, TS_MAXX, 0, screenWidth);
  tsPoint->x = (tsPoint->x - TS_MINX) * (screenWidth) / (TS_MAXX - TS_MINX);

#ifdef CALIBRATE_DEBUG
  tft.printf("map x:%4d y:%4d\n", tsPoint->x, tsPoint->y);
  Serial.print("map tsPoint.x = ");
  Serial.println(tsPoint->x);
  Serial.print("map tsPoint.y = ");
  Serial.println(tsPoint->y);
#endif

  *pressY = screenHeight - tsPoint->y;
  *pressX = tsPoint->x;

#ifdef CALIBRATE_DEBUG
  tft.printf("ali x:%4d y:%4d\n", *pressX, *pressY);
  Serial.print("aligned pressX = ");
  Serial.println(*pressX);
  Serial.print("aligned pressY = ");
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
  Serial.print("bounded pressX = ");
  Serial.println(*pressX);
  Serial.print("bounded pressY = ");
  Serial.println(*pressY);
  //delay(5000);
#endif

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

  tft.begin();
  if (!ts.begin()) { 
    Serial.println("Unable to start touchscreen.");
  } 
  else { 
    Serial.println("Touchscreen started."); 
  }

  tft.fillScreen(ILI9341_BLACK);
  // origin = left,top landscape (USB left upper)
  tft.setRotation(0);
  ts.setRotation(0);

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

  // Set up On Air
  onairSetup(&tft);
  

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
  wasTouched = false;
  shortPress = false;
  longPress = false;
  lastPressTime = millis();


  //
  // Done with Setup
  //
  ticker.detach();    // Stop blinking the LED
}


/*
 * Handle Touch Events along with any housekeeping tasks
 * like MDNS and OTA events.
 */
void loop() {
  // Handle any requests
  ArduinoOTA.handle();
  MDNS.update();

  bool redraw = false;

  // Get current state of screen being touched
  bool isTouched = ts.touched();

  // There is a change in touch state
  if (isTouched != wasTouched) {
    // Change in touch state
    lastPressTime = millis();
  }

  // Make sure the screen touch state has stabilized
  if ((millis() - lastPressTime) > DEBOUNCE_DELAY) {

    // Is there a change in touch state?
    if (isTouched != shortPress) {

      // New press
      if (isTouched) {
        // On first press:
        shortPress = true;        // New presses are treated as short to start
        longPress = false;        // New press, don't know if long yet
        pressedButton = nullptr;  // New press, don't know button yet

        // Find the calibrated X and Y of the press
        int16_t pressX;
        int16_t pressY;
        TS_Point point = ts.getPoint();
        calibrateTouch(&point, &pressX, &pressY);

        // Figure out which button was pressed
        tie(redraw, pressedButton) = menu.handleTouch(pressX, pressY);
        //if (pressedButton) {
          // Found a button pressed and it should now be set as active
          // need to redraw screen to show active
          //redraw = true;
        //}
      }
      else {
        // Stopped touching
        if (shortPress && !longPress) {
          // Handle any Short Press callback actions
          // Long press is fired as soon as it occurs
          if (pressedButton) {
            redraw = pressedButton->callbackShortPress();
          }
        }
        // Reset everything for next touch event
        shortPress = false;
        longPress = false;
        pressedButton = nullptr;
      }
    }
    // Is the screen still being touched?
    // Short Press being true ensures the press is still active
    if (shortPress && (millis() - lastPressTime) > LONGPRESS_DELAY) {
      if (!longPress) {
        // New Long press occurred
        longPress = true;
        // Handle any Long Press callback actions
        // Do immediately, do not wait for unTouch
        if (pressedButton) {
          redraw = pressedButton->callbackLongPress();
        }
      }
    }
  }

  // Re-draw the menus
  if (redraw) {
    menu.draw();
  }

  // Keep track of last state so changes can be identified.
  wasTouched = isTouched;
}
