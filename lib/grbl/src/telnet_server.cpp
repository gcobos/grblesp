

#include "config.h"
#include "telnet_server.hpp"
#include "grbl.hpp"
#include "report.hpp"

Telnet_Server telnetServer;
bool Telnet_Server::_setupdone = false;
uint16_t Telnet_Server::_port = 0;
AsyncServer *Telnet_Server::_telnetServer = NULL;
AsyncClient *Telnet_Server::_telnetClients[MAX_TELNET_CLIENTS] = {NULL};

Telnet_Server::Telnet_Server(){
  _RXbufferSize = 0;
  _RXbufferpos = 0;
}
Telnet_Server::~Telnet_Server(){
  end();
}

bool Telnet_Server::begin(){
  bool no_error = true;
  end();
  _RXbufferSize = 0;
  _RXbufferpos = 0;
#ifdef ENABLE_TELNET
  int8_t penabled = 1;
#else
  int8_t penabled = 0;
#endif
  _port = TELNET_SERVER_PORT;

  if (penabled == 0) return false;
  _telnetServer = new AsyncServer(_port);
  _telnetServer->setNoDelay(true);
  String s = "[MSG:TELNET Started " + String(_port) + "]\r\n";
  grbl_send(CLIENT_ALL,(char *)s.c_str());

  //start telnet server
  _telnetServer->onClient(&Telnet_Server::handleNewClient, _telnetServer);
  _telnetServer->begin();
  _setupdone = true;
  return no_error;
}

void Telnet_Server::end(){
  _setupdone = false;
  _RXbufferSize = 0;
  _RXbufferpos = 0;
  if (_telnetServer) {
    delete _telnetServer;
    _telnetServer = NULL;
  }
}

size_t Telnet_Server::write(const uint8_t *buffer, size_t size){
    size_t wsize = 0;
    if ( !_setupdone || _telnetServer == NULL) {
        return 0;
    }
    //push UART data to all connected telnet clients
    for(uint8_t i = 0; i < MAX_TELNET_CLIENTS; i++){
        if (_telnetClients[i] != NULL && _telnetClients[i]->connected()){
          wsize = _telnetClients[i]->write((const char*)buffer, size);
          delay(0);
        }
    }
    return wsize;
}

static void handleError(void* arg, AsyncClient* client, int8_t error) {
	Serial.printf("\n connection error %s from client %s \n", client->errorToString(error), client->remoteIP().toString().c_str());
}

void Telnet_Server::handleData(void* arg, AsyncClient* client, void *data, size_t len) {
  telnetServer.push((uint8_t *)data, len);
}

static void handleDisconnect(void* arg, AsyncClient* client) {
	Serial.printf("\n client %s disconnected \n", client->remoteIP().toString().c_str());
}

static void handleTimeOut(void* arg, AsyncClient* client, uint32_t time) {
	Serial.printf("\n client ACK timeout ip: %s \n", client->remoteIP().toString().c_str());
}

void Telnet_Server::handleNewClient(void* arg, AsyncClient* client)
{
  client->onData(&handleData, NULL);
  client->onError(&handleError, NULL);
  client->onDisconnect(&handleDisconnect, NULL);
  client->onTimeout(&handleTimeOut, NULL);

  uint8_t i = 0;
  for (i = 0; i < MAX_TELNET_CLIENTS; i++) {
    //find free/disconnected spot
    if (_telnetClients[i] == NULL || !_telnetClients[i]->connected()){
      _telnetClients[i] = client;
      break;
    }
  }

  if (i >= MAX_TELNET_CLIENTS) {
    //no free/disconnected spot so reject
    client->stop();
  }
}

int Telnet_Server::peek(void){
    //Serial.printf("TELNET PEEK");
    if (_RXbufferSize > 0)return _RXbuffer[_RXbufferpos];
    else return -1;
}

int Telnet_Server::available(){
    return _RXbufferSize;
}

int Telnet_Server::get_rx_buffer_available(){
    return TELNET_RXBUFFERSIZE - _RXbufferSize;
}

bool Telnet_Server::push (uint8_t data){
    if ((1 + _RXbufferSize) <= TELNET_RXBUFFERSIZE){
        int current = _RXbufferpos + _RXbufferSize;
        if (current > TELNET_RXBUFFERSIZE) current = current - TELNET_RXBUFFERSIZE;
        if (current > (TELNET_RXBUFFERSIZE-1)) current = 0;
        _RXbuffer[current] = data;
        _RXbufferSize++;
       return true;
    }
    return false;
}

bool Telnet_Server::push (const uint8_t * data, int data_size){
    if ((data_size + _RXbufferSize) <= TELNET_RXBUFFERSIZE) {
        int data_processed = 0;
        int current = _RXbufferpos + _RXbufferSize;
        if (current > TELNET_RXBUFFERSIZE) current = current - TELNET_RXBUFFERSIZE;
        for (int i = 0; i < data_size; i++){
          if (current > (TELNET_RXBUFFERSIZE-1)) current = 0;
          if (char(data[i]) != '\r' && data[i] != 0) {
            _RXbuffer[current] = data[i];
            current++;
            data_processed++;
          }
        }
        _RXbufferSize+=data_processed;
        return true;
    }
    return false;
}

int Telnet_Server::read(void){
    if (_RXbufferSize > 0) {
        int v = _RXbuffer[_RXbufferpos];
        _RXbufferpos++;
        if (_RXbufferpos > (TELNET_RXBUFFERSIZE-1))_RXbufferpos = 0;
        _RXbufferSize--;
        return v;
    } else return -1;
}
