#include <avr/sleep.h>
#include <avr/power.h>
#include <IRremote.h>
#include "audio.h"
#include <SoftwareSerial.h>
#include <Adafruit_NeoPixel.h> //引用此文件
#include <Servo.h>
#include <Microduino_Key.h>
#include <Wire.h>                                  //调用库  
#include "./ESP8266.h"
#include "I2Cdev.h"                                //调用库  
//温湿度   
#include <SHT2x.h>
//光照
#include <SoftwareSerial.h>
//SoftwareSerial mySerial(2, 3); /* RX:D3, TX:D2 */
ESP8266 wifi(mySerial);
//ESP8266 wifi(Serial1);                                      //定义一个ESP8266（wifi）的对象
unsigned long net_time1 = millis();                          //数据上传服务器时间
unsigned long sensor_time = millis();                        //传感器采样时间计时器

//int SensorData;                                   //用于存储传感器数据
String postString;                                //用于存储发送数据的字符串
//String jsonToSend;                                //用于存储发送的json格式参数


int RECV_PIN = 10;
int sensorValue;
int val = 0;
int music_vol = 26; //初始音量0~30
int soundNum = 1;
int pos = 0; 
int pin2 = 2;

#define PIN 6                         /*定义了控制LED的引脚，6表示Microduino的D6引脚，可通过Hub转接出来，用户可以更改 */
#define Light_PIN A0  //光照传感器接AO引脚
#define Light_value1 200
#define Light_value2 500
#define servo_pin SDA
#define PIN_KEY D8   //触摸接在8引脚
#define PIN_NUM 2 //允许接的led灯的个数
//#define buttonPin 12
#define val_max 255
#define val_min 0
#define INTERVAL_Time 10   
#define INTERVAL_SENSOR   17000             //定义传感器采样时间间隔  597000
#define INTERVAL_NET      17000             //定义发送时间
#define  sensorPin_1  A0

#define SSID           "127127"                   // cannot be longer than 32 characters!
#define PASSWORD       "127127127"

#define IDLE_TIMEOUT_MS  3000      // Amount of time to wait (in milliseconds) with no data 
                                   // received before closing the connection.  If you know the server
                                   // you're accessing is quick to respond, you can reduce this value.

//WEBSITE     
#define HOST_NAME   "api.heclouds.com"
#define DEVICEID   "20441028"
#define PROJECTID "105760"
#define HOST_PORT   (80)
#define INTERVAL_sensor 2000
#define INTERVAL_OLED 1000

IRrecv irrecv(RECV_PIN);
decode_results results;
AnalogKey keyAnalog(A0);
Servo myservo;  // create servo object to control a servo
// a maximum of eight servo objects can be created

long interval = 4000;           // 闪烁的时间间隔（毫秒）
unsigned long currentMillis=0;
bool isUP = true;
boolean on_off;
boolean statusChange;
bool playing = false;
unsigned long Time_millis = millis();
String apiKey="3akuAPAqVfn9hbsXm82YWT1gE1k=";
char buf[10];
unsigned long sensorlastTime = millis();
float tempOLED, humiOLED, lightnessOLED;
String mCottenData;
String jsonToSend;

//3,传感器值的设置 
float sensor_tem, sensor_hum, sensor_lux;                    //传感器温度、湿度、光照   
char  sensor_tem_c[7], sensor_hum_c[7], sensor_lux_c[7] ;    //换成char数组传输

Adafruit_NeoPixel strip = Adafruit_NeoPixel(2, PIN, NEO_GRB + NEO_KHZ800);
 //该函数第一个参数控制串联灯的个数，第二个是控制用哪个pin脚输出，第三个显示颜色和变化闪烁频率

 
/***************************************************
 *  Name:        setup
 *  Returns:     Nothing.
 *  Parameters:  None.
 *  Description: Setup for the Arduino.           
 ***************************************************/
