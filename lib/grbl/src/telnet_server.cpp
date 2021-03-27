

#include "config.h"
#include "telnet_server.hpp"
#include "grbl.hpp"
#include "report.hpp"

Telnet_Server telnetServer;
bool Telnet_Server::_setupdone = false;
uint16_t Telnet_Server::_port = 0;
AsyncServer *Telnet_Server::_telnetServer = NULL;
AsyncClient Telnet_Server::_telnetClients[MAX_TELNET_CLIENTS] = {NULL};
#ifdef ENABLE_TELNET_WELCOME_MSG
IPAddress Telnet_Server::_telnetClientsIP[MAX_TELNET_CLIENTS];
#endif

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

void Telnet_Server::clearClients()
{
  //check if there are any new clients
  uint8_t i;
  for(i = 0; i < MAX_TELNET_CLIENTS; i++) {
    // find free/disconnected spot
    if (_telnetClients[i] != NULL || !_telnetClients[i].connected()) {
#ifdef ENABLE_TELNET_WELCOME_MSG
      _telnetClientsIP[i] = IPAddress(0, 0, 0, 0);
#endif
      if(_telnetClients[i] != NULL) _telnetClients[i].stop();
      _telnetClients[i] = NULL;
      break;
    }
  }
  if (i >= MAX_TELNET_CLIENTS) {
    //no free/disconnected spot so reject
    _telnetServer->end();
  }
}

size_t Telnet_Server::write(const uint8_t *buffer, size_t size){

    size_t wsize = 0;
    if ( !_setupdone || _telnetServer == NULL) {
        return 0;
        }
    clearClients();
    //log_d("[TELNET out]");
    //push UART data to all connected telnet clients
    for(uint8_t i = 0; i < MAX_TELNET_CLIENTS; i++){
        if (_telnetClients[i] != NULL && _telnetClients[i].connected()){
            //log_d("[TELNET out connected]");
          wsize = _telnetClients[i].write((const char*)buffer, size);
        }
    }
    return wsize;
}

/**************************/
static void handleError(void* arg, AsyncClient* client, int8_t error) {
	Serial.printf("\n connection error %s from client %s \n", client->errorToString(error), client->remoteIP().toString().c_str());
}

static void handleData(void* arg, AsyncClient* client, void *data, size_t len) {
	Serial.printf("\n data received from client %s \n", client->remoteIP().toString().c_str());
	Serial.write((uint8_t*)data, len);

	// reply to client
	if (client->space() > 32 && client->canSend()) {
		char reply[32];
		sprintf(reply, "this is from the server");
		client->add(reply, strlen(reply));
		client->send();
	}
}

static void handleDisconnect(void* arg, AsyncClient* client) {
	Serial.printf("\n client %s disconnected \n", client->remoteIP().toString().c_str());
}

static void handleTimeOut(void* arg, AsyncClient* client, uint32_t time) {
	Serial.printf("\n client ACK timeout ip: %s \n", client->remoteIP().toString().c_str());
}


/**************************/

void Telnet_Server::handleNewClient(void* arg, AsyncClient* client)
{
  Serial.printf("\n new client has been connected to server, ip: %s", client->remoteIP().toString().c_str());

  client->onData(&handleData, NULL);
  client->onError(&handleError, NULL);
  client->onDisconnect(&handleDisconnect, NULL);
  client->onTimeout(&handleTimeOut, NULL);

  uint8_t i;
  for (i = 0; i < MAX_TELNET_CLIENTS; i++) {
    //find free/disconnected spot
    if (!_telnetClients[i].connected()) {
    #ifdef ENABLE_TELNET_WELCOME_MSG
      _telnetClientsIP[i] = IPAddress(0, 0, 0, 0);
    #endif
      _telnetClients[i].stop();
      Telnet_Server::_telnetClients[i] = *client;
      break;
    }
  }
  if (i >= MAX_TELNET_CLIENTS) {
    //no free/disconnected spot so reject
    client->stop();
  }
}

