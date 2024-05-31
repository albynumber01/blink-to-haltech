#include <mcp_can.h>  // library for MCP2515 ic
#include <SPI.h>      // library for SPI communication

long unsigned int rxId;  // storage for can data
unsigned char len = 0;   // storage for can data
unsigned char rxBuf[8];  // storage for can data

#define CAN0_INT 2  // Set INT to pin 2
#define CAN1_INT 3
MCP_CAN CAN0(10);   // set CS pin to 10
MCP_CAN CAN1(9); 

bool button1;
bool button2;
bool button3;
bool button4;
bool button5;
bool button6;
bool button7;
bool button8;
bool button9;
bool button10;
bool button11;
bool button12;

byte out1 = 4;
byte out2 = 5;
byte out3 = 6;
byte out4 = 7;

unsigned long KAinterval = 150;              // 50ms interval for keep alive frame
unsigned long ButtonInfoInterval = 30;      // 30ms interval for button info frame
unsigned long KAintervalMillis = 0;         // storage for millis counter
unsigned long ButtonInfoIntervalMillis = 0; // storage for millis counter

void setup() {
  Serial.begin(115200);
  Serial.println("Haltech 2x4 keypad ID B emulator");
  delay(50);

  //Haltech ECU

  if (CAN0.begin(MCP_ANY, CAN_1000KBPS, MCP_8MHZ) == CAN_OK)
    Serial.println("MCP2515 CAN0 Initialized Successfully!");
  else Serial.println("Error Initializing MCP2515 CAN0...");

  //BlinkMarine Keypad

  if (CAN1.begin(MCP_ANY, CAN_1000KBPS, MCP_8MHZ) == CAN_OK)
    Serial.println("MCP2515 CAN1 Initialized Successfully!");
  else Serial.println("Error Initializing MCP2515 CAN1...");

  CAN0.setMode(MCP_NORMAL);
  CAN1.setMode(MCP_NORMAL);

  pinMode(CAN0_INT, INPUT);
  digitalWrite(CAN0_INT, HIGH);

  pinMode(CAN1_INT, INPUT);
  digitalWrite(CAN1_INT, HIGH);

  pinMode(4, INPUT);
  digitalWrite(4, HIGH);

  KPstart();

  Serial.println("All OK");
}

void KPstart() {
  byte KeepAliveblink[2] = { 0x01, 0x15 };
  byte LEDShowoff[8] = { 0x2F, 0x14, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00 };  //setting off initial demo LED show
  byte Button1Red[8] = { 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };  //setting Button 1 red 
  byte IndicatorLEDBrightness[8] = { 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };  //setting indicator brightness level
  byte BacklightBrightness[8] = { 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; //setting brightness level
  byte Backlightcolor[8] = { 0x2F, 0x03, 0x20, 0x04, 0x07, 0x00, 0x00, 0x00 };  //setting backlight color white

  CAN1.sendMsgBuf(0x00, 0, 2, KeepAliveblink);
  delay(100);
  CAN1.sendMsgBuf(0x615, 0, 8, LEDShowoff);
  delay(100);
  CAN1.sendMsgBuf(0x215, 0, 8, Button1Red);
  delay(100);
  CAN1.sendMsgBuf(0x415, 0, 8, IndicatorLEDBrightness);
  delay(100);
  CAN1.sendMsgBuf(0x515, 0, 8, BacklightBrightness);
  delay(100);
  CAN1.sendMsgBuf(0x615, 0, 8, Backlightcolor);
}


void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - KAintervalMillis >= KAinterval) {
    KAintervalMillis = currentMillis;
    SendKeepAlive();
  }

  if (currentMillis - ButtonInfoIntervalMillis >= ButtonInfoInterval) {
    ButtonInfoIntervalMillis = currentMillis;
    SendButtonInfo();
  }

  if (!digitalRead(CAN0_INT)) {
    CAN0.readMsgBuf(&rxId, &len, rxBuf);
    canRead();
  }

  if (!digitalRead(CAN1_INT)) {
    CAN1.readMsgBuf(&rxId, &len, rxBuf);
    canRead1();
  }
}

