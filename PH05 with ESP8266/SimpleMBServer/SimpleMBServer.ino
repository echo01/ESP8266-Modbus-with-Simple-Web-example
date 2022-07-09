#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ModbusMaster.h>

#ifndef STASSID

#define STASSID "NAME"
#define STAPSK  "PWS"
#endif


const char* ssid = STASSID;
const char* password = STAPSK;
// instantiate ModbusMaster object
ModbusMaster node;
uint16_t MBdata[64];
int expect_reciveByte;

typedef struct{
  unsigned int AO[8];
  unsigned int Output_type[8];
  unsigned int BaudRate;
  unsigned int Parity;
  unsigned int Stopbits;
  unsigned int DelayReply;
  unsigned int sVer;
  unsigned int n_request;
  unsigned int n_respond;
  unsigned int Id;
}ph05reg;

typedef struct{
  unsigned int DO[8];
  unsigned int DOreg;
  unsigned int BaudRate;
  unsigned int Parity;
  unsigned int Stopbits;
  unsigned int DelayReply;
  unsigned int sVer;
  unsigned int n_request;
  unsigned int n_respond;
  unsigned int Id;
}ph07reg;

ph05reg ph05_1;
ph07reg ph07_2;
ph07reg ph07_3;

const unsigned long timeIntervall = 1000; // 1 sec
unsigned long timeStamp = 0;
unsigned long time_tick =0;
unsigned int q_cmd;
unsigned int clr_first_loop=0;

ESP8266WebServer server(80);

const int led = 13;
#define MAX485_DE      16
#define MAX485_RE_NEG  16

int count_n=0;
char strhttp[256];
char sStr[2]={'"',0};
float V1,V2,V3,V12;

void preTransmission()
{
  digitalWrite(MAX485_RE_NEG, 1);
  digitalWrite(MAX485_DE, 1);
}

void postTransmission()
{
  digitalWrite(MAX485_RE_NEG, 0);
  digitalWrite(MAX485_DE, 0);
}

