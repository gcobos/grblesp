/*
  websocket.cpp - Take advantage of the Wifi connection by using websockets instead
  of a serial connection
*/
#include "grbl.hpp"

#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESP8266mDNS.h>

AsyncWebServer webSocketServer(WEBSERVER_PORT);
AsyncWebSocket ws("/ws");
//AsyncEventSource events("/events");

void websocket_init()
{
  WiFi.hostname(WIFI_HOSTNAME);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  Serial.println(WiFi.localIP());
  MDNS.addService("http", "tcp", WEBSERVER_PORT);
  ws.onEvent(onWsEvent);
  webSocketServer.addHandler(&ws);
  webSocketServer.begin();
  Serial2Socket.attachWS(&ws);
}

void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len){
  if(type == WS_EVT_CONNECT){
    //Serial.printf("ws[%s][%u] connect\n", server->url(), client->id());
    //client->printf("Hello Client %u :)", client->id());
    //client->ping();
  } else if(type == WS_EVT_DISCONNECT){
    //Serial.printf("ws[%s][%u] disconnect: %u\n", server->url(), client->id());
  } else if(type == WS_EVT_ERROR){
    //Serial.printf("ws[%s][%u] error(%u): %s\n", server->url(), client->id(), *((uint16_t*)arg), (char*)data);
  } else if(type == WS_EVT_PONG){
    //Serial.printf("ws[%s][%u] pong[%u]: %s\n", server->url(), client->id(), len, (len)?(char*)data:"");
  } else if(type == WS_EVT_DATA){
    AwsFrameInfo * info = (AwsFrameInfo*)arg;
    String msg = "";
    if (info->final && info->index == 0 && info->len == len){
      //the whole message is in a single frame and we got all of it's data
      for(size_t i=0; i < info->len; i++) {
        msg += (char)data[i];
      }
      msg += '\n';
      Serial2Socket.push(msg.c_str());
    }
  }
}
