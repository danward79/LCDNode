#include "utility/font_helvB24.h"
#include "utility/font_helvB14.h"
#include "utility/font_helvB12.h"
#include "utility/font_clR4x6.h"
#include "utility/font_clR6x8.h"

//------------------------------------------------------------------
// Draws a page showing a single power and energy value in big font
//------------------------------------------------------------------
void draw_power_page(char* powerstr, double powerval, char* energystr,  double energyval)
{ 
  glcd.clear();
  glcd.fillRect(0,0,128,64,0);
  
  char str[30];    			 //variable to store conversion 
  glcd.setFont(font_clR6x8);      
  strcpy(str,powerstr);  
  strcat(str," NOW:"); 
  glcd.drawString(0,0,str);
  strcpy(str,energystr);  
  strcat(str," TODAY:"); 
  glcd.drawString(0,38,str);

  // power value
  glcd.setFont(font_helvB24);
  itoa((int)powerval,str,10);
  strcat(str,"w");   
  glcd.drawString(3,9,str);
  
  // kwh per day value
  glcd.setFont(font_clR6x8);
  if (energyval<10.0) dtostrf(energyval,0,1,str); else itoa((int)energyval,str,10);
  strcat(str,"kWh");
  glcd.drawString(87,38,str);        
}


//------------------------------------------------------------------
// Draws a footer showing GLCD temperature and the time
//------------------------------------------------------------------
void draw_temperature_time_footer(double temp, double mintemp, double maxtemp, double hour, double minute)
{
  glcd.drawLine(0, 47, 128, 47, WHITE);     //middle horizontal line 

  char str[30];
  // GLCD Temperature
  glcd.setFont(font_helvB12);  
  dtostrf(temp,0,1,str); 
  strcat(str,"C");
  glcd.drawString(0,50,str);  
  
  // Minimum and maximum GLCD temperature
  glcd.setFont(font_clR4x6);             
  itoa((int)mintemp,str,10);
  strcat(str,"C");
  glcd.drawString_P(46,51,PSTR("MIN"));
  glcd.drawString(62,51,str);
               
  itoa((int)maxtemp,str,10); 
  strcat(str,"C");
  glcd.drawString_P(46,59,PSTR("MAX"));
  glcd.drawString(62,59,str);
  
  // Time
  char str2[5];
  itoa((int)hour,str,10);
  if  (minute<10) strcat(str,": 0"); else strcat(str,": ");
  itoa((int)minute,str2,10);
  strcat(str,str2); 
  glcd.setFont(font_helvB12);
  glcd.drawString(82,50,str);

}


void draw_weather_page(int light, int humidity, int temperature, int dewpoint, int cloudbase, int32_t pressure)
{
  glcd.clear();
  glcd.setFont(font_clR6x8);
  glcd.drawString(40, 0, "WEATHER");
  
  glcd.setFont(font_clR4x6);           

  glcd.drawString_P(2,9,PSTR("Temperature: "));
  glcd.drawString_P(2,16,PSTR("Humidity: "));
  glcd.drawString_P(2,23,PSTR("Light: "));
  glcd.drawString_P(2,30,PSTR("Pressure: "));
  glcd.drawString_P(2,37,PSTR("Cloudbase: "));
  
  char str[20];
 
  sprintf(str, "%d.%d C", temperature/10, temperature%10);
  glcd.drawString(55, 9, str);
  
  sprintf(str, "%d", humidity/10);
  strcat(str, " %");
  glcd.drawString(55, 16, str);

  sprintf(str, "%d", light);
  strcat(str, " %");
  glcd.drawString(55, 23, str);
  
  sprintf(str, "%d hPa", pressure/100);
  glcd.drawString(55, 30, str);
  
  sprintf(str, "%d ft", cloudbase);
  glcd.drawString(55, 37, str);

}


void draw_history_page_nosolar(double usekwh[7])
{
  glcd.clear;
  glcd.fillRect(0,0,128,64,0);
  
  char str[30]; 
  
  glcd.setFont(font_clR6x8);
  glcd.drawString_P(40,0,PSTR("History"));
  
  glcd.setFont(font_clR4x6);           

  glcd.drawString_P(2,16,PSTR("Today"));
  glcd.drawString_P(2,23,PSTR("Yesterday"));
  glcd.drawString_P(2,30,PSTR("2 days ago"));
  glcd.drawString_P(2,37,PSTR("3 days ago"));
  glcd.drawString_P(2,44,PSTR("4 days ago"));
  glcd.drawString_P(2,51,PSTR("5 days ago"));
  glcd.drawString_P(2,58,PSTR("6 days ago"));
  
  // draw grid consumption history
  char kWh[4]="kWh";

  dtostrf((usekwh[0]),0,1,str); strcat(str,kWh);
  glcd.drawString(52,16,str);
  
  dtostrf((usekwh[1]),0,1,str); strcat(str,kWh);
  glcd.drawString(52,23,str);
  
  dtostrf((usekwh[2]),0,1,str);  strcat(str,kWh);
  glcd.drawString(52,30,str);
  
  dtostrf((usekwh[3]),0,1,str); strcat(str,kWh);
  glcd.drawString(52,37,str);
  
  dtostrf((usekwh[4]),0,1,str); strcat(str,kWh);
  glcd.drawString(52,44,str);
  
  dtostrf((usekwh[5]),0,1,str); strcat(str,kWh);
  glcd.drawString(52,51,str);
  
  dtostrf((usekwh[6]),0,1,str); strcat(str,kWh);
  glcd.drawString(52,58,str);
  
  glcd.refresh();
}