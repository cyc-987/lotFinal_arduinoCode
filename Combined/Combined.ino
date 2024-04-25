
// #include "./functions.ino"
#include <Wire.h>
#include <string.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <U8g2lib.h>
#include <Adafruit_MLX90614.h>
#include <dht11.h>
// #include <SoftwareWire.h>

//wifi和mqtt参数——————————————————————————————————————————————————————————————————————————————
// User Modified Part
#define wifi_ssid "xm"
#define wifi_psw "12090217"
#define clientIDstr "Smart_takeout_cabinet"
#define timestamp "666"
#define ProductKey "k0v6oal29SX"
#define DeviceName "Smart_takeout_cabinet"
#define DeviceSecret "74256bc8a9ecd481f60fa63c74226e43"
#define password "A3BC01B92D5005113580F007FD58FFADB7DE4AEE"


//引脚宏定义————————————————————————————————————————————————————————————————————————————————
// Logic Preset
#define OFF 0
#define ON 1
#define MUTE 2
#define KEEP_OFF 2
#define KEEP_ON 3

#define Buzzer_ON digitalWrite(BuzzerPin, HIGH)
#define Buzzer_OFF digitalWrite(BuzzerPin, LOW)

//fan
#define fanPin 11
//lock
#define lockInput A4
#define lockOutput A5
//heater
#define heater A2//高关低开，继电器是反的（x，别一直加热了
//DHT11
#define DHT11_PIN A0

#define SDA_PIN 20
#define SCL_PIN 21


#define openlockvoicePin A1


#define My_JSON_PACK        "{\"id\":\"66666\",\"version\":\"1.0\",\"method\":\"thing.event.property.post\",\"params\":{\"temperature\":%f,\"humidity\":%f,\"hasTakeout\":%d,\"heaterStatus\":%d,\"fanStatus\":%d}}\r"
//AT指令————————————————————————————————————————————————————————————————————————————
// ATcmd Format
#define AT "AT\r"
#define AT_OK "OK"
#define AT_REBOOT "AT+REBOOT\r"
#define AT_ECHO_OFF "AT+UARTE=OFF\r"
#define AT_MSG_ON "AT+WEVENT=ON\r"

#define AT_WIFI_START "AT+WJAP=%s,%s\r"
#define AT_WIFI_START_SUCC "+WEVENT:STATION_UP"

#define AT_MQTT_AUTH "AT+MQTTAUTH=%s&%s,%s\r"
#define AT_MQTT_CID "AT+MQTTCID=%s|securemode=3\\,signmethod=hmacsha1\\,timestamp=%s|\r"
#define AT_MQTT_SOCK "AT+MQTTSOCK=%s.iot-as-mqtt.cn-shanghai.aliyuncs.com,1883\r"

#define AT_MQTT_AUTOSTART_OFF "AT+MQTTAUTOSTART=OFF\r"
#define AT_MQTT_ALIVE "AT+MQTTKEEPALIVE=500\r"
#define AT_MQTT_START "AT+MQTTSTART\r"
#define AT_MQTT_START_SUCC "+MQTTEVENT:CONNECT,SUCCESS"
// #define AT_MQTT_PUB_SET "AT+MQTTPUB=/%s/%s/user/Arduino,1\r"
#define AT_MQTT_PUB_SET       "AT+MQTTPUB=/sys/%s/%s/thing/event/property/post,1\r"
#define AT_MQTT_PUB_ALARM_SET "AT+MQTTPUB=/sys/%s/%s/thing/event/GasAlarm/post,1\r"
#define AT_MQTT_PUB_DATA "AT+MQTTSEND=%d\r"
#define JSON_DATA_PACK "{\"id\":\"100\",\"version\":\"1.0\",\"method\":\"thing.event.property.post\",\"params\":{\"RoomTemp\":%d.%02d,\"AC\":%d,\"Fan\":%d,\"Buzzer\":%d,\"GasDetector\":%d}}\r"
#define JSON_DATA_PACK_3 "{\"id\":\"110\",\"version\":\"1.0.0\",\"method\":\"/%s/%s/thing/user/Arduino\",\"params\":{\"temperature\":%f,\"humidity\":%f}}\r"
#define AT_MQTT_PUB_DATA_SUCC "+MQTTEVENT:PUBLISH,SUCCESS"
#define AT_MQTT_UNSUB "AT+MQTTUNSUB=2\r"
#define AT_MQTT_SUB "AT+MQTTSUB=2,/sys/%s/%s/thing/service/property/set,1\r"
#define AT_MQTT_SUB_SUCC "+MQTTEVENT:2,SUBSCRIBE,SUCCESS"
#define AT_MQTT_CLOSE "AT+MQTTCLOSE\r"

