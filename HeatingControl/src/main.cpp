#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include <time.h>
#include <sstream>
#include <vector>
#include <thread>
#include <mosquittopp.h>
#include <mosquitto.h>
#include "Core.h"

//#define CLIENT_ID "Client_ID"
//#define MQTT_PORT 1883;
//#define MQTT_TOPIC "topic"
//#define MQTT_USER "USER"
//#define MQTT_PWD "PWD"

#define CLIENT_ID "HeatingControl"
#define MQTT_PORT 1883;
#define MQTT_TOPIC "topic"
#define MQTT_USER "thenewone"
#define MQTT_PWD "1991"

using namespace std;

int main() {

	////MQTT/////
	class Core* iot_client;
	//Connection data
	char client_id[] = CLIENT_ID;
	const char* USER = MQTT_USER;
	const char* PASSW = MQTT_PWD;
	char host[] = "localhost";
	int port = MQTT_PORT;
	string topicString = MQTT_TOPIC;
	mosqpp::lib_init();
	iot_client = new Core(client_id, host, port, USER, PASSW);
	iot_client->loop_start();
	////MQTT/////

	//Starting the main thread
	std::thread mainthread = iot_client->basicFuncthread();

	//Starting the user interface
	string comm;

	do
	{
		cout << "Waiting for command: ";
		cin >> comm;

		if (comm == "exit") {
			mainthread.join();
			iot_client->disconnect();
			cout << "Bye!\n";
		}
		else {
			cout << iot_client->commFunc(comm) << "\n";
		}
	} while (comm != "exit");

	return 0;
}
