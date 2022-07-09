#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ModbusMaster.h>

#ifndef STASSID
#define STASSID "NAME"
#define STAPSK  "PWS"
#endif
#define MbSlaveID 1

const char* ssid = STASSID;
const char* password = STAPSK;
// instantiate ModbusMaster object
ModbusMaster node;

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
  if(respond==0)    // Slave respond OK
  {
  //=======  Calculation from register    ====///
    buffData=*(ptdata);
    buffData=buffData<<16;
    buffData+=*(ptdata+1);
    V1=buffData;
    V1/=10;

    buffData=*(ptdata+2);
    buffData=buffData<<16;
    buffData+=*(ptdata+3);
    V2=buffData;
    V2/=10;

    buffData=*(ptdata+4);
    buffData=buffData<<16;
    buffData+=*(ptdata+5);
    V3=buffData;
    V3/=10;

    buffData=*(ptdata+6);
    buffData=buffData<<16;
    buffData+=*(ptdata+7);
    V12=buffData;
    V12/=10;
  }
  else
  {
    V1=V2=V3=V12=0;
  }
  String ptr = "<!DOCTYPE html>\n<html>\n";
  ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  
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
  sprintf(str,"<p>Count=%d</p>\n",count_n);
  ptr +=str;
  ptr +="</div>\n";
  ptr +="<div class=\"col-lg-3 col-md-4 col-sm-6\" style=\"background-color:yellow; border: 1px solid\">";
  sprintf(str,"<p>Slave ID=%d</p>\n",MbSlaveID);
  ptr +=str;
  ptr +="</div>\n";
  ptr +="<div class=\"col-lg-3 col-md-4 col-sm-6\" style=\"background-color:yellow; border: 1px solid\">";
  if(respond==0)
    ptr +="<p>respond=OK</p>\n";
  else
    { sprintf(str,"<p>respond=%d</p>\n",respond);
    ptr +=str;}
  ptr +="</div>\n";
  ptr +="</div>\n";

  // Table 1 row 2  //
  ptr +="<div class=\"row\">";
  ptr +="<div class=\"col-lg-1 col-md-1 col-sm-1\" style=\"background-color:#58D68D; border: 1px solid\">";
  ptr +="<p>Register read back</p>\n";
  ptr +="</div>\n";
  ptr +="<div class=\"col-lg-1 col-md-1 col-sm-1\" style=\"background-color:#58D68D; border: 1px solid\">";
  ptr +="<p>Reg 0</p>\n";
  ptr +="</div>\n";
  ptr +="<div class=\"col-lg-1 col-md-1 col-sm-1\" style=\"background-color:#58D68D; border: 1px solid\">";
  ptr +="<p>Reg 1</p>\n";
  ptr +="</div>\n";
  ptr +="<div class=\"col-lg-1 col-md-1 col-sm-1\" style=\"background-color:#58D68D; border: 1px solid\">";
  ptr +="<p>Reg 2</p>\n";
  ptr +="</div>\n";
  ptr +="<div class=\"col-lg-1 col-md-1 col-sm-1\" style=\"background-color:#58D68D; border: 1px solid\">";
  ptr +="<p>Reg 3</p>\n";
  ptr +="</div>\n";
  ptr +="<div class=\"col-lg-1 col-md-1 col-sm-1\" style=\"background-color:#58D68D; border: 1px solid\">";
  ptr +="<p>Reg 4</p>\n";
  ptr +="</div>\n";
  ptr +="<div class=\"col-lg-1 col-md-1 col-sm-1\" style=\"background-color:#58D68D; border: 1px solid\">";
  ptr +="<p>Reg 5</p>\n";
  ptr +="</div>\n";
  ptr +="<div class=\"col-lg-1 col-md-1 col-sm-1\" style=\"background-color:#58D68D; border: 1px solid\">";
  ptr +="<p>Reg 6</p>\n";
  ptr +="</div>\n";
  ptr +="<div class=\"col-lg-1 col-md-1 col-sm-1\" style=\"background-color:#58D68D; border: 1px solid\">";
  ptr +="<p>Reg 7</p>\n";
  ptr +="</div>\n";
  ptr +="</div>\n";
  // Table 1 row 3  Modbus Respond raw data value //
  ptr +="<div class=\"row\">";
  ptr +="<div class=\"col-lg-1 col-md-1 col-sm-1\" style=\"background-color:#58D68D; border: 1px solid\">";
  ptr +="<p>raw data (Word)</p>\n";
  ptr +="</div>\n";
  // R0
  ptr +="<div class=\"col-lg-1 col-md-1 col-sm-1\" style=\"background-color:#58D68D; border: 1px solid\">";
  sprintf(str,"<p>%d</p>\n",*ptdata);
  ptr +=str;
  ptr +="</div>\n";
  // R1
  ptr +="<div class=\"col-lg-1 col-md-1 col-sm-1\" style=\"background-color:#58D68D; border: 1px solid\">";
  sprintf(str,"<p>%d</p>\n",*(ptdata+1));
  ptr +=str;
  ptr +="</div>\n";
  // R2
  ptr +="<div class=\"col-lg-1 col-md-1 col-sm-1\" style=\"background-color:#58D68D; border: 1px solid\">";
  sprintf(str,"<p>%d</p>\n",*(ptdata+2));
  ptr +=str;
  ptr +="</div>\n";
  // R3
  ptr +="<div class=\"col-lg-1 col-md-1 col-sm-1\" style=\"background-color:#58D68D; border: 1px solid\">";
  sprintf(str,"<p>%d</p>\n",*(ptdata+3));
  ptr +=str;
  ptr +="</div>\n";
  // R4
  ptr +="<div class=\"col-lg-1 col-md-1 col-sm-1\" style=\"background-color:#58D68D; border: 1px solid\">";
  sprintf(str,"<p>%d</p>\n",*(ptdata+4));
  ptr +=str;
  ptr +="</div>\n";
  // R5
  ptr +="<div class=\"col-lg-1 col-md-1 col-sm-1\" style=\"background-color:#58D68D; border: 1px solid\">";
  sprintf(str,"<p>%d</p>\n",*(ptdata+5));
  ptr +=str;
  ptr +="</div>\n";
  // R6
  ptr +="<div class=\"col-lg-1 col-md-1 col-sm-1\" style=\"background-color:#58D68D; border: 1px solid\">";
  sprintf(str,"<p>%d</p>\n",*(ptdata+6));
  ptr +=str;
  ptr +="</div>\n";
  // R7
  ptr +="<div class=\"col-lg-1 col-md-1 col-sm-1\" style=\"background-color:#58D68D; border: 1px solid\">";
  sprintf(str,"<p>%d</p>\n",*(ptdata+7));
  ptr +=str;
  ptr +="</div>\n";
  ptr +="</div>\n";

  // Table 1 row 4  Modbus Converted value //
  ptr +="<div class=\"row\">";
  ptr +="<div class=\"col-lg-1 col-md-1 col-sm-1\" style=\"background-color:#AED6F1; border: 1px solid\">";
  ptr +="<p>Converted Value</p>\n";
  ptr +="</div>\n";
  // R0
  ptr +="<div class=\"col-lg-1 col-md-1 col-sm-1\" style=\"background-color:#AED6F1; border: 1px solid\">";
  sprintf(str,"<p>V1=%0.1f</p>\n",V1);
  ptr +=str;
  ptr +="</div>\n";
  ptr +="<div class=\"col-lg-1 col-md-1 col-sm-1\" style=\"background-color:#AED6F1; border: 1px solid\">";
  sprintf(str,"<p>V2=%0.1f</p>\n",V2);
  ptr +=str;
  ptr +="</div>\n";
  ptr +="<div class=\"col-lg-1 col-md-1 col-sm-1\" style=\"background-color:#AED6F1; border: 1px solid\">";
  sprintf(str,"<p>V3=%0.1f</p>\n",V3);
  ptr +=str;
  ptr +="</div>\n";
  ptr +="<div class=\"col-lg-1 col-md-1 col-sm-1\" style=\"background-color:#AED6F1; border: 1px solid\">";
  sprintf(str,"<p>V1-2=%0.1f</p>\n",V12);
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

void handleRoot() {
  uint16_t MBdata[10];
  uint8_t j, result;
  digitalWrite(led, 1);
  count_n++;
  //=====================Modbus master==============================//
  node.begin(MbSlaveID, Serial);
  // Callbacks allow us to configure the RS485 transceiver correctly
  node.preTransmission(preTransmission);
  node.postTransmission(postTransmission);

 
  result = node.readHoldingRegisters(6, 10);
  if (result == node.ku8MBSuccess)
  {
    for (j = 0; j < 10; j++)
    {
      MBdata[j] = node.getResponseBuffer(j);
    }
  }
  //====================================================//
  server.send(200, "text/html", SendHTML(count_n,MBdata,result));
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
  Serial.begin(19200);
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
  
  server.begin();
  Serial.println("HTTP server started");
}

void loop(void) {
  server.handleClient();
  MDNS.update();
}