void canRead1() {
  if (rxId == 0x715) {
    Serial.print("Received message on CAN1 with ID: ");
    Serial.print(rxId, HEX);
    Serial.print(", Data: ");
    for (int i = 0; i < len; i++) {
      Serial.print(rxBuf[i], HEX);
      Serial.print(" ");
    }
    Serial.println();
    KPstart();
  }

  if (rxId == 0x195) {
    button1 = bitRead(rxBuf[0], 0);
    button2 = bitRead(rxBuf[0], 1);
    button3 = bitRead(rxBuf[0], 2);
    button4 = bitRead(rxBuf[0], 3);
    button5 = bitRead(rxBuf[0], 4);
    button6 = bitRead(rxBuf[0], 5);
    button7 = bitRead(rxBuf[0], 6);
    button8 = bitRead(rxBuf[0], 7);
    button9 = bitRead(rxBuf[1], 0);
    button10 = bitRead(rxBuf[1], 1);
    button11 = bitRead(rxBuf[1], 2);
    button12 = bitRead(rxBuf[1], 3);

    digitalWrite(out1, button6);
    digitalWrite(out2, button10);
    digitalWrite(out3, button11);
    digitalWrite(out4, button12);

    byte ledstate[8];

    byte buttonState = rxBuf[0]; // Usare Byte 0 per determinare lo stato dei pulsanti premuti

    // Chiamare la funzione per inviare il messaggio dei LED blu
    SendBlueLEDMessage(buttonState);

    Serial.print("Sent ButtonInfo message on CAN1 with ID: ");
    Serial.print(0x215, HEX);
    Serial.print(", Data: ");
    for (int i = 0; i < 8; i++) {
      Serial.print(ledstate[i], HEX);
      Serial.print(" ");
    }
    Serial.println();
  }
}

void canRead() {
  // CAN Input from Haltech Keypad
  int b0;
  int b1;
  int b2;
  int b3;
  int b4;
  int b5;
  int b6;
  int b7;

  // Keypad Configuration Section

  if (rxId == 0X60B) {
    if ((rxBuf[0]) == 34) {
      b0 = 96;
      b1 = (rxBuf[1]);
      b2 = (rxBuf[2]);
      b3 = (rxBuf[3]);
      b4 = 0;
      b5 = 0;
      b6 = 0;
      b7 = 0;

    } else if ((rxBuf[0]) == 66) {
      b0 = 67;
      b1 = (rxBuf[1]);
      b2 = (rxBuf[2]);
      b3 = (rxBuf[3]);
      if ((b1 == 24) && (b2 == 16) && (b3 == 1)) {
        b4 = 7;
        b5 = 4;
        b6 = 0;
        b7 = 0;
      } else if ((b1 == 24) && (b2 == 16) && (b3 == 2)) {
        b4 = 75;
        b5 = 51;
        b6 = 0;
        b7 = 0;
      } else if ((b1 == 24) && (b2 == 16) && (b3 == 3)) {
        b4 = 1;
        b5 = 0;
        b6 = 0;
        b7 = 0;
      } else if ((b1 == 24) && (b2 == 16) && (b3 == 4)) {
        b4 = 207;
        b5 = 184;
        b6 = 25;
        b7 = 12;
      } else if ((b1 == 0) && (b2 == 24) && (b3 == 1)) {
        b4 = 139;
        b5 = 1;
        b6 = 0;
        b7 = 64;
      } else {
        b4 = 0;
        b5 = 0;
        b6 = 0;
        b7 = 0;
      }

    } else if (((rxBuf[0]) == 0) && ((rxBuf[7]) == 200)) {
      b0 = 128;
      b1 = 0;
      b2 = 0;
      b3 = 0;
      b4 = 1;
      b5 = 0;
      b6 = 4;
      b7 = 5;

    }
    byte txBuf[8] = { b0, b1, b2, b3, b4, b5, b6, b7 };

    CAN0.sendMsgBuf(0x58B, 0, 8, txBuf);
  }
}

void SendBlueLEDMessage(byte buttonState) {
  byte ledMessage[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  
  // Imposta lo stato dei LED blu in base allo stato dei pulsanti
  ledMessage[2] = buttonState;

  CAN1.sendMsgBuf(0x215, 0, 8, ledMessage); // Invia il messaggio CAN con l'ID 0x215
}

void SendButtonInfo() {
  byte ButtonInfo[3];                                 // declare an array for 2 bytes used for key pressed information
  bitWrite(ButtonInfo[0], 0, button1);                      // byte 0, bit 0, button 1 
  bitWrite(ButtonInfo[0], 1, button2);                      // byte 0, bit 0, button 2
  bitWrite(ButtonInfo[0], 2, button3);                      // byte 0, bit 0, button 3
  bitWrite(ButtonInfo[0], 3, button4);                      // byte 0, bit 0, button 4
  bitWrite(ButtonInfo[0], 4, button5);                      // byte 0, bit 0, button 5
  bitWrite(ButtonInfo[0], 5, button6);                      // byte 0, bit 0, button 6
  bitWrite(ButtonInfo[0], 6, button7);                      // byte 0, bit 0, button 7
  bitWrite(ButtonInfo[0], 7, button8);                      // byte 0, bit 0, button 8

  ButtonInfo[1] = 0;                                  // byte 2 filled with 0
  CAN0.sendMsgBuf(0x18B, 0, 2, ButtonInfo);           // send the 2 byte data buffer at adress 18D
}

void SendKeepAlive() {                                // send keep alive frame
  byte KeepAlive[1] = { 5 };                          // frame data is 0X05 for byte 0
  CAN0.sendMsgBuf(0x70B, 0, 1, KeepAlive);            // send the frame at 70D 
}