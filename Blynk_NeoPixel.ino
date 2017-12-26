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
#define BLYNK_DEBUG

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <Adafruit_NeoPixel.h>
#include "Config.h"

#define FLG uint8_t
#define U08 uint8_t
#define U16 uint16_t
#define U32 uint32_t
#define ArrayElements(ar) ( sizeof(ar) / sizeof(ar[0]) )

#define DEF_Crawl_Length  3
#define DEF_Crawl_LengthB 4

#define BOTTOM_INDEX  0
#define TOP_INDEX    (strip.numPixels() / 2)
#define EVENODD      (strip.numPixels() % 2)

/////////////////////////////////////////////////////////////////////
// Blynk App Project Token
//char auth[] = "MyToken";

/////////////////////////////////////////////////////////////////////
// Network Setup
//char ssid[] = "MySSID";
//char pass[] = "MyPassword";

/////////////////////////////////////////////////////////////////////
#define NeoPin 14         // D14 (SCL pin on SparkFun EPS8266 Thing)  NOTE: DON'T USE PIN 16 (XPD) FOR NEOPIXELS
#define NeoNum 150
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NeoNum, NeoPin, NEO_GRB + NEO_KHZ800);

BlynkTimer UpdateTimer;

/////////////////////////////////////////////////////////////////////
// Control vars
U32 Anim1_Colour;
U08 Anim1_Colour_Red;
U08 Anim1_Colour_Green;
U08 Anim1_Colour_Blue;

U32 Anim2_Colour;
U08 Anim2_Colour_Red;
U08 Anim2_Colour_Green;
U08 Anim2_Colour_Blue;

FLG LEDs[NeoNum];

U16 Animation    = 2;
U16 Offset1      = 0;
U16 Offset2      = 0;
U16 Speed        = 2;
U16 SpeedCounter = 0;
U16 Direction    = 0;

FLG OverlayNightRider   = 0;
U16 Direction_NR        = 0;
U16 Offset_NR           = 10;
U16 LengthNR            = 10;
U16 SpeedNR             = 2;
U32 AnimNR_Colour       = 0;
U08 AnimNR_Colour_Red   = 255;
U08 AnimNR_Colour_Green = 0;
U08 AnimNR_Colour_Blue  = 0;
U32 PixelsNR[NeoNum];

U16 idex = 0;        //-LED INDEX (0 to NUM_LEDS-1
U16 idx_offset = 0;  //-OFFSET INDEX (BOTTOM LED TO ZERO WHEN LOOP IS TURNED/DOESN'T REALLY WORK)
U16 ihue = 0;        //-HUE (0-360)
U16 ibright = 0;     //-BRIGHTNESS (0-255)
U16 isat = 0;        //-SATURATION (0-255)
U16 bouncedirection = 0;  //-SWITCH FOR COLOR BOUNCE (0-1)

/////////////////////////////////////////////////////////////////////
// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  if ( WheelPos < 85 ) {
    return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if ( WheelPos < 170 ) {
    WheelPos -= 85;
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
    WheelPos -= 170;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}

/////////////////////////////////////////////////////////////////////
//-CONVERT HSV VALUE TO RGB
void HSVtoRGB(int hue, int sat, int val, int colors[3]) {
  // hue: 0-359, sat: 0-255, val (lightness): 0-255
  int r, g, b, base;

  if (sat == 0) { // Achromatic color (gray).
    colors[0] = val;
    colors[1] = val;
    colors[2] = val;
  } else  {
    base = ((255 - sat) * val) >> 8;
    switch (hue / 60) {
      case 0:
        r = val;
        g = (((val - base) * hue) / 60) + base;
        b = base;
        break;
      case 1:
        r = (((val - base) * (60 - (hue % 60))) / 60) + base;
        g = val;
        b = base;
        break;
      case 2:
        r = base;
        g = val;
        b = (((val - base) * (hue % 60)) / 60) + base;
        break;
      case 3:
        r = base;
        g = (((val - base) * (60 - (hue % 60))) / 60) + base;
        b = val;
        break;
      case 4:
        r = (((val - base) * (hue % 60)) / 60) + base;
        g = base;
        b = val;
        break;
      case 5:
        r = val;
        g = base;
        b = (((val - base) * (60 - (hue % 60))) / 60) + base;
        break;
    }
    colors[0] = r;
    colors[1] = g;
    colors[2] = b;
  }
}

