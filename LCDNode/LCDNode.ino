/*
//------------------------------------------------------------------------------
// emonGLCD Based LCD Node
// Based on original code from Open Energy Monitor and Libraries from Jeelabs

// Desired functionality
// - LEDs
// - Power Page
// - History Page
// - Weather Page

// RTC to reset Kwh counters at midnight is implemented is software. 
// Correct time is updated via NanodeRF which gets time from internet
// Temperature recorded on the emonglcd is also sent to the NanodeRF for online graphing

// emonGLCD documentation http://openEnergyMonitor.org/emon/emonglcd
// GLCD library by Jean-Claude Wippler: JeeLabs.org
// 2010-05-28 <jcw@equi4.com> http://opensource.org/licenses/mit-license.php
// Authors: Glyn Hudson and Trystan Lea
// Part of the: openenergymonitor.org project
// Licenced under GNU GPL V3
// http://openenergymonitor.org/emon/license

// THIS SKETCH REQUIRES:
// Libraries in the standard arduino libraries folder:
//	- OneWire library	http://www.pjrc.com/teensy/td_libs_OneWire.html
//	- DallasTemperature	http://download.milesburton.com/Arduino/MaximTemperature
//                           or https://github.com/milesburton/Arduino-Temperature-Control-Library
//	- JeeLib		https://github.com/jcw/jeelib
//	- RTClib		https://github.com/jcw/rtclib
//	- GLCD_ST7565		https://github.com/jcw/glcdlib
//
// Other files in project directory (should appear in the arduino tabs above)
//	- templates.ino
//------------------------------------------------------------------------------
*/

#include <JeeLib.h>
#include <GLCD_ST7565.h>
#include <avr/pgmspace.h>
GLCD_ST7565 glcd;

#include <OneWire.h>		    
#include <DallasTemperature.h>     

#include <RTClib.h>                 
#include <Wire.h>                 
RTC_Millis RTC;

//------------------------------------------------------------------------------
// RFM12B Settings
//------------------------------------------------------------------------------
#define MYNODE 11 
#define freq RF12_433MHZ  
#define group 212 

#define ONE_WIRE_BUS 5  

unsigned long fast_update, slow_update, backLightOverrideTime;

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
double temp,maxtemp,mintemp;

//---------------------------------------------------
// Data structures for transfering data between units
//---------------------------------------------------
//Node ID 5
typedef struct { int power, voltage, battery; } PayloadTX;        
PayloadTX emontx;

//Node ID 18
typedef struct {int light, humidity, temperature, dewpoint, cloudbase, battery;} PayloadWx;
PayloadWx wxtx;

//Node ID 16
typedef struct {byte light; int humidity; int temperature; byte vcc;} PayloadOutdoor;
PayloadOutdoor outdoornode;

//Node ID 17
typedef struct {byte light; int16_t temperature; int32_t pressure; int battery;} PayloadBaro;
PayloadBaro barotx;

//Node ID 11
typedef struct { int temperature; } PayloadGLCD;
PayloadGLCD emonglcd;

int hour = 12, minute = 0;
double usekwh = 0;
double use_history[7];
boolean backLightOverride = false;


#define greenLED 6      // Green bi-color LED
#define redLED 9        // Red bi-color LED
#define LDRpin 4        // analog pin of onboard lightsensor 
#define switchEnter 15
#define switchUp 16
#define switchDown 19
int cval_use;

//------------------------------------------------------------------------------
// Flow control
//------------------------------------------------------------------------------
unsigned long last_emontx;  // Used to count time from last emontx update
unsigned long last_emonbase;  // Used to count time from last emontx update
int page;

//------------------------------------------------------------------------------
// Setup
//------------------------------------------------------------------------------
void setup()
{
  delay(300); 				   //wait for power to settle before firing up the RF
  rf12_initialize(MYNODE, freq, group);
  delay(100);				   //wait for RF to settle befor turning on display
  
  glcd.begin(0x19);
  glcd.backLight(200);
  
  sensors.begin();                // start up the DS18B20 temp sensor onboard  
  sensors.requestTemperatures();
  temp = (sensors.getTempCByIndex(0));     // get inital temperture reading
  mintemp = temp; maxtemp = temp;          // reset min and max

  pinMode(greenLED, OUTPUT); 
  pinMode(redLED, OUTPUT); 
  
  RTC.begin(DateTime(__DATE__, __TIME__));
  
  //Serial.begin(9600);
  //Serial.println("LCD Node");
}

