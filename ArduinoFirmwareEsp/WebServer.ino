// uint8_t MAC_array[6];   //mac_address
// char macAddress[12];    //mac_address
String newSSID_param;
String newPASSWORD_param;
const char* newSSID_par = "";
const char* newPASSWORD_par = "";
// IPAddress default_IP(192,168,240,1);  //defaul IP Address
bool SERVER_STOP = false;       //check stop server

// extern "C" void system_set_os_print(uint8 onoff);    //TODO to test without
// extern "C" void ets_install_putc1(void* routine);

int tot;        //need to save the number of the networks scanned
String staticIP_param ;
String netmask_param;
String gateway_param;
String dhcp = "on";
bool connect = false;
//String HOSTNAME = "arduino";


String getContentType(String filename){
  if(server.hasArg("download")) return "application/octet-stream";
  else if(filename.endsWith(".htm")) return "text/html";
  else if(filename.endsWith(".html")) return "text/html";
  else if(filename.endsWith(".css")) return "text/css";
  else if(filename.endsWith(".js")) return "application/javascript";
  else if(filename.endsWith(".png")) return "image/png";
  else if(filename.endsWith(".gif")) return "image/gif";
  else if(filename.endsWith(".jpg")) return "image/jpeg";
  else if(filename.endsWith(".ico")) return "image/x-icon";
  else if(filename.endsWith(".xml")) return "text/xml";
  else if(filename.endsWith(".pdf")) return "application/x-pdf";
  else if(filename.endsWith(".zip")) return "application/x-zip";
  else if(filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

bool handleFileRead(String path){
  if(path.endsWith("/")) path += "index.html";
  //Serial.println("13");
  String contentType = getContentType(path);
  //Serial.println("14");
  //String pathWithGz = path + ".gz";
  //Serial.println("15");
  //ETS_SPI_INTR_DISABLE();
  //if(SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)){
    //Serial.println("16");
    // if(SPIFFS.exists(pathWithGz))
    //   path += ".gz";
    //Serial.println("17");
    File file = SPIFFS.open(path, "r");
    //Serial.println("18");
    ETS_SPI_INTR_DISABLE();
    size_t sent = server.streamFile(file, contentType);
    ETS_SPI_INTR_ENABLE();
    //Serial.println("19");
    file.close();
    //Serial.println("20");
    return true;
  //}
  //return false;
  //ETS_SPI_INTR_ENABLE();
}

String toStringIp(IPAddress ip) {
  String res = "";
  for (int i = 0; i < 3; i++) {
    res += String((ip >> (8 * i)) & 0xFF) + ".";
  }
  res += String(((ip >> 8 * 3)) & 0xFF);
  return res;
}

String toStringWifiMode(int mod) {
  String mode;
  switch (mod) {
    case 0:
      mode = "OFF";
      break;
    case 1:
      mode = "STA";
      break;
    case 2:
      mode = "AP";
      break;
    case 3:
      mode = "AP+STA";
      break;
    case 4:
      mode = "----";
      break;
    default:
      break;
  }
  return mode;
}

WiFiMode intToWifiMode(int mod) {
  WiFiMode mode;
  switch (mod) {
    case 0:
      mode = WIFI_OFF;
      break;
    case 1:
      mode = WIFI_STA;
      break;
    case 2:
      mode = WIFI_AP;
      break;
    case 3:
      mode = WIFI_AP_STA;
      break;
    case 4:
      break;
    default:
      break;
  }
  return mode;
}

String toStringWifiStatus(int state) {
  String status;
  switch (state) {
    case 0:
      status = "connecting";
      break;
    case 1:
      status = "unknown status";
      break;
    case 2:
      status = "wifi scan completed";
      break;
    case 3:
      status = "got IP address";
      // statements
      break;
    case 4:
      status = "connection failed";
      break;
    default:
      break;
  }
  return status;
}

String toStringEncryptionType(int thisType) {
  String eType;
  switch (thisType) {
    case ENC_TYPE_WEP:
      eType = "WEP";
      break;
    case ENC_TYPE_TKIP:
      eType = "WPA";
      break;
    case ENC_TYPE_CCMP:
      eType = "WPA2";
      break;
    case ENC_TYPE_NONE:
      eType = "None";
      break;
    case ENC_TYPE_AUTO:
      eType = "Auto";
      break;
  }
  return eType;
}

IPAddress stringToIP(String address) {
  int p1 = address.indexOf('.'), p2 = address.indexOf('.', p1 + 1), p3 = address.indexOf('.', p2 + 1); //, 4p = address.indexOf(3p+1);
  String ip1 = address.substring(0, p1), ip2 = address.substring(p1 + 1, p2), ip3 = address.substring(p2 + 1, p3), ip4 = address.substring(p3 + 1);

  return IPAddress(ip1.toInt(), ip2.toInt(), ip3.toInt(), ip4.toInt());
}

void handleWebServer(){
  if(CommunicationLogic.UI_alert){			//stop UI SERVER
    if(!SERVER_STOP){
      server.stop();
      SERVER_STOP = true;
    }
  }
  else{
    server.handleClient();
  }
}

void initWebServer(){

  tot = WiFi.scanNetworks();

  //"wifi/info" information
  server.on("/wifi/info", []() {

    //ETS_SPI_INTR_DISABLE();
    String ipadd = (WiFi.getMode() == 1 || WiFi.getMode() == 3) ? toStringIp(WiFi.localIP()) : toStringIp(WiFi.softAPIP());
    String staticadd = dhcp.equals("on") ? "0.0.0.0" : staticIP_param;
    int change = WiFi.getMode() == 1 ? 3 : 1;
    String cur_ssid = (WiFi.getMode() == 2 )? "none" : WiFi.SSID();


    server.send(200, "text/plain", String("{\"ssid\":\"" + cur_ssid + "\",\"hostname\":\"" + WiFi.hostname() + "\",\"ip\":\"" + ipadd + "\",\"mode\":\"" + toStringWifiMode(WiFi.getMode()) + "\",\"chan\":\""
                                            + WiFi.channel() + "\",\"status\":\"" + toStringWifiStatus(WiFi.status()) + "\", \"gateway\":\"" + toStringIp(WiFi.gatewayIP()) + "\", \"netmask\":\"" + toStringIp(WiFi.subnetMask()) + "\",\"rssi\":\""
                                            + WiFi.RSSI() + "\",\"mac\":\"" + WiFi.macAddress() + "\",\"phy\":\"" + WiFi.getPhyMode() + "\", \"dhcp\": \"" + dhcp + "\", \"staticip\":\"" + staticadd +
                                            + "\", \"warn\": \"" + "<a href='#' class='pure-button button-primary button-larger-margin' onclick='changeWifiMode(" + change + ")'>Switch to " + toStringWifiMode(change) + " mode</a>\""
                                            + "}" ));
      //ETS_SPI_INTR_ENABLE();

    });

    //"system/info" information
    server.on("/system/info", []() {

            //ETS_SPI_INTR_DISABLE();
            server.send(200, "text/plain", String("{\"heap\":\""+ String(ESP.getFreeHeap()/1024)+" KB\",\"id\":\"" + String(ESP.getFlashChipId()) + "\",\"size\":\"" + (ESP.getFlashChipSize() / 1024 / 1024) + " MB\",\"baud\":\"9600\"}"));
            //ETS_SPI_INTR_ENABLE();

    });
    server.on("/heap", []() {

      server.send(200, "text/plain", String(ESP.getFreeHeap()));

    });

    server.on("/system/update", []() {

      String newhostname = server.arg("name");
      WiFi.hostname(newhostname);
      MDNS.begin(newhostname.c_str());
      MDNS.setInstanceName(newhostname);
      // Config.getParam("hostname");  //TODO remove
      // Config.getParam("ssid");
      // Config.getParam("password");
      server.send(200, "text/plain", newhostname);
      Config.setParam("hostname", newhostname);

    });

    server.on("/wifi/netNumber", []() {

        tot = WiFi.scanNetworks();
        server.send(200, "text/plain", String(tot));

    });

    server.on("/wifi/scan", []() {

      String scanResp = "";

      if (tot == 0) {
        server.send(200, "text/plain", "No networks found");
      }
      if (tot == -1 ) {
        server.send(500, "text/plain", "Error during scanning");
      }

      scanResp += "{\"result\": { \"APs\" : [ ";
      //ETS_SPI_INTR_DISABLE();
      for (int netIndex = 0; netIndex < tot; netIndex++) {
        scanResp += "{\"enc\" : \"";
        scanResp += toStringEncryptionType (WiFi.encryptionType(netIndex));
        scanResp += "\",";
        scanResp += "\"essid\":\"";
        scanResp += WiFi.SSID(netIndex);
        scanResp += "\",";
        scanResp += "\"rssi\" :\"";
        scanResp += WiFi.RSSI(netIndex);
        scanResp += "\"}";
        if (netIndex != tot - 1)
          scanResp += ",";
      }
      scanResp += "]}}";
      //ETS_SPI_INTR_ENABLE();
      server.send(200, "text/plain", scanResp);

    });

    server.on("/connect", []() {

      newSSID_param = server.arg("essid");
      newPASSWORD_param = server.arg("passwd");
      server.send(200, "text/plain", "1");
      WiFi.begin(newSSID_param.c_str(),newPASSWORD_param.c_str());
      //WiFi.softAPConfig(default_IP, default_IP, IPAddress(255, 255, 255, 0));   //set default ip for AP mode
      //WiFi.hostname(WiFi.hostname());
      Config.setParam("ssid", newSSID_param);
      Config.setParam("password", newPASSWORD_param);

    });

    server.on("/connstatus", []() {

        String ipadd = (WiFi.getMode() == 1 || WiFi.getMode() == 3) ? toStringIp(WiFi.localIP()) : toStringIp(WiFi.softAPIP());
        server.send(200, "text/plain", String("{\"url\":\"got IP address\", \"ip\":\""+ipadd+"\", \"modechange\":\"no\", \"ssid\":\""+WiFi.SSID()+"\", \"reason\":\"-\", \"status\":\""+ toStringWifiStatus(WiFi.status()) +"\"}"));

    });


    server.on("/setmode", []() {

      int newMode = server.arg("mode").toInt();

      switch (newMode){
        case 1 :
        case 3 :
          server.send(200, "text/plain", String("Mode changed " + toStringWifiMode(WiFi.getMode())));
          WiFi.mode(intToWifiMode(newMode));
          break;
        case 2 :
          server.send(200, "text/plain", String("Mode changed " + toStringWifiMode(WiFi.getMode())));
          WiFi.mode(WIFI_AP);
          break;
      }

    });

    server.on("/special", []() {

       dhcp = server.arg("dhcp");
       staticIP_param = server.arg("staticip");
       netmask_param = server.arg("netmask");
       gateway_param = server.arg("gateway");

       if (dhcp == "off") {
         server.send(200, "text/plain", String("{\"url\":\"" + staticIP_param + "\"}"));
         WiFi.config(stringToIP(staticIP_param), stringToIP(gateway_param), stringToIP(netmask_param));
       }
       else {
         server.send(200, "text/plain",  "1");

         ESP.restart();
         while ( WiFi.waitForConnectResult() != WL_CONNECTED ) {
           delay ( 5000 );
         }
       }

     });

     server.on("/boardInfo", []() {

        StaticJsonBuffer<200> jsonBuffer;
        JsonObject& boardInfo = jsonBuffer.createObject();
        String output = "";
        if (BOARDMODEL == "PRIMO"){
            boardInfo["name"] = "Primo";
            boardInfo["icon"] = "/img/logoPrimo.ico";
            boardInfo["logo"] = "/img/logoPrimo.png";
            boardInfo["link"] = "http://www.arduino.org/learning/getting-started/getting-started-with-arduino-primo";
        }
        else if (BOARDMODEL == "STAROTTO"){
            boardInfo["name"] = "Star Otto";
            boardInfo["icon"] = "/img/logoOtto.ico";
            boardInfo["logo"] = "/img/logoOtto.png";
            boardInfo["link"] = "http://www.arduino.org/learning/getting-started/getting-started-with-arduino-star-otto";
        }
        else if (BOARDMODEL =="UNOWIFIDEVED"){
            boardInfo["name"] = "Uno WiFi";
            boardInfo["icon"] = "/img/logoUnoWiFi.ico";
            boardInfo["logo"] = "/img/logoUnoWiFi.png";
            boardInfo["link"] = "http://www.arduino.org/learning/getting-started/getting-started-with-arduino-uno-wifi";
        }
        boardInfo.printTo(output);
        server.send(200, "text/json", output);

      });

    //called when the url is not defined here
    //use it to load content from SPIFFS
    server.onNotFound([](){

      if(!handleFileRead(server.uri()))
        server.send(404, "text/plain", "FileNotFound");

    });

    server.begin();

}
