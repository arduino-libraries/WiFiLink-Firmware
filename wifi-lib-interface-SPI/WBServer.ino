
String newSSID_param;
String newPASSWORD_param;
IPAddress default_IP(192,168,240,1);

// extern "C" void system_set_os_print(uint8 onoff);    //TODO to test without
// extern "C" void ets_install_putc1(void* routine);

int ledState = LOW;             // ledState used to set the LED
long previousMillis = 0;        // will store last time LED was updated
long bl_interval = 100;

int tot;        //need to save the number of the networks scanned
//bool conflag = false;
String staticIP_param ;
String netmask_param;
String gateway_param;
String dhcp = "on";
//bool reboot = false;
//int c_status = WL_IDLE_STATUS;

//holds the current upload
File fsUploadFile;

//format bytes
String formatBytes(size_t bytes){
  if (bytes < 1024){
    return String(bytes)+"B";
  } else if(bytes < (1024 * 1024)){
    return String(bytes/1024.0)+"KB";
  } else if(bytes < (1024 * 1024 * 1024)){
    return String(bytes/1024.0/1024.0)+"MB";
  } else {
    return String(bytes/1024.0/1024.0/1024.0)+"GB";
  }
}

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
  String contentType = getContentType(path);
  String pathWithGz = path + ".gz";
  if(SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)){
    if(SPIFFS.exists(pathWithGz))
      path += ".gz";
    File file = SPIFFS.open(path, "r");
    size_t sent = server.streamFile(file, contentType);
    file.close();
    return true;
  }
  return false;
}

void handleFileUpload(){
  if(server.uri() != "/edit") return;
  HTTPUpload& upload = server.upload();
  if(upload.status == UPLOAD_FILE_START){
    String filename = upload.filename;
    if(!filename.startsWith("/")) filename = "/"+filename;
    fsUploadFile = SPIFFS.open(filename, "w");
    filename = String();
  } else if(upload.status == UPLOAD_FILE_WRITE){
    //DBG_OUTPUT_PORT.print("handleFileUpload Data: "); DBG_OUTPUT_PORT.println(upload.currentSize);
    if(fsUploadFile)
      fsUploadFile.write(upload.buf, upload.currentSize);
  } else if(upload.status == UPLOAD_FILE_END){
    if(fsUploadFile)
      fsUploadFile.close();
  }
}

void handleFileDelete(){
  if(server.args() == 0) return server.send(500, "text/plain", "BAD ARGS");
  String path = server.arg(0);
  if(path == "/")
    return server.send(500, "text/plain", "BAD PATH");
  if(!SPIFFS.exists(path))
    return server.send(404, "text/plain", "FileNotFound");
  SPIFFS.remove(path);
  server.send(200, "text/plain", "");
  path = String();
}

void handleFileCreate(){
  if(server.args() == 0)
    return server.send(500, "text/plain", "BAD ARGS");
  String path = server.arg(0);
  if(path == "/")
    return server.send(500, "text/plain", "BAD PATH");
  if(SPIFFS.exists(path))
    return server.send(500, "text/plain", "FILE EXISTS");
  File file = SPIFFS.open(path, "w");
  if(file)
    file.close();
  else
    return server.send(500, "text/plain", "CREATE FAILED");
  server.send(200, "text/plain", "");
  path = String();
}

