
#ifndef _TELNET_SERVER_H
#define _TELNET_SERVER_H

#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include "config.h"

// how many clients should be able to telnet
#define MAX_TELNET_CLIENTS 1
#define TELNET_RXBUFFERSIZE 1200
#define TELNET_SERVER_PORT 23

class Telnet_Server {
    public:
    Telnet_Server();
    ~Telnet_Server();
    bool begin();
    void end();
    size_t write(const uint8_t *buffer, size_t size);
    int read(void);
    int peek(void);
    int available();
    int get_rx_buffer_available();
    bool push (uint8_t data);
    bool push (const uint8_t * data, int datasize);
    void clearClients();
    static void handleNewClient(void* arg, AsyncClient *client);
    static void handleData(void* arg, AsyncClient* client, void *data, size_t len);
    static uint16_t port(){return _port;}
    private:
    static bool _setupdone;
    static AsyncServer *_telnetServer;
    static AsyncClient *_telnetClients[MAX_TELNET_CLIENTS];
    static uint16_t _port;
    uint32_t _lastflush;
    uint8_t _RXbuffer[TELNET_RXBUFFERSIZE];
    uint16_t _RXbufferSize;
    uint16_t _RXbufferpos;
};

extern Telnet_Server telnetServer;

#endif
