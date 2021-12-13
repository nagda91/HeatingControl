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

#define CLIENT_ID "userID"
#define MQTT_PORT 1883;
#define MQTT_TOPIC "topic"
<<<<<<< HEAD
#define MQTT_USER "user"
#define MQTT_PWD "pwd"
=======
#define MQTT_USER "USR"
#define MQTT_PWD "PSW"
>>>>>>> f0fbb3ec257761caadf2024651ae886b231680f0

using namespace std;

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

		cout << iot_client->commFunc(comm) << endl;

		if (comm == "exit") {
			mainthread.join();
			iot_client->disconnect();
			cout << "Bye!\n";
		}
	} while (comm != "exit");

	return 0;
}
