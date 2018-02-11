/* Feb 27 2016
   by Sophie Amin for Makefashion 2016

  Push button jig will be used to control LEDS on a 46 x 8 neopixel matrix
  Run pushbuttons using a PC Interrupt on A8 and A9
  Add basic graphics to interact with cursor at the bottom of the matrix.
  Jewel drop game prototype.
  some code based on/helped by https://github.com/M-tech-Creations
  Generate up to 4 jewels to fall at a time
  Code modified to run with Arduino mega, originally written for the xadow mainboard
  Mega program changes:
  V2: Added Score Keeping for one jewel
  V3: Get more gems to fall and restart and keep track of the score for one jewel
      also switched the pins around for the gamepad jig for intutive control
  V4: refresh screen for gems at different intervals, jewel 1 can fall at 20ms, jewel 2 can fall at 70ms, jewel 3 can fall at 100ms, jewel 4 can fall at 200ms
      fixed random number generator's range from 0-7 to 0-8. Introduced scorekeeping for all 4 jewels: jewel 1 5 pts, jewel 2 3 pts, jewel 3 2 pts, jewel 4 1 pt.
      There are display bugs in this version including: cursor disappearing when the jewel restarts on its line, cursor getting "erased" by the jewel as it falls
  V5: Fixed cursor getting erased by jewels starting on the same line and jewels "running over" the cursor and erasing it. Might have introduced some lag issues with the fix?
  V6: Made the jewels one pixel longer with a dimmer jewel on top of the same color. Jewel four (currently dark blue) is not incrementing score.
  V7: Fixed jewel four score incrementing. Program runs the same otherwise.


  DressBLE V1: Ported code to fit on the 46x9 dress panel matrix. Added 4 more jewels to program. Readjusted Matrix parameters.
  DressBLE V2: Add in shoulders code and game end control flags.
  DressVLE V3: Change gems to GG colors.
  DressVLE V4: switch the BLE pins from 10 and 9 to and photomode pin 13 to 28. Made jewels worth 20  points.
               Used pullup resistors for the switch input to take out hardware. Added in a second demo mode of a different color.
  DressBLE V5: Change inputs for final wiring ease.
  DressBLE V6: The jewel values were lowered to be worth between 1-5 to lengthen game time for Fashionware.
*/



#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>
#include <SPI.h>
#include <Adafruit_BLE_UART.h>


#define MATRIXPIN 6 //led pin on ARDUINO MEGA
#define STRIPONEPIN 3
#define PHOTOMODEPIN 4
#define DEMOMODEPIN 25
#define GAMEMODEPIN 29


//COLOR DEFINITIONS
// Color definitions
// Color definitions
#define BLACK    0x0000
#define BLUE     0,0,255
#define RED      255,0,0
#define DIMRED   100,0,0
#define GREEN    0,255,0
#define DIMGREEN 0,100,0
#define BLUE     0,0,255
#define DIMBLUE  0,0,100
#define TEAL     0,255,255
#define DIMTEAL  0,100,100
#define PINK     255,51,153
#define DIMPINK  100,20,61
#define GOLD     245,211,98              //0x33FF33
#define AMETHYST 204,160,160 //186,85,211
#define DIMAMETHYST 81,64,64
#define WHITE    255,255,255
#define ROSEGOLD 245,152,141
#define SALMON   245,149,145
#define PURPLE   245,152,141 //originally purple 204,160,160
#define TRUPURP 255,0,255
#define DIMPURP 100,0,100
#define YUCKYYELLOW      255,255,0
#define DIMYUCKYYELLOW   100,100,0
#define WARMWHITE 245,152,141
#define ORANGE   255,140,0
#define OFF      0,0,0

//Gamergirlscolors
#define GGPINKGOLD 245,182,136 //old WHITE
#define DIMGGPINKGOLD 98,73,54

#define GGPINK 255,20,147
#define DIMGGPINK 100,8,59

#define GGPURPLE 255,0,255 //SIMILAR
#define DIMGGPURPLE 100,0,100

#define GGBLUEPURP 148,0,211
#define DIMGGBLUEPURP 59,0,84

#define GGBLUEPURPTWO 138,43,226
#define DIMGGBLUEPURPTWO 55,17,90

#define GGBLUE     0,0,255
#define DIMGGBLUE  0,0,100

#define GGBLUEBELL 123,104,238
#define DIMGGBLUEBELL 49,42,95

#define GGLIGHTPINK 255,105,180
#define DIMGGLIGHTPINK 100,42,72

#define YELLOW 255,255,0

#define ADAFRUITBLE_REQ 33 
#define ADAFRUITBLE_RDY 2     // This should be an interrupt pin, on Uno thats #2 or #3
#define ADAFRUITBLE_RST 39