String SendHTML(uint16_t count_n,uint16_t *ptdata,uint8_t respond){
  char str[256];

  uint32_t buffData;
  
  String ptr = "<!DOCTYPE html>\n<html>\n";
  ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr +="<meta http-equiv=\"refresh\" content=\"3\">\n";
  ptr +="<link rel=\"stylesheet\" href=\"https://cdn.jsdelivr.net/npm/bootstrap@4.5.3/dist/css/bootstrap.min.css\" integrity=\"sha384-TX8t27EcRE3e/ihU7zmQxVncDAy5uIKz4rEkgIXeMed4M0jlfIDPvg6uqKI2xXr2\" crossorigin=\"anonymous\">\n";
  ptr +="<title>Simple Modbus Master</title>\n";
  ptr +="</head>\n";
  
  ptr +="<body>\n";
  ptr +="<h1 style=\"text-align: center\">ESP8266 Web Server</h1>\n";

  // Table 1 row 1  //
  ptr +="<div class=\"row\">";
  ptr +="<div class=\"col-lg-3 col-md-4 col-sm-6\" style=\"background-color:yellow; border: 1px solid\">";
  ptr +="<p>esp8266 test Modbus Master</p>\n";
  ptr +="</div>\n";
  ptr +="<div class=\"col-lg-3 col-md-4 col-sm-6\" style=\"background-color:yellow; border: 1px solid\">";
  sprintf(str,"<p>total request=%d</p>\n",ph05_1.n_request+ph07_2.n_request+ph07_3.n_request);
  ptr +=str;
  ptr +="</div>\n";
  ptr +="<div class=\"col-lg-3 col-md-4 col-sm-6\" style=\"background-color:yellow; border: 1px solid\">";
  sprintf(str,"<p>total receive=%d</p>\n",ph05_1.n_respond+ph07_2.n_respond+ph07_3.n_respond);
  ptr +=str;
  ptr +="</div>\n";
  ptr +="<div class=\"col-lg-3 col-md-4 col-sm-6\" style=\"background-color:yellow; border: 1px solid\">";
  ptr +="<p> none </p>\n";
  //if(respond==0)
  //  ptr +="<p>respond=OK</p>\n";
  //else
  //  { sprintf(str,"<p>respond=%d</p>\n",respond);
  //  ptr +=str;}
  ptr +="</div>\n";
  ptr +="</div>\n";

  // Table 1 row 2  //
  ptr +="<div class=\"row\">";
  ptr +="<div class=\"col-lg-1 col-md-1 col-sm-1\" style=\"background-color:#58D68D; border: 1px solid\">";
  ptr +="<p>Register read back</p>\n";
  ptr +="</div>\n";
  ptr +="<div class=\"col-lg-1 col-md-1 col-sm-1\" style=\"background-color:#58D68D; border: 1px solid\">";
  ptr +="<p>PH-05 request</p>\n";
  ptr +="</div>\n";
  ptr +="<div class=\"col-lg-1 col-md-1 col-sm-1\" style=\"background-color:#58D68D; border: 1px solid\">";
  ptr +="<p>PH-05 receive</p>\n";
  ptr +="</div>\n";
  ptr +="<div class=\"col-lg-1 col-md-1 col-sm-1\" style=\"background-color:#58D68D; border: 1px solid\">";
  ptr +="<p>PH-07_2 request</p>\n";
  ptr +="</div>\n";
  ptr +="<div class=\"col-lg-1 col-md-1 col-sm-1\" style=\"background-color:#58D68D; border: 1px solid\">";
  ptr +="<p>PH-07_2 receive</p>\n";
  ptr +="</div>\n";
  ptr +="<div class=\"col-lg-1 col-md-1 col-sm-1\" style=\"background-color:#58D68D; border: 1px solid\">";
  ptr +="<p>PH-07_3 request</p>\n";
  ptr +="</div>\n";
  ptr +="<div class=\"col-lg-1 col-md-1 col-sm-1\" style=\"background-color:#58D68D; border: 1px solid\">";
  ptr +="<p>PH-07_3 receive</p>\n";
  ptr +="</div>\n";
  ptr +="<div class=\"col-lg-1 col-md-1 col-sm-1\" style=\"background-color:#58D68D; border: 1px solid\">";
  ptr +="<p>none</p>\n";
  ptr +="</div>\n";
  ptr +="<div class=\"col-lg-1 col-md-1 col-sm-1\" style=\"background-color:#58D68D; border: 1px solid\">";
  ptr +="<p>none</p>\n";
  ptr +="</div>\n";
  ptr +="</div>\n";
  // Table 1 row 3  Modbus Respond raw data value //
  ptr +="<div class=\"row\">";
  ptr +="<div class=\"col-lg-1 col-md-1 col-sm-1\" style=\"background-color:#58D68D; border: 1px solid\">";
  ptr +="<p>raw data (Word)</p>\n";
  ptr +="</div>\n";
  // R0
  ptr +="<div class=\"col-lg-1 col-md-1 col-sm-1\" style=\"background-color:#58D68D; border: 1px solid\">";
  sprintf(str,"<p>%d</p>\n",ph05_1.n_request);
  ptr +=str;
  ptr +="</div>\n";
  // R1
  ptr +="<div class=\"col-lg-1 col-md-1 col-sm-1\" style=\"background-color:#58D68D; border: 1px solid\">";
  sprintf(str,"<p>%d</p>\n",ph05_1.n_respond);
  ptr +=str;
  ptr +="</div>\n";
  // R2
  ptr +="<div class=\"col-lg-1 col-md-1 col-sm-1\" style=\"background-color:#58D68D; border: 1px solid\">";
  sprintf(str,"<p>%d</p>\n",ph07_2.n_request);
  ptr +=str;
  ptr +="</div>\n";
  // R3
  ptr +="<div class=\"col-lg-1 col-md-1 col-sm-1\" style=\"background-color:#58D68D; border: 1px solid\">";
  sprintf(str,"<p>%d</p>\n",ph07_2.n_respond);
  ptr +=str;
  ptr +="</div>\n";
  // R4
  ptr +="<div class=\"col-lg-1 col-md-1 col-sm-1\" style=\"background-color:#58D68D; border: 1px solid\">";
  sprintf(str,"<p>%d</p>\n",ph07_3.n_request);
  ptr +=str;
  ptr +="</div>\n";
  // R5
  ptr +="<div class=\"col-lg-1 col-md-1 col-sm-1\" style=\"background-color:#58D68D; border: 1px solid\">";
  sprintf(str,"<p>%d</p>\n",ph07_3.n_respond);
  ptr +=str;
  ptr +="</div>\n";
  // R6
  ptr +="<div class=\"col-lg-1 col-md-1 col-sm-1\" style=\"background-color:#58D68D; border: 1px solid\">";
  sprintf(str,"<p>%d</p>\n",0);
  ptr +=str;
  ptr +="</div>\n";
  // R7
  ptr +="<div class=\"col-lg-1 col-md-1 col-sm-1\" style=\"background-color:#58D68D; border: 1px solid\">";
  sprintf(str,"<p>%d</p>\n",0);
  ptr +=str;
  ptr +="</div>\n";
  ptr +="</div>\n";


  ptr +="<p style=\"text-align: left\">Modbus libaries is ModbusMaster </p>\n";
  ptr +="<p style=\"text-align: left\">Author: Doc Walker </p>\n";
  ptr +="<p style=\"text-align: left\">Maintainer: Doc Walker </p>\n";
  ptr +="<p style=\"text-align: left\">Author: Doc Walker </p>\n";
  ptr +="<p style=\"text-align: left\">Author Demo: Nakorn Thr.</p>\n";
  ptr +="<a href=\"https://www.arduino.cc/reference/en/libraries/modbusmaster/\">libray link detail on arduino.cc</a>";
  ptr +="<p style=\".\n.\n.\n</p>\n";

  ptr +="<footer id=\"footer\">";
  ptr +="<p style=\"text-align: center font-size: 0.5rem;\">©demo 2022 Nakorn.Thr</p>";
  ptr +="</footer>";
  ptr +="</body>\n";
  ptr +="</html>\n";
  
  return ptr;
}

