enum{ 
  RSSI=0, 
  SSID, 
  RSSI_idx,
  ECRY,
  ECRY_idx,
  MAC_ADDR,
  DISCONNECT,
  STATUS,
  BEGIN_SSID,
  BEGIN_SSID_PASS,
  START_SCAN_NET,
  SCAN_NET,
  BSSID  
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
     case 8: //begin with ssid
       Serial1.write(0xE0);
       Serial1.write(0x10);
       Serial1.write(1);
       Serial1.write(6);
         Serial1.write(0x44); 
         Serial1.write(0x48); 
         Serial1.write(0x4C); 
         Serial1.write(0x61); 
         Serial1.write(0x62); 
         Serial1.write(0x73);
       Serial1.write(0xEE);
       break;  
     case 9: //begin with ssid and password
       Serial1.write(0xE0);
       Serial1.write(0x11);
       Serial1.write(2);
       Serial1.write(6);
         Serial1.write(0x44); 
         Serial1.write(0x48); 
         Serial1.write(0x4C); 
         Serial1.write(0x61); 
         Serial1.write(0x62); 
         Serial1.write(0x73);
       Serial1.write(0x0C);
         Serial1.write(0x64); 
         Serial1.write(0x68); 
         Serial1.write(0x6C); 
         Serial1.write(0x61); 
         Serial1.write(0x62); 
         Serial1.write(0x73);
         Serial1.write(0x72); 
         Serial1.write(0x66); 
         Serial1.write(0x69); 
         Serial1.write(0x64); 
         Serial1.write(0x30); 
         Serial1.write(0x31);
       Serial1.write(0xEE);
       break;
      case 10: //START SCAN NET
         Serial1.write(0xE0);
         Serial1.write(0x36);
         Serial1.write(0);
         Serial1.write(0xEE);
        break;
      case 11: //SCAN NET
         Serial1.write(0xE0);
         Serial1.write(0x27);
         Serial1.write(0);
         Serial1.write(0xEE);
        break;
      case 12: //SCAN NET
         Serial1.write(0xE0);
         Serial1.write(0x24);
         Serial1.write(1);
         Serial1.write(1);
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

bool a = true;
void loop() {
       String prova;
       //command(ECRY_idx);
       //command(ECRY);
       //command(RSSI);
 
       //command(SSID);
       //command(RSSI_idx);
       //command(MAC_ADDR);
       //command(DISCONNECT);
       //command(STATUS);
       //command(BEGIN_SSID);
       //command(BEGIN_SSID_PASS);
       //command(BEGIN_SSID_PASS);
       //command(START_SCAN_NET);
       //command(SCAN_NET);
       command(BSSID);
       
       prova = readStringUntil(0xEE);
       Serial.print("cmd: ");
       for(int x=0;x<prova.length();x++){
          Serial.print((uint8_t)prova[x],HEX);
       }
       //uint8_t status = (prova[prova.length()-1]);
       //Serial.print(status);
       Serial.println();
      
       delay(5000); 

}