// MATRIX DECLARATION:
// Parameter 1 = width of NeoPixel matrix
// Parameter 2 = height of matrix
// Parameter 3 = pin number (most are valid)
// Parameter 4 = matrix layout flags, add together as needed:
//   NEO_MATRIX_TOP, NEO_MATRIX_BOTTOM, NEO_MATRIX_LEFT, NEO_MATRIX_RIGHT:
//     Position of the FIRST LED in the matrix; pick two, e.g.
//     NEO_MATRIX_TOP + NEO_MATRIX_LEFT for the top-left corner.
//   NEO_MATRIX_ROWS, NEO_MATRIX_COLUMNS: LEDs are arranged in horizontal
//     rows or in vertical columns, respectively; pick one or the other.
//   NEO_MATRIX_PROGRESSIVE, NEO_MATRIX_ZIGZAG: all rows/columns proceed
//     in the same order, or alternate lines reverse direction; pick one.
//   See example below for these values in action.
// Parameter 5 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)


Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(46, 9, MATRIXPIN,
                            NEO_MATRIX_TOP     + NEO_MATRIX_RIGHT +
                            NEO_MATRIX_ROWS + NEO_MATRIX_ZIGZAG,
                            NEO_GRB            + NEO_KHZ800);

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)

Adafruit_NeoPixel strip1 = Adafruit_NeoPixel(7, STRIPONEPIN, NEO_GRB + NEO_KHZ800); //left shoulder

Adafruit_NeoPixel strip = Adafruit_NeoPixel(414, MATRIXPIN, NEO_GRB + NEO_KHZ800); //"strip" is actually the matrix leds


Adafruit_BLE_UART BTLEserial = Adafruit_BLE_UART(ADAFRUITBLE_REQ, ADAFRUITBLE_RDY, ADAFRUITBLE_RST);


const uint16_t colors[] = {
  matrix.Color(245, 211, 98), matrix.Color(186, 85, 211), matrix.Color(38, 43, 226)
};


/*POSITION VARIABLES FOR JEWELS*/
int xCursorPos = 0;
int yCursorPosPrev = 0;
int yCursorPos = 0;
int score = 0; //start the game at 0 points
//4 jewels can fall simultaneously at different points on the matrix
int jewelOneX = 46;
int jewelOneY = 3;
int jewelOneXPrev = 0;

int jewelTwoX = 46;
int jewelTwoY = 5;
int jewelTwoXPrev = 0;

int jewelThreeX = 46;
int jewelThreeY = 3;
int jewelThreeXPrev = 0;

int jewelFourX = 46;
int jewelFourY = 3;
int jewelFourXPrev = 0;

int jewelFiveX = 46;
int jewelFiveY = 0;
int jewelFiveXPrev = 0;

int jewelSixX = 46;
int jewelSixY = 1;
int jewelSixXPrev = 0;

int jewelSevenX = 46;
int jewelSevenY = 2;
int jewelSevenXPrev = 0;

int jewelEightX = 46;
int jewelEightY = 5;
int jewelEightXPrev = 0;


/*shoulder light variables*/
int shoulderLEDPos = 0;
int prevShoulderLEDPos = 0;

/*switch mode variables for changing display*/
int demoModeFlag = 0;
int photoModeFlag = 0;
int gameModeFlag = 0;
/*GAME RUNNING VARIABLES*/
long previousMillis1 = 0;//used for the game timer for jewel1
long previousMillis2 = 0;//used for the game timer for jewel2
long previousMillis3 = 0;//used for the game timer for jewel3
long previousMillis4 = 0;//used for the game timer for jewel4
long previousMillis5 = 0;//used for the game timer for jewel5
long previousMillis6 = 0;//used for the game timer for jewel6
long previousMillis7 = 0;//used for the game timer for jewel7
long previousMillis8 = 0;//used for the game timer for jewel8
/*jewel speed variables*/
long interval1 = 350; //how long between each update of jewel1
long interval2 = 70; //how long between each update of jewel2
long interval3 = 100; //how long between each update of jewel3
long interval4 = 200; //how long between each update of jewel4
long interval5 = 50; //how long between each update of jewel5
long interval6 = 150; //how long between each update of jewel6
long interval7 = 20; //how long between each update of jewel7
long interval8 = 250; //how long between each update of jewel8
//add two more intervals as the game progresses
unsigned long currentMillis = 0;//used for the game timer

//when score is 0 to 10 do nothing, game has just started
int scoreStageOne = 0;
int scoreStageTwo = 0;
int scoreStageThree = 0;
int scoreStageFour = 0;

int gameEndFlag = 0;
//shoulder interval and Millis variables
long previousMillisShoulder = 0;
long intervalShoulder = 400;

