// This program is a testing ground for some of the ideas I want to implement into my LED Table code.
// setup: 1 x toggle switch to power the LEDS. 3 x NO buttons to control: Colours, Display mode, Random.
// Aim: The three buttons each control a different aspect.
// 1. The first one cycles through the various colours available
// 2. The second button cycles through the display effects of the LEDs (fadeIn, Breathing, blinking, etc...)
// 3. The third button acts as a randomiser for both arguments (randomizes colour and display effects)
//EACH BUTTON CONNECTS TO THE GROUND PIN
/* Possible Improvements:
    1. Store the palettes into an array and cycle through the array with a button press instead of having individual functions/methods
       to change the palette shown.

    2. Possibly store objects of the display mode functions in an array, eliminating the need for switch cases and increacing the
       efficiency of the program.
*/
//Load FastLED library
#include <FastLED.h>

//Define variables
#define LED_PIN 9
#define NUM_LEDS 110    //8 Maximum leds when the arduino Uno is running on USB power.
#define BRIGHTNESS 255
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];   //Array for the led strip

// Palettes
CRGBPalette16 currentPalette;
TBlendType currentBlending;
extern const TProgmemPalette16 myNewPal_p PROGMEM;
//Define buttons and variables required
#define COL_BTN 2
#define MOD_BTN 3
#define RND_BTN 4
int lastButtonState = LOW;
int lastButtonState2 = LOW;
int lastButtonState3 = LOW;
int buttonState = 0;
int buttonState2 = 0;
int buttonState3 = 0;
unsigned long time = 0;
unsigned long debounce = 10;
int state = LOW;
int palette = 0;
int mode = 0;

void setup() {
  delay(3000);  //Power-up safety delay
  Serial.begin(9600);
  pinMode(COL_BTN, INPUT);
  digitalWrite(COL_BTN, HIGH);
  pinMode(MOD_BTN, INPUT);
  digitalWrite(MOD_BTN, HIGH);
  pinMode(RND_BTN, INPUT);
  digitalWrite(RND_BTN, HIGH);

  /*Interrupt buttons for the colour and mode buttons.
     Only works for pins 2 and 3 on the Uno, Nano, Mini and other
     ATMEGA-328 based boards.
  */
  attachInterrupt(digitalPinToInterrupt(COL_BTN), isr, CHANGE);
  attachInterrupt(digitalPinToInterrupt(MOD_BTN), isr2, CHANGE);

  //Configure LED strip and set brightness
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(BRIGHTNESS);
  currentBlending = LINEARBLEND;
}

void loop() {
  Serial.println(mode);
  //Index/speed to fill the colour palette in the fillLedsfrompalette
  static uint8_t startIndex = 0;
  startIndex += 1; /* motion speed */

  //Switch case to change the palettes
  switch (palette) {
    case 0:
      cloud();
      break;
    case 1:
      synthWave();
      break;
    case 2:
      bhw_01();
      break;
    case 3:
      bhw_19();
      break;
    case 4:
      bhw_40();
      break;
    case 5:
      bhw_purp();
      break;
    case 6:
      currentPalette = RainbowColors_p;
      break;
    case 7:
      purpGreen();
      break;
    case 8:
      es_ocean_breeze_049();
      break;
    case 9:
      prism();
      break;
    default:
      FastLED.clear();
      break;
  }

  //Switch case for animation/ display code
  switch (mode) {
    case 0:
      staticLEDs(BRIGHTNESS);
      break;
    case 1:
      FillLEDsFromPaletteColors(startIndex);
      break;
    case 2:
      breathing();
      FastLED.delay(30);
      break;
    case 3:
      glitter(80);
      break;
  }
  //Restart value for the variables
  if (palette > 9) {
    palette = 0;
  }
  if (mode > 3) {
    mode = 0;
  }
  //Random Button configuration
  buttonState3 = digitalRead(RND_BTN);
  if (buttonState3 == LOW && lastButtonState3 != buttonState3 && (millis() - time) > debounce ) {
    if (state == HIGH) {
      mode = random(0, 3);
      palette = random(0, 6);
      state = LOW;
    } else {
      mode = random(0, 3);
      palette = random(0, 6);
      state = HIGH;
    }
    time = millis();
  }
  lastButtonState3 = buttonState3;
}

