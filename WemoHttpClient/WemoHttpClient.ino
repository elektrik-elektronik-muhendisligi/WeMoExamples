/*
  WeMo HTTP Client
  Context: Arduino, with WINC1500 module

  based on WeMo HTTP client from Making Things Talk, 3rd edition
  by Tom Igoe 
*/
// include required libraries and config files
#include <SPI.h>
#include <WiFi101.h>          // use this for the MKR 1000
//#include <WiFiNINA.h>       // use this for the MKR 1010
//#include <ESP8266WiFi.h>    // use this for ESP8266 modules
#include <ArduinoHttpClient.h>
#include "arduino_secrets.h"

WiFiClient netSocket;               // network socket to device
const char wemo[] = "192.168.0.13"; // device address
int port = 49153;                   // port number
String route = "/upnp/control/basicevent1";  // API route
String soap;                        // string for the SOAP request
boolean wemoState = 1;              // whether the WeMo is on or off

void setup() {
  Serial.begin(9600);               // initialize serial communication
  // while you're not connected to a WiFi AP,
  while ( WiFi.status() != WL_CONNECTED) {
    Serial.print("Attempting to connect to Network named: ");
    Serial.println(SECRET_SSID);
    WiFi.begin(SECRET_SSID, SECRET_PASS);     // try to connect
    delay(2000);
  }

  // When you're connected, print out the device's network status:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
  // set up the SOAP request string. This formatting is just
  // for readability of the code:
  soap = "<?xml version=\"1.0\" encoding=\"utf-8\"?>";
  soap += "<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\"";
  soap += "s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">";
  soap += "<s:Body>";
  soap += "<u:SetBinaryState xmlns:u=\"urn:Belkin:service:basicevent:1\">";
  soap += "<BinaryState>1</BinaryState></u:SetBinaryState>";
  soap += "</s:Body></s:Envelope>";
}

void loop() {
  HttpClient http(netSocket, wemo, port); // make an HTTP client
  http.connectionKeepAlive();             // keep the connection alive
  http.beginRequest();                    // start assembling the request
  http.post(route);                       // set the route
  // add the headers:
  http.sendHeader("Content-type", "text/xml; charset=utf-8");
  String soapAction = "\"urn:Belkin:service:basicevent:1#SetBinaryState\"";
  http.sendHeader("SOAPACTION", soapAction);
  http.sendHeader("Connection: keep-alive");
  http.sendHeader("Content-Length", soap.length());
  http.endRequest();                      // end the request
  http.println(soap);                     // add the body
  Serial.println("request sent");

  while (http.connected()) {       // while connected to the server,
    if (http.available()) {        // if there is a response from the server,
      String result = http.readString();  // read it
      Serial.print(result);               // and print it
    }
  }
  Serial.println();             // end of the response
  if (wemoState == 1) {         // if the wemo's on
    soap.replace(">1<", ">0<"); // turn it off next time
  } else {                      // otherwise
    soap.replace(">0<", ">1<"); // turn it on next time
  }
  wemoState = !wemoState;       // toggle wemoState
  delay(5000);                  // wait 5 seconds
}