void handleFileList() {
  if(!server.hasArg("dir")) {server.send(500, "text/plain", "BAD ARGS"); return;}

  String path = server.arg("dir");
  Dir dir = SPIFFS.openDir(path);
  path = String();

  String output = "[";
  while(dir.next()){
    File entry = dir.openFile("r");
    if (output != "[") output += ',';
    bool isDir = false;
    output += "{\"type\":\"";
    output += (isDir)?"dir":"file";
    output += "\",\"name\":\"";
    output += String(entry.name()).substring(1);
    output += "\"}";
    entry.close();
  }

  output += "]";
  server.send(200, "text/json", output);
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

void handleWBServer(){

  server.handleClient();
  wifiLed();
}

void wifiLed(){

  unsigned long currentMillis = millis();
  int wifi_status = WiFi.status();
  if ((WiFi.getMode() == 1 || WiFi.getMode() == 3) && wifi_status == WL_CONNECTED) {
    if (currentMillis - previousMillis > bl_interval) {
      previousMillis = currentMillis;
      if (ledState == LOW)
        ledState = HIGH;
      else
        ledState = LOW;
      digitalWrite(WIFI_LED, ledState);
    }
  }
  else if (WiFi.softAPgetStationNum() > 0 ) {   //LED on in AP mode
    digitalWrite(WIFI_LED, HIGH);
  }
  else //if (wifi_status !=WL_CONNECTED){
    digitalWrite(WIFI_LED, LOW);

}

void initMDNS(){
    MDNS.setInstanceName(HOSTNAME);
    MDNS.enableArduino(80, false);
}

void initWBServer(){
    SPIFFS.begin();
    {
    Dir dir = SPIFFS.openDir("/");
    while (dir.next()) {
      String fileName = dir.fileName();
      size_t fileSize = dir.fileSize();
    }
    }
    //initWebFS();
    pinMode(WIFI_LED, OUTPUT);
    digitalWrite(WIFI_LED, LOW);

   tot = WiFi.scanNetworks();
   //Serial1.println("3");
   //set default AP
   byte mac[6];
   WiFi.macAddress(mac);
   String tmpString = "Arduino-Primo-" +  String(mac[3], HEX) + String(mac[4], HEX) + String(mac[5], HEX);
   char softApssid[18];
   memset(softApssid,0,sizeof(softApssid));
   tmpString.toCharArray(softApssid, tmpString.length()+1);
   delay(1000);
   WiFi.softAP(softApssid);
   WiFi.softAPConfig(default_IP, default_IP, IPAddress(255, 255, 255, 0));   //set default ip for AP mode

    //Enable to start in AP+STA mode
   WiFi.mode(WIFI_AP_STA);
   WiFi.hostname(HOSTNAME);

   //Serial1.println("4");
    server.serveStatic("/fs", SPIFFS, "/");

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
      String newhostname = server.arg("name");// = request->getParam("name")->value();
      WiFi.hostname(newhostname);
      MDNS.begin(newhostname.c_str());
      server.send(200, "text/plain", newhostname);
    });

    server.on("/wifi/scan", []() {
      String scanResp = "";
      tot = WiFi.scanNetworks();

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
        newSSID_param = server.arg("essid"); //"DHLabs";// request->getParam("essid")->value();
        newPASSWORD_param = server.arg("passwd"); //"dhlabsrfid01";//request->getParam("passwd")->value();
        server.send(200, "text/plain", "1");
        // File f = SPIFFS.open("/wifi.txt", "w");
        // f.println(newSSID_param);
        // f.println(newPASSWORD_param);
        // f.close();
        const char* newSSID = newSSID_param.c_str();
        const char* newPASSWORD = newPASSWORD_param.c_str();
        ETS_SPI_INTR_DISABLE();
        WiFi.begin(newSSID,newPASSWORD);
        WiFi.hostname(WiFi.hostname()); //set hostname
        ETS_SPI_INTR_ENABLE();
    });


      server.on("/connstatus", []() {
        String ipadd = (WiFi.getMode() == 1 || WiFi.getMode() == 3) ? toStringIp(WiFi.localIP()) : toStringIp(WiFi.softAPIP());
        //request->send(200, "text/plain", String("{\"status\":\"connecting\"}"));
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
           //Serial.print( "." );
         }
         //Serial.print( "Connected" );
       }
     });

    //called when the url is not defined here
    //use it to load content from SPIFFS
    server.onNotFound([](){
      if(!handleFileRead(server.uri()))
        server.send(404, "text/plain", "FileNotFound");
    });

    server.begin();
    //Serial1.println("5");

}