void setup() {
    Serial.begin(9600);
    /* Setup the pin direction. */
    pinMode(pin2, INPUT);
    irrecv.enableIRIn(); // 启动红外解码
    Serial.println("Initialisation complete.");
    strip.begin();                             //准备对灯珠进行数据发送
    strip.show();
    myservo.attach(servo_pin);  // attaches the servo on pin SDA to the servo object
    pinMode(PIN_KEY, INPUT);//设置触摸输入状态
    audio_init(DEVICE_Flash,4,music_vol);    //初始化mp3模块
  //audio_init(DEVICE_TF, MODE_loopOne, music_vol);


         //初始化串口波特率  
    Wire.begin();  
    while(!Serial);
    pinMode(sensorPin_1, INPUT);

   //ESP8266初始化
    Serial.print("setup begin\r\n");   

  Serial.print("FW Version:");
  Serial.println(wifi.getVersion().c_str());

  if (wifi.setOprToStationSoftAP()) {
    Serial.print("to station + softap ok\r\n");
  } else {
    Serial.print("to station + softap err\r\n");
  }

  if (wifi.joinAP(SSID, PASSWORD)) {      //加入无线网
    Serial.print("Join AP success\r\n");  
    Serial.print("IP: ");
    Serial.println(wifi.getLocalIP().c_str());
  } else {
    Serial.print("Join AP failure\r\n");
  }

  if (wifi.disableMUX()) {
    Serial.print("single ok\r\n");
  } else {
    Serial.print("single err\r\n");
  }

  Serial.print("setup end\r\n");
    
  
}

void colorWipe(uint32_t c)
{
  for (uint16_t i = 0; i < strip.numPixels(); i++)  //i从0自增到LED灯个数减1
  {
    strip.setPixelColor(i, c); //将第i个灯点亮
    strip.show(); //led灯显示
   }
}


void colorSet(uint32_t c) 
{
  for (uint16_t i = 0; i < strip.numPixels(); i++) 
  //从0自增到LED灯个数减1
{
strip.setPixelColor(i, c); //此函数表示将第i个LED点亮
}
  strip.show(); //LED灯显示
}


void rainbowCycle( int r, int g, int b, uint8_t wait) {
  for (int val = 0; val < 255; val++) 
  //val由0自增到254不断循环
  {
colorSet(strip.Color(map(val, val_min, val_max, 0, r), map(val, val_min, val_max, 0, g), map(val, val_min, val_max, 0, b)));
//红绿蓝LED灯依次从暗到亮
/*“map(val,x,y,m,n)”函数为映射函数，可将某个区间的值（x-y）变幻成（m-n），val则是你需要用来映射的数据*/
    delay(wait); //延时
  }
  for (int val = 255; val >= 0; val--)  //val从255自减到0不断循环
  {
colorSet(strip.Color(map(val, val_min, val_max, 0, r), map(val, val_min, val_max, 0, g), map(val, val_min, val_max, 0, b)));
//红绿蓝LED灯依次由亮到暗
    delay(wait); //延时
  }
}

void getSensorData(){  
    sensor_tem = SHT2x.readT() ;   
    sensor_hum = SHT2x.readRH();   
    //获取光照
    sensor_lux = analogRead(A0);    
    delay(1000);
    dtostrf(sensor_tem, 2, 1, sensor_tem_c);
    dtostrf(sensor_hum, 2, 1, sensor_hum_c);
    dtostrf(sensor_lux, 3, 1, sensor_lux_c);
}
void updateSensorData() {
  if (wifi.createTCP(HOST_NAME, HOST_PORT)) { //建立TCP连接，如果失败，不能发送该数据
    Serial.print("create tcp ok\r\n");

jsonToSend="{\"Temperature\":";
    dtostrf(sensor_tem,1,2,buf);
    jsonToSend+="\""+String(buf)+"\"";
    jsonToSend+=",\"Humidity\":";
    dtostrf(sensor_hum,1,2,buf);
    jsonToSend+="\""+String(buf)+"\"";
    jsonToSend+=",\"Light\":";
    dtostrf(sensor_lux,1,2,buf);
    jsonToSend+="\""+String(buf)+"\"";
    jsonToSend+="}";



    postString="POST /devices/";
    postString+=DEVICEID;
    postString+="/datapoints?type=3 HTTP/1.1";
    postString+="\r\n";
    postString+="api-key:";
    postString+=apiKey;
    postString+="\r\n";
    postString+="Host:api.heclouds.com\r\n";
    postString+="Connection:close\r\n";
    postString+="Content-Length:";
    postString+=jsonToSend.length();
    postString+="\r\n";
    postString+="\r\n";
    postString+=jsonToSend;
    postString+="\r\n";
    postString+="\r\n";
    postString+="\r\n";

  const char *postArray = postString.c_str();                 //将str转化为char数组
  Serial.println(postArray);
  wifi.send((const uint8_t*)postArray, strlen(postArray));    //send发送命令，参数必须是这两种格式，尤其是(const uint8_t*)
  Serial.println("send success");   
     if (wifi.releaseTCP()) {                                 //释放TCP连接
        Serial.print("release tcp ok\r\n");
        } 
     else {
        Serial.print("release tcp err\r\n");
        }
      postArray = NULL;                                       //清空数组，等待下次传输数据
  
  } else {
    Serial.print("create tcp err\r\n");
  }
  
}