//Colour Button method. This is executed when there is an interruption on the button.
void isr() {
  buttonState = digitalRead(COL_BTN);
  //This increments the mode variable everytime the button is pressed.
  //The if statement checks the current state of the button (HIGH or LOW)
  //and if enough time has passed since the previous button press (debounce - to ignore any noise on the circuit).
  if (buttonState == LOW  && lastButtonState != buttonState && (millis() - time) > debounce ) {
    //This statement acts as a toggle for the state variable
    if (state == HIGH) {
      palette++;
      state = LOW;
    } else {
      state = HIGH;
      palette++;
    }
    time = millis();
  }
  lastButtonState = buttonState;
}

//Mode Button. This is executed when there is an interruption on the button.
void isr2() {
  buttonState2 = digitalRead(MOD_BTN);
  if (buttonState2 == LOW && lastButtonState2 != buttonState2 && (millis() - time) > debounce ) {
    if (state == HIGH) {
      mode++;
      state = LOW;
    } else {
      state = HIGH;
      mode++;
    }
    time = millis();
  }
  lastButtonState2 = buttonState2;
}

/**********************************************************/
/************* LED DISPLAY FUNCTIONS **********************/
/**********************************************************/

//Stores the palette into the led arry for display
//ColorFromPalette(palette,colourindex(position of colour within the palette), brightness, blendMethod);
//In the Loop section, the colour index argument is the index of the colours in the palette.
void FillLEDsFromPaletteColors( uint8_t colorIndex)
{
  uint8_t brightness = 255;
  for ( int i = 0; i < NUM_LEDS; i++) {
    leds[i] = ColorFromPalette( currentPalette, colorIndex, brightness, currentBlending);
    colorIndex += 3;
  }
  FastLED.show();
  FastLED.delay(30);
}

//Fills LED strip with colours from palette. Produces a static appearance, no animation
void staticLEDs( uint8_t brightness)
{
  uint8_t colIndex = 0;
  for ( int i = 0; i < NUM_LEDS; i++) {
    leds[i] = ColorFromPalette( currentPalette, colIndex, brightness, currentBlending);
    colIndex += 6;
  }
  FastLED.show();
}

//Breathing effect
void breathing() {
  for (int j = 0; j < BRIGHTNESS; j++) {
    staticLEDs(j);
    FastLED.show();
    FastLED.delay(10);
  }
  for (int i = BRIGHTNESS; i > 0; i = i - 2) {
    staticLEDs(i);
    FastLED.show();
    FastLED.delay(50);
  }
}
// This function should iterate through the strip and only illuminate led positions that are
// multiples of 2. Outcome: Flashing as it increments and loops
void test(uint8_t brightness, uint8_t colIndex) {
  for (int j = 0; j < NUM_LEDS; j++) {
    if (j%2== 0) {
      leds[j] = ColorFromPalette(currentPalette, colIndex, brightness, currentBlending);
      colIndex += 8;
    }else if(j%2!=0){
      leds[j]=CRGB::Black;
    }
  }
  FastLED.show();
}

void glitter(fract8 chanceOfGlitter){
  staticLEDs(BRIGHTNESS);
  if(random8()<chanceOfGlitter){
    leds[ random16(NUM_LEDS)] +=CRGB::White;
    FastLED.show();
  }
}
/**********************************************************/
/******************** PALETTES ****************************/
/**********************************************************/
// Gradient palette "bhw1_01_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/bhw/bhw1/tn/bhw1_01.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 12 bytes of program space.

DEFINE_GRADIENT_PALETTE( bhw1_01_gp ) {
  0, 227, 101,  3,
  117, 194, 18, 19,
  255,  92,  8, 192
};

// Gradient palette "cloud_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/rc/tn/cloud.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 12 bytes of program space.

DEFINE_GRADIENT_PALETTE( cloud_gp ) {
  0, 247, 149, 91,
  127, 208, 32, 71,
  255,  42, 79, 188
};