/////////////////////////////////////////////////////////////////////
//-FIND INDEX OF ANTIPODAL OPPOSITE LED
int antipodal_index(int i)
{
  int iN = i + TOP_INDEX;
  if (i >= TOP_INDEX) {
    iN = ( i + TOP_INDEX ) % strip.numPixels();
  }
  return iN;
}

/////////////////////////////////////////////////////////////////////
void Overlay_NightRider(FLG JustMerge)
{
  U16 Index;
  
  Serial.print  ("Add Night Rider Offset: ");
  Serial.print  (Offset_NR);
  Serial.print  (" Speed: ");
  Serial.print  (SpeedNR);
  Serial.print  (" Length: ");
  Serial.println(LengthNR);

  if ( Direction_NR == 0 ) {
    Offset_NR += SpeedNR;
    if ( Offset_NR >= (strip.numPixels() - LengthNR) ) {
      Offset_NR    = strip.numPixels() - LengthNR;
      Direction_NR = 1;
    }
  } else {
    if ( Offset_NR <= SpeedNR ) {
      Offset_NR    = 0;
      Direction_NR = 0;
    } else {
      Offset_NR -= SpeedNR;
    }
  }

  // remove
  if ( JustMerge == 0 ) {
    for ( Index = 0; Index < strip.numPixels(); Index++) {
      strip.setPixelColor(Index, strip.getPixelColor(Index) ^ PixelsNR[Index]);
    }
  }

  // clear old amination data
  for ( Index = 0; Index < strip.numPixels(); Index++) {
    PixelsNR[Index] = 0;
  }

  // build the new animation
  for ( Index = 0; Index < LengthNR; Index++ ) {
    PixelsNR[Offset_NR + Index] = AnimNR_Colour;
  }

  // merge
  for ( Index = 0; Index < strip.numPixels(); Index++) {
    strip.setPixelColor(Index, strip.getPixelColor(Index) ^ PixelsNR[Index]);
  }
}

/////////////////////////////////////////////////////////////////////
void AH_OFF(void)
{
  Speed = 50;
  strip.clear();
  strip.show();
}

/////////////////////////////////////////////////////////////////////
void AH_NightRider(void)
{
  // cylon / Knight Rider
  Speed = 0;

  if ( Direction == 0 ) {
    if ( ++Offset1 >= strip.numPixels() ) {
      Offset1    = strip.numPixels();
      Direction = 1;
    }
  } else {
    if ( --Offset1 == 0 ) {
      Offset1    = 0;
      Direction = 0;
    }
  }

  strip.clear();
  strip.setPixelColor(Offset1    , Anim1_Colour);
  strip.setPixelColor(Offset1 - 1, Anim2_Colour);
  strip.setPixelColor(Offset1 + 1, Anim2_Colour);
  strip.setPixelColor(Offset1 - 2, Anim2_Colour);
  strip.setPixelColor(Offset1 + 2, Anim2_Colour);
  strip.show();
}

/////////////////////////////////////////////////////////////////////
void AH_Rainbow(void)
{
  U16 Index;

  // rainbow
  Speed = 2;

  // make the animation
  strip.clear();
  for ( Index = 0; Index < strip.numPixels(); Index++ ) {
    strip.setPixelColor(Index, Wheel((Offset1 + Index) * 3));
  }
  if ( OverlayNightRider == 1 ) {
    Overlay_NightRider(1);
  }

  strip.show();

  // make a Rainbow of color
  Offset1++;
}

/////////////////////////////////////////////////////////////////////
void AH_CrawlForward(void)
{
  // crawling lights forward
  U16 Index;
  Speed = 3;

  if ( ++Offset1 >= DEF_Crawl_Length ) {
    Offset1 = 0;
  }

  // calc the data
  strip.clear();
  for ( Index = 0; Index < strip.numPixels(); Index++ ) {
    if ( ++Offset1 >= DEF_Crawl_Length ) {
      Offset1 = 0;
    }

    if ( Offset1 == 0 ) {
      strip.setPixelColor(Index, Anim1_Colour);
    }
  }

  if ( OverlayNightRider == 1 ) {
    Overlay_NightRider(1);
  }

  strip.show();
}

