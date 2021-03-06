#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include <time.h>
#include <sstream>
#include <vector>
#include <wiringPi.h>
#include <thread>
#include <mosquittopp.h>
#include <mosquitto.h>

#include "Core.h"

#define CLIENT_ID "HeatingControl"
#define MQTT_PORT 1883;
#define MQTT_TOPIC "topic"
#define MQTT_USER "thenewone"
#define MQTT_PWD "1991"
//#define SETTINGS_FILE "/home/pi/projects/HeatingControl/bin/ARM/Release/settings.txt"

//int mainpipe, boiler1, boiler2, heater, house, chimney, solar, solarboiler;

using namespace std;

//web		"/home/pi/Desktop/cb/log/strekszel.txt"

int main() {

	/////wiringPi///// its not necessaary, but good to be here
	wiringPiSetup();
	pinMode(0, OUTPUT);
	pinMode(1, OUTPUT);
	pinMode(2, OUTPUT);
	pinMode(3, OUTPUT);
	digitalWrite(0, HIGH);
	digitalWrite(1, HIGH);
	digitalWrite(2, HIGH);
	digitalWrite(3, HIGH);
	/////wiringPi/////

	////MQTT/////


	class Core* iot_client;
	//Connection data
	char client_id[] = CLIENT_ID;
	/*string mqttSet, mqttuser, mqttpwd;
	ifstream settings;
	settings.open(SETTINGS_FILE);
	if (settings.fail()) cout << "Could not open the settings file! :(\n";
	else {
		string mqttSet;
		do {

			getline(settings, mqttSet);
			if(mqttSet.find("#") != -1) getline(settings, mqttSet);
			if (mqttSet.find("-mqttUSER") != -1) {
				mqttuser = mqttSet.substr(mqttSet.find_first_of(' ') + 1, mqttSet.length()- mqttSet.find_first_of(' '));
				cout << "MQTT username: " << mqttuser << "MMM" << endl;
			}
			else if (mqttSet.find("-mqttPWD") != -1) {
				mqttpwd = mqttSet.substr(mqttSet.find_first_of(' ') + 1, mqttSet.length() - mqttSet.find_first_of(' '));
				cout << "MQTT password: " << mqttpwd << "MMM" << endl;
			}

		} while (!settings.eof() || (!mqttuser.empty() && !mqttpwd.empty()));

	}*/

	const char* USER = MQTT_USER;
	const char* PASSW = MQTT_PWD;
	//int sadf = 13245;
	//int* clint_id = &sadf;
	char host[] = "localhost";
	int port = MQTT_PORT;
	string topicString = MQTT_TOPIC;

	mosqpp::lib_init();

	iot_client = new Core(client_id, host, port, USER, PASSW);
	/*cout << "usr pw: " << iot_client->username_pw_set(USER, PASSW);
	cout << "sub: " << iot_client->subscribe(clint_id, MQTT_TOPIC, 2);
	cout << "loop: " << iot_client->loop_start();*/
	//iot_client->username_pw_set(USER, PASSW);
	//iot_client->subscribe(clint_id, MQTT_TOPIC, 2);
	//iot_client->subscribe(clint_id, "extsensors", 2);
	iot_client->loop_start();
	////MQTT/////

	std::thread mainthread = iot_client->basicFuncthread();

	string comm;
	do
	{
		cout << "Waiting for command: ";
		cin >> comm;

		cout << iot_client->commFunc(comm) << endl;

		if (comm == "exit") {
			mainthread.join();
			iot_client->disconnect();
			cout << "Bye!\n";
		}
	} while (comm != "exit");

	return 0;
}
