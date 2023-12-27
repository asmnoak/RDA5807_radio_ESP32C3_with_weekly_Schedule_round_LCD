//**********************************************************************************************************
//*    clock_wifi_radio_with_weekly_schedule(cg9a01) --  RDA5807 FM Radio which is controlled by weekly schedule using XIAO ESP32C3. 
//*                                              Clock time of XIAO ESP32C3 refers NTP using wifi network. 
//*                                              Display is 1.28 round LCD of which driver is CG9A01. 
//**********************************************************************************************************
//  
//
//  2023/11/28 created by asmaro

#include <TFT_eSPI.h> 
//#include "WiFi.h"
#include <WiFiMulti.h>
#include <WebServer.h>
#include "Wire.h"
#include <esp_sntp.h>           // esp lib
#include <TimeLib.h>            // https://github.com/PaulStoffregen/Time
#include <RDA5807.h>            // install using lib tool
//#include "EEPROM.h"
#include <Preferences.h>        // For permanent data
#include "Free_Fonts.h"         // CG9A01 Fonts

#define I2C_SDA      6          // I2C DATA
#define I2C_SCK      7          // I2C CLOCK
//#define OLED_I2C_ADDRESS 0x3C   // Check the I2C bus of your OLED device
#define SCREEN_WIDTH 240        // LCD display width, in pixels
#define SCREEN_HEIGHT 240        // LCD display height, in pixels
#//define OLED_RESET -1           // Reset pin # (or -1 if sharing Arduino reset pin)
#define MAXSTNIDX    7          // station index 0-7          
#define MAXSCEDIDX   8          // schedule table index 0-8          

#define backcolor 0x00A0
#define backcolor2 0x5AC0

WiFiMulti wifiMulti;
RDA5807 radio;

TFT_eSPI tft = TFT_eSPI(); 
TFT_eSprite spr = TFT_eSprite(&tft);

String ssid =     "SSID1";      // WiFi 1 , set your wifi station
String password = "PASSWORD1";  // set your password
String ssid2 =     "your_SSID2";     // WiFi 2, optional
String password2 = "your_password2"; // set your password

struct tm *tm;
int d_year;
int d_mon ;
int d_mday ;
int d_hour ;
int d_min ;
int d_sec ;
int d_wday ;
static const char *weekStr[7] = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"}; //3文字の英字

int  vol;
int  lastvol;
int  stnIdx;
int  laststnIdx;
int  stnFreq[] = {8040, 8250, 8520, 9040, 9150, 7620, 7810, 7860}; // frequency of radio station
String  stnName[] = {"AirG", "NW", "NHK", "STV", "HBC", "sanka", "karos", "nosut"}; // name of radio station max 5 char
//                      0      1     2      3      4       5        6       7
bool bassOnOff = false;
bool vol_ok = true;
bool stn_ok = true;
bool p_onoff_req = false;
bool p_on = false;