int Telnet_Server::peek(void){
    Serial.printf("TELNET PEEK");
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
    Serial.println("[TELNET]push ");
    Serial.println(data);
    if ((1 + _RXbufferSize) <= TELNET_RXBUFFERSIZE){
        int current = _RXbufferpos + _RXbufferSize;
        if (current > TELNET_RXBUFFERSIZE) current = current - TELNET_RXBUFFERSIZE;
        if (current > (TELNET_RXBUFFERSIZE-1)) current = 0;
        _RXbuffer[current] = data;
        _RXbufferSize++;
        Serial.print("[TELNET]buffer size ");
        Serial.print(_RXbufferSize);
       return true;
    }
    return false;
}

void Telnet_Server::handle()
{
  //check if can read
  if ( !_setupdone || _telnetServer == NULL) {
    return;
  }
  //Serial.println("[TELNET]HANDLE ");
  clearClients();
  //check clients for data
  //uint8_t c;
  for(uint8_t i = 0; i < MAX_TELNET_CLIENTS; i++){
    if (_telnetClients[i] != NULL && _telnetClients[i].connected()){
#ifdef ENABLE_TELNET_WELCOME_MSG
      if (_telnetClientsIP[i] != _telnetClients[i].remoteIP()){
        report_init_message(CLIENT_TELNET);
        _telnetClientsIP[i] = _telnetClients[i].remoteIP();
      }
#endif
      if(_telnetClients[i] != NULL) {
        uint8_t buf[1024];
        int readlen = _telnetClients[i].getNoDelay();
        int writelen = TELNET_RXBUFFERSIZE - available();
        if (readlen > 1024) readlen = 1024;
        if (readlen > writelen) readlen = writelen;
        if (readlen > 0) {
          //_telnetClients[i].read(buf, readlen);
          push(buf, readlen);
        }
        return;
      }
    } else {
      if (_telnetClients[i] != NULL) {
#ifdef ENABLE_TELNET_WELCOME_MSG
        _telnetClientsIP[i] = IPAddress(0, 0, 0, 0);
#endif
        _telnetClients[i].stop();
      }
    }
  }
}

bool Telnet_Server::push (const uint8_t * data, int data_size){
    if ((data_size + _RXbufferSize) <= TELNET_RXBUFFERSIZE){
        int data_processed = 0;
        int current = _RXbufferpos + _RXbufferSize;
        if (current > TELNET_RXBUFFERSIZE) current = current - TELNET_RXBUFFERSIZE;
        for (int i = 0; i < data_size; i++){
          if (current > (TELNET_RXBUFFERSIZE-1)) current = 0;
          if (char(data[i]) != '\r') {
            _RXbuffer[current] = data[i];
            current ++;
            data_processed++;
          }
          delay(0);
        }
        _RXbufferSize+=data_processed;
        return true;
    }
    return false;
}

int Telnet_Server::read(void){
    Serial.printf("\nTELNET READ\n");
    if (_RXbufferSize > 0) {
        int v = _RXbuffer[_RXbufferpos];
        Serial.print("[TELNET]read ");
        Serial.println(char(v));
        _RXbufferpos++;
        if (_RXbufferpos > (TELNET_RXBUFFERSIZE-1))_RXbufferpos = 0;
        _RXbufferSize--;
        return v;
    } else return -1;
}


 /* clients events
static void handleError(void* arg, AsyncClient* client, int8_t error) {
	Serial.printf("\n connection error %s from client %s \n", client->errorToString(error), client->remoteIP().toString().c_str());
}

static void handleData(void* arg, AsyncClient* client, void *data, size_t len) {
	Serial.printf("\n data received from client %s \n", client->remoteIP().toString().c_str());
	Serial.write((uint8_t*)data, len);

	// reply to client
	if (client->space() > 32 && client->canSend()) {
		char reply[32];
		sprintf(reply, "this is from the server");
		client->add(reply, strlen(reply));
		client->send();
	}
}

static void handleDisconnect(void* arg, AsyncClient* client) {
	Serial.printf("\n client %s disconnected \n", client->remoteIP().toString().c_str());
}

static void handleTimeOut(void* arg, AsyncClient* client, uint32_t time) {
	Serial.printf("\n client ACK timeout ip: %s \n", client->remoteIP().toString().c_str());
}
*/
