/*************************************************************
  Download latest Blynk library here:
    https://github.com/blynkkk/blynk-library/releases/latest

  Blynk is a platform with iOS and Android apps to control
  Arduino, Raspberry Pi and the likes over the Internet.
  You can easily build graphic interfaces for all your
  projects by simply dragging and dropping widgets.

    Downloads, docs, tutorials: http://www.blynk.cc
    Sketch generator:           http://examples.blynk.cc
    Blynk community:            http://community.blynk.cc
    Follow us:                  http://www.fb.com/blynkapp
                                http://twitter.com/blynk_app

  Blynk library is licensed under MIT license
  This example code is in public domain.

 *************************************************************

  Control a color gradient on NeoPixel strip using a slider!

  For this example you need NeoPixel library:
    https://github.com/adafruit/Adafruit_NeoPixel

  App project setup:
    Slider widget (0...500) on V1
 *************************************************************/

/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <Adafruit_NeoPixel.h>

#define FLG uint8_t
#define U08 uint8_t
#define U16 uint16_t
#define U32 uint32_t

/////////////////////////////////////////////////////////////////////
// setup
#define DEF_Crawl_Length  3

/////////////////////////////////////////////////////////////////////
// Blynk App Project Token
//char auth[] = "token";

/////////////////////////////////////////////////////////////////////
// Network Setup
//char ssid[] = "SSID";
//char pass[] = "password"
#include "Config.h"

/////////////////////////////////////////////////////////////////////
#define NeoPin 14         // D14 (SCL pin on SparkFun EPS8266 Thing)  NOTE: DON'T USE PIN 16 (XPD) FOR NEOPIXELS
#define NeoNum 150
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NeoNum, NeoPin, NEO_GRB + NEO_KHZ800);

BlynkTimer UpdateTimer;

/////////////////////////////////////////////////////////////////////
// Control vars
U32 Anim_Colour;
U08 Anim_Colour_Red;
U08 Anim_Colour_Green;
U08 Anim_Colour_Blue;

U16 Animation    = 1;
U16 Offset       = 0;
U16 Speed        = 3;
U16 SpeedCounter = 0;
FLG Direction    = 0;
FLG NewAnimation = 0; // =1 when Animation is changed. Allows animation to setup itself
FLG used[NeoNum];     // array to keep track of lit LEDs
int lights;
float Angle;

/////////////////////////////////////////////////////////////////////
// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  if ( WheelPos < 85 ) {
    return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if (WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
    WheelPos -= 170;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}

