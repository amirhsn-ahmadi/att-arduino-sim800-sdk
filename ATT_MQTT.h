// The MIT License (MIT)
//
// Copyright (c) 2015 Adafruit Industries
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
#ifndef _ADAFRUIT_MQTT_H_
#define _ADAFRUIT_MQTT_H_

#include "Arduino.h"

#if defined(ARDUINO_SAMD_ZERO) || defined(ARDUINO_STM32_FEATHER)
#define strncpy_P(dest, src, len) strncpy((dest), (src), (len))
#define strncasecmp_P(f1, f2, len) strncasecmp((f1), (f2), (len))
#endif

#define ADAFRUIT_MQTT_VERSION_MAJOR 0
#define ADAFRUIT_MQTT_VERSION_MINOR 16
#define ADAFRUIT_MQTT_VERSION_PATCH 2

#include "includes/platform/att_debug.h"

// Use 3 (MQTT 3.0) or 4 (MQTT 3.1.1)
#define MQTT_PROTOCOL_LEVEL 4

#define MQTT_CTRL_CONNECT     0x1
#define MQTT_CTRL_CONNECTACK  0x2
#define MQTT_CTRL_PUBLISH     0x3
#define MQTT_CTRL_PUBACK      0x4
#define MQTT_CTRL_PUBREC      0x5
#define MQTT_CTRL_PUBREL      0x6
#define MQTT_CTRL_PUBCOMP     0x7
#define MQTT_CTRL_SUBSCRIBE   0x8
#define MQTT_CTRL_SUBACK      0x9
#define MQTT_CTRL_UNSUBSCRIBE 0xA
#define MQTT_CTRL_UNSUBACK    0xB
#define MQTT_CTRL_PINGREQ     0xC
#define MQTT_CTRL_PINGRESP    0xD
#define MQTT_CTRL_DISCONNECT  0xE

#define MQTT_QOS_1 0x1
#define MQTT_QOS_0 0x0

#define CONNECT_TIMEOUT_MS 6000
#define PUBLISH_TIMEOUT_MS 500
#define PING_TIMEOUT_MS    500
#define SUBACK_TIMEOUT_MS  500

// Adjust as necessary, in seconds.  Default to 5 minutes.
#define MQTT_CONN_KEEPALIVE 300

// Largest full packet we're able to send.
// Need to be able to store at least ~90 chars for a connect packet with full
// 23 char client ID.
#define MAXBUFFERSIZE (150)

#define MQTT_CONN_USERNAMEFLAG    0x80
#define MQTT_CONN_PASSWORDFLAG    0x40
#define MQTT_CONN_WILLRETAIN      0x20
#define MQTT_CONN_WILLQOS_1       0x08
#define MQTT_CONN_WILLQOS_2       0x18
#define MQTT_CONN_WILLFLAG        0x04
#define MQTT_CONN_CLEANSESSION    0x02

// how many subscriptions we want to be able to track
#define MAXSUBSCRIPTIONS 5

// how much data we save in a subscription object
// eg max-subscription-payload-size
#if defined  (__AVR_ATmega32U4__) || defined(__AVR_ATmega328P__)
  #define SUBSCRIPTIONDATALEN 20
#else
  #define SUBSCRIPTIONDATALEN 100
#endif

#define MQTT_CALLBACK_SIGNATURE void (*callback)(const char*,const char*,unsigned int)



extern void printBuffer(uint8_t *buffer, uint16_t len);


class ATT_MQTT {
 public:
  ATT_MQTT(const char *server,
                uint16_t port,
                const char *cid,
                const char *user,
                const char *pass);

  ATT_MQTT(const char *server,
                uint16_t port,
                const char *user = "",
                const char *pass = "");
  virtual ~ATT_MQTT() {}

  // Connect to the MQTT server.  Returns 0 on success, otherwise an error code
  // that indicates something went wrong:
  //   -1 = Error connecting to server
  //    1 = Wrong protocol
  //    2 = ID rejected
  //    3 = Server unavailable
  //    4 = Bad username or password
  //    5 = Not authenticated
  //    6 = Failed to subscribe
  // Use connectErrorString() to get a printable string version of the
  // error.
  int8_t connect();
  int8_t connect(const char *user, const char *pass);

  //set the callback for receiving mqtt messages
  void setCallback(MQTT_CALLBACK_SIGNATURE);
   