long gameEndTimer = 0;

/*BLE VARIABLES*/
char c;
char RecievedBuf[5]; //buffer should recieve 5 characters per button push
//int flag = 0;
int count = 0;
int i = 0;

//test variables
int setStripAll = 0;


void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
  // make the pushbutton pins an input:
  Serial.println(F("Gamergirls Dress BLE Test"));

  randomSeed(analogRead(A5));
  pinMode(PHOTOMODEPIN, INPUT_PULLUP);
  pinMode(DEMOMODEPIN, INPUT_PULLUP);
  pinMode(GAMEMODEPIN, INPUT_PULLUP);
  matrix.begin();
  strip1.begin();
  strip.begin();
  matrix.setTextWrap(false);
  matrix.setBrightness(30);
  strip1.setBrightness(20);
  strip.setBrightness(30);
  matrixClear();
  matrix.drawPixel(jewelOneX, jewelOneY, matrix.Color(GREEN));
  while (setStripAll < 8)
  {
    strip1.setPixelColor(setStripAll, BLUE);
    strip.setPixelColor(setStripAll, BLUE);
    setStripAll++;
  }

  matrix.show();
  strip1.show(); // Initialize all pixels to 'off'
  strip.show(); // Initialize all pixels to 'off'
  BTLEserial.setDeviceName("DRESSA"); /* 7 characters max! */
  BTLEserial.begin();

}
aci_evt_opcode_t laststatus = ACI_EVT_DISCONNECTED;
// the loop routine runs over and over again forever:
void loop() {


  if (score == 0)
  {
    setStripAll = 0;
    while (setStripAll < 8)
    {
      strip1.setPixelColor(setStripAll, BLUE);
      setStripAll++;
    }
    strip1.show();
    setStripAll = 0;
    intervalShoulder = 400;
    scoreStageOne = 0;
    scoreStageTwo = 0;
    scoreStageThree = 0;
    scoreStageFour = 0;

    gameEndFlag = 0;
  }
  photoModeFlag = digitalRead(PHOTOMODEPIN);
  demoModeFlag = digitalRead(DEMOMODEPIN);
  gameModeFlag = digitalRead(GAMEMODEPIN);
  //  Serial.println("PhotoMode");
  //  Serial.println(photoModeFlag);
  //  Serial.println("DEMOMODE");
  //  Serial.println(demoModeFlag);
  // Tell the nRF8001 to do whatever it should be working on.
  if (photoModeFlag == 0 && demoModeFlag == 1 && gameModeFlag == 1)
  {
    score = 0;
    matrixClear();
    theaterChase(strip.Color(WHITE), 50);

  }
  else if (photoModeFlag == 1 && demoModeFlag == 0 && gameModeFlag == 1)//other demo mode
  {
    score = 0;
    matrixClear();
    theaterChase(strip.Color(GGPURPLE), 50);

  }
  else
  {

    BTLEserial.pollACI();

    // Ask what is our current status
    aci_evt_opcode_t status = BTLEserial.getState();
    // If the status changed....
    if (status != laststatus) {
      // print it out!
      if (status == ACI_EVT_DEVICE_STARTED) {
        Serial.println(F("* Advertising started"));
      }
      if (status == ACI_EVT_CONNECTED) {
        Serial.println(F("* Connected!"));
      }
      if (status == ACI_EVT_DISCONNECTED) {
        Serial.println(F("* Disconnected or advertising timed out"));
      }
      // OK set the last status change to this one
      laststatus = status;
    }

    if (status == ACI_EVT_CONNECTED) {
      // Lets see if there's any data for us!
      if (BTLEserial.available()) {
        // Serial.print("* "); Serial.print(BTLEserial.available()); Serial.println(F(" bytes available from BTLE"));
      }
      // OK while we still have something to read, get a character and print it out
      i = 0;
      while (BTLEserial.available()) {
        c = BTLEserial.read();
        //  Serial.print(c, DEC);
        //   Serial.println();

        RecievedBuf[i] = c;
        if (i == 4)
        {

          if ((RecievedBuf[3] == 49) && (RecievedBuf[4] == 51))//check for a right button press
          {
            pbRightInt();
          }
          else if ((RecievedBuf[3] == 49) && (RecievedBuf[4] == 52))//check for a left button press
          {
            pbLeftInt();
          }

        }
        i++;

      }


    }


    //jewel falls on the X axis, which starts at 46 and ends at 0

    currentMillis = millis();//get the current time
    /*comment below this line*/

    //game clock

    if (gameEndFlag == 0)
    {
      if (currentMillis - previousMillis1 >= interval1)
      { //check if the  current time - the last time is greater the the delay time you want
        previousMillis1 = currentMillis;
        //start first jewel at the very beginning of the program
        jewelOneXPrev = jewelOneX;
        jewelOneX--;
        matrix.drawPixel(jewelOneXPrev, jewelOneY, matrix.Color(OFF));
        //if the falling jewel is right above the cursor, increment the score
        if ((jewelOneX == 1) && (yCursorPos == jewelOneY))
        {
          score = score + 1;
        }


        //Reassign the jewels a new position at the top of the grid
        if (jewelOneX < 0)
        {
          matrix.drawPixel((jewelOneXPrev + 1), jewelOneY, matrix.Color(OFF));
          jewelOneY = random(0, 9);
          jewelOneX = 46;
        }

        //redraw the pixel positions
        //draw jewel one
        matrix.drawPixel((jewelOneXPrev + 1), jewelOneY, matrix.Color(OFF));
        if (jewelOneX < 46)
        {
          matrix.drawPixel((jewelOneXPrev), jewelOneY, matrix.Color(DIMTEAL));
        }
        matrix.drawPixel(jewelOneX, jewelOneY, matrix.Color(TEAL));
        //  matrix.drawPixel((jewelOneXPrev-1),jewelOneY,matrix.Color(OFF));
        matrix.drawPixel((jewelOneX - 1), jewelOneY, matrix.Color(DIMTEAL));
        matrix.drawPixel(xCursorPos, yCursorPosPrev, matrix.Color(OFF));
        matrix.drawPixel(xCursorPos, yCursorPos, matrix.Color(WHITE));
        matrix.show();
      }

      if (currentMillis - previousMillis2 >= interval2) //checks after 70ms
      { //check if the  current time - the last time is greater the the delay time you want
        previousMillis2 = currentMillis;
        if (currentMillis > 70) //start second jewel after 70 ms
        {
          jewelTwoXPrev = jewelTwoX;
          jewelTwoX--;
          matrix.drawPixel(jewelTwoXPrev, jewelTwoY, matrix.Color(OFF));
        }

        if ((jewelTwoX == 1) && (yCursorPos == jewelTwoY))
        {
          score = score + 5;
        }

        if (jewelTwoX < 0)
        {
          matrix.drawPixel((jewelTwoXPrev + 1), jewelTwoY, matrix.Color(OFF));
          jewelTwoY = random(0, 9);
          jewelTwoX = 46;
        }
        //draw jewel two
        matrix.drawPixel((jewelTwoXPrev + 1), jewelTwoY, matrix.Color(OFF));
        if (jewelTwoX < 46)
        {
          matrix.drawPixel((jewelTwoXPrev), jewelTwoY, matrix.Color(DIMGGPINK));
        }
        matrix.drawPixel(jewelTwoX, jewelTwoY, matrix.Color(GGPINK));
        matrix.drawPixel((jewelTwoX - 1), jewelTwoY, matrix.Color(DIMGGPINK));
        matrix.drawPixel(xCursorPos, yCursorPosPrev, matrix.Color(OFF));
        matrix.drawPixel(xCursorPos, yCursorPos, matrix.Color(WHITE));
        matrix.show();


      }
      if (currentMillis - previousMillis3 >= interval3) //checks after 100ms
      { //check if the  current time - the last time is greater the the delay time you want
        previousMillis3 = currentMillis;
        if (currentMillis > 100) //start second jewel after 70 ms
        {
          jewelThreeXPrev = jewelThreeX;
          jewelThreeX--;
          matrix.drawPixel(jewelThreeXPrev, jewelThreeY, matrix.Color(OFF));
        }

        if ((jewelThreeX == 1) && (yCursorPos == jewelThreeY))
        {
          score = score + 1;
        }

        if (jewelThreeX < 0)
        {
          matrix.drawPixel((jewelThreeXPrev + 1), jewelThreeY, matrix.Color(OFF));
          jewelThreeY = random(0, 9);
          jewelThreeX = 46;
        }
        //draw jewel three
        matrix.drawPixel((jewelThreeXPrev + 1), jewelThreeY, matrix.Color(OFF));
        if (jewelThreeX < 46)
        {
          matrix.drawPixel((jewelThreeXPrev), jewelThreeY, matrix.Color(DIMGGPINKGOLD));
        }
        matrix.drawPixel(jewelThreeX, jewelThreeY, matrix.Color(GGPINKGOLD));
        matrix.drawPixel((jewelThreeX - 1), jewelThreeY, matrix.Color(DIMGGPINKGOLD));
        matrix.drawPixel(xCursorPos, yCursorPosPrev, matrix.Color(OFF));
        matrix.drawPixel(xCursorPos, yCursorPos, matrix.Color(WHITE));
        matrix.show();

      }
      if (currentMillis - previousMillis4 >= interval4) //checks after 200ms
      { //check if the  current time - the last time is greater the the delay time you want
        previousMillis4 = currentMillis;
        if (currentMillis > 200) //start second jewel after 200 ms
        {
          jewelFourXPrev = jewelFourX;
          jewelFourX--;
          matrix.drawPixel(jewelFourXPrev, jewelFourY, matrix.Color(OFF));
        }

        if ((jewelFourX == 1) && (yCursorPos == jewelFourY))
        {
          score = score + 1;
        }


        if (jewelFourX < 0)
        {
          matrix.drawPixel((jewelFourXPrev + 1), jewelFourY, matrix.Color(OFF));
          jewelFourY = random(0, 9);
          jewelFourX = 46;
        }
        //draw jewel four
        matrix.drawPixel((jewelFourXPrev + 1), jewelFourY, matrix.Color(OFF));
        if (jewelFourX < 46)
        {
          matrix.drawPixel((jewelFourXPrev), jewelFourY, matrix.Color(DIMBLUE));
        }
        matrix.drawPixel(jewelFourX, jewelFourY, matrix.Color(BLUE));
        matrix.drawPixel((jewelFourX - 1), jewelFourY, matrix.Color(DIMBLUE));
        matrix.drawPixel(xCursorPos, yCursorPosPrev, matrix.Color(OFF));
        matrix.drawPixel(xCursorPos, yCursorPos, matrix.Color(WHITE));
        matrix.show();
      }
      if (currentMillis - previousMillis5 >= interval5) //checks after 200ms
      { //check if the  current time - the last time is greater the the delay time you want
        previousMillis5 = currentMillis;
        if (currentMillis > interval5) //start fifth jewel after 200 ms
        {
          jewelFiveXPrev = jewelFiveX;
          jewelFiveX--;
          matrix.drawPixel(jewelFiveXPrev, jewelFiveY, matrix.Color(OFF));
        }

        if ((jewelFiveX == 1) && (yCursorPos == jewelFiveY))
        {
          score = score + 1;
        }


        if (jewelFiveX < 0)
        {
          matrix.drawPixel((jewelFiveXPrev + 1), jewelFiveY, matrix.Color(OFF));
          jewelFiveY = random(0, 9);
          jewelFiveX = 46;
        }
        //draw jewel four
        matrix.drawPixel((jewelFiveXPrev + 1), jewelFiveY, matrix.Color(OFF));
        if (jewelFiveX < 46)
        {
          matrix.drawPixel((jewelFiveXPrev), jewelFiveY, matrix.Color(DIMPURP));
        }
        matrix.drawPixel(jewelFiveX, jewelFiveY, matrix.Color(TRUPURP));
        matrix.drawPixel((jewelFiveX - 1), jewelFiveY, matrix.Color(DIMPURP));
        matrix.drawPixel(xCursorPos, yCursorPosPrev, matrix.Color(OFF));
        matrix.drawPixel(xCursorPos, yCursorPos, matrix.Color(WHITE));
        matrix.show();

      }
      if (currentMillis - previousMillis6 >= interval6) //checks after 200ms
      { //check if the  current time - the last time is greater the the delay time you want
        previousMillis6 = currentMillis;
        if (currentMillis > interval6) //start fifth jewel after 200 ms
        {
          jewelSixXPrev = jewelSixX;
          jewelSixX--;
          matrix.drawPixel(jewelSixXPrev, jewelSixY, matrix.Color(OFF));
        }

        if ((jewelSixX == 1) && (yCursorPos == jewelSixY))
        {
          score = score + 3;
        }


        if (jewelSixX < 0)
        {
          matrix.drawPixel((jewelSixXPrev + 1), jewelSixY, matrix.Color(OFF));
          jewelSixY = random(0, 9);
          jewelSixX = 46;
        }
        //draw jewel four
        matrix.drawPixel((jewelSixXPrev + 1), jewelSixY, matrix.Color(OFF));
        if (jewelSixX < 46)
        {
          matrix.drawPixel((jewelSixXPrev), jewelSixY, matrix.Color(DIMGGBLUEBELL));
        }
        matrix.drawPixel(jewelSixX, jewelSixY, matrix.Color(GGBLUEBELL));
        matrix.drawPixel((jewelSixX - 1), jewelSixY, matrix.Color(DIMGGBLUEBELL));
        matrix.drawPixel(xCursorPos, yCursorPosPrev, matrix.Color(OFF));
        matrix.drawPixel(xCursorPos, yCursorPos, matrix.Color(WHITE));
        matrix.show();

      }
      if (currentMillis - previousMillis7 >= interval7) //checks after 200ms
      { //check if the  current time - the last time is greater the the delay time you want
        previousMillis7 = currentMillis;
        if (currentMillis > interval7) //start fifth jewel after 200 ms
        {
          jewelSevenXPrev = jewelSevenX;
          jewelSevenX--;
          matrix.drawPixel(jewelSevenXPrev, jewelSevenY, matrix.Color(OFF));
        }

        if ((jewelSevenX == 1) && (yCursorPos == jewelSevenY))
        {
          score = score + 1;
        }


        if (jewelSevenX < 0)
        {
          matrix.drawPixel((jewelSevenXPrev + 1), jewelSevenY, matrix.Color(OFF));
          jewelSevenY = random(0, 9);
          jewelSevenX = 46;
        }
        //draw jewel four
        matrix.drawPixel((jewelSevenXPrev + 1), jewelSevenY, matrix.Color(OFF));
        if (jewelSevenX < 46)
        {
          matrix.drawPixel((jewelSevenXPrev), jewelSevenY, matrix.Color(DIMPINK));
        }
        matrix.drawPixel(jewelSevenX, jewelSevenY, matrix.Color(PINK));
        matrix.drawPixel((jewelSevenX - 1), jewelSevenY, matrix.Color(DIMPINK));
        matrix.drawPixel(xCursorPos, yCursorPosPrev, matrix.Color(OFF));
        matrix.drawPixel(xCursorPos, yCursorPos, matrix.Color(WHITE));
        matrix.show();

      }
      if (currentMillis - previousMillis7 >= interval7) //checks after 200ms
      { //check if the  current time - the last time is greater the the delay time you want
        previousMillis7 = currentMillis;
        if (currentMillis > interval7) //start fifth jewel after 200 ms
        {
          jewelSevenXPrev = jewelSevenX;
          jewelSevenX--;
          matrix.drawPixel(jewelSevenXPrev, jewelSevenY, matrix.Color(OFF));
        }

        if ((jewelSevenX == 1) && (yCursorPos == jewelSevenY))
        {
          score = score + 4;
        }


        if (jewelSevenX < 0)
        {
          matrix.drawPixel((jewelSevenXPrev + 1), jewelSevenY, matrix.Color(OFF));
          jewelSevenY = random(0, 9);
          jewelSevenX = 46;
        }
        //draw jewel four
        matrix.drawPixel((jewelSevenXPrev + 1), jewelSevenY, matrix.Color(OFF));
        if (jewelSevenX < 46)
        {
          matrix.drawPixel((jewelSevenXPrev), jewelSevenY, matrix.Color(GGLIGHTPINK));
        }
        matrix.drawPixel(jewelSevenX, jewelSevenY, matrix.Color(GGLIGHTPINK));
        matrix.drawPixel((jewelSevenX - 1), jewelSevenY, matrix.Color(DIMGGLIGHTPINK));
        matrix.drawPixel(xCursorPos, yCursorPosPrev, matrix.Color(OFF));
        matrix.drawPixel(xCursorPos, yCursorPos, matrix.Color(WHITE));
        matrix.show();

      }
      if (currentMillis - previousMillis8 >= interval8) //checks after 200ms
      { //check if the  current time - the last time is greater the the delay time you want
        previousMillis8 = currentMillis;
        if (currentMillis > interval8) //start fifth jewel after 200 ms
        {
          jewelEightXPrev = jewelEightX;
          jewelEightX--;
          matrix.drawPixel(jewelEightXPrev, jewelEightY, matrix.Color(OFF));
        }

        if ((jewelEightX == 1) && (yCursorPos == jewelEightY))
        {
          score = score + 1;
        }


        if (jewelEightX < 0)
        {
          matrix.drawPixel((jewelEightXPrev + 1), jewelEightY, matrix.Color(OFF));
          jewelEightY = random(0, 9);
          jewelEightX = 46;
        }
        //draw jewel four
        matrix.drawPixel((jewelEightXPrev + 1), jewelEightY, matrix.Color(OFF));
        if (jewelEightX < 46)
        {
          matrix.drawPixel((jewelEightXPrev), jewelEightY, matrix.Color(DIMGGBLUEPURPTWO));
        }
        matrix.drawPixel(jewelEightX, jewelEightY, matrix.Color(GGBLUEPURPTWO));
        matrix.drawPixel((jewelEightX - 1), jewelEightY, matrix.Color(DIMGGBLUEPURPTWO));
        matrix.drawPixel(xCursorPos, yCursorPosPrev, matrix.Color(OFF));
        matrix.drawPixel(xCursorPos, yCursorPos, matrix.Color(WHITE));
        matrix.show();

      }

      if (currentMillis - previousMillisShoulder >= intervalShoulder)
      { //check if the  current time - the last time is greater the the delay time you want
        previousMillisShoulder = currentMillis;
        prevShoulderLEDPos = shoulderLEDPos;
        shoulderLEDPos++;
        shoulderBounds(); //reassign back to 0 if out of bounds
        if (scoreStageOne == 1) //FIRST STAGE
        {
          //draw the shoulder animation
          if (intervalShoulder == 400)
          {
            intervalShoulder = 300;
          }
          setShoulderColor(BLUE, TEAL);

        }
        else if (scoreStageTwo == 1) //SECOND STAGE
        {
          if (intervalShoulder == 300)
          {
            intervalShoulder = 200;
          }
          //draw the shoulder animation
          setShoulderColor(GGLIGHTPINK, GGBLUEPURP);
        }
        else if (scoreStageThree == 1) //THIRD STAGE
        {
          if (intervalShoulder == 200)
          {
            intervalShoulder = 150;
          }
          //draw the shoulder animation
          setShoulderColor(GGBLUEBELL, GGPURPLE);
        }
        else if ( scoreStageFour == 1)//LAST STAGE BEFORE WINNING
        {
          if (intervalShoulder == 150)
          {
            intervalShoulder = 100;
          }
          //draw the shoulder animation
          setShoulderColor(RED, GGPINK);
        }
        else if (gameEndFlag == 1) //YOU WIN THE GAME
        {
          if (intervalShoulder == 100)
          {
            intervalShoulder = 50;
          }
          //draw the shoulder animation
          strip1.setPixelColor(shoulderLEDPos - 6, RED);
          strip1.setPixelColor(shoulderLEDPos - 5, ORANGE);
          strip1.setPixelColor(shoulderLEDPos - 4, YELLOW);
          strip1.setPixelColor(shoulderLEDPos - 3, GREEN);
          strip1.setPixelColor(shoulderLEDPos - 2, BLUE);
          strip1.setPixelColor(shoulderLEDPos - 1, GGPURPLE);
          strip1.setPixelColor(shoulderLEDPos, RED);
          strip1.setPixelColor(shoulderLEDPos + 1, ORANGE);
          strip1.setPixelColor(shoulderLEDPos + 2, YELLOW);
          strip1.setPixelColor(shoulderLEDPos + 3, GREEN);
          strip1.setPixelColor(shoulderLEDPos + 4, BLUE);
          strip1.setPixelColor(shoulderLEDPos + 5, GGPURPLE);
          strip1.setPixelColor(shoulderLEDPos + 6, RED);
        }

        strip1.show();

      }
      // matrix.show();
      //if you use matrix show here there's more of a delay because it's running thru other code before it hits
      //Serial.println(score); //this is the only way to see the score at the moment

    }
    else if (gameEndFlag == 1)
    {
      matrixClear();

      if (currentMillis - previousMillisShoulder >= intervalShoulder)
      { //check if the  current time - the last time is greater the the delay time you want
        previousMillisShoulder = currentMillis;
        prevShoulderLEDPos = shoulderLEDPos;
        shoulderLEDPos++;
        shoulderBounds(); //reassign back to 0 if out of bounds
        if (scoreStageOne == 1) //FIRST STAGE
        {
          //draw the shoulder animation
          if (intervalShoulder == 400)
          {
            intervalShoulder = 300;
          }
          setShoulderColor(BLUE, TEAL);

        }
        else if (scoreStageTwo == 1) //SECOND STAGE
        {
          if (intervalShoulder == 300)
          {
            intervalShoulder = 200;
          }
          //draw the shoulder animation
          setShoulderColor(GGLIGHTPINK, GGBLUEPURP);
        }
        else if (scoreStageThree == 1) //THIRD STAGE
        {
          if (intervalShoulder == 200)
          {
            intervalShoulder = 150;
          }
          //draw the shoulder animation
          setShoulderColor(GGBLUEBELL, GGPURPLE);
        }
        else if ( scoreStageFour == 1)//LAST STAGE BEFORE WINNING
        {
          if (intervalShoulder == 150)
          {
            intervalShoulder = 100;
          }
          //draw the shoulder animation
          setShoulderColor(RED, GGPINK);
        }
        else if (gameEndFlag == 1) //YOU WIN THE GAME
        {
          if (intervalShoulder == 100)
          {
            intervalShoulder = 50;
          }
          //draw the shoulder animation


          strip1.setPixelColor(shoulderLEDPos - 6, RED);
          strip1.setPixelColor(shoulderLEDPos - 5, ORANGE);
          strip1.setPixelColor(shoulderLEDPos - 4, YELLOW);
          strip1.setPixelColor(shoulderLEDPos - 3, GREEN);
          strip1.setPixelColor(shoulderLEDPos - 2, BLUE);
          strip1.setPixelColor(shoulderLEDPos - 1, GGPURPLE);
          strip1.setPixelColor(shoulderLEDPos, RED);
          strip1.setPixelColor(shoulderLEDPos + 1, ORANGE);
          strip1.setPixelColor(shoulderLEDPos + 2, YELLOW);
          strip1.setPixelColor(shoulderLEDPos + 3, GREEN);
          strip1.setPixelColor(shoulderLEDPos + 4, BLUE);
          strip1.setPixelColor(shoulderLEDPos + 5, GGPURPLE);
          strip1.setPixelColor(shoulderLEDPos + 6, RED);
        }

        strip1.show();

      }
    }

    Serial.println(score); //this is the only way to see the score at the moment
    if (score > 10 && score < 20)
    {
      scoreStageOne = 1;
    }
    else if (score > 25 && score < 50)
    {
      scoreStageOne = 0;
      scoreStageTwo = 1;
    }
    else if (score > 50 && score < 75)
    {
      scoreStageOne = 0;
      scoreStageTwo = 0;
      scoreStageThree = 1;
    }
    else if (score > 75 && score < 100)
    {
      scoreStageOne = 0;
      scoreStageTwo = 0;
      scoreStageThree = 0;
      scoreStageFour  = 1;
    }
    else if (score > 100) //GAME END??
    {
      scoreStageOne = 0;
      scoreStageTwo = 0;
      scoreStageThree = 0;
      scoreStageFour  = 0;

      gameEndFlag = 1;
    }
  }

}