/////////////////////////////////////////////////////////////////////
// update Animation
void UpdateAnimation()
{
  U16 Index;
  Serial.print(SpeedCounter);
  Serial.print(",");

  if ( ++SpeedCounter <= Speed ) {
    // slow it down
  } else {
    SpeedCounter = 0;

    // blank line to show new data easily to user
    Serial.println("Update Animation..." );

    // what animation are we running
    switch ( Animation ) {
      case 1 : {
          // rainbow
          if ( NewAnimation == 1 ) {
            // Setup
          }
          Speed = 2;

          // make the animation
          for ( Index = 0; Index < strip.numPixels(); Index++ ) {
            strip.setPixelColor(Index, Wheel((Offset + Index) * 3));
          }
          strip.show();

          // make a Rainbow of color
          Offset++;
          break;
        }
      case 2 : {
          // cylon / Knight Rider
          if ( NewAnimation == 1 ) {
            // setup
          }
          Speed = 0;

          if ( Direction == 0 ) {
            if ( ++Offset >= strip.numPixels() ) {
              Offset    = strip.numPixels();
              Direction = 1;
            }
          } else {
            if ( --Offset == 0 ) {
              Offset    = 0;
              Direction = 0;
            }
          }

          strip.clear();
          strip.setPixelColor(Offset    , Anim_Colour);
          strip.setPixelColor(Offset - 1, Anim_Colour_Red / 4, Anim_Colour_Green / 4, Anim_Colour_Blue / 4);
          strip.setPixelColor(Offset + 1, Anim_Colour_Red / 4, Anim_Colour_Green / 4, Anim_Colour_Blue / 4);
          strip.setPixelColor(Offset - 2, Anim_Colour_Red / 5, Anim_Colour_Green / 5, Anim_Colour_Blue / 5);
          strip.setPixelColor(Offset + 2, Anim_Colour_Red / 5, Anim_Colour_Green / 5, Anim_Colour_Blue / 5);
          strip.show();
          break;
        }
      case 3 :  {
          // crawling lights forward
          if ( NewAnimation == 1 ) {
            // Setup
          }
          Speed = 4;

          if ( ++Offset >= DEF_Crawl_Length ) {
            Offset = 0;
          }

          // calc the data
          for ( Index = 0; Index < strip.numPixels(); Index++ ) {
            if ( ++Offset >= DEF_Crawl_Length ) {
              Offset = 0;
            }

            if ( Offset == 0 ) {
              strip.setPixelColor(Index, Anim_Colour);
            } else {
              strip.setPixelColor(Index, 0);
            }
          }

          strip.show();
          break;
        }
      case 4 :  {
          // crawling lights backward
          if ( NewAnimation == 1 ) {
            // Setup
          }
          Speed = 4;

          if ( Offset == 0 ) {
            Offset = DEF_Crawl_Length;
          } else {
            Offset--;
          }

          // calc the data
          for ( Index = 0; Index < strip.numPixels(); Index++ ) {
            if ( Offset == 0 ) {
              Offset = DEF_Crawl_Length;
            } else {
              Offset--;
            }

            if ( Offset == 0 ) {
              strip.setPixelColor(Index, Anim_Colour);
            } else {
              strip.setPixelColor(Index, 0);
            }
          }

          strip.show();
          break;
        }
      case 5 :  {
          // crawling lights backward
          if ( NewAnimation == 1 ) {
            // Setup
          }
          Speed = 4;

          if ( Offset == 0 ) {
            Offset = DEF_Crawl_Length;
          } else {
            Offset--;
          }

          // calc the data
          for ( Index = 0; Index < strip.numPixels(); Index++ ) {
            if ( Offset == 0 ) {
              Offset = DEF_Crawl_Length;
            } else {
              Offset--;
            }

            if ( Offset == 0 ) {
              strip.setPixelColor(Index, random(0, 255),  random(0, 255),  random(0, 255));
            } else {
              strip.setPixelColor(Index, 0);
            }
          }

          strip.show();
          break;
        }
      case 6 :  {
          // random -> twinkle
          if ( NewAnimation == 1 ) {
            // Setup
          }
          Speed = 2;

          // calc the data
          for ( Index = 0; Index < strip.numPixels(); Index++ ) {
            strip.setPixelColor(Index, random(0, 255),  random(0, 255),  random(0, 255));
          }

          strip.show();
          break;
        }
      case 7 : {
          // Fire
          if ( NewAnimation == 1 ) {
            // Setup
          }
          Speed = 2;

          int r = 255;
          int g = r - 40;
          int b = 40;
          for ( Index = 0; Index < strip.numPixels(); Index++ ) {
            int flicker = random(0, 150);
            int r1 = r - flicker;
            int g1 = g - flicker;
            int b1 = b - flicker;
            if (g1 < 0) g1 = 0;
            if (r1 < 0) r1 = 0;
            if (b1 < 0) b1 = 0;
            strip.setPixelColor(Index, r1, g1, b1);
          }
          strip.show();

          break;
        }
      case 8 : {
          // alternate
          if ( NewAnimation == 1 ) {
            // Setup
          }
          Speed = 2;

          if ( Direction == 0 ) {
            Direction = 1;
            for ( Index = 0; Index < strip.numPixels(); Index++ ) {
              if (Index % 2 == 0) { // set even LED to color 1
                strip.setPixelColor(Index, Anim_Colour);
              } else { // set odd LED to color 2
                strip.setPixelColor(Index, Anim_Colour_Red / 4, Anim_Colour_Green / 4, Anim_Colour_Blue / 4);
              }
            }
          } else {
            Direction = 0;
            for ( Index = 0; Index < strip.numPixels(); Index++ ) {
              if (Index % 2 == 0) { // set even LED to color 2
                strip.setPixelColor(Index, Anim_Colour_Red / 4, Anim_Colour_Green / 4, Anim_Colour_Blue / 4);
              } else { // set odd LED to color 1
                strip.setPixelColor(Index, Anim_Colour);
              }
            }
          }
          strip.show();
          break;
        }
      case 9 : {
          if ( NewAnimation == 1 ) {
            // Setup
            for ( Index = 0; Index < strip.numPixels(); Index++ ) { // fill array with 0
              used[Index] = 0;
            }
          }

          FLG any = 1;
          for ( Index = 0; Index < strip.numPixels(); Index++ ) { // fill array with 0
            any &= used[Index] == 1;
          }

          if ( any == 1 ) {
            Serial.println("Start Over");
            // all on, so start again
            strip.clear();

            for (Index = 0; Index < strip.numPixels(); Index++) { // fill array with 0
              used[Index] = 0;
            }
          } else {
            // find another spot to fill
            FLG Found = 0;
            do {
              Index = random(0, strip.numPixels()); // pick a random LED
              if ( used[Index] == 0 ) {
                strip.setPixelColor(Index, Anim_Colour);
                used[Index] = 1; // update array to remember it is lit
                Found = 10;
              }
            } while ( ++Found < 10 ) ;
          }

          strip.show(); // display
          break;
        }
      case 10 : {
          Speed = 2;

          if ( Offset++ > strip.numPixels() ) {
            Offset = 0;
          }

          for (Index = 0; Index < strip.numPixels(); Index++) {
            Angle += 0.1F;
            if ( Angle > 3.14F ) {
              Angle = 0.0;
            }
            strip.setPixelColor(Index, int(sin(Angle) * 255), int(sin(Angle + 60) * 255), int(sin(Angle + 120) * 255));
          }
          strip.show();

          break;
        }
      case 11 : {
          Speed = 0;
          if ( ++Offset > 255) {
            Offset = 0;
          }
          for (Index = 0; Index < strip.numPixels(); Index++) {

            strip.setPixelColor(Index, Offset, 0, 0);
          }
          strip.show();

          break;
        }
      case 12 : {
          Speed = 0;
          if ( ++Offset > 255) {
            Offset = 0;
          }
          for (Index = 0; Index < strip.numPixels(); Index++) {

            strip.setPixelColor(Index, 0, Offset, 0);
          }
          strip.show();

          break;
        }
      case 13 : {
          Speed = 0;
          if ( ++Offset > 255) {
            Offset = 0;
          }

          for (Index = 0; Index < strip.numPixels(); Index++) {
            strip.setPixelColor(Index, 0, 0, Offset);
          }
          strip.show();

          break;
        }
      case 14 : {
          // Light up the strip starting from the middle
          if ( Offset++ > strip.numPixels() / 2 ) {
            strip.clear();
            Offset = 0;
            Anim_Colour = strip.Color(random(0, 255), random(0, 255), random(0, 255));
          }

          strip.setPixelColor(strip.numPixels() / 2 + Offset, Anim_Colour);
          strip.setPixelColor(strip.numPixels() / 2 - Offset, Anim_Colour);
          strip.show();
          break;
        }
      case 15 : {
          // Light up the strip starting from the middle
          if ( Offset++ > strip.numPixels() / 2 ) {
            strip.clear();
            Offset = 0;
            Anim_Colour = strip.Color(random(0, 255), random(0, 255), random(0, 255));
          }

          strip.setPixelColor(Offset, Anim_Colour);
          strip.setPixelColor(strip.numPixels() - Offset, Anim_Colour);
          strip.show();
          break;
        }
      default : {
          // ensure that we are always correct
          Animation = 1;
        }
    }
  }

  // init completed
  NewAnimation = 0;
}

