
#include <SoftwareSerial.h>
#define DEBUG true
SoftwareSerial esp8266(10, 11);

int LED_PIN=13;
int SMOKE_PIN=0;
int FIRE_PIN=1;
int ALARM_LED_PIN=7;
int LIGHT_PIN=2;

long S = 115200;

String WIFI_SSID="";
String WIFI_PASSWD="";
String HOST_IP = "";
String HOST_PORT = "";

void init_conn(){
  String resp;
  
  esp8266.begin(S);
  Serial.println("Starting...");
  
  sendData("AT\r\n",2000,DEBUG); // reset module
  sendData("AT+CWMODE=3\r\n",2000,DEBUG); // configure as access point
  sendData("AT+RST\r\n",2000,DEBUG); // reset module
  resp = sendData("AT+CWJAP=\"" + WIFI_SSID + "\",\"" + WIFI_PASSWD + "\"\r\n",5000,DEBUG); // 连接AP
  Serial.println(resp);
  resp = sendData("AT+CIFSR\r\n",2000,DEBUG);
  Serial.println(resp);
  Serial.println("Connectted AP..."); 
  
  sendData("AT+CIPMUX=0\r\n",2000,DEBUG); // 设置为单线连接
  sendData("AT+CIPSTART=\"TCP\",\"" + HOST_IP + "\"," + HOST_PORT + "\r\n",3000,DEBUG);
  sendData("AT+CIPMODE=1\r\n",2000,DEBUG); // 使用透传

  sendData("AT+CIPSEND\r\n",2000,DEBUG);
  Serial.println("Connectted TCP..."); 
}

int get_smoke(){
  int val;
  val = analogRead(SMOKE_PIN);
  Serial.print("smoke:");
  Serial.println(val, DEC);
  return int(val);
}

int get_fire(){
  int val;
  val = analogRead(FIRE_PIN);
  Serial.print("fire:");
  Serial.println(val, DEC);
  return int(val);
}

int get_light(){
  int val;
  val = analogRead(LIGHT_PIN);
  Serial.print("light:");
  Serial.println(val, DEC);
  return int(val);
}

void setup(){
  pinMode(LED_PIN,OUTPUT);
  pinMode(ALARM_LED_PIN,OUTPUT); 
  
  digitalWrite(LED_PIN, LOW);
  Serial.begin(S);    
  init_conn();
}

void loop()
{
  int light_val = get_light();
  int checks[3][3] = {
    {get_smoke(), 30, 0},   // 数组第一项:采集的数据 第二项:告警的阈值  第三项:是否告警
    {get_fire(), 200, 0},
    {light_val, 10000, 0},
    };
  char* names[] = {
    "smoke",
    "fire",
    "light",
  };
  String req_str = "";
  int is_alarm = 0;
  
  for(int i=0;i<3;i++){
    int val = checks[i][0];
    int danger_val = checks[i][1];
    String sname = names[i];
    int tmp_danger_val = danger_val;

    if(val > tmp_danger_val){
        is_alarm = 1;
        checks[i][2] = 1;
    }
    req_str = req_str + sname + "," + val + "," + danger_val + "," + checks[i][2] + ";";
  }
  
  digitalWrite(LED_PIN, HIGH);
  String resp_data = request(req_str);
  Serial.println(resp_data);
  digitalWrite(LED_PIN, LOW);

  if(is_alarm == 1){
    for(int i=0;i<4;i++){
      digitalWrite(ALARM_LED_PIN, HIGH); 
      delay(500);
      digitalWrite(ALARM_LED_PIN, LOW); 
    }
  }else{
    delay(1000);
  }

}

String readLine(const int timeout, SoftwareSerial serial){
  String response = "";
  long int time = millis();
  while( (time+timeout) > millis())
  {
    while(serial.available())
    {
      char c = serial.read(); // read the next character.
      response+=c;
    }
  }
  return response;
}

String sendData(String command, const int timeout, boolean debug)
{
    esp8266.print(command); // send the read character to the esp8266

    String response = readLine(timeout, esp8266);
   
    if(debug)
    {
//      Serial.print(response);
    }
    return response;
}

String request(String data){
  data = "*" + data + "\r\n";
  String resp_data = sendData(data, 3000, DEBUG);
  return resp_data;
}