uint32_t currentFrequency;
float lastfreq;
struct elm {  // program
   int stime; // strat time(min)
   int fidx;  // frequency table index
   int duration; // length min
   int volstep; // volume
   int poweroff; // if 1, power off after duration
   int scheduled; // if 1, schedule done for logic
};
struct elm entity[7][MAXSCEDIDX + 1] = {
{{390,1,59,2,1,0},{540,6,59,1,0,0},{600,0,59,1,0,0},{660,3,119,1,0,0},{780,1,59,1,0,0},{840,0,59,1,0,0},{900,6,59,1,0,0},{1140,3,119,1,0,0},{1410,0,29,1,1,0}}, // sun
{{390,4,59,2,1,0},{480,3,119,1,0,0},{600,6,59,1,0,0},{720,2,119,1,0,0},{840,1,119,1,0,0},{0,0,0,0,0,0},{1020,1,119,1,0,0},{1200,6,89,1,0,0},{1410,0,29,1,1,0}}, // mon
{{390,4,59,2,1,0},{480,3,119,1,0,0},{720,2,89,1,0,0},{840,1,119,1,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},{1020,1,119,1,0,0},{1200,0,89,1,0,0},{1410,0,29,1,1,0}}, // tue
{{390,4,59,2,1,0},{480,3,119,1,0,0},{720,2,89,1,0,0},{840,1,119,1,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},{1020,1,119,1,0,0},{1200,0,89,1,0,0},{1410,0,29,1,1,0}}, // wed
{{390,4,59,2,1,0},{480,3,119,1,0,0},{600,6,59,1,0,0},{720,2,119,4,0,0},{840,1,119,1,0,0},{960,1,59,1,0,0},{1080,1,119,1,0,0},{1200,0,89,1,0,0},{1290,3,59,1,1,0}}, // thu
{{390,4,59,2,1,0},{480,3,119,1,0,0},{660,0,59,1,0,0},{720,2,119,1,0,0},{840,6,119,1,0,0},{0,0,0,0,0,0},{1080,1,119,1,0,0},{1200,1,89,1,0,0},{1290,3,59,1,1,1}}, // fri
{{390,0,29,2,0,0},{420,2,119,1,0,0},{540,2,110,1,0,0},{720,2,119,1,0,0},{840,2,119,1,0,0},{960,2,119,1,0,0},{1080,4,59,1,0,0},{1140,3,119,1,0,0},{1260,0,89,1,1,0}}  // sat
};
//struct elm rom_entity[7][MAXSCEDIDX + 1];
int last_d_min = 99;
int currIdx = 99;
int pofftm_h = 0;
int pofftm_m = 0;
const char* ntpServer = "ntp.nict.jp";
const long  gmtOffset_sec = 32400;
const int   daylightOffset_sec = 0;

volatile int counter = 0;

double rad=0.01745;
float x[360];
float y[360];


float xx[360];
float yy[360];

float fq = 90.4;

int r=108;
int cx=120;
int cy=120;

