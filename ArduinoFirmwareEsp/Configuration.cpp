#include "Configuration.h"


bool Configuration::setParam(String param, String value){
  if(value.length()>1 && param.length()>1){
    String paramConfig = "";
    DynamicJsonBuffer jsonBuffer;
    File configFile = SPIFFS.open(CONFIGFILENAME,"r");
    if(configFile){     //read Config file
      while(configFile.available()){
        paramConfig = paramConfig + char(configFile.read());
      }
    }
    else{
      paramConfig = JSONEMPTY;
    }
    configFile.close();
    JsonObject& json = jsonBuffer.parseObject(paramConfig);
    //write param to config file
    configFile = SPIFFS.open(CONFIGFILENAME, "w");
    json[param] = value;
    json.printTo(configFile);
    configFile.close();
    return true;
  }
  else{
    return false;
  }
}

String Configuration::getParam(String param){
  if(param.length()>1){
    File configFile = SPIFFS.open(CONFIGFILENAME, "r");
    if (!configFile) {
      return EMPTY;
    }
    size_t size = configFile.size();
    std::unique_ptr<char[]> buf(new char[size]);
    configFile.readBytes(buf.get(), size);

    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.parseObject(buf.get());

    if (!json.success()) {
      return EMPTY;
    }
    configFile.close();
    return json[param];
  }
  else{
    return EMPTY;
  }
}

Configuration Config;