#define AT_BUZZER_MUTE "\"Buzzer\":2"

#define DEFAULT_TIMEOUT 10 // seconds
#define BUF_LEN 100
#define BUF_LEN_DATA 190

char ATcmd[BUF_LEN];
char ATbuffer[BUF_LEN];
char ATdata[BUF_LEN_DATA];
#define BuzzerPin 3
int Buzzer = OFF;


//变量定义————————————————————————————————————————————————————————————————————————————————
StaticJsonDocument<300> doc;
// mydata
String data; // new add
float temperature;//temperature data
float humidity;//humidity data
int lockStatus;//lock status, 0 is lock, 1 is unlock
int hasTakeout;//takeout status, 0 is none, 1 is positive
int heaterStatus;//0 is off, 1 is on
int fanStatus;//0 is off, 1 is on
float targetTemperature;
float targetHumidity;
int voiceOutput;//0 is no output,,
int lightswitch;//0 is off, 1 is on
unsigned long timeStart;



//dht11
dht11 DHT;
// //oled
U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

//infrared temp
Adafruit_MLX90614 mlx = Adafruit_MLX90614(); 


void setup()
{

  timeStart = millis();
  // Serial Initial
  Serial3.begin(115200);
  Serial.begin(115200);
  String inString = "";

  //  Pin_init();
  Serial.println("Setup Start");

  // Cloud Initial
  while (1)
  {
    if (!WiFi_init())
      continue;
    Serial.println("WiFi Connected");
    if (!Ali_connect())
      continue;
    break;
  }
  Serial.println("Ali Connected");

  dataInit();
  fanInit();
  oledInit();
  lockInit();
  heaterInit();
  lightInit();
  itemperatureInit();
  voiceInit();
  Serial.println("dataInit Done");
}





void loop()
{

  delay(10);
  String inString = "";
  // 有串口的数据进来就暂存在inString里
  if (Serial3.available() > 0)
  {
    delay(10);
    inString = Serial3.readString();

    if (inString.indexOf("MQTTRECV") != -1)
    {
      Serial.println("==============print aliyun receive data================");
      Serial.print("json:");
      Serial.println(inString);
      data = inString;
      parse(data);
      Serial.println("==================End====================\n\n\n");
    }
  
  }
  

  if (Serial3.available() == 0)
  {
    updateData();
  }


//五秒上传一次数据
  if ((millis() - timeStart) > 5000)
  { 
    getData();
    Serial.println("==================print Data=================");
    Serial.println("temperature: "+String(temperature));
    Serial.println("humidity: "+String(humidity));
    Serial.println("lockStatus: "+String(lockStatus));
    Serial.println("hasTakeout: "+String(hasTakeout));
    Serial.println("heaterStatus: "+String(heaterStatus));
    Serial.println("fanStatus: "+String(fanStatus));
    Serial.println("targetTemperature: "+String(targetTemperature));
    Serial.println("targetHumidity: "+String(targetHumidity));
    Serial.println("voiceOutput: "+String(voiceOutput));
    Serial.println("==================End====================\n\n\n");

    
    
    Upload();
    // oledDisplay_th();
    timeStart = millis();
  }


}

// 设置了一个解析的函数，利用库ArdunioJson版本是6.2.13
int parse(String data)
{

  // 找到“{”，并截断到倒数第二个字符
  int commaPosition;
  commaPosition = data.indexOf('{');
  data = data.substring(commaPosition, data.length());
  Serial.println(data);

  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, data);

  // Test if parsing succeeds.
  /*
  if (error)
  {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }
  */

  // 获取数据
  const char *method = doc["method"];
  const char *id = doc["id"];
