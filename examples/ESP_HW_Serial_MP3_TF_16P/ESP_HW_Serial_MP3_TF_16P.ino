/***************************************************************************************************/
/*
   This Sketch uses "Checksum" packet validation for the MP3-TF-16P that uses the MH2024K-16SS chip.
   Assumes an ESP32 or ESP8266 with HW Serial2 as the connection to the MP3-TF-16P

   written by :             macca448
   source code:             https://github.com/macca448/DFPlayer

   Original Library Author: enjoyneering < https://github.com/enjoyneering >
  
   DFPlayer Mini features:
   - 3.2v..5.0v, typical 4.2v
   - 15mA without flash drive, typical 24mA
   - 24-bit DAC with 90dB output dynamic range and SNR over 85dB
   - micro SD-card, up to 32GB (FAT16, FAT32)
   - USB-Disk up to 32GB (FAT16, FAT32)
   - supports mp3 sampling rate 8KHz, 11.025KHz, 12KHz, 16KHz, 22.05KHz, 24KHz, 32KHz, 44.1KHz, 48KHz
   - supports up to 100 folders, each folder can be assigned to 001..255 songs
   - built-in 3W mono amplifier, NS8002 AB-Class with standby function
   - UART to communicate, 9600bps (parity:none, data bits:8, stop bits:1, flow control:none)

   NOTE:
   - if you hear a loud noise, add a 1K resistor in series with DFPlayer TX pin
   - move the jumper from right to left to automatically switch the amplifier to standby

   GNU GPL license, all text above must be included in any redistribution,
   see link for details  - https://www.gnu.org/licenses/licenses.html
*/
/***************************************************************************************************/

#include "DFPlayer.h"


#define MP3_SERIAL_SPEED    9600 //DFPlayer Mini suport only 9600-baud
#define MP3_SERIAL_TIMEOUT  350  //average DFPlayer response timeout 200msec..300msec for YX5200/AAxxxx chip & 350msec..500msec for GD3200B/MH2024K chip

uint32_t lastPeriod = 0;
bool print = true;

DFPlayer mp3;                    //connect DFPlayer RX-pin to ESP32 GPIO17(TX2) & DFPlayer TX-pin to ESP32 GPIO16(RX2)
                                 //connect DFPlayer RX-pin to ESP8266 GPIO15(TX2) & DFPlayer TX-pin to ESP8266 GPIO13(RX2)


/**************************************************************************/
/*
    setup()

    Main setup

    NOTE:
    - moduleType:
      - DFPLAYER_MINI:
        - DFPlayer Mini module
        - MP3-TF-16P module
        - FN-M16P module
        - YX5200 chip
        - YX5300 chip
        - JL AAxxxx chip
      - DFPLAYER_FN_X10P:
        - FN-M10P module
        - FN-S10P module
        - FN6100 chip
      - DFPLAYER_HW_247A:
        - HW-247A module
        - GD3200B chip
      - DFPLAYER_MP3_TF_16P
        - MP3-TF-16P
        - MH2024K-16SS chip
      - DFPLAYER_NO_CHECKSUM:
        - no checksum calculation (not recomended for MCU without external
          crystal oscillator)
*/
/**************************************************************************/
void setup()
{
  Serial.begin(115200);                       //Serial Monitor Input and Feedback
  Serial2.begin(MP3_SERIAL_SPEED);            //ESP32 RX2(gpio16) TX2(gpio17)   ESP8266 RX2(gpio13) TX2(gpio15)

  mp3.begin(Serial2, MP3_SERIAL_TIMEOUT, DFPLAYER_MP3_TF_16P, false); //"DFPLAYER_MP3_TF_16P" see NOTE, false=no feedback from module after the command

  delay(1000);

  mp3.stop();        //if player was runing during ESP8266 reboot
  mp3.reset();       //reset all setting to default
  
  mp3.setSource(2);  //1=USB-Disk, 2=TF-Card, 3=Aux, 4=Sleep, 5=NOR Flash
  
  mp3.setEQ(0);      //0=Off, 1=Pop, 2=Rock, 3=Jazz, 4=Classic, 5=Bass
  mp3.setVolume(25); //0..30, module persists volume on power failure

  //mp3.setFeedback(true);               //enable=request feedback to return not only errors but also OK statuses

  printMenu();
  //mp3.wakeup(2);                      //exit standby mode & initialize sourse 1=USB-Disk, 2=TF-Card, 3=Aux, 5=NOR Flash
}

void printMenu(void){
  Serial.println("\nMENU");
  Serial.println("Send = The Item");
  Serial.println("  D  = Get current status");
  Serial.println("  s  = Stop current file");
  Serial.println("  1  = Root DIR File 001");
  Serial.println("  2  = Root DIR File 002");
  Serial.println("  b  = DIR 01 File 001");
  Serial.println("  c  = DIR 01 File 002");
  Serial.println("  m  = DIR MP3 File 001");
  Serial.println("  M  = DIR MP3 File 1000");
  Serial.println("  a  = DIR ADVERT File 001");
  Serial.println("  A  = DIR ADVERT File 1000");
  Serial.println("  p  = Pause curent mp3");
  Serial.println("  P  = Play curent mp3");
  Serial.println("  v  = Set Volume to 30/30");
  Serial.println("  w  = Set Volume to 25/30");
  Serial.println("  x  = Set Volume to 20/30");
  Serial.println("  r  = Reset Device\n");
  print = false;
}

/**************************************************************************/
/*
    loop()

    Main loop
*/
 /**************************************************************************/
void loop(){ 
  if(print){
    if(millis() - lastPeriod >= 500){                  //This avoid's printing the menu twice
      printMenu();
    }
  }
  if(Serial.available() > 0){
    char inChar = Serial.read();
    print = true;
    lastPeriod = millis();
      if(inChar == '1'){
        mp3.playTrack(1);
        Serial.println("Root DIR File 001");
      }else if(inChar == '2'){
        mp3.playTrack(2);
        Serial.println("Jet Engine Shut-Down Playing");
      }else if(inChar == 'b'){
        mp3.playFolder(1, 1);
        Serial.println("Folder 01 File 001");
      }else if(inChar == 'c'){
        mp3.playFolder(1, 2);
        Serial.println("Folder 01 File 002");
      }else if(inChar == 'm'){
        mp3.playMP3Folder(1);
        Serial.println("Folder MP3 File 0001");
      }else if(inChar == 'M'){
        mp3.playMP3Folder(1000);
        Serial.println("Folder MP3 File 1000");
      }else if(inChar == 'a'){
        mp3.playAdvertFolder(1);
        Serial.println("Folder Advert File 0001");
      }else if(inChar == 'A'){
        mp3.playAdvertFolder(1000);
        Serial.println("Folder Advert File 1000");
      }else if(inChar == 'v'){
        mp3.setVolume(30);
        Serial.println("Volume Set to 30");
      }else if(inChar == 'w'){
        mp3.setVolume(25);
        Serial.println("Volume Set to 25");
      }else if(inChar == 'x'){
        mp3.setVolume(20);
        Serial.println("Volume Set to 20");
      }else if(inChar == 'r'){
        mp3.reset();
        Serial.println("Device Reset");
      }else if(inChar == 'P'){
        mp3.resume();
        Serial.println("Resume Play of Current File");
      }else if(inChar == 'p'){
        mp3.pause();
        Serial.println("Pause Play of Current File");
      }else if(inChar == 'D'){
        byte i = mp3.getStatus();
        Serial.printf("Status: %u\n", i);
      }else if(inChar == 's'){
        mp3.stop();
        Serial.println("Current File Play Stopped");
      }
  }
}
