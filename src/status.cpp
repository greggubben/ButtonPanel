/*
 * Functions for displaying button panel status
 * using the menu system.
 */

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>  // For REST based calls
#include <WiFiUdp.h>              // For running OTA
#include <NTPClient.h>            // For getting the latest Date/Time

#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include "Menu.h"
#include "status.h"

//
// NTP
//
const long utcOffsetInSeconds = -5 * 3600; // EDT is -5 hours from UTC
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);
String months[12]={"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

//
// Menu Definition
//
MenuButton statusResetButton = MenuButton("Reset", 0, 1, ILI9341_RED, &statusResetShortPress);
MenuButton statusConfigButton = MenuButton("Config", 1, 1, BUTTONPANEL_STATUS_COLOR, &statusConfigShortPress, &statusConfigLongPress);
vector<MenuButton*> statusButtonList = {&statusResetButton, &statusConfigButton};
MenuPage statusTopMenu = MenuPage("Status", 2, 2, &statusButtonList, BUTTONPANEL_STATUS_COLOR);



/*****************************
 * Status Menu functions and callbacks
 *****************************/

// Show the Status on the page
void showStatus (Adafruit_GFX *tft, int16_t panelX, int16_t panelY, int16_t panelWidth, int16_t panelHeight) {

    timeClient.update();
    unsigned long epochTime = timeClient.getEpochTime();
    struct tm *ptm = gmtime ((time_t *)&epochTime);
    //int currentMonth = ptm->tm_mon;
    int currentDay = ptm->tm_mday;
    String currentMonth = months[ptm->tm_mon];
    int currentYear = ptm->tm_year+1900;

    tft->setCursor(panelX, panelY + BUTTONPANEL_STATUS_PADDING_TOP);
    tft->setTextSize(2);
    tft->setTextColor(BUTTONPANEL_STATUS_COLOR);


    //tft->printf("Date: %d-%s-%d\n", currentDay, currentMonth, currentYear);
    tft->print("Date: ");
    tft->print(currentDay);
    tft->print("-");
    tft->print(currentMonth);
    tft->print("-");
    tft->println(currentYear);
    tft->print("Time: ");
    tft->println(timeClient.getFormattedTime());
    tft->print("SSID: ");
    tft->println(WiFi.SSID());
    tft->print("  IP: ");
    tft->println(WiFi.localIP());

}


//
// Perform Status Button Panel first time set up
void statusSetup (Adafruit_GFX *tft) {
  // Set up Status
  tft->println("Setup Status");


  // Configure Menu
  statusTopMenu.setDrawPanel(&showStatus);
  

  //Start NTP Client
  timeClient.begin();
  tft->println("Time Client Started");


}



//
// Reset the Button Panel
//
bool statusResetShortPress () {
    ESP.restart();
    delay(1000);
    return true;
}

//
// Turn on Configureation Web Page for Button Panel
//
bool statusConfigShortPress () {
    statusConfigButton.setActive();
    //wm.startConfigPortal();

    return true;
}

//
// Turn off Configureation Web Page for Button Panel
//
bool statusConfigLongPress () {
    statusConfigButton.setInactive();
    //wm.stopConfigPortal();

    return true;
}