if (doc.containsKey("params") && doc["params"].containsKey("lockStatus")) {
    lockStatus = doc["params"]["lockStatus"];
}

if (doc.containsKey("params") && doc["params"].containsKey("targetTemperature")) {
    targetTemperature = doc["params"]["targetTemperature"];
}

if (doc.containsKey("params") && doc["params"].containsKey("targetHumidity")) {
    targetHumidity = doc["params"]["targetHumidity"];
}

if (doc.containsKey("params") && doc["params"].containsKey("voiceOutput")) {
    voiceOutput = doc["params"]["voiceOutput"];

if (doc.containsKey("params") && doc["params"].containsKey("voiceOutput")) {
    voiceOutput = doc["params"]["voiceOutput"];
}
if (doc.containsKey("params") && doc["params"].containsKey("hasTakeout")) {
    hasTakeout = doc["params"]["hasTakeout"];
}
if (doc.containsKey("params") && doc["params"].containsKey("lightswitch")) {
    lightswitch = doc["params"]["lightswitch"];
}
  // Print values.
  // Serial.println("==============Start================");
  // Serial.println(method);
  // Serial.println(id)
  Serial.print("lockStatus: ");
  Serial.println(lockStatus);
  Serial.print("targetTemperature: ");
  Serial.println(targetTemperature);
  Serial.print("targetHumidity: ");
  Serial.println(targetHumidity);
  Serial.print("voiceOutput: ");
  Serial.println(voiceOutput);
  return 0;
}

bool Upload()
{
  bool flag;
  int len;
  Serial.println("===============Upload==============");
  //AT connect
  cleanBuffer(ATcmd, BUF_LEN);
  snprintf(ATcmd, BUF_LEN, AT_MQTT_PUB_SET, ProductKey, DeviceName);
  flag = check_send_cmd(ATcmd, AT_OK, DEFAULT_TIMEOUT);

  //prepare data to upload
  cleanBuffer(ATdata, BUF_LEN_DATA);
  len = snprintf(ATdata, BUF_LEN_DATA, My_JSON_PACK, temperature,humidity,hasTakeout,heaterStatus,fanStatus);
  Serial.println(ATdata);

  //upload data
  cleanBuffer(ATcmd, BUF_LEN);
  snprintf(ATcmd, BUF_LEN, AT_MQTT_PUB_DATA, len - 1);
  flag = check_send_cmd(ATcmd, ">", DEFAULT_TIMEOUT);
  if (flag)
    flag = check_send_cmd(ATdata, AT_MQTT_PUB_DATA_SUCC, 20);

  Serial.println("==================Upload End=================\n\n\n");
  return flag;
}


// bool Upload()
// {
//   bool flag;
//   int  frac;
//   int zs;
//   int len;
//   cleanBuffer(ATcmd,BUF_LEN);
//   snprintf(ATcmd,BUF_LEN,AT_MQTT_PUB_SET,ProductKey,DeviceName);
//   flag = check_send_cmd(ATcmd,AT_OK,DEFAULT_TIMEOUT);
//   if(!flag) {
//     Serial.println("send AT_MQTT_PUB_SET false");
//     return false;
//   }

//   cleanBuffer(ATdata,BUF_LEN_DATA);
//   zs=(int) temperature;
//   frac=(temperature-zs)*100;
//   len = snprintf(ATdata,BUF_LEN_DATA,My_JSON_PACK,temperature,humidity,hasTakeout,heaterStatus,fanStatus);

//   cleanBuffer(ATcmd,BUF_LEN);
//   snprintf(ATcmd,BUF_LEN,AT_MQTT_PUB_DATA,len-1);
//   flag=0;
//   flag = check_send_cmd(ATcmd,">",DEFAULT_TIMEOUT);
//   if(flag) {
//     flag = check_send_cmd(ATdata,AT_MQTT_PUB_DATA_SUCC,20);
    
//   } 
//   else {
//     Serial.println("> accept wrong");
//     return false;
//   }
//     return true;

// }

