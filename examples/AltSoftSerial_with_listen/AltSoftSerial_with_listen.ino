/*
  This sketch uses 4 LED's to give a binary reading of the file number playing (MAX 15 files)
  Pressing the button randomly selects the next file to be played
  Listen feature is used to watch for file end and do board reset
  Builtin LED is used as viswual ready status.
  Uses AltSoftSerial so pins are definde by library depending on your board
*/
#define DEBUG_PRINT true

#include <AltSoftSerial.h>
#include <DFPlayer.h>

const byte ledPins[]{11,12,15,4};
#define LED                 13
#define BTN                 7
#define MP3_SERIAL_SPEED    9600 //DFPlayer Mini suport only 9600-baud
#define MP3_SERIAL_TIMEOUT  300  //average DFPlayer response timeout 200msec..300msec

AltSoftSerial mp3Serial;
DFPlayer mp3;

byte fileNo = 1;
bool pressed = false, lastBtnState = 1;

void setLEDS(byte x){
  for(byte i = 0; i < 4; i++){
    digitalWrite(ledPins[i], bitRead(x, i));
  }
}

void doReset(void){
  //mp3.stop();         //if player was runing during ESP8266 reboot
  mp3.reset();          //reset all setting to default
  mp3.setSource(1);     //1=USB-Disk, 2=TF-Card, 3=Aux, 4=Sleep, 5=NOR Flash
  //mp3.setEQ(0);       //0=Off, 1=Pop, 2=Rock, 3=Jazz, 4=Classic, 5=Bass
  mp3.setVolume(25);    //0..30, module persists volume on power failure
  flushSS();
}

void flushSS(){
  while (mp3Serial.available()) {
    mp3Serial.read();
  }
  setLEDS(0);
  digitalWrite(LED, HIGH);
}

byte getRandomNo(){
  byte x = random(1, 16);
  return x;
}

void inst(void){
  Serial.println(F("Finished Playing File\n"));
  Serial.println(F("Press Button to Play Next Random Clip\n"));
}

void setup() {
  delay(200);
  if(DEBUG_PRINT)Serial.begin(115200);
  while (!Serial) ;  // wait for Arduino Serial Monitor
  pinMode(BTN, INPUT_PULLUP);
  pinMode(LED, OUTPUT);
  for(byte i = 0; i < 4; i++){
    pinMode(ledPins[i], OUTPUT);
    digitalWrite(ledPins[i], HIGH);
    delay(250);
    digitalWrite(ledPins[i], LOW);
  }
  mp3Serial.begin(MP3_SERIAL_SPEED);
  mp3.begin(mp3Serial, MP3_SERIAL_TIMEOUT, DFPLAYER_MP3_TF_16P, false, false); 
  delay(1000);
  doReset();
  randomSeed(analogRead(A0));
  if(DEBUG_PRINT){
    Serial.println(F("Serial Config done\nLoop now Running.........\n"));
    inst();
  }
}

void loop() {
  static uint32_t pressTime;
  volatile uint32_t now = millis();

  bool btnState = digitalRead(BTN);
  if(btnState != lastBtnState){
    pressed = true;
    pressTime = now;
  }

  if(now - pressTime >= 50){
    if(pressed){
      if(digitalRead(BTN) == LOW){
        fileNo = getRandomNo();
        mp3.playTrack(fileNo);
          if(DEBUG_PRINT){
            Serial.print(F("Playing random file No: "));
            Serial.println(fileNo);
          }
          digitalWrite(LED, LOW);
          setLEDS(fileNo);
          delay(300);
      }
      pressed = false;
    }
  }
  // Listen for a command byte while clip playing

  int newReply = mp3.listen();
    if(newReply > 0){
      if(newReply == 0x4C){
        if(DEBUG_PRINT)Serial.println(F("SD Card File End"));
      }else if(newReply == 0x4B){
        if(DEBUG_PRINT)Serial.println(F("USB File End"));
      }else{
        if(DEBUG_PRINT){
          Serial.print(F("New Reply: 0x"));Serial.println(newReply, HEX);
        }
      }
      doReset();
      if(DEBUG_PRINT)inst();
    }
  lastBtnState = btnState;
}