// Gradient palette "bhw1_purplered_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/bhw/bhw1/tn/bhw1_purplered.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 8 bytes of program space.

DEFINE_GRADIENT_PALETTE( bhw1_purplered_gp ) {
  0, 255,  0,  0,
  255, 107,  1, 205
};

// Gradient palette "bhw1_19_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/bhw/bhw1/tn/bhw1_19.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 20 bytes of program space.

DEFINE_GRADIENT_PALETTE( bhw1_19_gp ) {
  0,  82,  1,  3,
  79,  20,  1,  1,
  181, 139,  1,  0,
  252,  20,  1,  1,
  255,  20,  1,  1
};

// Gradient palette "bhw4_040_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/bhw/bhw4/tn/bhw4_040.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 32 bytes of program space.

DEFINE_GRADIENT_PALETTE( bhw4_040_gp ) {
  0,  26,  7, 69,
  73, 110, 36, 240,
  109, 249, 69, 245,
  142, 255, 182, 255,
  173, 120, 124, 245,
  204,  98, 59, 207,
  249,  30,  9, 103,
  255,  30,  9, 103
};

// Gradient palette "es_ocean_breeze_049_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/es/ocean_breeze/tn/es_ocean_breeze_049.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 24 bytes of program space.

DEFINE_GRADIENT_PALETTE( es_ocean_breeze_049_gp ) {
  0, 184, 231, 250,
  76,   0, 112, 203,
  77,  29, 168, 228,
  79, 179, 235, 255,
  153,  64, 189, 255,
  255,   0, 124, 199
};

// Gradient palette "prism_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/h5/tn/prism.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 64 bytes of program space.

DEFINE_GRADIENT_PALETTE( prism_gp ) {
  0, 255,  0,  0,
  17, 255, 55,  0,
  33, 255, 255,  0,
  51,   0, 255,  0,
  68,   0,  0, 255,
  84,  88,  0, 255,
  102, 255,  0,  0,
  119, 255, 55,  0,
  135, 255, 255,  0,
  153,   0, 255,  0,
  170,   0,  0, 255,
  186,  88,  0, 255,
  204, 255,  0,  0,
  221, 255, 55,  0,
  237, 255, 255,  0,
  255,   0, 255,  0
};


/**********************************************************/
/*************** PALETTE FUNCTIONS ************************/
/**********************************************************/
void synthWave()
{
  CRGB deepPurple = CRGB(28, 6, 88); //CHSV(256,93,35);
  CRGB purple = CRGB(93, 8, 134); //CHSV(280,94,53);
  CRGB pink = CRGB(255, 22, 144); //CHSV(329,91,100);
  CRGB skyBlue = CRGB(54, 185, 196); //CHSV(185,72,77);

  currentPalette = CRGBPalette16(
                     purple, purple, pink, pink,
                     deepPurple, deepPurple, skyBlue, skyBlue,
                     deepPurple, deepPurple, pink, pink,
                     skyBlue, skyBlue, deepPurple, deepPurple);
}

//Function that assigns a new palette to the currentPalette variable
void purpGreen()
{
  CRGB purple = CHSV( HUE_PURPLE, 255, 255);
  CRGB green  = CHSV( HUE_GREEN, 255, 255);
  CRGB black  = CRGB::Black;

  currentPalette = CRGBPalette16(
                     green,  green,  black,  black,
                     purple, purple, black,  black,
                     green,  green,  black,  black,
                     purple, purple, black,  black );
}

void bhw_01() {
  currentPalette = CRGBPalette16(bhw1_01_gp);
}

void bhw_19() {
  currentPalette = CRGBPalette16(bhw1_19_gp);
}

void bhw_40() {
  currentPalette = CRGBPalette16(bhw4_040_gp);
}

void bhw_purp() {
  currentPalette = CRGBPalette16(bhw1_purplered_gp);
}

void cloud() {
  currentPalette = CRGBPalette16(cloud_gp);
}

void es_ocean_breeze_049() {
  currentPalette = CRGBPalette16(es_ocean_breeze_049_gp);
}

void prism() {
  currentPalette = CRGBPalette16(prism_gp);
}