bool Ali_connect()
{
  bool flag;
  bool flag1;

  cleanBuffer(ATcmd, BUF_LEN);
  snprintf(ATcmd, BUF_LEN, AT_MQTT_AUTH, DeviceName, ProductKey, password);
  flag = check_send_cmd(ATcmd, AT_OK, DEFAULT_TIMEOUT);
  if (!flag)
    return false;

  cleanBuffer(ATcmd, BUF_LEN);
  snprintf(ATcmd, BUF_LEN, AT_MQTT_CID, clientIDstr, timestamp);
  flag = check_send_cmd(ATcmd, AT_OK, DEFAULT_TIMEOUT);
  if (!flag)
    return false;

  cleanBuffer(ATcmd, BUF_LEN);
  snprintf(ATcmd, BUF_LEN, AT_MQTT_SOCK, ProductKey);
  flag = check_send_cmd(ATcmd, AT_OK, DEFAULT_TIMEOUT);
  if (!flag)
    return false;

  flag = check_send_cmd(AT_MQTT_AUTOSTART_OFF, AT_OK, DEFAULT_TIMEOUT);
  if (!flag)
    return false;

  flag = check_send_cmd(AT_MQTT_ALIVE, AT_OK, DEFAULT_TIMEOUT);
  if (!flag)
    return false;

  flag = check_send_cmd(AT_MQTT_START, AT_MQTT_START_SUCC, 20);
  if (!flag)
    return false;

  cleanBuffer(ATcmd, BUF_LEN);
  snprintf(ATcmd, BUF_LEN, AT_MQTT_PUB_SET, ProductKey, DeviceName);
  flag = check_send_cmd(ATcmd, AT_OK, DEFAULT_TIMEOUT);
  if (!flag)
    return false;

  // flag = check_send_cmd(AT_MQTT_UNSUB,AT_OK,DEFAULT_TIMEOUT);
  // if(!flag)return false;

  cleanBuffer(ATcmd, BUF_LEN);
  snprintf(ATcmd, BUF_LEN, AT_MQTT_SUB, ProductKey, DeviceName);
  flag = check_send_cmd(ATcmd, AT_MQTT_SUB_SUCC, DEFAULT_TIMEOUT);
  if (!flag)
  {
    BEEP(4);
    flag1 = check_send_cmd(AT_MQTT_CLOSE, AT_OK, DEFAULT_TIMEOUT);
  }
  return flag;
}

bool WiFi_init()
{
  bool flag;

  flag = check_send_cmd(AT, AT_OK, DEFAULT_TIMEOUT);
  if (!flag)
    return false;

  flag = check_send_cmd(AT_REBOOT, AT_OK, 20);
  if (!flag)
    return false;
  delay(5000);

  flag = check_send_cmd(AT_ECHO_OFF, AT_OK, DEFAULT_TIMEOUT);
  if (!flag)
    return false;

  flag = check_send_cmd(AT_MSG_ON, AT_OK, DEFAULT_TIMEOUT);
  if (!flag)
    return false;

  cleanBuffer(ATcmd, BUF_LEN);
  snprintf(ATcmd, BUF_LEN, AT_WIFI_START, wifi_ssid, wifi_psw);
  flag = check_send_cmd(ATcmd, AT_WIFI_START_SUCC, 20);
  return flag;
}

bool check_send_cmd(const char *cmd, const char *resp, unsigned int timeout)
{
  int i = 0;
  unsigned long timeStart;
  timeStart = millis();
  cleanBuffer(ATbuffer, BUF_LEN);
  Serial3.print(cmd);
  Serial3.flush();
  while (1)
  {
    while (Serial3.available())
    {
      ATbuffer[i++] = Serial3.read();
      if (i >= BUF_LEN)
        i = 0;
    }
    if (NULL != strstr(ATbuffer, resp))
      break;
    if ((unsigned long)(millis() - timeStart > timeout * 1000))
      break;
  }

  if (NULL != strstr(ATbuffer, resp))
    return true;
  return false;
}

void cleanBuffer(char *buf, int len)
{
  for (int i = 0; i < len; i++)
  {
    buf[i] = '\0';
  }
}

void BEEP(int b_time)
{
  for (int i = 1; i <= b_time; i++)
  {
    digitalWrite(BuzzerPin, HIGH);
    delay(100);
    digitalWrite(BuzzerPin, LOW);
    delay(100);
  }
}
void Buzzer_mute()
{
  Buzzer_OFF;
  Buzzer = MUTE;
}