/////////////////////////////////////////////////////////////////////
void AH_CrawlBackwards(void)
{
  // crawling lights backward
  U16 Index;
  Speed = 3;

  if ( Offset1 == 0 ) {
    Offset1 = DEF_Crawl_Length;
  } else {
    Offset1--;
  }

  // calc the data
  strip.clear();
  for ( Index = 0; Index < strip.numPixels(); Index++ ) {
    if ( Offset1 == 0 ) {
      Offset1 = DEF_Crawl_Length;
    } else {
      Offset1--;
    }

    if ( Offset1 == 0 ) {
      strip.setPixelColor(Index, Anim1_Colour);
    }
  }

  if ( OverlayNightRider == 1 ) {
    Overlay_NightRider(1);
  }

  strip.show();
}

/////////////////////////////////////////////////////////////////////
void AH_CrawlRandom(void)
{
  // crawling lights backward with random colour
  U16 Index;
  Speed = 4;

  if ( Offset1 == 0 ) {
    Offset1 = DEF_Crawl_Length;
  } else {
    Offset1--;
  }

  // calc the data
  strip.clear();
  for ( Index = 0; Index < strip.numPixels(); Index++ ) {
    if ( Offset1 == 0 ) {
      Offset1 = DEF_Crawl_Length;
    } else {
      Offset1--;
    }

    if ( Offset1 == 0 ) {
      strip.setPixelColor(Index, random(0, 255),  random(0, 255),  random(0, 255));
    }
  }

  if ( OverlayNightRider == 1 ) {
    Overlay_NightRider(1);
  }

  strip.show();
}

/////////////////////////////////////////////////////////////////////
void AH_Twinkle(void)
{
  // random -> twinkle
  U16 Index;
  Speed = 2;

  // calc the data
  strip.clear();
  for ( Index = 0; Index < strip.numPixels(); Index++ ) {
    strip.setPixelColor(Index, random(0, 255),  random(0, 255),  random(0, 255));
  }

  if ( OverlayNightRider == 1 ) {
    Overlay_NightRider(1);
  }

  strip.show();
}

/////////////////////////////////////////////////////////////////////
void AH_Fire(void)
{
  // Fire
  U16 Index;
  Speed = 2;

  int r = 255;
  int g = r - 40;
  int b = 40;
  strip.clear();
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

  if ( OverlayNightRider == 1 ) {
    Overlay_NightRider(1);
  }

  strip.show();
}

/////////////////////////////////////////////////////////////////////
void AH_Alternate(void)
{
  // alternate
  U16 Index;
  Speed = 4;

  Direction++;
  strip.clear();
  for ( Index = 0; Index < strip.numPixels(); Index++ ) {
    Direction++;
    if ( (Direction & 1) == 0) {
      // set even LED to color 1
      strip.setPixelColor(Index, Anim1_Colour);
    } else {
      // set odd LED to color 2
      strip.setPixelColor(Index, Anim2_Colour);
    }
  }

  if ( OverlayNightRider == 1 ) {
    Overlay_NightRider(1);
  }

  strip.show();
}

/////////////////////////////////////////////////////////////////////
void AH_FillRandom(void)
{
  // Random Fill
  U16 Index;
  FLG any = 1;
  for ( Index = 0; Index < strip.numPixels(); Index++ ) { // fill array with 0
    any &= LEDs[Index] == 1;
  }

  if ( any == 1 ) {
    Serial.println("Start Over");
    // all on, so start again
    strip.clear();

    for (Index = 0; Index < strip.numPixels(); Index++) { // fill array with 0
      LEDs[Index] = 0;
    }
  } else {
    // find another spot to fill
    FLG Found = 0;
    do {
      Index = random(0, strip.numPixels()); // pick a random LED
      if ( LEDs[Index] == 0 ) {
        strip.setPixelColor(Index, Anim1_Colour);
        LEDs[Index] = 1; // update array to remember it is lit
        Found = 10;
      }
    } while ( ++Found < 10 ) ;

    if ( OverlayNightRider == 1 ) {
      Overlay_NightRider(0);
    }
  }

  strip.show(); // display
}

/////////////////////////////////////////////////////////////////////
void AH_RandomClear(void)
{
  // random colours with random clear
  Speed = 0;

  if ( random(0, 255) == 250 ) {
    strip.clear();
  }

  strip.setPixelColor(random(0, strip.numPixels()), random(0, 255),  random(0, 255),  random(0, 255));

  if ( OverlayNightRider == 1 ) {
    Overlay_NightRider(0);
  }

  strip.show();
}