void matrixClear(void)
{
  matrix.fillScreen(0);
  matrix.show();

}

void xcursorBounds(void)
{
  if (yCursorPos > 8)
  {
    yCursorPos = 8;
  }
  else if (yCursorPos < 0)
  {
    yCursorPos = 0;
  }

}

void pbLeftInt ()
{
  yCursorPosPrev = yCursorPos;
  yCursorPos--;
  xcursorBounds(); //check to see if the basket is at the edge of the matrix
  matrix.drawPixel(xCursorPos, yCursorPosPrev, matrix.Color(OFF));
  // matrix.drawPixel(xCursorPos,yCursorPos,matrix.Color(WHITE));
  //  Serial.print("yCursorPos:\t");
  //  Serial.println(yCursorPos);

}

void pbRightInt ()
{
  yCursorPosPrev = yCursorPos;
  yCursorPos++;
  xcursorBounds(); //check to see if the basket is at the edge of the matrix
  matrix.drawPixel(xCursorPos, yCursorPosPrev, matrix.Color(OFF));
  //  matrix.drawPixel(xCursorPos,yCursorPos,matrix.Color(WHITE));
  //  Serial.print("yCursorPos:\t");
  //  Serial.println(yCursorPos);

}

void setShoulderColor(uint8_t r, uint8_t g, uint8_t b, uint8_t rAlt, uint8_t gAlt, uint8_t bAlt)
{
  strip1.setPixelColor(shoulderLEDPos - 6, r, g, b);
  strip1.setPixelColor(shoulderLEDPos - 5, rAlt, gAlt, bAlt);
  strip1.setPixelColor(shoulderLEDPos - 4, r, g, b);
  strip1.setPixelColor(shoulderLEDPos - 3, rAlt, gAlt, bAlt);
  strip1.setPixelColor(shoulderLEDPos - 2, r, g, b);
  strip1.setPixelColor(shoulderLEDPos - 1, rAlt, gAlt, bAlt);
  strip1.setPixelColor(shoulderLEDPos, r, g, b);
  strip1.setPixelColor(shoulderLEDPos + 2, r, g, b);
  strip1.setPixelColor(shoulderLEDPos + 1, rAlt, gAlt, bAlt);
  strip1.setPixelColor(shoulderLEDPos + 4, r, g, b);
  strip1.setPixelColor(shoulderLEDPos + 3, rAlt, gAlt, bAlt);
  strip1.setPixelColor(shoulderLEDPos + 6, r, g, b);
  strip1.setPixelColor(shoulderLEDPos + 5, rAlt, gAlt, bAlt);


}

//Theatre-style crawling lights.
void theaterChase(uint32_t c, uint8_t wait) {
  for (int j = 0; j < 10; j++) { //do 10 cycles of chasing
    for (int q = 0; q < 3; q++) {
      for (uint16_t i = 0; i < strip.numPixels(); i = i + 3) {
        strip.setPixelColor(i + q, c);  //turn every third pixel on
      }
      strip.show();

      delay(wait);

      for (uint16_t i = 0; i < strip.numPixels(); i = i + 3) {
        strip.setPixelColor(i + q, 0);      //turn every third pixel off
      }
    }
  }
}

void shoulderBounds(void)
{
  if (shoulderLEDPos > 6)
  {
    shoulderLEDPos = 0;
  }

}



