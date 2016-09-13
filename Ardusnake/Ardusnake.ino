#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Max72xxPanel.h>
//#include "pitches.h"

int pinCS = 10; // Attach CS to this pin, DIN to MOSI and CLK to SCK (cf http://arduino.cc/en/Reference/SPI )
int numberOfHorizontalDisplays = 2;
int numberOfVerticalDisplays = 1;

Max72xxPanel matrix = Max72xxPanel(pinCS, numberOfHorizontalDisplays, numberOfVerticalDisplays);

//Title
String title = "Ardusnake";
int wait = 40; // In milliseconds
int spacer = 1;
int width = 5 + spacer; // The font width is 5 pixels
int titlePos = 0;

//Input
int pinX = A0;
int pinY = A1;
int pinSound = 6;
int pinStart = 7;

//Game Control
int state = 0; //0-title
int sLength = 2;
int sDir = 0;
const int maxLength = 50;
int sPosX[maxLength];
int sPosY[maxLength];

int fruitX = -1;
int fruitY = -1;

int deadZone = 200;

void setup() {

  //setup matrix
  matrix.setIntensity(3); // Use a value between 0 and 15 for brightness
  // Adjust to your own needs
  matrix.setPosition(0, 0, 0); // The first display is at <0, 0>
  matrix.setPosition(1, 1, 0); // The second display is at <1, 0>
  
  matrix.setRotation(0, 2);    // The first display is position upside down
  matrix.setRotation(1, 2);
  
  //setup input/output
  pinMode(pinX, INPUT);
  pinMode(pinY, INPUT);
  pinMode(pinStart, INPUT);
  //pinMode(pinSound, OUTPUT);
  
  //uses static on A0 to create a random seed
  randomSeed(analogRead(5));
  
  tone(6, 2500, 16);
  delay(30);
  tone(6, 3750, 16);
  
  Serial.begin(9600);
}

void loop() {
  switch (state) {
    case 0: if (!digitalRead(pinStart)) doTitle(); 
      else { state = 1;
        tone(6, 1750, 8);
        delay(10);
        tone(6, 2250, 8); } break;
    case 1: beginGame(); break;
    case 2: updateGame(); break;
  }
}

void drawScreen() {
  //clear screen
  matrix.fillScreen(LOW);
  
  //draw snake
  for (int i = 0; i <= sLength; i++) {
    if (sPosX[i] != -1 && sPosY[i] != -1)
      matrix.drawPixel(sPosX[i], sPosY[i], HIGH);
  }
  
  //draw fruit
  if (fruitX != -1 && fruitY != -1)
    matrix.drawPixel(fruitX, fruitY, HIGH);
  
  //write to screen
  matrix.write();
}

void updateGame() {
  Serial.println(analogRead(pinX));
  Serial.println(analogRead(pinY));
  int px = analogRead(pinX);
  int py = analogRead(pinY);
  if (px < 512 - deadZone && sDir != 0)
    sDir = 2;
  else if (px > 512 + deadZone && sDir != 2)
    sDir = 0;
  else if (py < 512 - deadZone && sDir != 3)
    sDir = 1;
  else if (py > 512 + deadZone && sDir != 1)
    sDir = 3;
  
  for (int i = 1; i <= sLength; i++) {
    sPosX[i] = sPosX[i-1];
    sPosY[i] = sPosY[i-1];
  }
  switch (sDir) {
    case 0: sPosX[0] += 1; break;
    case 1: sPosY[0] -= 1; break;
    case 2: sPosX[0] -= 1; break;
    case 3: sPosY[0] += 1; break;
  }
  checkPos();
  drawScreen();
  delay(200);
}

void checkPos() {
  bool hit = false;
  if (sPosX[0] == fruitX && sPosY[0] == fruitY) {
    sLength += 1;
    sPosX[sLength] = 0;//sPosX[sLength-1];
    sPosY[sLength] = 0;//sPosY[sLength-1];
    spawnFruit();
  }
  for (int i = 1; i <= sLength; i++) {
    if (sPosX[0] == sPosX[i] && sPosY[0] == sPosY[i]) {
      hit = true;
    }
  }
  if (hit) {
    //end game
    state = 0;
  }
  else {
    //wrap screen
    if (sPosX[0] < 0)
      sPosX[0] = 15;
    if (sPosX[0] > 15)
      sPosX[0] = 0;
    if (sPosY[0] < 0)
      sPosY[0] = 7;
    if (sPosY[0] > 7)
      sPosY[0] = 0;
  }
}

void beginGame() {
  Serial.println("Starting");
  sLength = 2;
  initSnake();
  sPosX[0] = 3;
  sPosY[0] = 3;
  sPosX[1] = 2;
  sPosY[1] = 3;
  sPosX[2] = 1;
  sPosY[2] = 3;
  sDir = 0;
  
  spawnFruit();
  
  drawScreen();
  delay(200);
  state = 2;
}

void spawnFruit() {
  bool valid = false;
  bool hit = false;
  do {
    fruitX = random(0, 16);
    fruitY = random(0, 8);
    
    for (int i = 0; i <= sLength; i++) {
      if (fruitX == sPosX[i] && fruitY == sPosY[i])
        hit = true;
    }
    if (!hit)
      valid = true;
    else
      valid = false;
  } while (valid == false);
}

void initSnake() {
  Serial.println("Init");
  for (int i = 0; i <= 49; i++) {
    sPosX[i] = -1;
    sPosY[i] = -1;
  }
}

void doTitle() {
  //Loop through title message
  if (titlePos < width*title.length()+matrix.width()-1-spacer) {
    matrix.fillScreen(LOW);

    int letter = titlePos / width;
    int x = (matrix.width() - 1) - titlePos % width;
    int y = (matrix.height() - 8) / 2; // center the text vertically

    while ( x + width - spacer >= 0 && letter >= 0 ) {
      if ( letter < title.length() ) {
        matrix.drawChar(x, y, title[letter], HIGH, LOW, 1);
      }

      letter--;
      x -= width;
    }

    matrix.write(); // Send bitmap to display
    titlePos++;
    delay(wait);
  }
  else
    titlePos = 0;
}