int fg[12]={2,3,3,5,3,3,4,3,3,1,3,3}; // start at 15 min location. 1,2,4,5:rect, 3:circle
String days[]={"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};
int circ2[12];
int circ1[60];

String h,m,s,d1,d2,m1,m2,f,v,p;

WebServer server(80);  // port 80(default)
//WiFiServer server(80);  // port 80(default)
// Operation by server
int s_srv = 1;
int a_srv = 1;
int b_srv = 1;
char titlebuf[166];
char rstr[128];
char stnurl[128];  // current internet station url
String msg = "";

Preferences preferences; // Permanent data

int split(String data, char delimiter, String *dst){
  int index = 0;
  int arraySize = (sizeof(data))/sizeof((data[0]));
  int datalength = data.length();
  
  for(int i = 0; i < datalength; i++){
    char tmp = data.charAt(i);
    if( tmp == delimiter ){
      index++;
      if( index > (arraySize - 1)) return -1;
    }
    else dst[index] += tmp;
  }
  return (index + 1);
}

int dayofWeek(String dow) {
  dow.trim();
  //Serial.println("dow");
  //Serial.println(dow);
  if (dow.equals("Sun")) return 0; 
  else if (dow.equals("Mon")) return 1;
  else if (dow.equals("Tue")) return 2;
  else if (dow.equals("Wed")) return 3;
  else if (dow.equals("Thu")) return 4;
  else if (dow.equals("Fri")) return 5;
  else if (dow.equals("Sat")) return 6;
  else return 9;
}

int setWeeksced(String val1){
  String instr[12] = {"\n"};
  String instr2[8] = {"\n"};
  String instr3[4] = {"\n"};
  int ix = split(val1,';',instr);
  if (ix != 11) {
    msg = "different number of arguments.";
    return 4;
  } else {
    //msg = "arguments. ok.";
    int down = dayofWeek(instr[0]);
    if (down > 6) { msg = "invalid day of week."; return 4;}
    else {
      // normal process
      msg = "normal process.";
      instr[0].trim();
      Serial.println(instr[0]);
      for(int j = 0; j <= MAXSCEDIDX; j++) {
        instr[j+1].trim();
        msg = "normal process 2.";
        //Serial.println(instr[j+1]);
        String val2 = instr[j+1];
        ix = split(val2,',',instr2);
        if (ix != 5) { 
            msg = "different number of  2nd level arguments.";
            return 4;
        } else {
            //for(int i = 0; i < 5; i++) {
              msg = "OK! Processing.";
              //Serial.println(instr2[i]);
              val2 = instr2[0];
              ix = split(val2,':',instr3);
              if (ix != 2) {
                msg = "different number of  3rd level arguments.";
                return 4;
              }
              instr3[0].trim();
              instr3[1].trim();
              entity[down][j].stime = instr3[0].toInt() * 60 + instr3[1].toInt();
              instr3[0] = "";
              instr3[1] = "";
              instr2[0] = "";

              entity[down][j].fidx = instr2[1].toInt();
              instr2[1] = "";
              entity[down][j].duration = instr2[2].toInt();
              instr2[2] = "";
              entity[down][j].volstep = instr2[3].toInt();
              instr2[3] = "";
              entity[down][j].poweroff = instr2[4].toInt();
              instr2[4] = "";
              preferences.putString(weekStr[down], val1);  // save permanently
              
            //}
        }
        
      }
      msg = "OK! Done.";
      return 0;
    }
  } 

}

void handleRoot(void)
{
    String html;
    String val1;
    String val2;
    String val3;
    String val4;
    String val5;
    String val6;
    char htstr[180];
    char stnno[4];
    Serial.println("web received");
    if (server.method() == HTTP_POST) { // submitted with string
      val1 = server.arg("daysced");
      val2 = server.arg("vup");
      val3 = server.arg("vdown");
      val4 = server.arg("stnup");
      val5 = server.arg("stndown");
      val6 = server.arg("pwonoff");
      if (val2.length() != 0) {
        Serial.println("vup");
        vol_setting(); 
        msg = "control vup";
      }
      else if (val3.length() != 0) {
        Serial.println("vdown");
        vol_setting_2(); 
        msg = "control vdown";
      }
      else if (val4.length() != 0) {
        Serial.println("stnup");
        station_setting(); 
        msg = "control stnup";
      }
      else if (val5.length() != 0) {
        Serial.println("stndown");
        station_setting_2(); 
        msg = "control stndown";
      }
      else if (val6.length() != 0) {
        Serial.println("pwonoff");
        power_onoff_setting(); 
        msg = "control pwonoff";
      }
      else {
      //EEPROM.begin(sizeof(rom_entity[7][MAXSCEDIDX + 1]));
      //sprintf(htstr,"size %d",sizeof(rom_entity[7][MAXSCEDIDX + 1]));
      //Serial.println(htstr);
      //EEPROM.get<elm>(0,rom_entity[7][MAXSCEDIDX + 1]);
      //EEPROM.put<elm>(0, entity[0][0]);
      //EEPROM.commit();
      //EEPROM.get<elm>(0,rom_entity[0][1]); 
      //sprintf(htstr,"[%d,%d,%d,%d,%d]",rom_entity[0][1].stime,rom_entity[0][1].fidx,rom_entity[0][1].duration,rom_entity[0][1].volstep,rom_entity[0][1].poweroff);
      //Serial.println(htstr);

        if (val1.length() == 0) {
          msg ="no input.";
        } else {
          int rc = setWeeksced(val1);
        }
      }
    }
    html = "<!DOCTYPE html><html><head><meta charset=\"utf-8\"><title>Week schedule</title>";
    //html += "<html>";
    //html += "<head>";
    //html += "<meta charset=\"utf-8\">";
    //html += "<title>Week schedule</title>";
    html += "</head><body><p>Select a day of the week, change it, then submit.</p><form action=\"\" method=\"post\">";
    //html += "</head><body>";
    //html += "<form action=\"\" method=\"post\">";
    html += "<p>";
    html += "<input type=\"text\" id=\"daysced\" name=\"daysced\" size=\"120\" value=\"\">";
    html += "</p><p><input type=\"submit\" value=\"submit\" class=\"btn\"></p></form>";
    html += "<p>" + msg + "</p>";
    html += "<p>Arguments of enrty: Start time(hour:min),Station(See below),Duration(min),Volume,Pweroff</p>";
    html += "<p>Station List: 0=" + stnName[0] + ",1=" + stnName[1] + ",2=" + stnName[2] + ",3=" + stnName[3] + ",4=" + stnName[4];
    html += ",5=" + stnName[5] + ",6=" + stnName[6] +  ",7=" + stnName[7] + "</p>";
    html += "<script>";
//    html += "let entity = [[[390,1,59,4,1],[540,6,59,2,0],[600,0,59,2,0],[660,3,119,2,0],[780,1,59,2,0],[840,0,59,2,0],[900,1,59,2,0],[1140,3,119,2,0],[1410,0,29,2,1]],";
//    html += "[[390,1,59,4,1],[540,6,59,2,0],[600,0,59,2,0],[660,3,119,2,0],[780,1,59,2,0],[840,0,59,2,0],[900,1,59,2,0],[1140,3,119,2,0],[1410,0,29,2,1]]]";
    html += "let entity = [";
    for (int i = 0; i < 7; i++){
      html += "[";
      for(int j = 0; j <= MAXSCEDIDX; j++) {
        sprintf(htstr,"['%d:%02d',%d,%d,%d,%d]",entity[i][j].stime / 60,entity[i][j].stime % 60,entity[i][j].fidx,entity[i][j].duration,entity[i][j].volstep,entity[i][j].poweroff);
        html += htstr;
        if (j != MAXSCEDIDX) html += ",";
      }
      html += "]";
      if (i != 6) html += ",";
    }
    html += "];";
    html += "let week = [\"Sun\",\"Mon\",\"Tue\",\"Wed\",\"Thu\",\"Fri\",\"Sat\"];";
    html += "document.write('<table id=\"tbl\" border=\"1\" style=\"border-collapse: collapse\">');";
    html += "for (let i = 0; i < 7; i++){";
    html += "let wstr ='';";
    html += "wstr ='<tr>' + '<td>' + '<input type=\"radio\" name=\"week\" value=\"\" onclick=\"setinput(' + i + ')\">' + '</td>' + '<td>' + week[i] + '</td>';";
    html += "document.write(wstr);";
    html += "for (let j = 0; j < 9; j++){";
    html += "document.write('<td>');";
    html += "document.write(entity[i][j]);";
    html += "document.write('</td>');}";
    html += "document.write('</tr>');";
    html += "}";
    html += "document.write('</table>');";
    html += "function setinput(trnum) {";
    html += "var input = document.getElementById(\"daysced\");";
    html += "var table = document.getElementById(\"tbl\");";
    html += "var cells = table.rows[trnum].cells;";
    html += "let istr = '';";
    html += "for (let j = 1; j <= 10; j++){";
    html += "istr = istr + cells[j].innerText + ';';";
    html += "}";
    html += "input.value = istr;";
    html += "}";
    html += "</script>";
    html += "<style>.lay_i input:first-of-type{margin-right: 20px;}</style>";
    html += "<style>.btn {width: 300px; padding: 10px; box-sizing: border-box; border: 1px solid #68779a; background: #cbe8fa; cursor: pointer;}</style>";
    html += "<p><form action=\"\" method=\"post\">";
    html += "<p>Control Functions</p>";
    html += "<p><div class=\"lay_i\"><input type=\"submit\" name=\"vup\"  value=\"volume up\" class=\"btn\"><input type=\"submit\" name=\"vdown\" value=\"volume down\" class=\"btn\"></div></p>";
    html += "<p><div class=\"lay_i\"><input type=\"submit\" name=\"stnup\"  value=\"station up\" class=\"btn\"><input type=\"submit\" name=\"stndown\" value=\"station down\" class=\"btn\"></div></p>";
    html += "<p><div class=\"lay_i\"><input type=\"submit\" name=\"pwonoff\"  value=\"pwr_on_off\" class=\"btn\"></div></p>";
    html += "</form></p></body>";
    html += "</html>";
    server.send(200, "text/html", html);
    Serial.println("web send response");

}
void handleNotFound(void)
{
    server.send(404, "text/plain", "Not Found.");
}

void setup()
{
    Serial.begin(115200);
    Serial.println("start");
    pinMode(2, INPUT_PULLUP);  // power on_off
    digitalWrite(2, HIGH);
    attachInterrupt(2, power_onoff_setting, FALLING); // 

    //EEPROM.begin(sizeof(rom_entity[7][MAXSCEDIDX + 1]));
    //EEPROM.get<elm>(0,rom_entity[7][MAXSCEDIDX + 1]);

    // Permanent data check
    preferences.begin("week_sced", false);
    for (int i = 0; i < 7; i++){
       String val1 = preferences.getString(weekStr[i],"");       
       if (val1 != "") {
         //Serial.println(val1);
         int rc = setWeeksced(val1);
       }
    }

    WiFi.mode(WIFI_STA);
    wifiMulti.addAP(ssid.c_str(), password.c_str());  
    wifiMulti.addAP(ssid2.c_str(), password2.c_str());
    wifiMulti.run();   // It may be connected to strong one
    tft.begin();  // initialize
    delay(100);
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);

    spr.setColorDepth(8);      // Create an 8bpp Sprite of 240x240 pixels
    spr.createSprite(240, 240);
    spr.fillSprite(TFT_BLACK); // Fill the Sprite with black
    spr.setTextColor(TFT_GREEN);        // Green text
    spr.setTextDatum(4);
    
    Wire.begin(); //

    while (true) {
      if(WiFi.status() == WL_CONNECTED){ break; }  // WiFi connect OK then next step
      Serial.println("WiFi Err");
      tft.drawString("WiFi Err",cx-80,cy,4);
      WiFi.disconnect(true);
      delay(5000);
      wifiMulti.run();
      delay(1000*300);  // Wait for Wifi ready
    }
    Serial.println("wifi start");
    wifisyncjst(); // refer time and day
        
    splash();
    //WiFi.disconnect(true);  // Connection is no longer needed if no web server
    //
    radio.setup(); // Stats the receiver with default valuses. Normal operation
    delay(500);
    radio.setBand(2); //#### 
    radio.setSpace(0); //####
    delay(300);
    p_on = true;
    radio.setVolume(1);
    vol=1;
    lastvol=1;
    stnIdx = 3;
    laststnIdx = 3;
    radio.setFrequency(9040);  // Tune on 90.4 MHz
    lastfreq=9040;
    Serial.println("Setup done");
    // web server
    server.on("/", handleRoot);
    //server.on("/oper", handleOper);
    server.onNotFound(handleNotFound);
    server.begin();
    Serial.print("IP = ");
    Serial.println(WiFi.localIP());
    titlebuf[0] = 0;
    int b=0;
    int b2=0;

    for(int i=0;i<360;i++)
    {
       x[i]=(r*cos(rad*i))+cx;
       y[i]=(r*sin(rad*i))+cy;
       xx[i]=((r-16)*cos(rad*i))+cx;
       yy[i]=((r-16)*sin(rad*i))+cy;

       //lx[i]=((r-26)*cos(rad*i))+cx;
       //ly[i]=((r-26)*sin(rad*i))+cy;
       
       if(i%30==0){
        circ2[b]=i;
        b++;
       }

       if(i%6==0){
        circ1[b2]=i;
        b2++;
       }
      }

}
void splash()
{
  IPAddress ipadr = WiFi.localIP();
  tft.drawString("** Clock Radio **",cx-100,cy-40,4);
  tft.drawString("IP4=",cx-80,cy,4); // display last octet
  tft.drawNumber(ipadr[3],cx+40,cy,4);
  tft.drawString("V:0.7.2",cx-80,cy+40,4);
  delay(1000);
}