void init_mb_node()
{
  ph05_1.Id=1;
  ph05_1.n_request=0;
  ph05_1.n_respond=0;

  ph07_2.Id=2;
  ph07_2.n_request=0;
  ph07_2.n_respond=0;

  ph07_3.Id=3;
  ph07_3.n_request=0;
  ph07_3.n_respond=0;

  clr_first_loop=1;
}

void mb_run()
{
  uint8_t j, result;
  digitalWrite(led, 1);
  node.preTransmission(preTransmission);
  node.postTransmission(postTransmission);
  count_n++;
  //=====================Modbus master==============================//
  if(q_cmd++>5) 
    { 
    q_cmd=0;
    if(clr_first_loop)
      {
        clr_first_loop=0;
        ph05_1.n_request=0;
        ph05_1.n_respond=0;

        ph07_2.n_request=0;
        ph07_2.n_respond=0;

        ph07_3.n_request=0;
        ph07_3.n_respond=0;
      }
    }
  switch(q_cmd)
  {
    case 0:
    default:
      node.begin(ph05_1.Id, Serial);
      result = node.readHoldingRegisters(0, 8);
      ph05_1.n_request++;
      if (result == node.ku8MBSuccess)
        ph05_1.n_respond++;
      expect_reciveByte=16+5;
      break;
    case 1:
      node.begin(ph05_1.Id, Serial);
    // Setting reg. data for Transmit
      ph05_1.AO[0]+=1000;
      ph05_1.AO[1]+=2000;
      ph05_1.AO[2]+=5000;
      ph05_1.AO[3]+=8000;
      if(ph05_1.AO[0]>20000)  ph05_1.AO[0]=0;
      if(ph05_1.AO[1]>20000)  ph05_1.AO[1]=0;
      if(ph05_1.AO[2]>20000)  ph05_1.AO[2]=0;
      if(ph05_1.AO[3]>20000)  ph05_1.AO[3]=0;
    
    // Setting TransmitBuffer for cmd Mulitpreset register
      node.setTransmitBuffer(0,ph05_1.AO[0]);
      node.setTransmitBuffer(1,ph05_1.AO[1]);
      node.setTransmitBuffer(2,ph05_1.AO[2]);
      node.setTransmitBuffer(3,ph05_1.AO[3]);
    // send cmd
      result = node.writeMultipleRegisters(0, 4);

      ph05_1.n_request++;
      if (result == node.ku8MBSuccess)
        ph05_1.n_respond++;
      expect_reciveByte=8;
    break;
    case 2:
      node.begin(ph07_2.Id, Serial);
    // Callbacks allow us to configure the RS485 transceiver correctly
      result = node.readHoldingRegisters(0, 8);
      ph07_2.n_request++;
      if (result == node.ku8MBSuccess)
        ph07_2.n_respond++;
      expect_reciveByte=16+5;
    break;
    case 3:
      node.begin(ph07_2.Id, Serial);
      ph07_2.DO[0]^=0xFFFF;
      ph07_2.DO[1]^=0xFFFF;
      ph07_2.DO[2]^=0xFFFF;
      ph07_2.DO[3]^=0xFFFF;
    // Setting TransmitBuffer for Command Mulitpreset register
      node.setTransmitBuffer(0,ph07_2.DO[0]);
      node.setTransmitBuffer(1,ph07_2.DO[1]);
      node.setTransmitBuffer(2,ph07_2.DO[2]);
      node.setTransmitBuffer(3,ph07_2.DO[3]);
    // send cmd
      result = node.writeMultipleRegisters(0, 4);
      ph07_2.n_request++;
      if (result == node.ku8MBSuccess)
        ph07_2.n_respond++;
      expect_reciveByte=8;
    break;
    case 4:
      node.begin(ph07_3.Id, Serial);
      result = node.readHoldingRegisters(0, 8);
      ph07_3.n_request++;
      if (result == node.ku8MBSuccess)
        ph07_3.n_respond++;
      expect_reciveByte=16+5;
    break;
    case 5:
      node.begin(ph07_3.Id, Serial);
    //node.preTransmission(preTransmission);
    //node.postTransmission(postTransmission);
      ph07_3.DO[0]^=0xFFFF;
      ph07_3.DO[1]^=0xFFFF;
      ph07_3.DO[2]^=0xFFFF;
      ph07_3.DO[3]^=0xFFFF;
    // Setting TransmitBuffer for Command Mulitpreset register
      node.setTransmitBuffer(0,ph07_3.DO[0]);
      node.setTransmitBuffer(1,ph07_3.DO[1]);
      node.setTransmitBuffer(2,ph07_3.DO[2]);
      node.setTransmitBuffer(3,ph07_3.DO[3]);
    // Send Cmd
      result = node.writeMultipleRegisters(0, 4);
      ph07_3.n_request++;
      if (result == node.ku8MBSuccess)
        ph07_3.n_respond++;
      expect_reciveByte=8;
    break;
  }
  
 
  if (result == node.ku8MBSuccess)
  {
    for (j = 0; j < expect_reciveByte; j++)
    {
      MBdata[j] = node.getResponseBuffer(j);
    }
  }
}

