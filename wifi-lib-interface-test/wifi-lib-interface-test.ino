enum{ 
  RSSI=0, 
  SSID, 
  RSSI_idx,
  ECRY,
  ECRY_idx,
  MAC_ADDR,
  DISCONNECT,
  STATUS
};

unsigned long _startMillis;
unsigned long _timeout=1000;

int timedRead()
{
  int c;
  _startMillis = millis();
  do {
    c = Serial1.read();
    if (c >= 0) return c;
  } while(millis() - _startMillis < _timeout);
  return -1;     // -1 indicates timeout
}

String readStringUntil(char terminator)
{
  String ret;
  int c = timedRead();

  while (c >= 0 && (char)c != terminator)
  {
    ret += (char)c;
    c = timedRead();
  }
  return ret;
}

void command(int cmd){
    switch(cmd){
      
    case 0:   //current RSSI
       Serial1.write(0xE0);
       Serial1.write(0x25);
       Serial1.write(1);
       Serial1.write(1);
       Serial1.write(0xFF);
       Serial1.write(0xEE);
       break;
    case 1:   //current SSID
       Serial1.write(0xE0);
       Serial1.write(0x23);
       Serial1.write(1);
       Serial1.write(1);
       Serial1.write(0xFF);
       Serial1.write(0xEE);
       break;
    case 2:   //RSSI index
       Serial1.write(0xE0);
       Serial1.write(0x32);
       Serial1.write(1);
       Serial1.write(1);
       Serial1.write(0); //index value
       Serial1.write(0xEE);
       break;
     case 3:   //encryption current
       Serial1.write(0xE0);
       Serial1.write(0x26);
       Serial1.write(1);
       Serial1.write(1);
       Serial1.write(0xFF);
       Serial1.write(0xEE);
       break;
     case 4:   //Encryption index
       Serial1.write(0xE0);
       Serial1.write(0x33);
       Serial1.write(1);
       Serial1.write(1);
       Serial1.write(3); //index value
       Serial1.write(0xEE);
       break;
     case 5:   //get MacAddress
       Serial1.write(0xE0);
       Serial1.write(0x22);
       Serial1.write(1);
       Serial1.write(1);
       Serial1.write(0xFF);
       Serial1.write(0xEE);
       break;
     case 6:   //get MacAddress
       Serial1.write(0xE0);
       Serial1.write(0x30);
       Serial1.write(1);
       Serial1.write(1);
       Serial1.write(0xFF);
       Serial1.write(0xEE);
       break;
     case 7:   //get Status
       Serial1.write(0xE0);
       Serial1.write(0x20);
       Serial1.write(0);
       Serial1.write(0xFF);
       Serial1.write(0xEE);
       break;       
       }

  }
void setup() {
  // put your setup code here, to run once:
  Serial1.begin(9600);
  Serial.begin(9600);

}

void loop() {
       
       //command(ECRY_idx);
       //command(ECRY);
       //command(RSSI);
       //command(SSID);
       //command(RSSI_idx);
       //command(MAC_ADDR);
       //command(DISCONNECT);
       //command(STATUS);
       
       String prova = readStringUntil(0xEE);
       Serial.print("cmd: ");
       for(int x=0;x<prova.length();x++){
          Serial.print((uint8_t)prova[x],HEX);
       }
       //uint8_t status = (prova[prova.length()-1]);
       //Serial.print(status);
       Serial.println();


       delay(5000); 

}
