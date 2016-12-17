/*
   Copyright 2014-2016 AllThingsTalk

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.

Original author: Peter Leemans (2014)
2/5/2014	Jan Bogaerts	Convert to library
2/3/2015	Jan Bogaerts 	release 2
07/11/2016	Jan Bogaerts 	rebrand to AllThingsTalk
*/

#ifndef ATTDevice_h
#define ATTDevice_h

//#include "Arduino.h"
#include <Client.h>
#include "ATT_MQTT_SIM800.h"
#include <string.h>

//this class represents the ATT cloud platform.
class ATTDevice
{
	public:
		//create the object
		ATTDevice(String deviceId, String clientId, String clientKey);
		
		bool InitGPRS(Adafruit_FONA* fona, const char *apn, const char *username, const char *password);
		
		/*connect with the http server (call first)
		-fona: the object representing the fona modem.
		-httpServer: the name of the http server to use, kept in memory until after calling 'Subscribe' 
		returns: true when subscribe was successful, otherwise false.*/
		bool Connect(char httpServer[]);
		
		//create or update the specified asset. (call after connecting)
		//note: after this call, the name will be in lower case, so that it can be used to compare with the topic of incomming messages.
		void AddAsset(int id, String name, String description, bool isActuator, String type);

		/*Stop http processing & make certain that we can receive data from the mqtt server. 
		returns true when successful, false otherwise*/
		bool Subscribe(ATT_MQTT_SIM800& mqttclient);
		
		bool Subscribe(ATT_MQTT_SIM800& mqttclient, const char* username, const char* pwd);
		
		//send a data value to the cloud server for the sensor with the specified id.
		//returns true upon success.
		bool Send(String value, int id);
		
		//closes any open connections (http & mqtt) and resets the device. After this call, you 
		//can call connect and/or subscribe again. Credentials remain stored.
		//Note: all clients (httpclient & pubsubClient) are the caller's responsibility to clean up
		void Close();
	
		//check for any new mqtt messages.
		bool Process();
		
		//returns the pin nr found in the topic
		int GetPinNr(char* topic, int topicLength);
	private:	
		String _serverName;				//stores the name of the http server that we should use.
		String _clientKey;				//the client key provided by the user.
		Adafruit_FONA* _fona;				//raw http communication. Possible to save some memory here: pass the client as a param in connect, put the object local in the setup function.
		
		const char* _mqttUserName;		//we store a local copy of the the mqtt username and pwd, so we can auto reconnect if the connection was lost.
		const char* _mqttpwd;	
		
		//subscribe to the mqtt topic so we can receive data from the server.
		void MqttSubscribe();
		
		//read all the data from the Ethernet card and display on the debug screen.
		void GetHTTPResult();
		
		//builds the content that has to be sent to the cloud using mqtt (either a csv value or a json string)
		char* BuildContent(String value);
		
		//closes the http connection, if any.
		void CloseHTTP();
		
		bool _initGprs();
		
		ATT_MQTT_SIM800* _mqttclient;		//provides mqtt support (placed as protected, so we can build inheriters that can access the mqtt client directly (ex: network watchdog)
		
		//tries to create a connection with the mqtt broker. also used to try and reconnect.
		bool MqttConnect();				//so inheriters can reconnect with the mqtt server if they detect a network loss.
		String _deviceId;				//the device id provided by the user.
		String _clientId;				//the client id provided by the user.
		String _apn;					//for fona network
		String _apnUser;				//username for gprs
		String _apnPwd;					//pwd for gprs
};

#endif