void handleRoot() {
  
  //====================================================//
  server.send(200, "text/html", SendHTML(count_n,MBdata,0));
  digitalWrite(led, 0);
}

void handleNotFound() {
  digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  digitalWrite(led, 0);
}

void setup(void) {
  pinMode(led, OUTPUT);
  pinMode(MAX485_RE_NEG, OUTPUT);
  pinMode(MAX485_DE, OUTPUT);
  // Init in receive mode
  digitalWrite(MAX485_RE_NEG, 0);
  digitalWrite(MAX485_DE, 0);
  
  digitalWrite(led, 0);
  Serial.begin(9600);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("start Boot");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);

  server.on("/inline", []() {
    server.send(200, "text/plain", "this works as well");
  });

  server.on("/gif", []() {
    static const uint8_t gif[] PROGMEM = {
      0x47, 0x49, 0x46, 0x38, 0x37, 0x61, 0x10, 0x00, 0x10, 0x00, 0x80, 0x01,
      0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x2c, 0x00, 0x00, 0x00, 0x00,
      0x10, 0x00, 0x10, 0x00, 0x00, 0x02, 0x19, 0x8c, 0x8f, 0xa9, 0xcb, 0x9d,
      0x00, 0x5f, 0x74, 0xb4, 0x56, 0xb0, 0xb0, 0xd2, 0xf2, 0x35, 0x1e, 0x4c,
      0x0c, 0x24, 0x5a, 0xe6, 0x89, 0xa6, 0x4d, 0x01, 0x00, 0x3b
    };
    char gif_colored[sizeof(gif)];
    memcpy_P(gif_colored, gif, sizeof(gif));
    // Set the background to a random set of colors
    gif_colored[16] = millis() % 256;
    gif_colored[17] = millis() % 256;
    gif_colored[18] = millis() % 256;
    server.send(200, "image/gif", gif_colored, sizeof(gif_colored));
  });

  server.onNotFound(handleNotFound);

  /////////////////////////////////////////////////////////
  // Hook examples

  server.addHook([](const String & method, const String & url, WiFiClient * client, ESP8266WebServer::ContentTypeFunction contentType) {
    (void)method;      // GET, PUT, ...
    (void)url;         // example: /root/myfile.html
    (void)client;      // the webserver tcp client connection
    (void)contentType; // contentType(".html") => "text/html"
    //Serial.printf("A useless web hook has passed\n");
    //Serial.printf("(this hook is in 0x%08x area (401x=IRAM 402x=FLASH))\n", esp_get_program_counter());
    return ESP8266WebServer::CLIENT_REQUEST_CAN_CONTINUE;
  });

  server.addHook([](const String&, const String & url, WiFiClient*, ESP8266WebServer::ContentTypeFunction) {
    if (url.startsWith("/fail")) {
      Serial.printf("An always failing web hook has been triggered\n");
      return ESP8266WebServer::CLIENT_MUST_STOP;
    }
    return ESP8266WebServer::CLIENT_REQUEST_CAN_CONTINUE;
  });

  server.addHook([](const String&, const String & url, WiFiClient * client, ESP8266WebServer::ContentTypeFunction) {
    if (url.startsWith("/dump")) {
      Serial.printf("The dumper web hook is on the run\n");

      // Here the request is not interpreted, so we cannot for sure
      // swallow the exact amount matching the full request+content,
      // hence the tcp connection cannot be handled anymore by the
      // webserver.
#ifdef STREAMSEND_API
      // we are lucky
      client->sendAll(Serial, 500);
#else
      auto last = millis();
      while ((millis() - last) < 500) {
        char buf[32];
        size_t len = client->read((uint8_t*)buf, sizeof(buf));
        if (len > 0) {
          Serial.printf("(<%d> chars)", (int)len);
          Serial.write(buf, len);
          last = millis();
        }
      }
#endif
      // Two choices: return MUST STOP and webserver will close it
      //                       (we already have the example with '/fail' hook)
      // or                  IS GIVEN and webserver will forget it
      // trying with IS GIVEN and storing it on a dumb WiFiClient.
      // check the client connection: it should not immediately be closed
      // (make another '/dump' one to close the first)
      Serial.printf("\nTelling server to forget this connection\n");
      static WiFiClient forgetme = *client; // stop previous one if present and transfer client refcounter
      return ESP8266WebServer::CLIENT_IS_GIVEN;
    }
    return ESP8266WebServer::CLIENT_REQUEST_CAN_CONTINUE;
  });

  // Hook examples
  /////////////////////////////////////////////////////////
  init_mb_node();

  server.begin();
  Serial.println("HTTP server started");
}

void loop(void) {
  //ArduinoOTA.handle();
  server.handleClient();
  MDNS.update();
  if(millis() - timeStamp > timeIntervall/10 ){
   mb_run();
   timeStamp = millis();  // reset the timer
   }
}