/////////////////////////////////////////////////////////////////////
void AH_AllRed(void)
{
  // all Red
  U16 Index;
  Speed = 0;

  if ( ++Offset1 > 255) {
    Offset1 = 0;
  }

  strip.clear();
  for ( Index = 0; Index < strip.numPixels(); Index++ ) {
    strip.setPixelColor(Index, Offset1, 0, 0);
  }

  if ( OverlayNightRider == 1 ) {
    Overlay_NightRider(1);
  }

  strip.show();
}

/////////////////////////////////////////////////////////////////////
void AH_AllGreen(void)
{
  // all Green
  U16 Index;
  Speed = 0;

  if ( ++Offset1 > 255) {
    Offset1 = 0;
  }

  strip.clear();
  for (Index = 0; Index < strip.numPixels(); Index++) {
    strip.setPixelColor(Index, 0, Offset1, 0);
  }

  if ( OverlayNightRider == 1 ) {
    Overlay_NightRider(1);
  }

  strip.show();
}

/////////////////////////////////////////////////////////////////////
void AH_AllBlue(void)
{
  U16 Index;

  // All Blue
  Speed = 0;
  if ( ++Offset1 > 255) {
    Offset1 = 0;
  }

  strip.clear();
  for (Index = 0; Index < strip.numPixels(); Index++) {
    strip.setPixelColor(Index, 0, 0, Offset1);
  }

  if ( OverlayNightRider == 1 ) {
    Overlay_NightRider(1);
  }

  strip.show();
}

/////////////////////////////////////////////////////////////////////
void AH_MiddleFill(void)
{
  // Light up the strip starting from the middle
  if ( Offset1++ > strip.numPixels() / 2 ) {
    strip.clear();
    Offset1 = 0;
    Anim1_Colour = strip.Color(random(0, 255), random(0, 255), random(0, 255));
  }

  strip.setPixelColor(strip.numPixels() / 2 + Offset1, Anim1_Colour);
  strip.setPixelColor(strip.numPixels() / 2 - Offset1, Anim1_Colour);

  if ( OverlayNightRider == 1 ) {
    Overlay_NightRider(0);
  }

  strip.show();
}

/////////////////////////////////////////////////////////////////////
void AH_EndFill(void)
{
  // Light up the strip starting from the ends
  if ( Offset1++ > strip.numPixels() / 2 ) {
    strip.clear();
    Offset1 = 0;
    Anim1_Colour = strip.Color(random(0, 255), random(0, 255), random(0, 255));
  }

  strip.setPixelColor(Offset1, Anim1_Colour);
  strip.setPixelColor(strip.numPixels() - Offset1, Anim1_Colour);

  if ( OverlayNightRider == 1 ) {
    Overlay_NightRider(0);
  }

  strip.show();
}

/////////////////////////////////////////////////////////////////////
void AH_CrawlDual(void)
{
  U16 Index;

  // crawling lights backward and backwards
  Speed = 4;

  if ( Offset1 == 0 ) {
    Offset1 = DEF_Crawl_LengthB;
  } else {
    Offset1--;
  }
  if ( ++Offset2 >= DEF_Crawl_LengthB ) {
    Offset2 = 0;
  }

  // calc the data
  strip.clear();
  for ( Index = 0; Index < strip.numPixels(); Index++ ) {
    if ( Offset1 == 0 ) {
      Offset1 = DEF_Crawl_LengthB;
    } else {
      Offset1--;
    }
    if ( ++Offset2 >= DEF_Crawl_LengthB ) {
      Offset2 = 0;
    }

    if ( Offset1 == 0 ) {
      strip.setPixelColor(Index, Anim1_Colour);
    }
    if ( Offset2 == 1 ) {
      if ( strip.getPixelColor(Index) == Anim1_Colour ) {
        strip.setPixelColor(Index, Anim1_Colour | Anim2_Colour);
      } else {
        // unique
        strip.setPixelColor(Index, Anim2_Colour);
      }
    }
  }

  if ( OverlayNightRider == 1 ) {
    Overlay_NightRider(1);
  }

  strip.show();
}

