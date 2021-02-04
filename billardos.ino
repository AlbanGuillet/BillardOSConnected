#include <TimerOne.h>
#include <stdio.h>
#include <math.h>


// TCS230 or TCS3200 pins wiring to Arduino
//Color captors 1
#define S0 10
#define S1 9
#define S2 8
#define S3 17
#define sensorOut 6
#define sensorOut 15

#define buttonPin 34
#define buttonPinCalibrate 36

#define mvtPin 3

// LCD Screen params
#include <LiquidCrystal.h>
LiquidCrystal lcd(22, 24, 26, 28, 30, 32);

// Score Params
int currentPlayer = 0;
int currentBall;
int playerBoulesPleines = 0;
int playerBoulesRayees = 0;
bool firstBallIn = false;

int lastMovPinState = 0;

int player1Score = 0;
int player2Score = 0;
int mvtState = LOW;

//Buttons
int buttonState = 0;
int buttonStateCalibrate;
int lastButtonState = 0;
int buttonTime = 0;

// Color Structure
typedef struct {
  float r, g, b;
  String title;
} Color;

// Color Sensor Calibration structure
typedef struct {
  const float rmin, rmax, gmin, gmax, bmin, bmax;
  Color colors[9];
} SensorColorCalibration;

// Color Sensor Structure
typedef struct {
  const short int pin0, pin1, pin2, pin3, pinOut;
  SensorColorCalibration calibration;
} SensorColor;

SensorColor sensorColor1 = {
  9,
  10,
  11,
  12,
  13,
  {
    10, 100,
    10, 140,
    10, 120,
    {
      {28, 35, 25, "blanc"},
      {16, 55, 65, "jaune"},
      {75.7, 100, 79, "rouge"},
      {68, 136, 117, "rouge fonce"},
      {70, 90, 70, "vert"},
      {83.7, 109, 83.6, "noir"},
      {68.48, 95.5, 68.5, "bleu"},
      {92, 147, 107, "violet"},
      {80, 104.7, 80.3, "orange"}
    }
  }
};

bool calibration1 = false;

SensorColor sensorColor2 = {
  4,
  5,
  6,
  7,
  8,
  {
    10, 100,
    10, 140,
    10, 120,
    {
      {43.3, 55, 44.50, "blanc"},
      {68.50, 73.5, 68.50, "jaune"},
      {270, 450, 370, "rouge"},
      {87, 113, 87, "rouge fonce"},
      {214, 546, 470, "vert"},
      {805, 787, 619, "noir"},
      {1000, 805, 506, "bleu"},
      {85, 116, 85, "violet"},
      {360, 660, 530, "orange"}
    }
  }
};

bool calibration = false;

void setup() {
  currentPlayer = 1;

  pinMode(buttonPin, INPUT);
  pinMode(buttonPinCalibrate, INPUT);
  pinMode(mvtPin, INPUT);

  lcd.begin(16, 2);
  
  // Setting the outputs Sensor1
  pinMode(sensorColor1.pin0, OUTPUT);
  pinMode(sensorColor1.pin1, OUTPUT);
  pinMode(sensorColor1.pin2, OUTPUT);
  pinMode(sensorColor1.pin3, OUTPUT);
  // Setting the outputs Sensor2
  pinMode(sensorColor2.pin0, OUTPUT);
  pinMode(sensorColor2.pin1, OUTPUT);
  pinMode(sensorColor2.pin2, OUTPUT);
  pinMode(sensorColor2.pin3, OUTPUT);

  // Setting the input Sensor 1
  pinMode(sensorColor1.pinOut, INPUT);
  // Setting the input Sensor 2
  pinMode(sensorColor2.pinOut, INPUT);

  // Setting frequency scaling to 20% Sensor 1
  digitalWrite(sensorColor1.pin0, HIGH);
  digitalWrite(sensorColor1.pin1, LOW);

  // Setting frequency scaling to 20% Sensor 2
  digitalWrite(sensorColor2.pin0, HIGH);
  digitalWrite(sensorColor2.pin1, LOW);

  // Begins serial communication
  Serial.begin(9600);

  // LCD init
  lcd.begin(16, 2);
  lcd.setCursor(0, 1);

  // Calibration display
  if (calibration) {
    lcd.print("init");
    lcd.setCursor(0, 0);
    lcd.print("press to callibrate");
    lcd.setCursor(0, 1);
    lcd.print(sensorColor1.calibration.colors[0].title);
    delay(3000);
  }

  // Score display
  lcd.clear();
  String score1String = String(player1Score);
  String score2String = String(player2Score);
  lcd.setCursor(0, 0);
  lcd.print("Player 1 : " + score1String);
  lcd.setCursor(0, 1);
  lcd.print("Player 2 : " + score2String);
}


