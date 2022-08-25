#define RL 47  //The value of resistor RL is 47K
#define m -0.263 //Enter calculated Slope 
#define b 0.42 //Enter calculated interceptm
#define Ro 20 //Enter found Ro value
#define MQ_sensor 34 //Sensor is connected to A4

/*
  현재 발생한 오류 : 
  Serial2()함수가 정의되어 있지 않음
  내가 모르는 문제(?)가 다수 발생하고 있음..
*/

#include "DFRobotDFPlayerMini.h"
DFRobotDFPlayerMini myDFPlayer;
// Potentiometer is connected to GPIO 34 (Analog ADC1_CH6) 

// variable for storing the potentiometer value
int potValue = 0;

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600);
  
  if (!myDFPlayer.begin(Serial2)) {  //Use softwareSerial to communicate with mp3.
    Serial.println(F("Unable to begin:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
    while(true){
      delay(0); // Code to compatible with ESP8266 watch dog.
    }
  }
  myDFPlayer.volume(30);  //Set volume value. From 0 to 30
}

void loop() {
  boolean play_state = digitalRead(23);
  unsigned int Value = 0.0;
  float VRL; //Voltage drop across the MQ sensor
  float Rs; //Sensor resistance at gas concentration 
  float ratio; //Define variable for ratio
   
  VRL = analogRead(MQ_sensor)*(3.3/4095.0); //Measure the voltage drop and convert to 0-5V
  Rs = ((3.3*RL)/VRL)-RL; //Use formula to get Rs value
  ratio = Rs/Ro;  // find ratio Rs/Ro
 
  float ppm = pow(10, ((log10(ratio)-b)/m)); //use formula to calculate ppm
  Value = ppm;
  Serial.print(ppm);
  Serial.print(" ppm,");
  Serial.print(VRL);
  Serial.println(" v");

  if (VRL > 1) {
    if (play_state == HIGH) {
      myDFPlayer.play(1);
    }
  }
}