void wifisyncjst() {
  //---------内蔵時計のJST同期--------
  // NTPサーバからJST取得
  int lcnt = 0;
  //configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  configTzTime("JST-9", "ntp.nict.jp", "ntp.jst.mfeed.ad.jp");
  delay(500);
  // 内蔵時計の時刻がNTP時刻に合うまで待機
  while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET) {
    delay(500);
    lcnt++;
    if (lcnt > 100) {
      Serial.println("time not sync within 50 sec");
      break;
    }
  }
}


void loop()
{
  //---------set clock--------
  char ts[80];
  float tf = lastfreq/100;
  time_t t = time(NULL);
  tm = localtime(&t);
  d_mon  = tm->tm_mon+1;
  d_mday = tm->tm_mday;
  d_hour = tm->tm_hour;
  d_min  = tm->tm_min;
  d_sec  = tm->tm_sec;
  d_wday = tm->tm_wday;
  //Serial.print("time ");
  
  s=String(d_sec); 
  m=String(d_min);
  h=String(d_hour);
  f=String(tf);
  v=String(vol);
  p=p_on ? "1" : "0";
  if(s.toInt()<10)
   s="0"+s;
  if(m.toInt()<10)
   m="0"+m;
  if(h.toInt()<10)
   h="0"+h;
  if(d_mday>10)
  {
    d1=d_mday/10;
    d2=d_mday%10;
    }
    else
    {
      d1="0";
      d2=String(d_mday);
    }

   if(d_mon>10)
   {
    m1=d_mon/10;
    m2=d_mon%10;
   }
   else
   {
      m1="0";
      m2=String(d_mon);
   }
  
     
 spr.fillSprite(TFT_BLACK);
 spr.fillCircle(cx,cy,124,backcolor);
 
 spr.setTextColor(TFT_WHITE,backcolor);
  
 for(int i=0;i<12;i++)
  {
    if (fg[i]==1) 
      //spr.drawTriangle(x[circ2[i]-2],y[circ2[i]],x[circ2[i]+2],y[circ2[i]],x[circ2[i]],y[circ2[i]+8],TFT_ORANGE);
      spr.fillRect(x[circ2[i]]-2,y[circ2[i]],5,10,TFT_ORANGE); // 0 min
    else if(fg[i]==2)
      spr.fillRect(x[circ2[i]]-10,y[circ2[i]]-2,10,5,TFT_ORANGE); // 15 min
    else if(fg[i]==3)
      spr.fillCircle(x[circ2[i]],y[circ2[i]],2,TFT_ORANGE);
    else if(fg[i]==4)
      spr.fillRect(x[circ2[i]],y[circ2[i]]-2,10,5,TFT_ORANGE);  // 45 min
    else if(fg[i]==5)
      spr.fillRect(x[circ2[i]]-2,y[circ2[i]]-10,5,10,TFT_ORANGE); // 30 min
  }
 
 spr.setFreeFont(FMB9); // MONO BOLD
 spr.drawString("Vol:"+v+" P:"+p,120,64);
 
 spr.setFreeFont(FMB12);
 spr.drawString(f+"MHz",cx,cy-36);
 spr.setFreeFont(FMB12);
 spr.drawString(h+":"+m+".",cx-20,cy+40); // 
 spr.drawString(s,cx+40,cy+40); //
 spr.setTextFont(0);

 spr.fillRect(62,116,12,20,backcolor2);  // MONTH
 spr.fillRect(76,116,12,20,backcolor2);
 spr.fillRect(100,116,12,20,backcolor2);  // DAY
 spr.fillRect(114,116,12,20,backcolor2);
 
 spr.fillRect(142,116,36,20,backcolor2);  // WEEK

 spr.setTextColor(0x35D7,TFT_BLACK);
 spr.drawString("MONTH",76,108);
 spr.drawString("DAY",112,108);
 spr.drawString("WEEK",160,108);

 spr.setTextColor(TFT_WHITE,backcolor2);
 spr.drawString(m1,71,126,2);
 spr.drawString(m2,83,126,2);

 spr.drawString(d1,107,126,2);
 spr.drawString(d2,121,126,2);

 spr.drawString(days[d_wday],161,126,2);
 
 for(int i=0;i<60;i++)
   spr.fillCircle(xx[circ1[i]],yy[circ1[i]],1,TFT_WHITE);
 if (d_sec<15){
   spr.fillCircle(xx[circ1[d_sec+45]],yy[circ1[d_sec+45]],6,TFT_YELLOW);
 } else {
   spr.fillCircle(xx[circ1[d_sec-15]],yy[circ1[d_sec-15]],6,TFT_YELLOW);
 }
 spr.pushSprite(0, 0); 
  server.handleClient();

  if (p_onoff_req) {
    if (p_on) {
      radio.powerDown();
      Serial.println("pw off");
      p_on = false;
    } else {
      radio.powerUp();
      delay(300);
      radio.setVolume(vol);
      radio.setFrequency(stnFreq[stnIdx]);
      delay(300);
      p_on = true;
    }
    p_onoff_req = false;
  }
   if (lastvol != vol) {
    Serial.println("vol changed");
    radio.setVolume(vol);
    lastvol = vol;
    vol_ok = true;
  }
  if (laststnIdx != stnIdx) {
    Serial.println("stn changed");
    radio.setFrequency(stnFreq[stnIdx]);
    lastfreq = stnFreq[stnIdx];
    laststnIdx = stnIdx;
    stn_ok = true;
  }
 
  if (last_d_min != d_min) {
    last_d_min = d_min;
    if (pofftm_h == d_hour && pofftm_m == d_min && p_on) { // power off time ?
      p_onoff_req = true;
      pofftm_h = 0;
      pofftm_m = 0;
    } else {
      for(int i = 0; i <= MAXSCEDIDX; i++) {
        if (entity[d_wday][i].stime == 0 ) {     
          //nop
          //Serial.println(d_min);
        } else {
          //Serial.println(entity[d_wday][i].stime);
          if ((entity[d_wday][i].stime <= d_hour * 60 + d_min) && 
              ((entity[d_wday][i].stime + entity[d_wday][i].duration) >= (d_hour * 60 + d_min ))
              && (entity[d_wday][i].scheduled != 1)) {
            if (lastfreq == stnFreq[entity[d_wday][i].fidx]) {
              //entity[d_wday][i].scheduled = 1; // mark it scheduled
              
            } else {          
              //radio.setFrequency(stnFreq[entity[d_wday][i].fidx]);
              stnIdx =  entity[d_wday][i].fidx;                     
              //lastfreq = stnFreq[stnIdx];
            }
            //radio.setVolume(entity[d_wday][i].volstep);
            vol = entity[d_wday][i].volstep;
            currIdx = i;
            entity[d_wday][i].scheduled = 1; // mark it scheduled
            Serial.println("scheduled");
            if (entity[d_wday][i].poweroff==1) { // power off ?
              pofftm_h = (entity[d_wday][i].stime + entity[d_wday][i].duration) / 60; // set power off time
              pofftm_m = (entity[d_wday][i].stime + entity[d_wday][i].duration) % 60;
              sprintf(ts,"%02d:%02d %s",pofftm_h,pofftm_m,"poff scheduled");
              Serial.println(ts);
            }
            if (p_on==false) {
              p_onoff_req = true;  //  if power off currently then power on req
              pofftm_h = 0;        // reset
              pofftm_m = 0;
              Serial.println("pw on req");
            }  
          }
        }
      }
    }
  }
  delay(1000);   // Comment out when you use sleep API below 
 
  /* esp_sleep_enable_timer_wakeup(1000 * 1000); // wake after 1 sec 
  esp_light_sleep_start();   // sleep and wake up here */
}
void vol_setting() {
  if (vol_ok) {  // wait last req
    vol++;
    vol_ok = false;
    if (vol > 8) vol = 1; // turn around to support single button
    //radio.setVolume(vol);
  }
}
void vol_setting_2() { 
  if (vol_ok) {  // wait last req
    vol--;
    vol_ok = false;
    if (vol < 0) vol = 0; 
  }
}
void station_setting_2() {
  if (stn_ok) {  // wait last req
    stnIdx--;
    stn_ok = false;
    if (stnIdx < 0) stnIdx = 0;
  }
}
void station_setting() {
  if (stn_ok) {  // wait last req
    stnIdx++;
    stn_ok = false;
    if (stnIdx > MAXSTNIDX) stnIdx = 0;  // turn around to support single button
    //radio.setFrequency(stnFreq[stnIdx]);
    //lastfreq = stnFreq[stnIdx];
    //if (currIdx <= MAXSCEDIDX) entity[d_wday][currIdx].stime = 0; // reset  last schedule
  }
}
void power_onoff_setting() {
  if (p_onoff_req==false) {  // wait last req
     p_onoff_req = true;  // req
  }
}