void loop() {

  // Read buttons state
  buttonState = digitalRead(buttonPin);
  mvtState = digitalRead(mvtPin);
  
  if (mvtState == HIGH && lastMovPinState != mvtState) {
    // Caption Sensor activated
    Serial.println("detected!");
    delay(5000);

    // Color mean init
    Color mean_color1 = {0, 0, 0, ""};
    Color mean_color2 = {0, 0, 0, ""};
    int n = 50;
    for (int i = 0; i < n; i++)
    {
      // Color caption
      Color color1 = get_current_color(sensorColor1);
      Color color2 = get_current_color(sensorColor2);

      // Mean building
      mean_color1.r += color1.r;
      mean_color1.g += color1.g;
      mean_color1.b += color1.b;

      mean_color2.r += color2.r;
      mean_color2.g += color2.g;
      mean_color2.b += color2.b;

      delay(10);
    }

    // Mean compute
    mean_color1.r /= n;
    mean_color1.g /= n;
    mean_color1.b /= n;

    mean_color2.r /= n;
    mean_color2.g /= n;
    mean_color2.b /= n;

    // Print color infos
    String temp_print = "Color Frenquency : " + (String)mean_color1.r + ", " + (String)mean_color1.g + ", " + (String)mean_color1.b;
    Serial.println(temp_print);
    temp_print = "Color Frenquency : " + (String)mean_color2.r + ", " + (String)mean_color2.g + ", " + (String)mean_color2.b;
    Serial.println(temp_print);

    // Find nearest color
    Color color1 = nearestColor(sensorColor1.calibration.colors, mean_color1);
    Color color2 = nearestColor(sensorColor2.calibration.colors, mean_color2);
    
    // print infos on LCD
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("1:" + color1.title);
    Serial.println("1:" + color1.title);
    lcd.setCursor(0, 0);
    lcd.print("1:" + color2.title);
    Serial.println("2:" + color2.title);

    // Compute Score
    Color colorFromSensors[2] = {color1, color2};
    currentBall = getBallNumber(colorFromSensors);

    scoreWithBall();

  }

  // Change player manually
  if (buttonState != lastButtonState) {
    if (buttonState == HIGH) {
      swapPlayer(currentPlayer);
      buttonTime = 0;
    }
  }

  // Calibrate sensors (Do not use, better version on an other file
  buttonStateCalibrate = digitalRead(buttonPinCalibrate);

  if (buttonStateCalibrate == HIGH) {
    for (int i = 0; i < sizeof(sensorColor1.calibration.colors) / sizeof(sensorColor1.calibration.colors[0]) && calibration;) {
      delay(1000);
      if ( buttonState != 1 && digitalRead(buttonPin) == HIGH) {
        Serial.println("Calibration " + (String)(i + 1));
        Serial.println(sensorColor1.calibration.colors[i].title);
        Color color = get_current_color(sensorColor1);
        sensorColor1.calibration.colors[i].r = color.r;
        sensorColor1.calibration.colors[i].g = color.g;
        sensorColor1.calibration.colors[i].b = color.b;

        i++;

        if (i == sizeof(sensorColor1.calibration.colors) / sizeof(sensorColor1.calibration.colors[0])) {
          calibration = false;
          Serial.print("no calibration");
          break;
        }

        lcd.clear();
        lcd.setCursor(0, 1);
        lcd.print(sensorColor1.calibration.colors[i].title);
        lcd.setCursor(0, 0);
        lcd.print("press to callibrate");
      }
      buttonState = digitalRead(buttonPin);
    }
  }


  // update button
  lastButtonState = buttonState;
  lastMovPinState = mvtState;
}

Color get_current_color(SensorColor sensorColor)
{
  // Setting RED (R) filtered photodiodes to be read
  digitalWrite(sensorColor.pin1, LOW);
  digitalWrite(sensorColor.pin3, LOW);

  // Reading the output frequency
  float redFrequency = pulseIn(sensorColor.pinOut, LOW);
  float redColor = map(redFrequency, sensorColor.calibration.rmin, sensorColor.calibration.rmax, 255, 0);


  // Setting GREEN (G) filtered photodiodes to be read
  digitalWrite(sensorColor.pin2, HIGH);
  digitalWrite(sensorColor.pin3, HIGH);

  // Reading the output frequency
  float greenFrequency = pulseIn(sensorColor.pinOut, LOW);
  float greenColor = map(greenFrequency, sensorColor.calibration.gmin, sensorColor.calibration.gmax, 255, 0);

  // Printing the GREEN (G) value

  // Setting BLUE (B) filtered photodiodes to be read
  digitalWrite(sensorColor.pin2, LOW);
  digitalWrite(sensorColor.pin3, HIGH);

  // Reading the output frequency
  float blueFrequency = pulseIn(sensorColor.pinOut, LOW);
  float blueColor = map(blueFrequency, sensorColor.calibration.bmin, sensorColor.calibration.bmax, 255, 0);

//  String temp_print = "Color Color : " + (String)redColor + ", " + (String)greenColor + ", " + (String)blueColor;
//  Serial.println(temp_print);
//  temp_print = "Color Frenquency : " + (String)redFrequency + ", " + (String)greenFrequency + ", " + (String)blueFrequency;
//  Serial.println(temp_print);

  //  return {redColor,greenColor,blueColor,""};
  return {redFrequency, greenFrequency, blueFrequency, ""};
}