/////////////////////////////////////////////////////////////////////
// FADE ALL LEDS THROUGH HSV RAINBOW
void AH_RainbowFade(void)
{
  U16 Index;

  ihue++;
  if (ihue >= 359) {
    ihue = 0;
  }
  int thisColor[3];
  HSVtoRGB(ihue, 255, 255, thisColor);

  strip.clear();
  for ( Index = 0 ; Index < strip.numPixels(); Index++ ) {
    strip.setPixelColor(Index, thisColor[0], thisColor[1], thisColor[2]);
  }

  if ( OverlayNightRider == 1 ) {
    Overlay_NightRider(1);
  }

  strip.show();
}

/////////////////////////////////////////////////////////////////////
// LOOP HSV RAINBOW
void AH_Rainbow_Loop(void)
{
  idex++;
  ihue = ihue + 10;
  int icolor[3];

  if ( idex >= strip.numPixels() ) {
    idex = 0;
  }
  if ( ihue >= 359 ) {
    ihue = 0;
  }

  HSVtoRGB(ihue, 255, 255, icolor);
  strip.setPixelColor(idex, icolor[0], icolor[1], icolor[2]);

  if ( OverlayNightRider == 1 ) {
    Overlay_NightRider(0);
  }

  strip.show();
}

/////////////////////////////////////////////////////////////////////
//-POLICE LIGHTS (TWO COLOR SINGLE LED)
void AH_Police_LightsONE(void)
{
  idex++;
  if (idex >= strip.numPixels()) {
    idex = 0;
  }

  int idexR = idex;
  int idexB = antipodal_index(idexR);

  strip.clear();
  for ( int i = 0; i < strip.numPixels(); i++ ) {
    if ( i == idexR ) {
      strip.setPixelColor(i, Anim1_Colour);
    }
    if ( i == idexB ) {
      strip.setPixelColor(i, Anim2_Colour);
    }
  }

  if ( OverlayNightRider == 1 ) {
    Overlay_NightRider(1);
  }

  strip.show();
}

/////////////////////////////////////////////////////////////////////
//-POLICE LIGHTS (TWO COLOR SOLID)
void AH_police_lightsALL(void)
{
  if ( ++idex >= strip.numPixels() ) {
    idex = 0;
  }

  int idexR = idex;
  int idexB = antipodal_index(idexR);

  strip.setPixelColor(idexR, Anim1_Colour);
  strip.setPixelColor(idexB, Anim2_Colour);

  if ( OverlayNightRider == 1 ) {
    Overlay_NightRider(0);
  }

  strip.show();
}

/////////////////////////////////////////////////////////////////////
void AH_2ColourFader(void)
{
  Speed = 10; // each second

  float correctionFactor = 0.0f;
  float corFactorStep    = 1.0f / strip.numPixels();

  strip.clear();
  for ( int Index = 0; Index < strip.numPixels(); Index++ ) {
    correctionFactor += corFactorStep;
    float red   = (Anim2_Colour_Red   - Anim1_Colour_Red  ) * correctionFactor + Anim1_Colour_Red;
    float green = (Anim2_Colour_Green - Anim1_Colour_Green) * correctionFactor + Anim1_Colour_Green;
    float blue  = (Anim2_Colour_Blue  - Anim1_Colour_Blue ) * correctionFactor + Anim1_Colour_Blue;
    strip.setPixelColor(Index, round(red), round(green), round(blue));
  }

  if ( OverlayNightRider == 1 ) {
    Overlay_NightRider(1);
  }

  strip.show();
}

/////////////////////////////////////////////////////////////////////
void AH_SineMover(void)
{
  Speed = 0; // each second

  float CycleLength = (2.0f * 3.141592654f) / strip.numPixels();
  float Sine;
  float Cosine;

  strip.clear();
  if ( Offset1++ > strip.numPixels() ) {
    Offset1 = 0;
  }

  Sine  = sin(Offset1 * CycleLength) * (strip.numPixels() / 2);
  Sine += strip.numPixels() / 2;
  strip.setPixelColor(round(Sine)  , Anim1_Colour);
  strip.setPixelColor(round(Sine) - 1, Anim1_Colour);
  strip.setPixelColor(round(Sine) + 1, Anim1_Colour);

  Cosine  = cos(Offset1 * CycleLength) * (strip.numPixels() / 2);
  Cosine += strip.numPixels() / 2;
  strip.setPixelColor(round(Cosine)    , Anim2_Colour);
  strip.setPixelColor(round(Cosine) - 1, Anim2_Colour);
  strip.setPixelColor(round(Cosine) + 1, Anim2_Colour);

  if ( OverlayNightRider == 1 ) {
    Overlay_NightRider(1);
  }

  strip.show();
}

