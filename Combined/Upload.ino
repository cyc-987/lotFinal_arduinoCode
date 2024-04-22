
// #include "./functions.ino"
#include <Wire.h>
#include <string.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <U8g2lib.h>

// User Modified Part
#define wifi_ssid "K3C-2.4G"
#define wifi_psw "1145141919810"
#define clientIDstr "ArduinoDev"
#define timestamp "996"
#define ProductKey "k0v6qShfY3U"
#define DeviceName "ArduinoDev"
#define DeviceSecret "722c737fd7f6d8df7943ad29980026fa"
#define password "BD0D1B61510F60CC3A2502CFA9F2D13CF8531D58"

// Logic Preset
#define OFF 0
#define ON 1
#define MUTE 2
#define KEEP_OFF 2
#define KEEP_ON 3

#define Buzzer_ON digitalWrite(BuzzerPin, HIGH)
#define Buzzer_OFF digitalWrite(BuzzerPin, LOW)

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
#define AT_MQTT_PUB_SET "AT+MQTTPUB=/%s/%s/user/Arduino,1\r"
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
unsigned long timeStart;

// my start data
#define fanPin 11
#include <dht11.h>
dht11 DHT;
#define DHT11_PIN A5
U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

void setup()
{

  timeStart = millis();
  // Serial Initial
  Serial3.begin(115200);
  Serial.begin(115200);
  String inString = "";

  //  Pin_init();
  Serial.println("Start");

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
    if (inString != "")
    {
      data = inString;
      parse(data);
    }
    // Serial.println(data);
  }

  if (Serial3.available() == 0)
  {
    updateData();
  }

  if ((millis() - timeStart) > 5000)
  {
    getData();
    Upload();
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
  lockStatus = doc["params"]["lockStatus"];
  targetTemperature = doc["params"]["targetTemperature"];
  targetHumidity = doc["params"]["targetHumidity"];
  voiceOutput = doc["params"]["voiceOutput"];

  // Print values.
  Serial.println("==============Start================");
  // Serial.println(method);
  // Serial.println(id);
  Serial.println(lockStatus);
  Serial.println(targetTemperature);
  Serial.println(targetHumidity);
  Serial.println(voiceOutput);
  return 0;
}

bool Upload()
{
  bool flag;
  int len;

  //AT connect
  cleanBuffer(ATcmd, BUF_LEN);
  snprintf(ATcmd, BUF_LEN, AT_MQTT_PUB_SET, ProductKey, DeviceName);
  flag = check_send_cmd(ATcmd, AT_OK, DEFAULT_TIMEOUT);

  //prepare data to upload
  cleanBuffer(ATdata, BUF_LEN_DATA);
  len = snprintf(ATdata, BUF_LEN_DATA, JSON_DATA_PACK_3, ProductKey, DeviceName,temperature,humidity);
  Serial.println(ATdata);

  //upload data
  cleanBuffer(ATcmd, BUF_LEN);
  snprintf(ATcmd, BUF_LEN, AT_MQTT_PUB_DATA, len - 1);
  flag = check_send_cmd(ATcmd, ">", DEFAULT_TIMEOUT);
  if (flag)
    flag = check_send_cmd(ATdata, AT_MQTT_PUB_DATA_SUCC, 20);

  return flag;
}

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