/////////////////////////////////////////////////////////////////////
BLYNK_CONNECTED() {
  // Request Blynk server to re-send latest values for all pins
  Blynk.syncAll();
}

/////////////////////////////////////////////////////////////////////
BLYNK_WRITE(V1)
{
  Animation    = param.asInt();  // first index = 1
  NewAnimation = 1;

  // reset the Anim SETUP
  Offset       = 0;
  Speed        = 0;
  SpeedCounter = 0;
  Direction    = 0;
  Angle        = 0.0F;

  strip.clear();

  Serial.print  ("New Animation is: ");
  Serial.println(Animation);
}

/////////////////////////////////////////////////////////////////////
// Change the colour of the Animation
BLYNK_WRITE(V2)
{
  // as colours
  Anim_Colour_Red   = param[0].asInt();
  Anim_Colour_Green = param[1].asInt();
  Anim_Colour_Blue  = param[2].asInt();

  // combined
  Anim_Colour = strip.Color(Anim_Colour_Red, Anim_Colour_Green, Anim_Colour_Blue);

  Serial.print("New Animation Colour is: ");
  Serial.print(" Red: ");
  Serial.print(Anim_Colour_Red);
  Serial.print(" Green: ");
  Serial.print(Anim_Colour_Green);
  Serial.print(" Blue: ");
  Serial.println(Anim_Colour_Blue );
}

/////////////////////////////////////////////////////////////////////
void setup()
{
  // Debug console
  Serial.begin(115200);

  // Setup Blynk
  Blynk.begin(auth, ssid, pass);

  // Setup NeoPixel
  strip.begin();
  strip.show();
  Anim_Colour = strip.Color(255, 0, 0);

  // Setup a function to be called every so often
  UpdateTimer.setInterval(50L, UpdateAnimation);
}

/////////////////////////////////////////////////////////////////////
void loop()
{
  Blynk.run();
  UpdateTimer.run();
}


