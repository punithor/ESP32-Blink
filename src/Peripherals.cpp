#include <Arduino.h>
#include <DHT.h>
#include "State.h"
#include "Peripherals.h"
#include "config.h"


DHT   dht(DHT_PIN , DHT_TYPE);

float mapFloat(long x, long in_min, long in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void sample_sensor(void)
{
    int raw_current = analogRead(CURRENT_PIN); // 0 to 4095
    int raw_voltage = analogRead(VOLTAGE_PIN); // 0 to 4095

    
    voltage = mapFloat(raw_voltage, 0, 4095, 0, 250); // map to 0-250V

    if(bayStatus == "CHARGING")
    {
    current = mapFloat(raw_current, 0, 4095, 0, 32); // map to 0-32A
    }
    else
   {
     current = 0; // if not charging, current is 0
   }

    

    power = voltage * current; // calculate power in watts  
   
    //to read temperature 
    float t  = dht.readTemperature(DHT_PIN );
    if(!(isnan(t))) temperature = t;
    



}


float recentAvgCurrent(void)
{
   float sum = 0;

   for(int i = 0; i < 10; i++)
   {
      sum += current;
   }
   return sum / 5;
}

bool plugin_flag_once  = 1;
bool plugout_flag_once  = 1;

void plug_status(void)
{
   bool pluginReading = digitalRead(BTN_PLUGIN);
   // detect the sw is pressed

   if( pluginReading == LOW && plugin_flag_once )
   {
      sessionStartMs = millis(); // record the start time of the session
   // plug in switch is pressed
   plugin_flag_once = 0;
   // change bay_status FREE to charging
   if(bayStatus == "FREE")
   {
      bayStatus = "CHARGING";
      Serial.println("Bay1 plugin detected, Bay is Charging");
      
      digitalWrite(RELAY_PIN, HIGH); // turn on relay
   }
   //update leds
   }

   if( pluginReading == HIGH )
   {
      plugin_flag_once = 1;
   }
   // plug out switch is pressed
   bool plugoutReading = digitalRead(BTN_PLUGOUT);
   // detect the sw is pressed

   if( plugoutReading == LOW && plugout_flag_once )
   {
   // plug in switch is pressed
   plugout_flag_once = 0;
   // change bay_status FREE to charging
   if(bayStatus == "CHARGING")
   {
      bayStatus = "FREE";
      Serial.println("Bay1 plugout detected, Bay is FREE");
   }
   //update leds
   }

   if( plugoutReading == HIGH )
   {
      plugout_flag_once = 1;
   }
   

}

// void update_led_status(void)
// {
// if (bayStatus == "FREE")
// {
//    digitalWrite(LED_GREEN, HIGH);
//    digitalWrite(LED_YELLOW, LOW);
   
// }
// else if (bayStatus == "CHARGING")
// {
//    digitalWrite(LED_GREEN, LOW);
//    digitalWrite(LED_YELLOW, HIGH);
   
// }
// }