/////////////////////////////////////////////////////////////////////
typedef void ANIM_HANDLER ( void );

// state handler function pointers
static ANIM_HANDLER * const AnimationHandlers[] = {
  NULL,
  &AH_OFF,
  &AH_Rainbow,
  &AH_NightRider,
  &AH_CrawlForward,
  &AH_CrawlBackwards,
  &AH_CrawlRandom,
  &AH_Twinkle,
  &AH_Fire,
  &AH_Alternate,
  &AH_FillRandom,
  &AH_RandomClear,
  &AH_AllRed,
  &AH_AllGreen,
  &AH_AllBlue,
  &AH_MiddleFill,
  &AH_EndFill,
  &AH_CrawlDual,
  &AH_RainbowFade,
  &AH_Rainbow_Loop,
  &AH_Police_LightsONE,
  &AH_police_lightsALL,
  &AH_2ColourFader,
  &AH_SineMover,
};

/////////////////////////////////////////////////////////////////////
// update Animation
void UpdateAnimation()
{
  if ( SpeedCounter++ >= Speed ) {
    SpeedCounter = 0;

    if ( Animation < ArrayElements(AnimationHandlers) ) {
      Serial.print  ("Update Animation: " );
      Serial.println(Animation);

      // ONLY run if is valid
      AnimationHandlers[Animation]();
    } else {
      // fix any problem
      Animation = 1;
    }
  } else {
    // skip for OFF & Knight Rider animations
    if ( OverlayNightRider == 1 ) {
      if ( Animation > 1 ) {
        // just update cylon
        Overlay_NightRider(0);   // XOR Cylon to remove it, move the anim, XOR back in
        strip.show();
      }
    }
  }
}

/////////////////////////////////////////////////////////////////////
// this may cause the conenction to Blynk to fails... and the Blynk libs to just keep saying "Connection..."
BLYNK_CONNECTED() {
  // Request Blynk server to re-send latest values for all pins
  // NOTE: This can cause the ESP8266 to crash !!
  Serial.println("Sync All Blynk Values...");
  Blynk.syncAll();

  // If the ESP keeps crashing at boot, then uncomment JUST one line below to find which value sync is causing the problem
  //  Blynk.syncVirtual(V0);
  //  Blynk.syncVirtual(V1);
  //  Blynk.syncVirtual(V2);
  //  Blynk.syncVirtual(V3);
}

/////////////////////////////////////////////////////////////////////
BLYNK_WRITE(V1)
{
  Animation    = param.asInt();  // first index = 1
  Speed        = 0;
  SpeedCounter = 0;
  Offset1      = 0;
  Offset2      = 0;
  Direction    = 0;
  Direction_NR = 0;
  Offset_NR    = 0;

  U16 Index;
  for ( Index = 0; Index < strip.numPixels(); Index++ ) { // fill array with 0
    LEDs[Index] = 0;
  }

  strip.clear();
  Serial.print("New Animation is: ");
  Serial.println(Animation);
}

/////////////////////////////////////////////////////////////////////
// Change the colour of the Animation
//  -->> Ensure you set the RGB Zebra vales to Min=1, Max=255
BLYNK_WRITE(V2)
{
  Serial.println("New Animation Colour 1 is: ");
  if ( param[0].isEmpty() == 0 ) {
    Serial.print("Get-R1 :");
    Anim1_Colour_Red   = param[0].asInt();
  } else {
    Serial.print("Get-R1 is Empty :");
    Anim1_Colour_Red   = 0;
  }
  Serial.println(Anim1_Colour_Red);

  if ( param[1].isEmpty() == 0 ) {
    Serial.print("Get-G1: ");
    Anim1_Colour_Green = param[1].asInt();
  } else {
    Serial.print("Get-G1 is Empty: ");
    Anim1_Colour_Green = 0;
  }
  Serial.println(Anim1_Colour_Green);

  if ( param[2].isEmpty() == 0 ) {
    Serial.print("Get-B1: ");
    Anim1_Colour_Blue  = param[2].asInt();
  } else {
    Serial.print("Get-B1 is Empty: ");
    Anim1_Colour_Blue = 0;
  }
  Serial.println(Anim1_Colour_Blue );

  // combined
  Anim1_Colour = strip.Color(Anim1_Colour_Red, Anim1_Colour_Green, Anim1_Colour_Blue);
}

