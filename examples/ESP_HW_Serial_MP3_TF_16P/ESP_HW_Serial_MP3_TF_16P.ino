/***************************************************************************************************/
/*
   This Sketch uses "Checksum" packet validation for the GD3200B (MH2024K-16SS) chipset.
   The GD3200B is the replacement for the YX5200
   The player model is MP3-TF-16P and underneath you will see the MH2024K-16SS chip

   This sketch assumes an ESP32 or ESP8266 with HW Serial as the connection to the MP3-TF-16P
   Power the Player with +5V. It's logic TX/RX is 3v3 so you can conndirect ESP TX2 to Player RX and ESP RX2 to Player TX

   There is no reason you can't use this for different DFplaers just change the "DFPLAYER_MP3_TF_16P" 
   in the mp3.begin() to a definition that suits your device.

   written by :             macca448
   source code:             https://github.com/macca448/DFPlayer
  
  ********************************************************************************************************
  ********************************************************************************************************
  
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

#include <DFPlayer.h>

#if defined (ESP32)
  #define MP_SERIAL Serial2            //ESP32 TX2 (GPIO17) / RX2 (GPIO16)
#elif defined (ESP8266)
  #define MP_SERIAL Serial1            //ESP8266 TX1 (GPIO2) ONLY  IE No Feedback from Player
#endif

#define MP3_SERIAL_SPEED    9600 //DFPlayer Mini suport only 9600-baud
#define MP3_SERIAL_TIMEOUT  350  //average DFPlayer response timeout 200msec..300msec for YX5200/AAxxxx chip & 350msec..500msec for GD3200B/MH2024K chip

uint32_t lastPeriod = 0;
bool print = false, menu = 0;

DFPlayer mp3;                    

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
  Serial.begin(115200);                                      //Serial Monitor Input and Feedback
  MP_SERIAL.begin(MP3_SERIAL_SPEED, SERIAL_8N1);             //ESP32 RX2(gpio16) TX2(gpio17)   ESP8266 TX1 ONLY (gpio2)

  mp3.begin(MP_SERIAL, MP3_SERIAL_TIMEOUT, DFPLAYER_MP3_TF_16P, false); //"DFPLAYER_MP3_TF_16P" see NOTE, false=no feedback from module after the command

  delay(1000);

  mp3.stop();        //if player was runing during ESP8266 reboot
  mp3.reset();       //reset all setting to default
  
  mp3.setSource(2);  //1=USB-Disk, 2=TF-Card, 3=Aux, 4=Sleep, 5=NOR Flash
  
  mp3.setEQ(0);      //0=Off, 1=Pop, 2=Rock, 3=Jazz, 4=Classic, 5=Bass
  mp3.setVolume(25); //0..30, module persists volume on power failure

  //mp3.setFeedback(true);               //enable=request feedback to return not only errors but also OK statuses

  printMenu(0);
  //mp3.wakeup(2);                      //exit standby mode & initialize sourse 1=USB-Disk, 2=TF-Card, 3=Aux, 5=NOR Flash
}

void printMenu(bool list){
  if(list){
    Serial.println("\n  FILE    MENU");
    Serial.println("  Send  = The Item");
    Serial.println("C || c  = Print Command List");
    Serial.println("    z1  = Root DIR File 001");        //file name "001 text.mp3"
    Serial.println("    z2  = Root DIR File 002");        //file name "002 text.mp3"
    Serial.println("    z3  = Root DIR File 003");        //file name "003 text.mp3"
    Serial.println("  f011  = DIR 01 File 001");          //directory 01 the file name "001 text.mp3"
    Serial.println("  f012  = DIR 01 File 002");          //directory 01 the file name "002 text.mp3"
    Serial.println("f01255  = DIR 01 File 255");          //directory 01 the file name "255 text.mp3"
    Serial.println("  f991  = DIR 99 File 001");          //directory 99 the file name "001 text.mp3"
    Serial.println("  f992  = DIR 99 File 002");          //directory 99 the file name "002 text.mp3"
    Serial.println(" f99255 = DIR 99 File 255");          //directory 01 the file name "255 text.mp3"
    Serial.println("    m1  = DIR MP3 File 0001");        //directory mp3 the nfile name "0001 text.mp3"
    Serial.println(" m1000  = DIR MP3 File 1000");        //directory mp3 the nfile name "1000 text.mp3"
    Serial.println(" m9999  = DIR MP3 File 9999");        //directory mp3 the nfile name "9999 text.mp3"
    Serial.println("    a1  = DIR ADVERT File 0001");     //directory advert the nfile name "0001 text.mp3"
    Serial.println(" a1000  = DIR ADVERT File 1000");     //directory advert the nfile name "1000 text.mp3"
    Serial.println(" a9999  = DIR ADVERT File 9999\n");   //directory advert the nfile name "9999 text.mp3"
  }else{
    Serial.println("\nCOMMAND   MENU");
    Serial.println("L || l  = Print File list");
    Serial.println("D || d  = Get current status");
    Serial.println("S || s  = Stop current file");
    Serial.println("P || p  = Pause curent play");
    Serial.println("R || r  = Resume curent play");
    Serial.println("Vxx vXX = Set Volume Level 01 - 30");
    Serial.println("     $  = Reset Device\n");
  }
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
    if(millis() - lastPeriod >= 1000){                  //This avoid's printing the menu twice
      printMenu(menu);
    }
  }
  if(Serial.available() > 0){
    String inString = Serial.readStringUntil('\n');
      if(inString.startsWith("L") || inString.startsWith("l") ){
          menu = 1;
      }else if(inString.startsWith("C") || inString.startsWith("c")){
          menu = 0;
      }else if(inString.startsWith("D") || inString.startsWith("d")){
        uint8_t i = mp3.getStatus();
        Serial.printf("Status: %u\n", i);
      }else if(inString.startsWith("S") || inString.startsWith("s")){
        mp3.stop();
        Serial.println("Current File Play Stopped");
      }else if(inString.startsWith("P") || inString.startsWith("p")){
        mp3.pause();
        Serial.println("Play Paused");
      }else if(inString.startsWith("R") || inString.startsWith("r")){
        mp3.resume();
        Serial.println("Play Resume");
      }else if(inString.startsWith("$")){
        mp3.reset();
        Serial.println("Device Reset");
      }else if(inString.startsWith("V") || inString.startsWith("v")){
        String sub = inString.substring(1);
        uint8_t vol = sub.toInt();
        mp3.setVolume(vol);
        Serial.printf("Volume Set to: %u\n", vol);
      }else if(inString.startsWith("z") || inString.startsWith("Z")){        
        String sub = inString.substring(1);
        uint8_t file = sub.toInt();
        mp3.playTrack(file);
        Serial.printf("Root DIR File#: %u\n", file);
      }else if(inString.startsWith("f") || inString.startsWith("F")){
        String sub1 = inString.substring(1, 3);
        String sub2 = inString.substring(3);
        uint8_t fdr = sub1.toInt();
        uint8_t file = sub2.toInt();
        mp3.playFolder(fdr, file);
        Serial.printf("Playing DIR: %u\tFile#: %u\n", fdr, file);
      }else if(inString.startsWith("m") || inString.startsWith("M")){
        String sub = inString.substring(1);
        uint16_t file = sub.toInt();
        mp3.playMP3Folder(file);
        Serial.printf("Folder MP3 File#: %u\n", file);
      }else if(inString.startsWith("a") || inString.startsWith("A")){
        String sub = inString.substring(1);
        uint16_t file = sub.toInt();
        mp3.playAdvertFolder(file);
        Serial.printf("Folder Advert File#: %u\n", file);
      }
      print = true;
      inString = "";
      lastPeriod = millis();
  }
}
