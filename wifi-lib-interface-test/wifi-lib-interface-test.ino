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
	BSSID,
	CONFIG_1,
	CONFIG_2,
	CONFIG_3,

	START_SERVER,
	SERVER_STATUS,

	REQ_HOST,
	GET_HOST,

	GET_FW_VER
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
	return -1;		 // -1 indicates timeout
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
			 //command(BSSID);
			 //command(CONFIG_1); // send ip, gateway and subnet
			 //command(CONFIG_2); // send ip, gateway, subnet and primary dns
			 //command(CONFIG_3);	// send ip, gateway, subnet, primary dns and secondary dns
			 command(START_SERVER);
			 command(SERVER_STATUS);
			 //command(REQ_HOST);
			 //command(GET_HOST);
			 //command(GET_FW_VER);

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

void command(int cmd){
		switch(cmd){

		case 0:	 //current RSSI
			 Serial1.write(0xE0);
			 Serial1.write(0x25);
			 Serial1.write(1);
			 Serial1.write(1);
			 Serial1.write(0xFF);
			 Serial1.write(0xEE);
			 break;
		case 1:	 //current SSID
			 Serial1.write(0xE0);
			 Serial1.write(0x23);
			 Serial1.write(1);
			 Serial1.write(1);
			 Serial1.write(0xFF);
			 Serial1.write(0xEE);
			 break;
		case 2:	 //RSSI index
			 Serial1.write(0xE0);
			 Serial1.write(0x32);
			 Serial1.write(1);
			 Serial1.write(1);
			 Serial1.write(0); //index value
			 Serial1.write(0xEE);
			 break;
		 case 3:	 //encryption current
			 Serial1.write(0xE0);
			 Serial1.write(0x26);
			 Serial1.write(1);
			 Serial1.write(1);
			 Serial1.write(0xFF);
			 Serial1.write(0xEE);
			 break;
		 case 4:	 //Encryption index
			 Serial1.write(0xE0);
			 Serial1.write(0x33);
			 Serial1.write(1);
			 Serial1.write(1);
			 Serial1.write(3); //index value
			 Serial1.write(0xEE);
			 break;
		 case 5:	 //get MacAddress
			 Serial1.write(0xE0);
			 Serial1.write(0x22);
			 Serial1.write(1);
			 Serial1.write(1);
			 Serial1.write(0xFF);
			 Serial1.write(0xEE);
			 break;
		 case 6:	 //get MacAddress
			 Serial1.write(0xE0);
			 Serial1.write(0x30);
			 Serial1.write(1);
			 Serial1.write(1);
			 Serial1.write(0xFF);
			 Serial1.write(0xEE);
			 break;
		 case 7:	 //get Status
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
		 case BEGIN_SSID_PASS: //begin with ssid and password
			 Serial1.write(0xE0);
			 Serial1.write(0x11);
			 Serial1.write(2);
			 Serial1.write(6);
				 Serial1.write(0x44);Serial1.write(0x48);Serial1.write(0x4C);Serial1.write(0x61);Serial1.write(0x62);Serial1.write(0x73);
			 Serial1.write(0x0C);
				 Serial1.write(0x64);Serial1.write(0x68);Serial1.write(0x6C);Serial1.write(0x61);Serial1.write(0x62);Serial1.write(0x73);
				 Serial1.write(0x72);Serial1.write(0x66);Serial1.write(0x69);Serial1.write(0x64);Serial1.write(0x30);Serial1.write(0x31);
			 Serial1.write(0xEE);
			 break;
			case START_SCAN_NET: //START SCAN NET
				 Serial1.write(0xE0);
				 Serial1.write(0x36);
				 Serial1.write(0);
				 Serial1.write(0xEE);
				break;
			case SCAN_NET: //get the ssid of found networks
				 Serial1.write(0xE0);
				 Serial1.write(0x27);
				 Serial1.write(0);
				 Serial1.write(0xEE);
				break;
			case BSSID: //get the BSSID
				 Serial1.write(0xE0);
				 Serial1.write(0x24);
				 Serial1.write(1);
				 Serial1.write(1);
				 Serial1.write(0xFF);
				 Serial1.write(0xEE);
				break;
			 case CONFIG_1: //Set the WiFi local ip, gateway ip and subnet mask
				 Serial1.write(0xE0);
				 Serial1.write(0x14);
				 Serial1.write(3);
				 Serial1.write(4);
					 //192.168.60.182
					 Serial1.write(0xC0); Serial1.write(0xA8); Serial1.write(0x3C); Serial1.write(0xB6);
				 Serial1.write(4);
					 //192.168.60.1
					 Serial1.write(0xC0);Serial1.write(0xA8);Serial1.write(0x3C);Serial1.write(0x01);
				 Serial1.write(4);
					 //255.255.255.0
					 Serial1.write(0xFF);Serial1.write(0xFF);Serial1.write(0xFF);Serial1.write(0x00);
				 Serial1.write(0xEE);
				 break;
			 case CONFIG_2:	//Set the WiFi local ip, gateway ip, subnet mask and primary dns
				 Serial1.write(0xE0);
				 Serial1.write(0x14);
				 Serial1.write(4);
				 Serial1.write(4);
					 //192.168.60.182
					 Serial1.write(0xC0); Serial1.write(0xA8); Serial1.write(0x3C); Serial1.write(0xB6);
				 Serial1.write(4);
					 //192.168.60.1
					 Serial1.write(0xC0);Serial1.write(0xA8);Serial1.write(0x3C);Serial1.write(0x01);
				 Serial1.write(4);
					 //255.255.255.0
					 Serial1.write(0xFF);Serial1.write(0xFF);Serial1.write(0xFF);Serial1.write(0x00);
					 //192.168.60.1
					Serial1.write(4);
					Serial1.write(0xC0);Serial1.write(0xA8);Serial1.write(0x3C);Serial1.write(0x01);
				 Serial1.write(0xEE);
					break;
			 case CONFIG_3: //Set the WiFi local ip, gateway ip, subnet mask, primary dns and secondary dns
				 Serial1.write(0xE0);
				 Serial1.write(0x14);
				 Serial1.write(5);
				 Serial1.write(4);
					 //192.168.60.182
					 Serial1.write(0xC0); Serial1.write(0xA8); Serial1.write(0x3C); Serial1.write(0xB6);
				 Serial1.write(4);
					 //192.168.60.1
					 Serial1.write(0xC0);Serial1.write(0xA8);Serial1.write(0x3C);Serial1.write(0x01);
				 Serial1.write(4);
					 //255.255.255.0
					 Serial1.write(0xFF);Serial1.write(0xFF);Serial1.write(0xFF);Serial1.write(0x00);
					 //8.8.8.8
					Serial1.write(4);
					Serial1.write(0x08);Serial1.write(0x08);Serial1.write(0x08);Serial1.write(0x08);
					//8.8.4.4
					Serial1.write(4);
					Serial1.write(0x08);Serial1.write(0x08);Serial1.write(0x04);Serial1.write(0x04);
				 Serial1.write(0xEE);
				 break;
				case START_SERVER:	//START SERVER
					Serial1.write(0xE0);
					Serial1.write(0x28);
					Serial1.write(3);
					Serial1.write(2);
					Serial1.write(0x00); //00	- PORT
					Serial1.write(0x50); //80
					Serial1.write(1);
					Serial1.write(0x00); //00	- SOCKET
					Serial1.write(1);
					Serial1.write(0x00); //00	- PROT MODE
					Serial1.write(0xEE);
				break;
				case SERVER_STATUS:	//SERVER STATUS
					Serial1.write(0xE0);
					Serial1.write(0x29);
					Serial1.write(1);
					Serial1.write(1);
					Serial1.write(0x00);
					Serial1.write(0xEE);
				break;
				case REQ_HOST:	//Send a command to request the ip address of the specified host
					Serial1.write(0xE0);
					Serial1.write(0x34);
					Serial1.write(1);
					Serial1.write(0x0E);
						//www.
						Serial1.write(0x77);Serial1.write(0x77);Serial1.write(0x77);Serial1.write(0x2E);
						//google
						Serial1.write(0x67);Serial1.write(0x6F);Serial1.write(0x6F);Serial1.write(0x67);Serial1.write(0x6C);Serial1.write(0x65);
						//.com
						Serial1.write(0x2E);Serial1.write(0x63);Serial1.write(0x6F);Serial1.write(0x6D);
					Serial1.write(0xEE);
				break;
				case GET_HOST:	//send a command to get the response of REQ_HOST command. it gets the IP address of the host specified before
					Serial1.write(0xE0);
					Serial1.write(0x35);
					Serial1.write(0);
					Serial1.write(0xEE);
				break;
				case GET_FW_VER: //Get the firmware version on the ESP8266
					Serial1.write(0xE0);
					Serial1.write(0x37);
					Serial1.write(0);
					Serial1.write(0xEE);
				break;

		 }

	}