double ColourDistance(Color e1, Color e2)
{
  //  long rmean = ( (long)e1.r + (long)e2.r ) / 2;
  long r = (long)e1.r - (long)e2.r;
  long g = (long)e1.g - (long)e2.g;
  long b = (long)e1.b - (long)e2.b;
  //  return sqrt((((512+rmean)*r*r)>>8) + 4*g*g + (((767-rmean)*b*b)>>8));
  return sqrt(r * r + g * g + b * b);
}


Color nearestColor(Color colors[], Color currentColor) {
  Color n_Color = colors[0];
  double n_Color_values = ColourDistance(currentColor, colors[0]);
  for (int i = 1; i < 9; i++) {
    double i_Color_values = ColourDistance(currentColor, colors[i]);
    if (n_Color_values > i_Color_values)
    {
      n_Color_values = i_Color_values;
      n_Color = colors[i];
//      Serial.println(i);
//      Serial.println(n_Color_values);
//      Serial.println(n_Color.title);
    }
  }
  return n_Color;
}

int getBallNumber(Color sensors[]) {
  float isWhite = valueinarray("white", sensors);
  if (!isWhite && valueinarray("jaune", sensors)) {
    return 1;
  }
  else if (isWhite && valueinarray("jaune", sensors)) {
    return 9;
  }
  else if (!isWhite && valueinarray("bleu", sensors)) {
    return 2;
  }
  else if (isWhite && valueinarray("bleu", sensors)) {
    return 10;
  }
  else if (!isWhite && valueinarray("rouge", sensors)) {
    return 3;
  }
  else if (isWhite && valueinarray("rouge", sensors)) {
    return 11;
  }
  else if (!isWhite && valueinarray("violet", sensors)) {
    return 4;
  }
  else if (isWhite && valueinarray("violet", sensors)) {
    return 12;
  }
  else if (!isWhite && valueinarray("orange", sensors)) {
    return 5;
  }
  else if (isWhite && valueinarray("orange", sensors)) {
    return 13;
  }
  else if (!isWhite && valueinarray("vert", sensors)) {
    return 6;
  }
  else if (isWhite && valueinarray("vert", sensors)) {
    return 14;
  }
  else if (!isWhite && valueinarray("rouge fonce", sensors)) {
    return 7;
  }
  else if (isWhite && valueinarray("rouge fonce", sensors)) {
    return 15;
  }
  else if (valueinarray("black", sensors)) {
    return 8;
  }
  else if (isWhite) {
    return 0;
  }
}
void swapPlayer(int currentJoueur) {
  currentPlayer = currentJoueur == 1 ? 2 : 1;
}

void incrementScore(int player) {
  if (player == 1) {
    player1Score++;
  } else if (player == 2) {
    player2Score++;
  }

  String score1String = String(player1Score);
  String score2String = String(player2Score);
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("Player 1 : " + score1String);
  lcd.setCursor(0, 1);
  lcd.print("Player 2 : " + score2String);
}

void reset() {
  player1Score = 0;
  player2Score = 0;

  String score1String = String(player1Score);
  String score2String = String(player2Score);
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("Player 1 : " + score1String);
  lcd.setCursor(0, 1);
  lcd.print("Player 2 : " + score2String);
}

void scoreWithBall() {

  Serial.print(currentBall);

  if (!firstBallIn) {
    if (currentBall < 8) {
      playerBoulesPleines = currentPlayer;
      playerBoulesRayees = currentPlayer == 1 ? 2 : 1;
    } else {
      playerBoulesRayees = currentPlayer;
      playerBoulesPleines = currentPlayer == 1 ? 2 : 1;
    }
    firstBallIn = true;
  }

  if (currentBall < 8 && currentBall > 0 ) {
    incrementScore(playerBoulesPleines);
  } else if (currentBall > 8) {
    incrementScore(playerBoulesRayees);
  }

  if (currentBall == 8) {
    lcd.clear();
    if (currentPlayer == 1) {
      if (player1Score >= 7) {
        lcd.setCursor(0, 0);
        lcd.print("Victoire Player 1");
      } else if (player1Score != 7) {
        lcd.setCursor(0, 0);
        lcd.print("Victoire Player 2");
      }
    } else if (currentPlayer == 2) {
      if (player2Score >= 7) {
        lcd.setCursor(0, 0);
        lcd.print("Victoire Player 2");
      } else if (player2Score != 7) {
        lcd.setCursor(0, 0);
        lcd.print("Victoire Player 1");
      }
    }
    delay(1000);
    reset();
  }

  if (currentBall == 0) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Faute !");
    delay(1000);
    lcd.begin(16, 2);
    lcd.setCursor(0, 0);
    String score1String = String(player1Score);
    String score2String = String(player2Score);
    lcd.print("Player 1 : " + score1String);
    lcd.setCursor(0, 1);
    lcd.print("Player 2 : " + score2String);
  }
}

int valueinarray(String val, Color sensors[])
{
  int i;
  for (i = 0; i < 3; i++)
  {
    if (sensors[i].title == val)
      return 1;
  }
  return 0;
}
