/*
   Fibonacci Demo Reel: https://github.com/jasoncoon/fibonacci-demoreel
   Copyright (C) 2020 Jason Coon

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <FastLED.h>  // https://github.com/FastLED/FastLED

#include "Adafruit_FreeTouch.h" //https://github.com/adafruit/Adafruit_FreeTouch

FASTLED_USING_NAMESPACE

#include "GradientPalettes.h"

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

#define DATA_PIN      A10
#define LED_TYPE      WS2812B
#define COLOR_ORDER   GRB
// #define NUM_LEDS      64
#define NUM_LEDS_IN_FIB  64 // two fibs
#define NUM_LEDS      128 // two fibs

#define MILLI_AMPS         500 // IMPORTANT: set the max milli-Amps of your power supply (4A = 4000mA)
#define FRAMES_PER_SECOND  120  // here you can control the speed. With the Access Point / Web Server the animations run a bit slower.

CRGB leds[NUM_LEDS];

const uint8_t brightnessCount = 5;
uint8_t brightnessMap[brightnessCount] = { 16, 32, 64, 128, 255 };
int8_t brightnessIndex = 0;
uint8_t brightness = brightnessMap[brightnessIndex];

uint8_t power = 1;

// ten seconds per color palette makes a good demo
// 20-120 is better for deployment
uint8_t secondsPerPalette = 10;

// COOLING: How much does the air cool as it rises?
// Less cooling = taller flames.  More cooling = shorter flames.
// Default 50, suggested range 20-100
uint8_t cooling = 49;

// SPARKING: What chance (out of 255) is there that a new spark will be lit?
// Higher chance = more roaring fire.  Lower chance = more flickery fire.
// Default 120, suggested range 50-200.
uint8_t sparking = 60;

uint8_t speed = 30;
uint8_t speed_i = 4;

#define NUM_SPEEDS  7
uint8_t speeds[NUM_SPEEDS] = {1,3,9,18,30,50,80};


//  Goggle things:
enum lenstype {LT_LEFT, LT_RIGHT, LT_DUPE, LT_EACH, LT_WHOLE};

///////////////////////////////////////////////////////////////////////

//  Touch things:



Adafruit_FreeTouch touch0 = Adafruit_FreeTouch(A0, OVERSAMPLE_4, RESISTOR_0, FREQ_MODE_NONE);
Adafruit_FreeTouch touch1 = Adafruit_FreeTouch(A1, OVERSAMPLE_4, RESISTOR_0, FREQ_MODE_NONE);
Adafruit_FreeTouch touch2 = Adafruit_FreeTouch(A2, OVERSAMPLE_4, RESISTOR_0, FREQ_MODE_NONE);
Adafruit_FreeTouch touch3 = Adafruit_FreeTouch(A3, OVERSAMPLE_4, RESISTOR_0, FREQ_MODE_NONE);

#define touchPointCount 4

// These values were discovered using the commented-out Serial.print statements in handleTouch below

// minimum values for each touch pad, used to filter out noise
uint16_t touchMin[touchPointCount] = { 558, 259, 418, 368 };
// uint16_t touchMin[touchPointCount] = { 558, 259, 118, 168 };

// maximum values for each touch pad, used to determine when a pad is touched
uint16_t touchMax[touchPointCount] = { 1016, 1016, 1016, 1016 };
// uint16_t touchMax[touchPointCount] = { 1016, 1016, 616, 616 };

// raw capacitive touch sensor readings
uint16_t touchRaw[touchPointCount] = { 0, 0, 0, 0 };

// capacitive touch sensor readings, mapped/scaled one one byte each (0-255)
uint8_t touch[touchPointCount] = { 0, 0, 0, 0 };

// whether we've read a touch here or not
uint8_t touchActive[touchPointCount] = { 0, 0, 0, 0 };

// coordinates of the touch points
uint8_t touchPointX[touchPointCount] = { 255, 0, 0, 255 };
uint8_t touchPointY[touchPointCount] = { 0, 0, 255, 255 };

boolean activeWaves = false;


///////////////////////////////////////////////////////////////////////

// Forward declarations of an array of cpt-city gradient palettes, and
// a count of how many there are.  The actual color palette definitions
// are at the bottom of this file.
extern const TProgmemRGBGradientPalettePtr gGradientPalettes[];

uint8_t gCurrentPaletteNumber = 0;

CRGBPalette16 gCurrentPalette( CRGB::Black);
CRGBPalette16 gTargetPalette( gGradientPalettes[0] );

CRGBPalette16 IceColors_p = CRGBPalette16(CRGB::Black, CRGB::Blue, CRGB::Aqua, CRGB::White);

int8_t currentPatternIndex = 0; // Index number of which pattern is current
uint8_t autoplay = 1;

uint8_t autoplayDuration = 10;
unsigned long autoPlayTimeout = 0;

uint8_t showClock = 0;
uint8_t clockBackgroundFade = 240;

uint8_t currentPaletteIndex = 0;

uint8_t gHue = 0; // rotating "base color" used by many of the patterns

CRGB solidColor = CRGB::Blue;

// scale the brightness of all pixels down
void dimAll(byte value)
{
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i].nscale8(value);
  }
}

typedef struct {
  CRGBPalette16 palette;
  String name;
} PaletteAndName;
typedef PaletteAndName PaletteAndNameList[];

const CRGBPalette16 palettes[] = {
  RainbowColors_p,
  RainbowStripeColors_p,
  CloudColors_p,
  LavaColors_p,
  OceanColors_p,
  ForestColors_p,
  PartyColors_p,
  HeatColors_p
};

const uint8_t paletteCount = ARRAY_SIZE(palettes);

typedef void (*Pattern)();
typedef Pattern PatternList[];

#include "Twinkles.h"
#include "TwinkleFOX.h"
#include "Map.h"
//#include "Noise.h"

// List of patterns to cycle through.  Each is defined as a separate function below.

PatternList patterns = {
//  pride,
  prideFibonacci,
//  colorWaves,
  colorWavesFibonacci,

  // matrix patterns
  anglePalette,
  radiusPalette,
  xPalette,
  yPalette,
  xyPalette,

  angleGradientPalette,
  radiusGradientPalette,
  xGradientPalette,
  yGradientPalette,
  xyGradientPalette,

//  // noise patterns
//  fireNoise,
//  fireNoise2,
//  lavaNoise,
//  rainbowNoise,
//  rainbowStripeNoise,
//  partyNoise,
//  forestNoise,
//  cloudNoise,
//  oceanNoise,
//  blackAndWhiteNoise,
//  blackAndBlueNoise,

  // twinkle patterns
  rainbowTwinkles,
  snowTwinkles,
  cloudTwinkles,
  incandescentTwinkles,

  // TwinkleFOX patterns
  retroC9Twinkles,
  redWhiteTwinkles,
  blueWhiteTwinkles,
  redGreenWhiteTwinkles,
  fairyLightTwinkles,
  snow2Twinkles,
  hollyTwinkles,
  iceTwinkles,
  partyTwinkles,
  forestTwinkles,
  lavaTwinkles,
  fireTwinkles,
  cloud2Twinkles,
  oceanTwinkles,

  rainbow,
  rainbowWithGlitter,
  rainbowSolid,
  confetti,
  sinelon,
  bpm,
  juggle,
  fire,
  water,

//  strandTest,
//
//  showSolidColor
};

const uint8_t patternCount = ARRAY_SIZE(patterns);

void setup() {
  Serial.begin(115200);
  delay(100);


  if (!touch0.begin())
    Serial.println("Failed to begin qt on pin A0");
  if (!touch1.begin())
    Serial.println("Failed to begin qt on pin A1");
  if (!touch2.begin())
    Serial.println("Failed to begin qt on pin A2");
  if (!touch3.begin())
    Serial.println("Failed to begin qt on pin A3");


  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setDither(false);
  FastLED.setCorrection(TypicalSMD5050);
  FastLED.setBrightness(brightness);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, MILLI_AMPS);
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();

  FastLED.setBrightness(brightness);

  autoPlayTimeout = millis() + (autoplayDuration * 1000);
}

void loop() {
  // Add entropy to random number generator; we use a lot of it.
  random16_add_entropy(random(65535));


  handleTouch();


  if (power == 0) {
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    FastLED.show();
    delay(1000 / FRAMES_PER_SECOND);
    return;
  }

  // change to a new cpt-city gradient palette
  EVERY_N_SECONDS( secondsPerPalette ) {
    gCurrentPaletteNumber = addmod8( gCurrentPaletteNumber, 1, gGradientPaletteCount);
    gTargetPalette = gGradientPalettes[ gCurrentPaletteNumber ];
  }

  EVERY_N_MILLISECONDS(40) {
    // slowly blend the current palette to the next
    nblendPaletteTowardPalette( gCurrentPalette, gTargetPalette, 8);
    gHue++;  // slowly cycle the "base color" through the rainbow
  }

  if (autoplay && (millis() > autoPlayTimeout)) {
    adjustPattern(true);
    autoPlayTimeout = millis() + (autoplayDuration * 1000);
  }

  // Call the current pattern function once, updating the 'leds' array
  patterns[currentPatternIndex]();


  // ADD touch layer on top:
  // if (!activeWaves)
    // colorWavesFibonacci();

  touchControls();
  // touchDemo();



  FastLED.show();

  // insert a delay to keep the framerate modest
  FastLED.delay(1000 / FRAMES_PER_SECOND);
}

// increase or decrease the current pattern number, and wrap around at the ends
void adjustPattern(bool up)
{
  if (up)
    currentPatternIndex++;
  else
    currentPatternIndex--;

  // wrap around at the ends
  if (currentPatternIndex < 0)
    currentPatternIndex = patternCount - 1;
  if (currentPatternIndex >= patternCount)
    currentPatternIndex = 0;

  Serial.print("pattern: ");
  Serial.println(currentPatternIndex);
}

// increase or decrease the current brightness, and wrap around at the ends
void adjustBrightness(bool up)
{
  if (up)
    brightnessIndex++;
  else
    brightnessIndex--;

  // wrap around at the ends
  if (brightnessIndex < 0)
    brightnessIndex = brightnessCount - 1;
  if (brightnessIndex >= brightnessCount)
    brightnessIndex = 0;

  brightness = brightnessMap[brightnessIndex];
  FastLED.setBrightness(brightness);

  Serial.print("brightness: ");
  Serial.println(brightness);
}

void strandTest()
{
  uint8_t i = speed;

  if (i >= NUM_LEDS) i = NUM_LEDS - 1;

  fill_solid(leds, NUM_LEDS, CRGB::Black);

  leds[i] = solidColor;
}

void showSolidColor()
{
  fill_solid(leds, NUM_LEDS, solidColor);
}

// Patterns from FastLED example DemoReel100: https://github.com/FastLED/FastLED/blob/master/examples/DemoReel100/DemoReel100.ino

void rainbow()
{
  // FastLED's built-in rainbow generator
  fill_rainbow( leds, NUM_LEDS, gHue, 255 / NUM_LEDS);
}

void rainbowWithGlitter()
{
  // built-in FastLED rainbow, plus some random sparkly glitter
  rainbow();
  addGlitter(80);
}

void rainbowSolid()
{
  fill_solid(leds, NUM_LEDS, CHSV(gHue, 255, 255));
}

void confetti()
{
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( leds, NUM_LEDS, 10);
  int pos = random16(NUM_LEDS);
  // leds[pos] += CHSV( gHue + random8(64), 200, 255);
  leds[pos] += ColorFromPalette(palettes[currentPaletteIndex], gHue + random8(64));
}

void sinelon()
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, NUM_LEDS, 20);
  int pos = beatsin16(speed, 0, NUM_LEDS);
  static int prevpos = 0;
  CRGB color = ColorFromPalette(palettes[currentPaletteIndex], gHue, 255);
  if ( pos < prevpos ) {
    fill_solid( leds + pos, (prevpos - pos) + 1, color);
  } else {
    fill_solid( leds + prevpos, (pos - prevpos) + 1, color);
  }
  prevpos = pos;
}

void bpm()
{
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t beat = beatsin8( speed, 64, 255);
  CRGBPalette16 palette = palettes[currentPaletteIndex];
  for ( int i = 0; i < NUM_LEDS; i++) {
    leds[i] = ColorFromPalette(palette, gHue + (i * 2), beat - gHue + (i * 10));
  }
}

void juggle()
{
  static uint8_t    numdots =   4; // Number of dots in use.
  static uint8_t   faderate =   2; // How long should the trails be. Very low value = longer trails.
  static uint8_t     hueinc =  255 / numdots - 1; // Incremental change in hue between each dot.
  static uint8_t    thishue =   0; // Starting hue.
  static uint8_t     curhue =   0; // The current hue
  static uint8_t    thissat = 255; // Saturation of the colour.
  static uint8_t thisbright = 255; // How bright should the LED/display be.
  static uint8_t   basebeat =   5; // Higher = faster movement.

 static uint8_t lastSecond =  99;  // Static variable, means it's only defined once. This is our 'debounce' variable.
  uint8_t secondHand = (millis() / 1000) % 30; // IMPORTANT!!! Change '30' to a different value to change duration of the loop.

  if (lastSecond != secondHand) { // Debounce to make sure we're not repeating an assignment.
    lastSecond = secondHand;
    switch (secondHand) {
      case  0: numdots = 1; basebeat = 20; hueinc = 16; faderate = 2; thishue = 0; break; // You can change values here, one at a time , or altogether.
      case 10: numdots = 4; basebeat = 10; hueinc = 16; faderate = 8; thishue = 128; break;
      case 20: numdots = 8; basebeat =  3; hueinc =  0; faderate = 8; thishue = random8(); break; // Only gets called once, and not continuously for the next several seconds. Therefore, no rainbows.
      case 30: break;
    }
  }

  // Several colored dots, weaving in and out of sync with each other
  curhue = thishue; // Reset the hue values.
  fadeToBlackBy(leds, NUM_LEDS, faderate);
  for ( int i = 0; i < numdots; i++) {
    //beat16 is a FastLED 3.1 function
    leds[beatsin16(basebeat + i + numdots, 0, NUM_LEDS)] += CHSV(gHue + curhue, thissat, thisbright);
    curhue += hueinc;
  }
}

void fire()
{
  heatMap(HeatColors_p, true);
}

void water()
{
  heatMap(IceColors_p, false);
}

// Pride2015 by Mark Kriegsman: https://gist.github.com/kriegsman/964de772d64c502760e5
// This function draws rainbows with an ever-changing,
// widely-varying set of parameters.
void pride() {
  fillWithPride(false);
}

void prideFibonacci() {
  // fillWithPride(true, LT_DUPE);
  fillWithPride(true, LT_WHOLE);
}

void fillWithPride(bool useFibonacciOrder) 
{
  fillWithPride(useFibonacciOrder, LT_WHOLE);
}
void fillWithPride(bool useFibonacciOrder, lenstype lt) 
{
  static uint16_t sPseudotime = 0;
  static uint16_t sLastMillis = 0;
  static uint16_t sHue16 = 0;

  uint8_t sat8 = beatsin88( 87, 220, 250);
  uint8_t brightdepth = beatsin88( 341, 96, 224);
  uint16_t brightnessthetainc16 = beatsin88( 203, (25 * 256), (40 * 256));
  uint8_t msmultiplier = beatsin88(147, 23, 60);

  uint16_t hue16 = sHue16;//gHue * 256;
  uint16_t hueinc16 = beatsin88(113, 1, 3000)/4;

  uint16_t ms = millis();
  // uint16_t deltams = ms - sLastMillis ;
  uint16_t deltams = ((ms - sLastMillis) * speed) /32 ; // scale to speed
  sLastMillis  = ms;
  sPseudotime += deltams * msmultiplier;
  sHue16 += deltams * beatsin88( 400, 5, 9);
  uint16_t brightnesstheta16 = sPseudotime;

  if (lt == LT_DUPE)
  {
    for ( uint16_t i = 0 ; i < NUM_LEDS_IN_FIB; i++) {
      hue16 += hueinc16;
      uint8_t hue8 = hue16 / 256;

      brightnesstheta16  += brightnessthetainc16;
      uint16_t b16 = sin16( brightnesstheta16  ) + 32768;

      uint16_t bri16 = (uint32_t)((uint32_t)b16 * (uint32_t)b16) / 65536;
      uint8_t bri8 = (uint32_t)(((uint32_t)bri16) * brightdepth) / 65536;
      bri8 += (255 - brightdepth);

      CRGB newcolor = CHSV( hue8, sat8, bri8);

      uint16_t pixelnumber = i;

      if (useFibonacciOrder) pixelnumber = fibonacciToPhysical[i];
      
      pixelnumber = (NUM_LEDS_IN_FIB - 1) - pixelnumber;

      nblend( leds[pixelnumber], newcolor, 64);
      // DUPE:
      nblend( leds[pixelnumber+NUM_LEDS_IN_FIB], newcolor, 64);
    }
  }
  else // LT_WHOLE
  {
    for ( uint16_t i = 0 ; i < NUM_LEDS; i++) {
      hue16 += hueinc16;
      uint8_t hue8 = hue16 / 256;

      brightnesstheta16  += brightnessthetainc16;
      uint16_t b16 = sin16( brightnesstheta16  ) + 32768;

      uint16_t bri16 = (uint32_t)((uint32_t)b16 * (uint32_t)b16) / 65536;
      uint8_t bri8 = (uint32_t)(((uint32_t)bri16) * brightdepth) / 65536;
      bri8 += (255 - brightdepth);

      CRGB newcolor = CHSV( hue8, sat8, bri8);

      uint16_t pixelnumber = i;

      if (useFibonacciOrder) pixelnumber = fibonacciToPhysical[i%NUM_LEDS_IN_FIB];
      
      pixelnumber = (NUM_LEDS - 1) - pixelnumber;
      if (i >= NUM_LEDS_IN_FIB)
        pixelnumber -= NUM_LEDS_IN_FIB;

      nblend( leds[pixelnumber], newcolor, 64);
    }
  }
}

void radialPaletteShift()
{
  for (uint16_t i = 0; i < NUM_LEDS; i++) {
    // leds[i] = ColorFromPalette( gCurrentPalette, gHue + sin8(i*16), brightness);
    leds[fibonacciToPhysical[i]] = ColorFromPalette(gCurrentPalette, i + gHue, 255, LINEARBLEND);
  }
}

void radialPaletteShiftOutward()
{
  for (uint16_t i = 0; i < NUM_LEDS; i++) {
    leds[fibonacciToPhysical[i]] = ColorFromPalette(gCurrentPalette, i - gHue, 255, LINEARBLEND);
  }
}

// based on FastLED example Fire2012WithPalette: https://github.com/FastLED/FastLED/blob/master/examples/Fire2012WithPalette/Fire2012WithPalette.ino
void heatMap(CRGBPalette16 palette, bool up)
{
  fill_solid(leds, NUM_LEDS, CRGB::Black);

  // Add entropy to random number generator; we use a lot of it.
  random16_add_entropy(random(256));

  // Array of temperature readings at each simulation cell
  static byte heat[NUM_LEDS];

  byte colorindex;

  // Step 1.  Cool down every cell a little
  for ( uint16_t i = 0; i < NUM_LEDS; i++) {
    heat[i] = qsub8( heat[i],  random8(0, ((cooling * 10) / NUM_LEDS) + 2));
  }

  // Step 2.  Heat from each cell drifts 'up' and diffuses a little
  for ( uint16_t k = NUM_LEDS - 1; k >= 2; k--) {
    heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2] ) / 3;
  }

  // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
  if ( random8() < sparking ) {
    int y = random8(7);
    heat[y] = qadd8( heat[y], random8(160, 255) );
  }

  // Step 4.  Map from heat cells to LED colors
  for ( uint16_t j = 0; j < NUM_LEDS; j++) {
    // Scale the heat value from 0-255 down to 0-240
    // for best results with color palettes.
    colorindex = scale8(heat[j], 190);

    CRGB color = ColorFromPalette(palette, colorindex);

    if (up) {
      leds[j] = color;
    }
    else {
      leds[(NUM_LEDS - 1) - j] = color;
    }
  }
}

void addGlitter( uint8_t chanceOfGlitter)
{
  if ( random8() < chanceOfGlitter) {
    leds[ random16(NUM_LEDS) ] += CRGB::White;
  }
}

///////////////////////////////////////////////////////////////////////

// Forward declarations of an array of cpt-city gradient palettes, and
// a count of how many there are.  The actual color palette definitions
// are at the bottom of this file.
extern const TProgmemRGBGradientPalettePtr gGradientPalettes[];
extern const uint8_t gGradientPaletteCount;

uint8_t beatsaw8( accum88 beats_per_minute, uint8_t lowest = 0, uint8_t highest = 255,
                  uint32_t timebase = 0, uint8_t phase_offset = 0)
{
  uint8_t beat = beat8( beats_per_minute, timebase);
  uint8_t beatsaw = beat + phase_offset;
  uint8_t rangewidth = highest - lowest;
  uint8_t scaledbeat = scale8( beatsaw, rangewidth);
  uint8_t result = lowest + scaledbeat;
  return result;
}

void colorWaves() {
  fillWithColorWaves(leds, NUM_LEDS, gCurrentPalette, false);
}

void colorWavesFibonacci() {
  fillWithColorWaves(leds, NUM_LEDS, gCurrentPalette, true);
}

void fillWithColorWaves( CRGB* ledarray, uint16_t numleds, CRGBPalette16& palette, bool useFibonacciOrder)
{
  fillWithColorWaves(ledarray, numleds, palette, useFibonacciOrder, LT_WHOLE);

}
// ColorWavesWithPalettes by Mark Kriegsman: https://gist.github.com/kriegsman/8281905786e8b2632aeb
// This function draws color waves with an ever-changing,
// widely-varying set of parameters, using a color palette.
void fillWithColorWaves( CRGB* ledarray, uint16_t numleds, CRGBPalette16& palette, bool useFibonacciOrder, lenstype lt)
{
  static uint16_t sPseudotime = 0;
  static uint16_t sLastMillis = 0;
  static uint16_t sHue16 = 0;

  // uint8_t sat8 = beatsin88( 87, 220, 250);
  uint8_t brightdepth = beatsin88( 341, 96, 224);
  uint16_t brightnessthetainc16 = beatsin88( 203, (25 * 256), (40 * 256));
  uint8_t msmultiplier = beatsin88(147, 23, 60);

  uint16_t hue16 = sHue16;//gHue * 256;
  uint16_t hueinc16 = beatsin88(113, 300, 1500);

  uint16_t ms = millis();
  // uint16_t deltams = ms - sLastMillis ;
  uint16_t deltams = ((ms - sLastMillis) * speed) /32 ; // scale to speed
  sLastMillis  = ms;
  sPseudotime += deltams * msmultiplier;
  sHue16 += deltams * beatsin88( 400, 5, 9);
  uint16_t brightnesstheta16 = sPseudotime;

  if (lt == LT_DUPE)
  {
    for ( uint16_t i = 0 ; i < numleds; i++) {
      hue16 += hueinc16;
      uint8_t hue8 = hue16 / 256;
      uint16_t h16_128 = hue16 >> 7;
      if ( h16_128 & 0x100) {
        hue8 = 255 - (h16_128 >> 1);
      } else {
        hue8 = h16_128 >> 1;
      }

      brightnesstheta16  += brightnessthetainc16;
      uint16_t b16 = sin16( brightnesstheta16  ) + 32768;

      uint16_t bri16 = (uint32_t)((uint32_t)b16 * (uint32_t)b16) / 65536;
      uint8_t bri8 = (uint32_t)(((uint32_t)bri16) * brightdepth) / 65536;
      bri8 += (255 - brightdepth);

      uint8_t index = hue8;
      //index = triwave8( index);
      index = scale8( index, 240);

      CRGB newcolor = ColorFromPalette( palette, index, bri8);

      uint16_t pixelnumber = i;

      if (useFibonacciOrder) pixelnumber = fibonacciToPhysical[i];
      
      pixelnumber = (numleds - 1) - pixelnumber;

      nblend( ledarray[pixelnumber], newcolor, 128);
      // DUPE
      nblend( ledarray[pixelnumber-NUM_LEDS_IN_FIB], newcolor, 128);
    }
  }
  else // LT_WHOLE  yeah
  {
    for ( uint16_t i = 0 ; i < numleds; i++) {
      hue16 += hueinc16;
      uint8_t hue8 = hue16 / 256;
      uint16_t h16_128 = hue16 >> 7;
      if ( h16_128 & 0x100) {
        hue8 = 255 - (h16_128 >> 1);
      } else {
        hue8 = h16_128 >> 1;
      }

      brightnesstheta16  += brightnessthetainc16;
      uint16_t b16 = sin16( brightnesstheta16  ) + 32768;

      uint16_t bri16 = (uint32_t)((uint32_t)b16 * (uint32_t)b16) / 65536;
      uint8_t bri8 = (uint32_t)(((uint32_t)bri16) * brightdepth) / 65536;
      bri8 += (255 - brightdepth);

      uint8_t index = hue8;
      //index = triwave8( index);
      index = scale8( index, 240);

      CRGB newcolor = ColorFromPalette( palette, index, bri8);

      uint16_t pixelnumber = i;

      if (useFibonacciOrder) pixelnumber = fibonacciToPhysical[i%NUM_LEDS_IN_FIB];

      pixelnumber = (numleds - 1) - pixelnumber;

      if (i>=NUM_LEDS_IN_FIB)
        pixelnumber -= NUM_LEDS_IN_FIB;

      nblend( ledarray[pixelnumber], newcolor, 128);
    }
  }
}

// Alternate rendering function just scrolls the current palette
// across the defined LED strip.
void palettetest( CRGB* ledarray, uint16_t numleds, const CRGBPalette16& gCurrentPalette)
{
  static uint8_t startindex = 0;
  startindex--;
  fill_palette( ledarray, numleds, startindex, (256 / NUM_LEDS) + 1, gCurrentPalette, 255, LINEARBLEND);
}


bool touchChanged = true;

void handleTouch() {
  for (uint8_t i = 0; i < touchPointCount; i++) {
    if (i == 0) touchRaw[i] = touch0.measure();
    else if (i == 1) touchRaw[i] = touch1.measure();
    else if (i == 2) touchRaw[i] = touch2.measure();
    else if (i == 3) touchRaw[i] = touch3.measure();

    // // uncomment to display raw touch values in the serial monitor/plotter
    //    Serial.print(touchRaw[i]);
    //    Serial.print(" ");

    if (touchRaw[i] < touchMin[i]) {
      touchMin[i] = touchRaw[i];
      touchChanged = true;
    }

    if (touchRaw[i] > touchMax[i]) {
      touchMax[i] = touchRaw[i];
      touchChanged = true;
    }

    touch[i] = map(touchRaw[i], touchMin[i], touchMax[i], 0, 255);

    // // uncomment to display mapped/scaled touch values in the serial monitor/plotter
    //    Serial.print(touch[i]);
    //    Serial.print(" ");
  }

  // // uncomment to display raw and/or mapped/scaled touch values in the serial monitor/plotter
  //  Serial.println();

  // uncomment to display raw, scaled, min, max touch values in the serial monitor/plotter
  //  if (touchChanged) {
  //    for (uint8_t i = 0; i < touchPointCount; i++) {
  //      Serial.print(touchRaw[i]);
  //      Serial.print(" ");
  //      Serial.print(touch[i]);
  //      Serial.print(" ");
  //      Serial.print(touchMin[i]);
  //      Serial.print(" ");
  //      Serial.print(touchMax[i]);
  //      Serial.print(" ");
  //    }
  //
  //    Serial.println();
  //
  //    touchChanged = false;
  //  }
}


// adds a color to a pixel given it's XY coordinates and a "thickness" of the logical pixel
// since we're using a sparse logical grid for mapping, there isn't an LED at every XY coordinate
// thickness adds a little "fuzziness"
void addColorXY(int x, int y, CRGB color, uint8_t thickness = 0)
{
  // ignore coordinates outside of our one byte map range
  if (x < 0 || x > 255 || y < 0 || y > 255) return;

  // loop through all of the LEDs
  for (uint8_t i = 0; i < NUM_LEDS; i++) {
    // get the XY coordinates of the current LED
    uint8_t ix = coordsX[i];
    uint8_t iy = coordsY[i];

    // are the current LED's coordinates within the square centered
    // at X,Y, with width and height of thickness?
    if (ix >= x - thickness && ix <= x + thickness &&
        iy >= y - thickness && iy <= y + thickness) {

      // add to the color instead of just setting it
      // so that colors blend
      // FastLED automatically prevents overflowing over 255
      leds[i] += color;
    }
  }
}

// algorithm from http://en.wikipedia.org/wiki/Midpoint_circle_algorithm
void drawCircle(int x0, int y0, int radius, const CRGB color, uint8_t thickness = 0)
{
  int a = radius, b = 0;
  int radiusError = 1 - a;

  if (radius == 0) {
    addColorXY(x0, y0, color, thickness);
    return;
  }

  while (a >= b)
  {
    addColorXY(a + x0, b + y0, color, thickness);
    addColorXY(b + x0, a + y0, color, thickness);
    addColorXY(-a + x0, b + y0, color, thickness);
    addColorXY(-b + x0, a + y0, color, thickness);
    addColorXY(-a + x0, -b + y0, color, thickness);
    addColorXY(-b + x0, -a + y0, color, thickness);
    addColorXY(a + x0, -b + y0, color, thickness);
    addColorXY(b + x0, -a + y0, color, thickness);

    b++;
    if (radiusError < 0)
      radiusError += 2 * b + 1;
    else
    {
      a--;
      radiusError += 2 * (b - a + 1);
    }
  }
}

// algorithm from http://en.wikipedia.org/wiki/Midpoint_circle_algorithm
void drawLayerCircle(int x0, int y0, int radius, const CRGB color, uint8_t thickness = 0) //, uint8_t blend = 0)
{
  int a = radius, b = 0;
  int radiusError = 1 - a;

  if (radius == 0) {
    addColorXY(x0, y0, color, thickness);
    return;
  }

  while (a >= b)
  {
    addColorXY(a + x0, b + y0, color, thickness);
    addColorXY(b + x0, a + y0, color, thickness);
    addColorXY(-a + x0, b + y0, color, thickness);
    addColorXY(-b + x0, a + y0, color, thickness);
    addColorXY(-a + x0, -b + y0, color, thickness);
    addColorXY(-b + x0, -a + y0, color, thickness);
    addColorXY(a + x0, -b + y0, color, thickness);
    addColorXY(b + x0, -a + y0, color, thickness);

    b++;
    if (radiusError < 0)
      radiusError += 2 * b + 1;
    else
    {
      a--;
      radiusError += 2 * (b - a + 1);
    }
  }
}

const uint8_t waveCount = 8;

// track the XY coordinates and radius of each wave
uint16_t radii[waveCount];
uint8_t waveX[waveCount];
uint8_t waveY[waveCount];
CRGB waveColor[waveCount];

const uint16_t maxRadius = 512;


void touchControls() 
{

  for (uint8_t i = 0; i < touchPointCount; i++) {
    // change a setting when there's a new touch
    if (touch[i] > 127) 
    {
      if (touchActive[i] == 5) // debounce
      {
        if (i==0) // speed A0 - speed up
        {
          speed_i ++;
          speed_i %= NUM_SPEEDS;
          speed = speeds[speed_i];
        }
        if (i==1) // mode A1 - go forward, turn on autoplay
        {
          adjustPattern(true);
          autoplay = true;
          autoPlayTimeout = millis() + (autoplayDuration * 1000);
        }
      }
      else if (touchActive[i] == 40) // long press
      {
        if (i==0) // speed A0 - slow down
        {
          speed_i += NUM_SPEEDS -2 ;
          speed_i %= NUM_SPEEDS;
          speed = speeds[speed_i];
        }
        if (i==1) // mode A1 - go back and stop autoplay
        {
          adjustPattern(false);
          adjustPattern(false);
          autoplay = false;
          // autoPlayTimeout = millis() + (autoplayDuration * 1000);
        }

      }
      else if (touchActive[i] == 100) // longer press
      {
        if (i==0) // speed A0 - slow down
        {
          adjustBrightness(true);
          touchActive[i]= 41; // gear up for another brightness increase
        }
        if (i==1) // mode A1 - go back and stop autoplay
        {
        }

      }
      else if (touchActive[i] == 250) // hold
      {
        touchActive[i]--;
      }
      touchActive[i]++;
    }
    else
    {
      touchActive[i] = 0;
    }
  }

}

void touchDemo() {
  // fade all of the LEDs a small amount each frame
  // increasing this number makes the waves fade faster
  // fadeToBlackBy(leds, NUM_LEDS, 30);

  for (uint8_t i = 0; i < touchPointCount; i++) 
  {
    // start new waves when there's a new touch
    if (touch[i] > 127 && radii[i] == 0) {
      radii[i] = 32;
      waveX[i] = touchPointX[i];
      waveY[i] = touchPointY[i];
      waveColor[i] = CHSV(random8(), 255, 255);
    }
  }

  activeWaves = false;

  for (uint8_t i = 0; i < waveCount; i++)
  {
    // increment radii if it's already been set in motion
    if (radii[i] > 0 && radii[i] < maxRadius) radii[i] = radii[i] + 8;

    // reset waves already at max
    if (radii[i] >= maxRadius) {
      activeWaves = true;
      radii[i] = 0;
    }

    if (radii[i] == 0)
      continue;

    activeWaves = true;

    CRGB color = waveColor[i];

    uint8_t x = waveX[i];
    uint8_t y = waveY[i];

    // draw waves starting from the corner closest to each touch sensor
    // drawCircle(x, y, radii[i], color, 4);
    drawCircle(x, y, radii[i], color, 8);
    if (radii[i] > 8)
    {
      color = color.nscale8(128);
      drawCircle(x, y, radii[i]-8, color, 8);
      if (radii[i] > 16)
      {
        color = color.nscale8(128);
        drawCircle(x, y, radii[i]-16, color, 8);
      }
    }
  }
}

