/*
  wifi.cpp - Handles connection to wifi and makes web, websockets and telnet
  connections available
*/

#include <ESP8266WiFi.h>
//#include <ESP8266mDNS.h>
#include "credentials.hpp"
#include "report.hpp"
#include "config.hpp"
#ifdef ENABLE_WEBSOCKET
  #include "websocket.hpp"
  #include "serial2socket.hpp"
#endif
#ifdef ENABLE_TELNET
  #include "telnet_server.hpp"
#endif

void wifi_init()
{
  WiFi.mode(WIFI_STA);
  WiFi.hostname(WIFI_HOSTNAME);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  Serial.print("\n[Connected: ");
  Serial.print(WiFi.localIP());
  Serial.println("]");

  //MDNS.addService("http", "tcp", WEBSOCKET_SERVER_PORT);

#ifdef ENABLE_TELNET
  telnetServer.begin();
#endif
#ifdef ENABLE_WEBSOCKET
	websocket_init();
#endif
  //be sure we are not is mixed mode in setup
  WiFi.scanNetworks(true);
}

void wifi_handle()
{
  if (WiFi.getMode() == WIFI_AP_STA) {
    if (WiFi.scanComplete() != WIFI_SCAN_RUNNING) {
      WiFi.enableSTA(false);
    }
  }
#ifdef ENABLE_OTA
  ArduinoOTA.handle();
#endif
#ifdef ENABLE_HTTP
  web_server.handle();
#endif
#ifdef ENABLE_TELNET
  telnetServer.handle();
#endif
}