//------------------------------------------------------------------------------
// Loop
//------------------------------------------------------------------------------
void loop()
{
  
  if (rf12_recvDone())
  {
    if (rf12_crc == 0 && (rf12_hdr & RF12_HDR_CTL) == 0)  // and no rf errors
    {
      int node_id = (rf12_hdr & 0x1F);
      if (node_id == 5) //emonTx NodeID
      {
        emontx = *(PayloadTX*) rf12_data;
        last_emontx = millis();
      }  
      
      if (node_id == 16)
      {
        outdoornode = *(PayloadOutdoor*) rf12_data;
      }
      
      if (node_id == 17)
      {
        barotx = *(PayloadBaro*) rf12_data;
      }
      
      if (node_id == 18)
      {
        wxtx = *(PayloadWx*) rf12_data;
      }
      
      if (node_id == 31)			//Time transmission
      {
        RTC.adjust(DateTime(2012, 1, 1, rf12_data[1], rf12_data[2], rf12_data[3]));
        last_emonbase = millis();
      } 
    }
  }

  //------------------------------------------------------------------------------
// Display update every 200ms
//------------------------------------------------------------------------------
  if ((millis()-fast_update)>200)
  {
    fast_update = millis();
    
    DateTime now = RTC.now();
    int last_hour = hour;
    hour = now.hour();
    minute = now.minute();

    usekwh += (emontx.power * 0.2) / 3600000;
    if (last_hour == 23 && hour == 00) 
    {
      usekwh = 0; //reset Kwh/d at midnight
      for(int i = 6; i > 0; i--)
      {
        use_history[i] = use_history[i-1];
      }
    }
    
    use_history[0] = usekwh;

    cval_use = cval_use + (emontx.power - cval_use)*0.50;//smooth transitions
    
    int LDR = analogRead(LDRpin); // Read the LDR Value
    int LDRbacklight = map(LDR, 0, 1023, 50, 250); // Map the LDR from 0-1023 (Max seen 1000) to var GLCDbrightness min/max
    LDRbacklight = constrain(LDRbacklight, 0, 255); // Constrain PWM value 0-255
  
    if ((hour > 21) ||  (hour < 6)) {
      if (!backLightOverride) {
        glcd.backLight(0); 
      } else {
        glcd.backLight(LDRbacklight);
      }
      
      if ((millis() - backLightOverrideTime) > 10000) {
        glcd.backLight(0);
        backLightOverride = false;
      }
      
    } else {
      
      glcd.backLight(LDRbacklight);
      
    }
    
    //Page Control
    bool switch_state = digitalRead(switchEnter);  
    
    if (switch_state == 1)
    {
      page += 1;
      if (page > 2)
      {
        page = 0;
      }
    }
    
    switch_state = digitalRead(switchUp) or digitalRead(switchDown);  
    
    if (switch_state == 1)
    {
      backLightOverride = true;
      backLightOverrideTime = millis();
    }
    
    if (page == 0) //Standard Power Page
    {
      draw_power_page( "POWER" ,cval_use, "USE", usekwh);
      draw_temperature_time_footer(temp, mintemp, maxtemp, hour,minute);
      glcd.refresh();
    }
    else if (page == 1) //Weather Page
    {
      draw_weather_page((outdoornode.light * 100/255), outdoornode.humidity, outdoornode.temperature, wxtx.dewpoint, wxtx.cloudbase, barotx.pressure);
      
      draw_temperature_time_footer(temp, mintemp, maxtemp, hour,minute);
      glcd.refresh();
    }
    else if (page == 2) //History Page
    {
      draw_history_page_nosolar(use_history);
    }
    
  } 
  
  if ((millis()-slow_update)>10000)
  {
    slow_update = millis();

    sensors.requestTemperatures();
    temp = (sensors.getTempCByIndex(0));
    if ((temp > -50) && (temp < 70))
    {
      if (temp > maxtemp) maxtemp = temp;
      if (temp < mintemp) mintemp = temp;
    }
   
    // set emonglcd payload
     emonglcd.temperature = (int) (temp * 100); 
                     
    //send temperature data via RFM12B using new rf12_sendNow wrapper -glynhudson
     rf12_sendNow(0, &emonglcd, sizeof emonglcd);
     rf12_sendWait(2);    
  }
}