/////////////////////////////////////////////////////////////////////
// Change the Secondary colour of the Animation
//  -->> Ensure you set the RGB Zebra vales to Min=1, Max=255
BLYNK_WRITE(V3)
{
  Serial.println("New Animation Colour 2 is: ");
  if ( param[0].isEmpty() == 0 ) {
    Serial.print("Get-R2 :");
    Anim2_Colour_Red   = param[0].asInt();
  } else {
    Serial.print("Get-R2 is Empty :");
    Anim2_Colour_Red   = 0;
  }
  Serial.println(Anim2_Colour_Red);

  if ( param[1].isEmpty() == 0 ) {
    Serial.print("Get-G2: ");
    Anim2_Colour_Green = param[1].asInt();
  } else {
    Serial.print("Get-G2 is Empty: ");
    Anim2_Colour_Green = 0;
  }
  Serial.println(Anim2_Colour_Green);

  if ( param[2].isEmpty() == 0 ) {
    Serial.print("Get-B2: ");
    Anim2_Colour_Blue  = param[2].asInt();
  } else {
    Serial.print("Get-B2 is Empty: ");
    Anim2_Colour_Blue = 0;
  }
  Serial.println(Anim1_Colour_Blue );

  // combined
  Anim2_Colour = strip.Color(Anim2_Colour_Red, Anim2_Colour_Green, Anim2_Colour_Blue);
}

/////////////////////////////////////////////////////////////////////
BLYNK_WRITE(V4)
{
  OverlayNightRider = param.asInt();

  Serial.print  ("Overlay Night Rider: ");
  Serial.println(OverlayNightRider);
}

/////////////////////////////////////////////////////////////////////
// Change the colour of the Animation Night Rider Overlay
//  -->> Ensure you set the RGB Zebra vales to Min=1, Max=255
BLYNK_WRITE(V5)
{
  Serial.println("New Animation Colour NR is: ");
  if ( param[0].isEmpty() == 0 ) {
    Serial.print("Get-R-NR :");
    AnimNR_Colour_Red   = param[0].asInt();
  } else {
    Serial.print("Get-R-NR is Empty :");
    AnimNR_Colour_Red   = 0;
  }
  Serial.println(AnimNR_Colour_Red);

  if ( param[1].isEmpty() == 0 ) {
    Serial.print("Get-G-NR: ");
    AnimNR_Colour_Green = param[1].asInt();
  } else {
    Serial.print("Get-G-NR is Empty: ");
    AnimNR_Colour_Green = 0;
  }
  Serial.println(AnimNR_Colour_Green);

  if ( param[2].isEmpty() == 0 ) {
    Serial.print("Get-B2-NR: ");
    AnimNR_Colour_Blue  = param[2].asInt();
  } else {
    Serial.print("Get-B-NR is Empty: ");
    AnimNR_Colour_Blue = 0;
  }
  Serial.println(AnimNR_Colour_Blue );

  // combined
  AnimNR_Colour = strip.Color(AnimNR_Colour_Red, AnimNR_Colour_Green, AnimNR_Colour_Blue);
}

/////////////////////////////////////////////////////////////////////
BLYNK_WRITE(V6)
{
  LengthNR = param.asInt();

  Serial.print  ("Overlay Night Rider length: ");
  Serial.println(LengthNR);
}

/////////////////////////////////////////////////////////////////////
BLYNK_WRITE(V7)
{
  SpeedNR = param.asInt();

  Serial.print  ("Overlay Night Rider Speed: ");
  Serial.println(SpeedNR);
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
  Anim1_Colour = strip.Color(255, 0  , 0);
  Anim2_Colour = strip.Color(0  , 255, 0);

  // Setup a function to be called every so often
  UpdateTimer.setInterval(100L, UpdateAnimation);
}

/////////////////////////////////////////////////////////////////////
void loop()
{
  Blynk.run();
  UpdateTimer.run();
}


