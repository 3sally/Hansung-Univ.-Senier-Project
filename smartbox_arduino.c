


#include "HX711.h"   // 초하빨검 
#include <Wire.h>
#include <SoftwareSerial.h>

#define calibration_factor  - 7050.0
#define DOUT 6 // dt
#define CLK 7 // sck
HX711 scale(DOUT, CLK);
int count = 0;
double w = 0;
double pre_w = 0;
int touch_count = 0; 
//int touch_count = -1;
int up_count = 0;
int put_count = 0;
int pre_up = 0;
int pre_put = 0;
int favorite = 0;
boolean timeSet = false;  //타이머가 실행되는지 여부
boolean dataSet = false; //데이터가 들어왔는지 여부

unsigned long startTime = 0;
unsigned long finishTime = 0;
unsigned long middleTime = 0;
unsigned long dataTime = 0;



SoftwareSerial esp8266(2, 3); // RX/TX 설정, serial 객체생성

boolean test = false; //업로드할때 무게 0점 

void setup() 
{
  Serial.begin(9600);
  scale.set_scale(calibration_factor);
  scale.tare();
  
  esp8266.begin(9600);        //소프트웨어 시리얼 시작
  esp8266.println("AT+RST");  //ESP8266 Reset

}

void loop() 
{

  if(!test){ //test가 false일때 
  Serial.println(" Test Start  ");
  delay(4000);
    test =true;
  } 
  delay(1500);
  unsigned long currentTime = millis();
  w = scale.get_units();
  
  Serial.print("무게 = ");
  Serial.println(w);

  // 터치
  if(w - pre_w > 1 || w - pre_w < -1){ 
        pre_w = w;
        touch_counter();
        } 

  // 들렸을때
  if(-1<w && w<=1){
  //if(w<=1){// 시간 측정 시작
  if(startTime==0){ // 시간 측정 시작
    if(!timeSet){
      startTime = currentTime;
    }
      timeSet = true;
          Serial.print("time Start==");
          Serial.println(startTime/1000);
  }
    }


    if(w>=1&&startTime!=0){
      
      if(w>4){ // 옷+옷걸이 걸린 무게 
        timeSet = false;
        finishTime = currentTime;
        up_counter();//들어봄
        }
       else if(3<w<4){ // 옷걸이 무게만 걸린거
        timeSet = false;
        middleTime = currentTime;
        put_counter();//입어봄
        }
       else{
        Serial.println("No if");
        delay(1500);
        }
    }
    
    checkTime(dataSet);
}//loop끝남

void checkTime(boolean dataSet){
//  delay(1500);
  if(dataSet){
    
  if(!timeSet){

//실제 데이터 쏘는부분   finishTime, middleTime 이게 0이 아닌 경우에 dataTime을 계산해
//   dataTime = finishTime - startTime;
//////////////////////////////////////////
 
  favorite = touch_count + (up_count*5) + (put_count*15) ;
 
    ///초기화
//  dataTime = 0;
    finishTime = 0;
    middleTime = 0;
    startTime = 0;
    timeSet = false;
    dataSet = false;
    
    Serial.print("터치함==");
    Serial.println(touch_count);
    Serial.print("들어봤다==");
    Serial.println(up_count);
    Serial.print("입어봤다==");
    Serial.println(put_count);            
    Serial.print("호감도==");
    Serial.println(favorite);   

   if(up_count - pre_up >= 1 || put_count - pre_put >= 1){ 
        pre_up = up_count;
        pre_put = put_count;

        //와이파이코드   
        String cmd = "AT+CIPSTART=\"TCP\",\"";
        cmd += "52.78.132.239"; // server 접속 IP
        cmd += "\",3000";           // 접속 포트, 3000
        esp8266.println(cmd);
        if(esp8266.find("Error")){
          Serial.println("AT+CIPSTART error");
          return;
        }
        // GET 방식으로 보내기 위한 String, Data 설정
        String getStr = "GET /list/relist?name=";
        getStr +="myj";
        getStr +="&favorite=";
        getStr += String(favorite);
        getStr +="&touch=";
        getStr += String(touch_count);
        getStr +="&shake=";
        getStr += String(up_count);
        getStr +="&tryon=";
        getStr += String(put_count);
        getStr += "\r\n\r\n";
        // Send Data
        cmd = "AT+CIPSEND=";
        cmd += String(getStr.length());
        esp8266.println(cmd);
       if(esp8266.find(">")) {
          esp8266.print(getStr);
          Serial.println("@send!!");
       }
        else {
          esp8266.println("AT+CIPCLOSE");
          Serial.println("AT+CIPCLOSE");  // alert user
        }
        delay(1000); // Thingspeak 최소 업로드 간격 15초를 맞추기 위한 delay


        
        }

 

   }
   }





   
  }
   //////////////////////// 코드 단순화 함수들 /////////////////////

int touch_counter(){
    touch_count ++;
    Serial.print(touch_count);
    Serial.println("번 터치됨! ");
    dataSet = true;
    delay(1000);
  }

int up_counter(){ // 들어봄 카운터
    up_count ++;
    touch_count --;
    Serial.print(up_count);
    Serial.println("번 들어봄! ");
    dataSet = true;
    delay(1000);
  }

int put_counter(){    // 입어봄 카운터
    put_count ++;
    touch_count --;
    Serial.print(put_count);
    Serial.println("번 입어봄! ");
    dataSet = true;
    delay(1000);
  }


