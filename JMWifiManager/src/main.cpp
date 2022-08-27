#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>

AsyncWebServer server(80);

#define AP_SSID "JMWiFiManager"
#define AP_PASS "12345678"
#define SSID_PATH "/WIFI/SSID"
#define PASS_PATH "/WIFI/PASS"

String ssid,pass;

void initSPIFFS(){
  if(!SPIFFS.begin(true)){
    Serial.println("SPIFFS掛載失敗!");
  }
  else{
    Serial.println("SPIFFS掛載成功!");
  }
}

String ReadSPIFFS(const char *path){

  File f = SPIFFS.open(path , "r");
  if(!f){
    Serial.println("文件開啟失敗!");
    return "";
  }
  String payload = f.readStringUntil('\n');
  Serial.printf("從%s中，讀取資料%s \n", path, payload);
  return payload;
}

void WriteSPIFFS(const char* path, const char *message){

  File f = SPIFFS.open(path , "w");
  if(!f){
    Serial.println("文件開啟失敗!");
  }
  if(!f.print(message)){
     Serial.println("寫入訊息失敗");
  }
  Serial.printf("寫入%s至%s中 \n", message, path);

}
void initAPModeESP(){
  WiFi.softAP(AP_SSID, AP_PASS);
  Serial.print("AP IP:");
  Serial.println(WiFi.softAPIP());
}

void initServerHandle(){
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS,"/wifimanager.html");
  });
  server.on("/", HTTP_POST, [](AsyncWebServerRequest *request){
    int post_count = request->params();
    for(int i=0;i<post_count;i++){
      AsyncWebParameter *p = request->getParam(i);
      if(p->isPost()){
        if(p->name() == "ssid"){
          String ssid_val = p->value();
          Serial.printf("從表單ssid獲取%s \n", ssid_val);
          WriteSPIFFS(SSID_PATH, ssid_val.c_str());
        }
        if(p->name() == "password"){
          String pass_val = p->value();
          Serial.printf("從表單password獲取%s \n", pass_val);
          WriteSPIFFS(PASS_PATH, pass_val.c_str());
        }

      }
    }
    request->send(404, "text/plain", "Done. ESP will restart");
    vTaskDelay(5000 / portTICK_PERIOD_MS);
    ESP.restart();
  });

  server.begin();
}

void setup() {
  
  int n = 0;
  Serial.begin(115200);
  initSPIFFS();
  ssid = ReadSPIFFS(SSID_PATH);
  pass = ReadSPIFFS(PASS_PATH);

  WiFi.setSleep(false);
  WiFi.begin(ssid.c_str(), pass.c_str());
  while(WiFi.status() != WL_CONNECTED && n<50){
    vTaskDelay(200 / portTICK_PERIOD_MS);
    n++;
  }
  if(WiFi.status() != WL_CONNECTED){
    Serial.println("WIFI已逾時，開啟WIFI AP模式");
    initAPModeESP();
  }
  else if(WiFi.status() == WL_CONNECTED){
    Serial.println("WIFI成功連線");
    Serial.print("IP:");
    Serial.println(WiFi.localIP());
  }

  initServerHandle();



  
  
}

void loop() {
 
}