/***************************************************
 *  Name:        loop
 *  Returns:     Nothing.
 *  Parameters:  None.
 *  Description: Main application loop.
 ***************************************************/
void loop() 
{
 L1: if (irrecv.decode(&results)) 
    {
      Serial.println(results.value, HEX);
      if (results.value==0X1FE807F)
         { 
             Serial.println("A：观赏模式 ");
             //程序1:观赏模式        
             
              if (digitalRead(PIN_KEY))  //检测按键状态
                {
                Serial.println("KEY NO");//串口打印没有触摸
                for (pos = 0; pos <= 180; pos += 1) // goes from 0 degrees to 180 degrees
                      { // in steps of 1 degree
                        myservo.write(pos);              // tell servo to go to position in variable 'pos'
                        delay(15);                       // waits 15ms for the servo to reach the position
                       }
                for (pos = 180; pos >= 0; pos -= 1) // goes from 180 degrees to 0 degrees
                      {
                        myservo.write(pos);              // tell servo to go to position in variable 'pos'
                        delay(15);
                        irrecv.resume(); //  接收下一个值
                       }
                }
              else   
                {delay(10000000);
                irrecv.resume(); //  接收下一个值
                if (irrecv.decode(&results))
             goto L1;}  
         }

            
          else if (results.value==0X1FE40BF)
          {
             Serial.println("B：整蛊模式-眼睛发光+叫");

             //程序2:整蛊模式-眼睛发光+叫
              audio_play();
              Serial.println("start sound.....");
              audio_choose(1);
              playing = true;
              rainbowCycle( 255, 0, 0, 10); //红色呼吸
              rainbowCycle( 0, 255, 0, 10); //绿色呼吸
              rainbowCycle( 0, 0, 255, 10); //蓝色呼吸
             irrecv.resume(); //  接收下一个值
             if (irrecv.decode(&results))
             goto L1;  
          }

          else if (results.value==0X1FEC03F)
         { 
          Serial.println("3 气象站模式 ");
             //程序1:气象站模式       
          if (sensor_time > millis())  sensor_time = millis();  
    
  if(millis() - sensor_time > INTERVAL_SENSOR)              //传感器采样时间间隔  
  {  
    getSensorData();                                        //读串口中的传感器数据
    sensor_time = millis();
  }  

    
  if (net_time1 > millis())  net_time1 = millis();
  
  if (millis() - net_time1 > INTERVAL_NET)                  //发送数据时间间隔
  {                
    updateSensorData();                                     //将数据上传到服务器的函数
    net_time1 = millis();
  }
             irrecv.resume(); //  接收下一个值 
             if (irrecv.decode(&results))
             goto L1;  
          }

             else //if (results.value !=0X1FE40BF&&results.value!=0X1FE807F)
             {
             Serial.println("C：光照感应灯模式");
             //程序3：光照感应灯模式

                sensorValue = analogRead(Light_PIN);             //光检测
                Serial.println(sensorValue);                    //彩色led灯根据光强调节颜色和亮度
                if (sensorValue < Light_value1)                  //若光强小于400
                colorWipe(strip.Color(map(sensorValue, 0, 200, 225, 150), 0, 0));
                     /*“map(val,x,y,m,n)”函数为映射函数，可将某个区间的值（x-y）变幻成（m-n），val则是你需要用来映射的数据，这里是将10到400的光对应用0到25的绿光标示*/
                else if (sensorValue >= Light_value1 && sensorValue < Light_value2)
                     //若光强大于400小于800
                colorWipe(strip.Color(map(sensorValue, 200, 500,150, 60), 0, 0));
                     //将400到800的光分别用0到255的蓝光表示
                else if (sensorValue >=500&&sensorValue<=700)
                colorWipe(strip.Color(map(sensorValue, 500, 700, 60, 0), 0, 0));
                if(sensorValue>700)
                strip.setPixelColor(0, strip.Color(0, 0, 0));//灭
                     //将800到960的光用0到255的红光表示
             irrecv.resume(); //  接收下一个值 
             if (irrecv.decode(&results))
             goto L1;  
             }

               
        
             //if(results.value==0)
             //Serial.println("NULL");
    }
   }
