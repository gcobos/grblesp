/*
  websocket.hpp - Take advantage of the Wifi connection by using websockets instead
  of a websocket connection
*/

#ifndef websocket_h
#define websocket_h

#include <ESPAsyncWebServer.h>

#define WEBSERVER_PORT 80
#define WEBSOCKET_NO_DATA 0xff

void websocket_init();

// Called on every event received by websocket
void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len);

// Writes one byte to the TX websocket buffer. Called by main program.
void websocket_write(uint8_t data);

// Fetches the first byte in the websocket read buffer. Called by main program.
uint8_t websocket_read();

// Reset and empty data in read buffer. Used by e-stop and reset.
void websocket_reset_read_buffer();

// Returns the number of bytes available in the RX websocket buffer.
uint8_t websocket_get_rx_buffer_available();

// Returns the number of bytes used in the RX websocket buffer.
// NOTE: Deprecated. Not used unless classic status reports are enabled in config.h.
uint8_t websocket_get_rx_buffer_count();

// Returns the number of bytes used in the TX websocket buffer.
// NOTE: Not used except for debugging and ensuring no TX bottlenecks.
uint8_t websocket_get_tx_buffer_count();

// Handles connections/disconnection events on the websocket server
void websocket_check_connection();

#endif