   //store the username and password
   void setCredentials(const char* cid, const char* user, const char* pwd);
  
  // Return a printable string version of the error code returned by
  // connect(). This returns a __FlashStringHelper*, which points to a
  // string stored in flash, but can be directly passed to e.g.
  // Serial.println without any further processing.
  const __FlashStringHelper* connectErrorString(int8_t code);

  // Sends MQTT disconnect packet and calls disconnectServer()
  bool disconnect();

  // Return true if connected to the MQTT server, otherwise false.
  virtual bool connected() = 0;  // Subclasses need to fill this in!

  // Set MQTT last will topic, payload, QOS, and retain. This needs
  // to be called before connect() because it is sent as part of the
  // connect control packet.
  bool will(const char *topic, const char *payload, uint8_t qos = 0, uint8_t retain = 0);

  // Publish a message to a topic using the specified QoS level.  Returns true
  // if the message was published, false otherwise.
  bool publish(const char *topic, const char *payload, uint8_t qos = 0);
  bool publish(const char *topic, uint8_t *payload, uint16_t bLen, uint8_t qos = 0);

  // Add a subscription to receive messages for a topic.  Returns true if the
  // subscription could be added or was already present, false otherwise.
  // Must be called before connect(), subscribing after the connection
  // is made is not currently supported.
  bool subscribe(const char* topic, uint8_t qos);

  // Unsubscribe from a previously subscribed MQTT topic.
  bool unsubscribe(const char* topic, uint8_t qos);
    
  

  // Check if any subscriptions have new messages.  Will return a reference to
  // an ATT_MQTT_Subscribe object which has a new message.  Should be called
  // in the sketch's loop function to ensure new messages are recevied.  Note
  // that subscribe should be called first for each topic that receives messages!
  bool readSubscription(char* topic,  char* data, unsigned int& datalen, int16_t timeout=0);

  void processPackets(int16_t timeout);

  // Ping the server to ensure the connection is still alive.
  bool ping(uint8_t n = 1);

 protected:
  // Interface that subclasses need to implement:

  // Connect to the server and return true if successful, false otherwise.
  virtual bool connectServer() = 0;

  // Disconnect from the MQTT server.  Returns true if disconnected, false otherwise.
  virtual bool disconnectServer() = 0;  // Subclasses need to fill this in!

  // Send data to the server specified by the buffer and length of data.
  virtual bool sendPacket(uint8_t *buffer, uint16_t len) = 0;

  // Read MQTT packet from the server.  Will read up to maxlen bytes and store
  // the data in the provided buffer.  Waits up to the specified timeout (in
  // milliseconds) for data to be available. 
  virtual uint16_t readPacket(uint8_t *buffer, uint16_t maxlen, int16_t timeout) = 0;

  // Read a full packet, keeping note of the correct length
  uint16_t readFullPacket(uint8_t *buffer, uint16_t maxsize, uint16_t timeout);
  // Properly process packets until you get to one you want
  uint16_t processPacketsUntil(uint8_t *buffer, uint8_t waitforpackettype, uint16_t timeout);

  // Shared state that subclasses can use:
  const char *servername;
  int16_t portnum;
  const char *clientid;
  const char *username;
  const char *password;
  const char *will_topic;
  const char *will_payload;
  uint8_t will_qos;
  uint8_t will_retain;
  uint8_t buffer[MAXBUFFERSIZE];  // one buffer, used for all incoming/outgoing
  uint16_t packet_id_counter;
  MQTT_CALLBACK_SIGNATURE;
 private:
  //ATT_MQTT_Subscribe *subscriptions[MAXSUBSCRIPTIONS];
 
  void    flushIncoming(uint16_t timeout);

  // Functions to generate MQTT packets.
  uint8_t connectPacket(uint8_t *packet);
  uint8_t disconnectPacket(uint8_t *packet);
  uint16_t publishPacket(uint8_t *packet, const char *topic, uint8_t *payload, uint16_t bLen, uint8_t qos);
  uint8_t subscribePacket(uint8_t *packet, const char *topic, uint8_t qos);
  uint8_t unsubscribePacket(uint8_t *packet, const char *topic);
  uint8_t pingPacket(uint8_t *packet);
  uint8_t pubackPacket(uint8_t *packet, uint16_t packetid);
};



#endif
