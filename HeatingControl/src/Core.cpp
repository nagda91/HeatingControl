#include <iostream>
#include <fstream>
#include <string>
#include <string.h>
#include <ctime>
#include <time.h>
#include <sstream>
#include <vector>
#include <thread>
#include <stdlib.h>
#include <stdio.h>
#include <wiringPi.h>
#include <future>
#include <chrono>
#include <math.h>
#include <mosquittopp.h>
#include <mosquitto.h>

#include "Temp.h"
#include "Core.h"
#include "Relay.h"

using namespace std;

#define MAX_PAYLOAD 50
#define DEFAULT_KEEP_ALIVE 60

#define BROKER_ADDRESS "dummy"

/////MQTT///////////////////////////////////////////////////////////////////////////////////
Core::Core(const char* id, const char* host, int port, const char* user, const char* passw) : mosquittopp(id)
{
	meret = 0;
	mainpipe = 0;
	boiler1 = 1;
	boiler2 = 2;
	heater = 3;
	house = 4;
	chimney = 5;
	solar = 6;
	solarboiler = 7;
	heatingMode = 1;

	int keepalive = DEFAULT_KEEP_ALIVE;
	this->will_set(MQTT_TOPIC, strlen(WILL_MESSAGE), WILL_MESSAGE, 0, false);
	username_pw_set(user, passw);
	connect(host, port, keepalive);
	//cout << "connect: " << connect(host, port, keepalive) << '\n';
}

void Core::on_connect(int rc)
{
	mainLog("on_Connect");

	this->subscribe(clint_id, MQTT_TOPIC, 2);
	// <extsensors> topic can be changed, but dont forget to use that topic in your wireless sensors' code
	this->subscribe(clint_id, "extsensors", 2);

	if (!rc)
	{
#ifdef DEBUG
		std::cout << "Connected - code " << rc << std::endl;
#endif
	}
}

void Core::on_subscribe(int mid, int qos_count, const int* granted_qos)
{
	mainLog("on_Subscribe");
#ifdef DEBUG
	std::cout << "Subscription succeeded." << std::endl;
#endif
}

string Core::cmd(const struct mosquitto_message* cmdmsg, string mes)
{
	static const size_t npos = -1;
	if (mes.find("thermN") != npos) {
		int neu = szam(mes.substr(mes.find('=') + 1));
		//cout << "cmd(), thermostatNight modified from: " << thermostatNight;
		//thermostatNight = szam(mes.substr(mes.find('=') + 1));
		string rep;
		if (neu < 15000 || neu > thermostatDay) {
			rep = "fault new thermNight was invalid value: " + to_string(neu);
			sendTopic("fail,thermNight was invalid");
		}
		else {
			rep = "thermNight changed from: " + to_string(thermostatNight);
			thermostatNight = neu;
			rep = rep + ", to: " + to_string(thermostatNight);
		}
		mqttLog(rep);
		return mes;
	}
	else if (mes.find("setthermD") != npos) {
		int neu = szam(mes.substr(mes.find('=') + 1));
		string rep;
		//cout << "cmd(), thermostatDay modified from: " << thermostatDay;
		if (neu < 15000 || neu > 24000) {
			rep = "fault new thermDay was invalid value: " + to_string(neu);
			sendTopic("fail,thermDay was invalid");
		}
		else {
			rep = "thermDay changed from: " + to_string(thermostatDay);
			thermostatDay = neu;
			rep = rep + ", to: " + to_string(thermostatDay);
		}
		//thermostatDay = szam(mes.substr(mes.find('=') + 1));
		mqttLog(rep);
		return mes;
	}
	else if (mes.find("nStim") != npos) {
		int neu = szam(mes.substr(mes.find('=') + 1));
		string rep;
		if (neu < 0 || neu >1439) {
			rep = "fault new nightStartTime was invalid value: " + to_string(neu);
			sendTopic("fail,nightStartTime was invalid");
		}
		else {
			rep = "nightStarttime changed from: " + to_string(nightStarttime);
			nightStarttime = neu;
			rep = rep + ", to: " + to_string(nightStarttime);
		}
		mqttLog(rep);
		return mes;
	}
	else if (mes.find("nEtim") != npos) {
		int neu = szam(mes.substr(mes.find('=') + 1));
		//cout << "cmd(), nightEndtime modified from: " << nightEndtime;
		string rep;
		if (neu < 0 || neu > 1439) {
			rep = "fault new nightEndTime was invalid value: " + to_string(neu);
			sendTopic("fail,nightEndTime was invalid");
		}
		else {
			rep = "nightStarttime changed from: " + to_string(nightEndtime);
			nightEndtime = neu;
			rep = rep + ", to: " + to_string(nightEndtime);
		}
		mqttLog(rep);
		return mes;
	}
	else if (mes.find("getThermd") != npos) {
		send("topic", getThermd());
		mqttLog("cmd(), getThermd()");
		return mes;
	}
	else if (mes.find("getWTs") != npos) {
		send("topic", getWT());
		mqttLog("cmd(), getWT()");
		return mes;
	}
	else if (mes.find("getTDT") != npos) {
		sendTopic(getTDT());
		mqttLog("cmd(), getTDT()");
		return mes;
	}
	else if (mes.find("getTemps") != npos) {
		//cout << "cmd(), getTemps()";
		sendTopic(getTemps());
		//cout << ", sent: " << nightEndtime << endl;
		mqttLog("cmd(), getTemps()");
		return mes;
	}
	else if (mes.find("wS") != npos) {
		cout << "cmd(), wS()";
		int neu = szam(mes.substr(mes.find('=') + 1));
		string rep;
		if (neu < 0 || neu > 11) {
			rep = "fault new winterStart was invalid value: " + to_string(neu);
			sendTopic("fail,winterStart was invalid");
		}
		else {
			rep = "winterStart changed from: " + to_string(winterStart);
			winterStart = neu;
			rep = rep + ", to: " + to_string(winterStart);
		}
		mqttLog(rep);
		return mes;
	}
	else if (mes.find("wE") != npos) {
		int neu = szam(mes.substr(mes.find('=') + 1));
		string rep;
		if (neu < 0 || neu > 11) {
			rep = "fault new winterEnd was invalid value: " + to_string(neu);
			sendTopic("fail,winterEnd was invalid");
		}
		else {
			rep = "winterEnd changed from: " + to_string(winterEnd);
			winterEnd = neu;
			rep = rep + ", to: " + to_string(winterEnd);
		}
		mqttLog(rep);
		return mes;
	}
	else if (mes.find("sDiff") != npos) {
		int neu = szam(mes.substr(mes.find('=') + 1));
		string rep;
		if (neu < 0 || neu > 50000) {
			rep = "fault new solarDiff was invalid value: " + to_string(neu);
			sendTopic("fail,solarDiff was invalid");
		}
		else {
			string rep = "solarDiff changed from: " + to_string(solarDiff);
			solarDiff = neu;
			rep = rep + ", to: " + to_string(solarDiff);
		}
		mqttLog(rep);
		return mes;
	}
	else if (mes.find("hdiff") != npos) {
		int neu = szam(mes.substr(mes.find('=') + 1));
		string rep;
		if (neu < 0 || neu > 5000) {
			rep = "fault new houseDiff was invalid value: " + to_string(neu);
			sendTopic("fail,houseDiff was invalid");
		}
		else {
			rep = "houseDiff changed from: " + to_string(houseDiff);
			houseDiff = neu;
			rep = rep + ", to: " + to_string(houseDiff);
		}
		mqttLog(rep);
		return mes;
	}
	else if (mes.find("whDiff") != npos) {
		int neu = szam(mes.substr(mes.find('=') + 1));
		string rep;
		if (neu < 0 || neu > 2000) {
			rep = "fault new whileHouseDiff was invalid value: " + to_string(neu);
			sendTopic("fail,whileHouseDiff was invalid");
		}
		else {
			rep = "whilehouseDiff changed from: " + to_string(whilehouseDiff);
			whilehouseDiff = neu;
			rep = rep + ", to: " + to_string(whilehouseDiff);
		}
		mqttLog(rep);
		return mes;
	}
	else if (mes.find("bDiff") != npos) {
		int neu = szam(mes.substr(mes.find('=') + 1));
		string rep;
		if (neu < 0 || neu > 20000) {
			rep = "fault new boilerDiff was invalid value: " + to_string(neu);
			sendTopic("fail,boilerDiff was invalid");
		}
		else {
			rep = "boilerDiff changed from: " + to_string(boilerDiff);
			boilerDiff = neu;
			rep = rep + ", to: " + to_string(boilerDiff);
		}
		mqttLog(rep);
		return mes;
	}
	else if (mes.find("oPchim") != npos) {
		int neu = szam(mes.substr(mes.find('=') + 1));
		string rep;
		if (neu < 20000 || neu > 35000) {
			rep = "fault new onlyPumpChimneyminimum was invalid value: " + to_string(neu);
			sendTopic("fail,onlyPumpChimneyminimum was invalid");
		}
		else {
			rep = "onlyPumpchimneyminimum changed from: " + to_string(onlyPumpchimneymin);
			onlyPumpchimneymin = neu;
			rep = rep + ", to: " + to_string(onlyPumpchimneymin);
		}
		mqttLog(rep);
		return mes;
	}
	else if (mes.find("oP") != npos) {
		int neu = szam(mes.substr(mes.find('=') + 1));
		string rep;
		if (neu < 30000 || neu > 65000) {
			rep = "fault new onlyPump was invalid value: " + to_string(neu);
			sendTopic("fail,onlyPump was invalid");
		}
		else {
			rep = "onlyPump changed from: " + to_string(onlyPump);
			onlyPump = neu;
			rep = rep + ", to: " + to_string(onlyPump);
		}
		mqttLog(rep);
		return mes;
	}
	else if (mes.find("aftCirc") != npos) {
		int neu = szam(mes.substr(mes.find('=') + 1));
		string rep;
		if (neu < 30000 || neu > 60000) {
			rep = "fault new afterCirculation was invalid value: " + to_string(neu);
			sendTopic("fail,afterCirculation was invalid");
		}
		else {
			rep = "afterCirculation changed from: " + to_string(afterCirculation);
			afterCirculation = neu;
			rep = rep + ", to: " + to_string(afterCirculation);
		}
		mqttLog(rep);
		return mes;
	}
	else if (mes.find("hMax") != npos) {
		int neu = szam(mes.substr(mes.find('=') + 1));
		string rep;
		if (neu < 40000 || neu > 70000) {
			rep = "fault new heaterMax was invalid value: " + to_string(neu);
			sendTopic("fail,heaterMax was invalid");
		}
		else {
			rep = "heaterMax changed from: " + to_string(heaterMax);
			heaterMax = neu;
			rep = rep + ", to: " + to_string(heaterMax);
		}
		mqttLog(rep);
		return mes;
	}
	else if (mes.find("getGPIO") != npos) {
		send("topic", getGPIO());
		mqttLog("cmd(), getGPIO()");
		return mes;
	}
	else if (mes.find("setEXT") != npos) {
		int neu = szam(mes.substr(mes.find('=') + 1));
		string name = mes.substr(mes.find(',') + 1, (mes.find('=') - mes.find(',') - 1));
		int key = szam(mes.substr(mes.find('<') + 1, mes.find_last_of('>') - mes.find('<')));
		//cout << "EXTkey: " << key << endl;
		int sensor;
		try {
			throw getSensorNumber(name);
		}
		catch (int z) {
			if (z == -1) {
				mainLog("No external sensor with the same name!(" + name + ")");
				return ("No sensor with the same name!(" + name + ")");
			}
			else {
				sensor = z;
			}
		}
		/*if neu = 200000 -> sensor error(85000)
		if neu = 200001 -> no sensor, it's broken or smg
		*/
		if (key == EXTkey) {
			if (neu == 200000) {
				temperatureSensors[sensor].setTemp(20202);
				temperatureSensors[sensor].getBP();
				mainLog("EXTsensor fail(332), neu= " + to_string(neu));
				sendTopic("alert,EXTsensor fail(332)");
			}
			else if (neu == 200001) {
				temperatureSensors[sensor].setTemp(20202);
				temperatureSensors[sensor].getBP();
				mainLog("EXTsensor fail(338), neu= " + to_string(neu));
				sendTopic("alert,EXTsensor fail(338)");
			}
			else {
				temperatureSensors[sensor].setTemp(neu);
				temperatureSensors[sensor].setLastUpdate();
				temperatureSensors[sensor].setFresh(1);
			}
		}
		else {
			temperatureSensors[sensor].getBP();
			temperatureSensors[sensor].setFresh(0);
			debugLog("Key was not similar.");
		}
		return mes;
	}
	else if (mes.find("setheatingMode") != npos) {
		int neu = szam(mes.substr(mes.find('=') + 1));
		setHeatingMode(neu);
		return mes;
	}
	else if (mes.find("gasheater sensor disconnected!") != npos) {
		temperatureSensors[getSensorNumber("gasheater")].setWorking(0);
		setHeatingMode(1);
		return mes;
	}
	else if (mes.find("getvlog") != npos) {
		send("topic", getVlog());
		mqttLog("cmd(), getVlog()");
		return mes;
	}
	return "0";
	/*winterStart, winterEnd, nightStarttime,
	nightEndtime, solarDiff, houseDiff, whilehouseDiff,
	boilerDiff, onlyPumpchimneymin, onlyPump, afterCirculation,
	thermostatDay, thermostatNight, heaterMax;*/
}

int Core::send(string topicstr, string newPl)
{
	int result;
	char* buf, * topic;
	buf = new char[newPl.length() + 1];
	topic = new char[topicstr.length() + 1];
	strcpy(buf, newPl.c_str());
	strcpy(topic, topicstr.c_str());
	result = this->publish(NULL, topic, strlen(buf), buf, 2, false);
	//cout << "send(), publish: " << result << endl;

	return result;
}

int Core::sendTopic(string newPl)
{
	int result;
	char* buf;
	buf = new char[newPl.length() + 1];
	strcpy(buf, newPl.c_str());
	result = this->publish(NULL, "topic", strlen(buf), buf, 2, false);
	//cout << "send(), publish: " << result << endl;

	return result;
}

int Core::sendReport(const struct mosquitto_message* msg, string newPl)
{
	int result;
	char* buf;
	buf = new char[newPl.length() + 1];
	strcpy(buf, newPl.c_str());
	result = this->publish(NULL, (char*)"report", strlen(buf), buf, 2, false);
	//cout << "sendReport(), publish: " << result << endl;

	return result;
}

void Core::on_message(const struct mosquitto_message* message)
{
	if (message->payloadlen >= 1) {
		static const size_t npos = -1;
		string strPayload = (char*)message->payload;
		mqttLog("on_message: " + strPayload);
		/*if (strPayload == "disc") {
			int ri = this->disconnect();
			if (ri != 0) cout << "disconnect: " << ri << endl;
			else
			{
				cout << "Disconnected!" << endl;
			}
		}*/
		if (strPayload.find("cmd") != npos) {
			string ret = cmd(message, strPayload);
			if (ret != "0") sendReport(message, ret);
		}
	}
}

void Core::on_disconnect(int rc) {
	this->mainLog("on_Disconnect");
	this->mqttLog("on_Disconnect");
	this->reconnect();
	//if(STOP) this->mqttReset();
}

int Core::mqttReset() {
	try {
		throw this->disconnect();
	}
	catch (int z) {
		mainLog("Disconnect(mqttreset):" + to_string(z));
		if (z != 0 && z != 4) return z;
	}

	try {
		throw this->loop_stop(true);
	}
	catch (int z) {
		mainLog("Loop_stop(mqttreset):" + to_string(z));
		if (z != 0) return z;
	}

	try {
		throw this->connect(BROKER_ADDRESS, 1883, DEFAULT_KEEP_ALIVE);
	}
	catch (int z) {
		mainLog("Connect(mqttreset):" + to_string(z));
		if (z != 0) return z;
	}
	try {
		throw this->subscribe(clint_id, MQTT_TOPIC, 2);
	}
	catch (int z) {
		mainLog("Subscribe, topic(mqttreset):" + to_string(z));
		if (z != 0) return z;
	}
	try {
		throw this->subscribe(clint_id, "extsensors", 2);
	}
	catch (int z) {
		mainLog("Subscribe, extsensors(mqttreset):" + to_string(z));
		if (z != 0) return z;
	}
	try {
		throw this->loop_start();
	}
	catch (int z) {
		mainLog("Loop_start(mqttreset):" + to_string(z));
		if (z != 0) return z;
	}
	return 0;
}
/////MQTT/////////////////////////////////////////////////////////////////////

Core::Core() {
	meret = 0;
	mainpipe = 0;
	boiler1 = 1;
	boiler2 = 2;
	heater = 3;
	house = 4;
	chimney = 5;
	solar = 6;
	solarboiler = 7;
	//Temp wen;
	//tomb.push_back(wen);
}

Core::~Core() {
	temperatureSensors.clear();
}

string Core::settingsFromFile(string x)
{
	heating = 0;
	thermostatDay = 20000;
	thermostatNight = 19000;
	string row, name;
	int pin;
	ifstream f;

	f.open(x.c_str());

	if (f.fail()) {
		cout << x << " -- Unable to open sensors.txt, please look at readme.txt, :)" << endl;
		return "Unable to open sensorfilename.txt, please look at readme.txt, :)";
	}
	else {
		do {
			size_t z = -1;
			getline(f, row);
			if (row.find("#") != z) getline(f, row);
			if (row.find("-tlp") != z) {
				row.erase(row.length());
				this->tempsfilename = row.substr(row.find(' ') + 1, row.find(';'));
			}
			else if (row.find("-log") != z) {
				row.erase(row.length());
				this->logfilename = row.substr(row.find(' ') + 1, row.find(';'));
			}
			else if (row.find("-o") != z) {

				Temp newSensor;

				newSensor.setName(row.substr(row.find_first_of(' ') + 1, row.find_last_of(' ') - row.find_first_of(' ') - 1));
				newSensor.setID(row.substr(row.find_last_of(' ') + 1, 15));
				temperatureSensors.push_back(newSensor);
			}
			else if (row.find("-d") != z) {

				name = row.substr(row.find(' ') + 1, row.find_last_of(' ') - row.find_first_of(' ') - 1);
				pin = szam(row.substr(row.find_last_of(' ') + 1, row.length() - row.find_last_of(" ")));

				Relay newRelay = Relay(name, pin, TEST);

				Devices.push_back(newRelay);

			}
			else if (row.find("-HS") != z) { 
				row = row.substr(row.find_first_of(' ') + 1);
				try
				{
					vectorUploader(row, heatingSensors);
				}
				catch (int e)
				{
					if (e != 1) mainLog("HeatingSensors could not be uploaded!");
				}
				
			}
			else if (row.find("-HD") != z) {
				row = row.substr(row.find_first_of(' ') + 1);
				try
				{
					vectorUploader(row, heatingDevices);
				}
				catch (int e)
				{
					if (e != 1) mainLog("HeatingDevices settings could not be uploaded!");
				}
			}

		} while (!f.eof());
		f.close();
	}

	mainLog("Uploaded sensors:(Name -- ID)");
	for (size_t i = 0; i < temperatureSensors.size(); i++) {
		mainLog((to_string(i + 1) + ". " + temperatureSensors[i].getName() + " -- " + temperatureSensors[i].getFilename()),true, false);
	}
	mainLog("Uploaded devices:(Name -- GPIO)");
	for (size_t i = 0; i < Devices.size(); i++) {
		mainLog((to_string(i + 1) + ". " + Devices[i].getName() + " -- " + to_string(Devices[i].getPIN())),true,false);
	}
	mainLog("HeatingSensors settings: ", false);
	for (size_t i = 0; i < heatingSensors.size(); i++) {
		if (i != 0) mainLog(",", false, false);
		mainLog(to_string(heatingSensors[i]), false, false);
	}
	mainLog("", true, false);
	mainLog("HeatingDevices settings: ", false);
	for (size_t i = 0; i < heatingDevices.size(); i++) {
		if (i != 0) mainLog(",", false, false);
		mainLog(to_string(heatingDevices[i]), false, false);
	}
	mainLog("", true, false);
	return "0";
}

void Core::setTempsthread() {
	//refresh the temperatures every ~10sec
	int ret = 0;
	bool newWeek = true;

	//first row the tempslog
	tempsFilename = filename(tempsfilename);
	tempsLog(tempslogFirstRow());

	while (STOP) {
		for (size_t i = 0; i < temperatureSensors.size(); i++) {
			if (!temperatureSensors[i].getEXT()) {
				ret = temperatureSensors[i].setTempfromfile(TEST);
			}
			else {
				if (!temperatureSensors[i].getFresh()) debugLog(temperatureSensors[i].getName() + " - FRESH is false");
				setExtTemp(i);
				if (!temperatureSensors[i].getWorking()) {
					debugLog(temperatureSensors[i].getName() + " - WORKING is false");
					send("topic", "alert," + temperatureSensors[i].getName() + " sensor is not working!");
				}
				debugLog(temperatureSensors[i].getName() + " blackpoints: " + to_string(temperatureSensors[i].showBP()));
				debugLog(temperatureSensors[i].getName() + " sameCounter: " + to_string(temperatureSensors[i].getsameCounter()));

				/*external sensors updating
				if no response set inactive
				somehow the mqtt response decide when its ready
				int lastupdate=time(0);*/

			}
			if (ret != 0) {
				mainLog("error: " + to_string(ret) + ", " + temperatureSensors[i].getName() + " was the problem");
				mainLog(getGPIO());
				OK = false;
			}
		}
		isboilerMax();
		if (dayOftheweek() == 1 && newWeek) {
			newWeek = false;
			tempsFilename = filename(tempsfilename);
			//tempslog << "datum ; cso ; bojler teteje ; felso hocserelo ; kazan ; lakas ; kemeny ; napko. ; napk.bojler" << endl;
			tempsLog(tempslogFirstRow());
		}
		else {
			tempsLog();
		}
		if (dayOftheweek() != 1 && !newWeek) {
			newWeek = true;
			tempsLog(" #### End ###################################");
		}

		//Samples for Neural Network maybe
		if(!TEST) AIsamples();

		delay(1000);
	}
}

int Core::getMeret() {
	return meret;
}

Temp Core::operator[](int x) {
	return temperatureSensors[x];
}

void Core::basicFunc() {

	int tempslogI = 1, thisDay = dayOftheweek();
	//ofstream log;
	vector<int> day;
	logFilename = filename("/home/pi/Desktop/log/mainlog");
	//cout << "basicFunc() logFilename: " << logFilename << endl;
	//log.open(logFilename.c_str(), ios_base::app);//, ios_base::app
	/*if (log.fail()) {
		cout << "basicFunc()  fail to open logFilename: " << logFilename << endl;
		send("report", "basicFunc()  fail to open logFilename");
		sendTopic("fault, basicFunc()  fail to open logFilename");
	}
	else {
		//log << "Start: " << longtime() << endl;
		//log.close();
		mainLog("Start: " + longtime() + "\n");
	}*/

	mainLog("################ Start -- " + longtime() + " ################", true, false);

	delay(3000);

	/*try
	{
		throw setDevices("/home/pi/projects/thenewone/bin/ARM/Release/devices.txt");
	}
	catch (const int e)
	{
		if (e == 0) {
			mainLog("Devices uploaded!(696)");
			mainLog(Devs.getDevicesData());
		}
		else {
			mainLog("Devices - Something went wrong!(700)");
		}
	}*/

	try {
		throw settingsFromFile("/home/pi/projects/thenewone/bin/ARM/Release/settings.txt");
	}
	catch (string e) {
		if (e != "0") mainLog(e);
	}

	try {
		throw setTimeDiffTemps();
	}
	catch (string e) {
		if (e != "0") mainLog(e);
	}

	std::thread first = std::thread(&Core::setTempsthread, this);
	delay(8000);
	//std::thread second = std::thread(&Core::logThread, this);

	//log.open(logFilename.c_str(), ios_base::app);

	std::thread td_heatingFunc;

	while (STOP) {

		if (OK) {
			if ((nightEndtime - avgHeatingTime()) < timeinMin() && timeinMin() < (nightStarttime - avgHeatingTime())) {

				//Day
				thermostat = &thermostatDay;

				if (winterEnd < month() && month() < winterStart) {
					//Summer day
					try
					{
						throw solarFunc("Solarpump");
					}
					catch (string e)
					{
						if (TEST) cout << "Core - basicFunc - 1375" << endl;
						if (e != "0") mainLog(e);
					}
					//log << solarFunc() << endl;
				}
				else {
					//Winter day
					if (td_heatingFunc.joinable()) {
						td_heatingFunc.join();
						heating = false;
						//log << heatingFuncreturn << endl;
						mainLog(heatingFuncreturn);
					}
					else {
						if (!heating) {
							//td_heatingFunc = std::thread(&Core::heaterFunc, this, &thermostatDay);

							switch (heatingMode) {
							case 1: td_heatingFunc = std::thread(&Core::heaterFunc, this, thermostat);
								break;
							case 2: td_heatingFunc = std::thread(&Core::heaterFunc_mode2, this, thermostat);
								break;
							default: td_heatingFunc = std::thread(&Core::heaterFunc, this, thermostat);
							}

							delay(500);
							if (td_heatingFunc.joinable()) {
								td_heatingFunc.join();
								heating = false;
								if (!heatingFuncreturn.empty()) {
									//log << heatingFuncreturn << endl;
									mainLog(heatingFuncreturn);
									heatingFuncreturn.clear();
								}
							}
							else {
								mainLog("Heating is started, thermostat: " + to_string(thermostatDay));
							}
						}
					};

					if (!heating) {
						try
						{
							throw boilerFunc();
						}
						catch (string e)
						{
							//if (e != "0") log << e << endl;
							if (e != "0") mainLog(e);
						}
					};

					if (450 < timeinMin() && timeinMin() < 1020) {
						try
						{
							throw solarFunc("Solarpump");
						}
						catch (string e)
						{
							//if (e != "0") log << e << endl;
							if (e != "0") mainLog(e);
						}
					}
					else {
						if (digitalRead(1) != 1) digitalWrite(1, HIGH);
					}
				};
			}
			else {
				//Night
				thermostat = &thermostatNight;
				if (winterEnd < month() && month() < winterStart) {
					//winterEnd < month() && month() < winterStart
					//Summer night
				}
				else {
					//Winter night
					if (td_heatingFunc.joinable()) {
						td_heatingFunc.join();
						heating = false;
						//log << heatingFuncreturn << endl;
						mainLog(heatingFuncreturn);
					}
					else {
						if (!heating) {
							//td_heatingFunc = std::thread(&Core::heaterFunc, this, &thermostatNight);

							switch (heatingMode) {
							case 1: td_heatingFunc = std::thread(&Core::heaterFunc, this, thermostat);
								break;
							case 2: td_heatingFunc = std::thread(&Core::heaterFunc_mode2, this, thermostat);
								break;
							default: td_heatingFunc = std::thread(&Core::heaterFunc, this, thermostat);
							}

							delay(500);
							if (td_heatingFunc.joinable()) {
								td_heatingFunc.join();
								heating = false;
								if (!heatingFuncreturn.empty()) {
									//log << heatingFuncreturn << endl;
									mainLog(heatingFuncreturn);
									heatingFuncreturn.clear();
								}
							}
							else {
								mainLog("Heating is started, thermostat: " + to_string(thermostatNight));
							}
						}
					}
					if (!heating) {
						try
						{
							throw boilerFunc();
						}
						catch (string e)
						{
							//if (e != "0") log << e << endl;
							if (e != "0") mainLog(e);
						}
					}
				}
			}

			/*"datum ; ido ; cso ; bojler teteje ; felso hocserelo ; kazan ; lakas ; kemeny ; napko. ; napk.bojler:"*/
			if (dayOftheweek() != 0 && tempslogI == 1) tempslogI = 0;
			if (dayOftheweek() == 0 && tempslogI == 0) {
				//boilerMax
				mainLog("Boiler maximums(0=Sunday):");
				/*for (int i=0; i < boilerMaxofWeek.size(); i++) {
					for (int j=0; j < day.size(); j++) {
						//log << to_string(boilerMaxofWeek[i][j]);
						mainLog(to_string(boilerMaxofWeek[i][j]));
					}
					//log << endl;
					mainLog("");
				}*/
				for (size_t i = 0; i < boilerMaxofWeek.size(); i++) {
					mainLog(to_string(boilerMaxofWeek[i][0]) + " - " + to_string(boilerMaxofWeek[i][1]));
				}
				boilerMaxofWeek.clear();
				///////////
				//log << "End: " << longtime();
				//log.close();
				mainLog("End");
				logFilename = filename(logfilename);
				//log.open(logFilename.c_str(), ios_base::app);
				//log << "Start: " << longtime() << endl;
				mainLog("Start");
				tempslogI = 1;
			}
			//boilerMax set in vector every day
			if (dayOftheweek() != thisDay) {
				day.clear();
				day.push_back(thisDay);
				day.push_back(boilerMax);
				boilerMaxofWeek.push_back(day);
				thisDay = dayOftheweek();
				mainLog("Yesterday the boiler max. was: " + to_string(boilerMax));
				boilerMax = 0;
				//log << "Today the soalrpump worked: " << to_string(solarPumpWT) << "sec" << endl;
				mainLog("Yesterday the soalrpump worked: " + minToTime(solarPumpWT));
				solarPumpWT = 0;
				mainLog("Yesterday the boilerpump worked: " + minToTime(boilerPumpWT));
				boilerPumpWT = 0;
				//log << "Today the heater worked: " << to_string(heaterWorkingTime) << "sec" << endl;
				mainLog("Yesterday the heater worked: " + minToTime(heaterWorkingTime));
				heaterWorkingTime = 0;
				mainLog("Gasheater working times each heating sessions:");
				mainLog(getgasheaterWTs(), false, false);
				gasheaterWTs.clear();

				for (size_t i = 0; i < temperatureSensors.size(); i++) {
					mainLog(temperatureSensors[i].EndOfDay());
				}

			}
			delay(10500);
		}//if(OK)
		else {
			//log << longtime() << " -- OK = false, one of the temperature sensor not working" << endl;
			if (td_heatingFunc.joinable()) {
				td_heatingFunc.join();
				heating = false;
				mainLog(heatingFuncreturn);
			}
			mainLog("OK = false, one of the temperature sensor not working");
			send("topic", "alert,OK = false");

			for (size_t i = 0; i < Devices.size(); i++) 	Devices[i].OFF();

			/*if (digitalRead(0) != 1) {
				digitalWrite(0, HIGH);
			}
			if (digitalRead(1) != 1) {
				digitalWrite(1, HIGH);
			}
			if (digitalRead(2) != 1) {
				digitalWrite(2, HIGH);
			}
			if (digitalRead(3) != 1) {
				digitalWrite(3, HIGH);
			}*/

			OK = true;

			if (RESET) {
				temperatureSensors.clear();

				mainLog("Reset");
				try {
					throw settingsFromFile("/home/pi/projects/thenewone/bin/ARM/Release/settings.txt");
				}
				catch (string e) {
					if (e != "0") mainLog(e);
				}
				try {
					throw setTimeDiffTemps();
				}
				catch (string e) {
					if (e != "0") mainLog(e);

				}

				RESET = false;
			}
			delay(10500);
		}
	}//while()
	//log << "End: " << longtime();
	first.join();
	//second.join();
	if (heating) td_heatingFunc.join();
	mainLog("Today the soalrpump worked: " + minToTime(solarPumpWT));
	mainLog("Today the boilerpump worked: " + minToTime(boilerPumpWT));
	mainLog("Today the heater worked: " + minToTime(heaterWorkingTime));
	mainLog("Boiler maximums(0=Sunday):");

	for (size_t i = 0; i < boilerMaxofWeek.size(); i++) {
		for (size_t j = 0; j < day.size(); j++) {
			//log << to_string(boilerMaxofWeek[i][j]);
			mainLog(to_string(i) + " - " + to_string(boilerMaxofWeek[i][j]));
		}
	}
	boilerMaxofWeek.clear();
	heaterWorkingTime = 0;
	mainLog("End");
	//log.close();
}

thread Core::basicFuncthread()
{
	std::thread f = std::thread(&Core::basicFunc, this);
	return f;
}

void Core::heaterFunc(int* therm) {

	int start, status = 0, heatingTime = 0, mainPipeStartTemp, gastime = 0;

	if (temperatureSensors[heatingSensors[0]].getTemp() < *therm - houseDiff) {//house heating

		Devices[heatingDevices[2]].OFF();
		start = time(0);
		mainLog("Heating started, mode 1");
		mainPipeStartTemp = temperatureSensors[heatingSensors[2]].getTemp();
		heating = true;

		do {
			if (temperatureSensors[heatingSensors[3]].getTemp() > onlyPumpchimneymin) {//only housepump, becouse the chimney is hot

				Devices[heatingDevices[1]].ON();
				Devices[heatingDevices[0]].OFF();

			}
			else {//gasheater and housepump ON
				if (temperatureSensors[heatingSensors[2]].getTemp() < onlyPump) {

					Devices[heatingDevices[1]].ON();
					Devices[heatingDevices[0]].ON();
					status = 1;

				}
				else {//gasheater OFF, mainpipe < 50C

					Devices[heatingDevices[1]].ON();
					Devices[heatingDevices[0]].OFF();

				}
			}
			delay(5000);

			if (temperatureSensors[heatingSensors[2]].getTemp() < mainPipeStartTemp) mainPipeStartTemp = temperatureSensors[heatingSensors[2]].getTemp();

			if (Devices[heatingDevices[0]].getState() == 0 && (time(0) - heatingTime) > 600 && temperatureSensors[heatingSensors[2]].getTemp() - mainPipeStartTemp <= 1500) {
				sendTopic("alert,Heater is not working!(1043)");
				mainLog("Heater is not working!(1044)");
			}

			if (time(0) - start > 420 && temperatureSensors[heatingSensors[0]].getTemp() > * therm && temperatureSensors[heatingSensors[2]].getTemp() > onlyPump) {
				mainLog("heating finished(mode 1, 1050), it lasted for " + to_string(time(0) - start));
				if (status != 0) {
					mainLog(" sec, with natural gas", false, false);
					status = 0;
				}
				else {
					mainLog(" sec, without natural gas", false, false);
				}
				goto there;
			}
			if (!STOP) goto there1;

		} while (temperatureSensors[heatingSensors[0]].getTemp() < *therm + whilehouseDiff);//dowhileEND

		mainLog("heating finished(mode 1, 1066), it lasted: " + to_string(time(0) - start));
		if (status != 0) {
			mainLog(" sec, with natural gas", false, false);
			status = 0;
		}
		else {
			mainLog(" sec, without natural gas", false, false);
		}
	}
	//lakas > termosztat
there:
	if (temperatureSensors[heatingSensors[1]].getTemp() > heaterMax) {//heatertemp. is higher then allowed

		Devices[heatingDevices[1]].ON();
		Devices[heatingDevices[0]].OFF();

		if (heating != 0) {
			mainLog("heaters temperature reached the allewed temperature(1087)");
		}
		else {
			mainLog("heaters temperature reached the allewed temperature(1091)");
		}
	}
	else {//heatertemp. < 60C;

		Devices[heatingDevices[0]].ON();

		if (temperatureSensors[heatingSensors[2]].getTemp() > afterCirculation && temperatureSensors[heatingSensors[3]].getTemp() < onlyPumpchimneymin) {
			//aftercirculation

			Devices[heatingDevices[1]].ON();

			if (heating != 0)	mainLog("aftercirculation ON", false, false);
			else {
				mainLog("aftercirculation ON", false, false);
			}

		}
		else {//stop aftercirculation

			Devices[heatingDevices[1]].OFF();
			if (heating != 0) {
				mainLog("no aftercirculation");
			}
			else {
				mainLog("aftercirculation OFF");
			}

		}

	}//else kazan > 60
	// lakas > termosztat end
there1:
	//if (!r.empty()) heatingFuncreturn = r;
	if (gastime != 0) {
		vector<int> temp;
		temp.push_back(timeinMin());
		temp.push_back(gastime);
		gasheaterWTs.push_back(temp);
	}
}

void Core::heaterFunc_mode2(int* therm)
{
	int start, status, heatingTime = 0, mainPipeStartTerm, gastime = 0;
	string r;

	if (temperatureSensors[house].getTemp() < *therm - houseDiff) {//house heating

		digitalWrite(0, HIGH);
		start = time(0);
		mainLog("Heating started, mode 2");
		//heatingTime = time(0);
		mainPipeStartTerm = temperatureSensors[mainpipe].getTemp();
		heating = true;
		//cout << longtime() << " -- heating started\n" << "start: " << to_string(start) << endl;
		//cout << "mainpipestartterm: " << to_string(mainPipeStartTerm) << endl;

		if (temperatureSensors[getSensorNumber("gasheater")].getWorking() == 1) {
			do {
				if (temperatureSensors[chimney].getTemp() > onlyPumpchimneymin) {//only housepump
					if (digitalRead(2) != 1) {
						digitalWrite(2, HIGH);
						if (heatingTime != 0) {
							heaterWorkingTime += time(0) - heatingTime;
							gastime += time(0) - heatingTime;
							heatingTime = 0;
						}
					}
					if (digitalRead(3) != 0) {
						digitalWrite(3, LOW);
					}
				}
				else {//2 gasheater and 3 housepump ON
					if (temperatureSensors[mainpipe].getTemp() < onlyPump) {
						if (temperatureSensors[getSensorNumber("gasheater")].getTemp() < onlyPump) {
							if (digitalRead(2) != 0) {
								digitalWrite(2, LOW);
								heatingTime = time(0);
								//cout << "gasheater ON heatingTime: " << to_string(heatingTime) << endl;
								status = 1;
							}
						}
						else {
							if (digitalRead(2) != 1) {
								digitalWrite(2, HIGH);
								heaterWorkingTime += time(0) - heatingTime;
								gastime += time(0) - heatingTime;
								heatingTime = 0;
							};
						}

						if (digitalRead(3) != 0 && temperatureSensors[getSensorNumber("gasheater")].getTemp() > temperatureSensors[mainpipe].getTemp() + 10000) digitalWrite(3, LOW);
						else if (temperatureSensors[getSensorNumber("gasheater")].getTemp() < afterCirculation) digitalWrite(3, HIGH);

						/*if (!digitalRead(2) && tomb[getSensorNumber("gasheater")].getTemp() < tomb[mainpipe].getTemp() && tomb[mainpipe].getTemp() > afterCirculation) {
							digitalWrite(3, LOW);
							heatingMode = 1;
							mainLog("Sensor(gasheater) was not working, heating mode set to 1(1151)");
							r = "heating finished(mode 2, 1148), it lasted for " + to_string(time(0) - start);
							if (status != 0) {
								r = r + " sec, with natural gas";
								status = 0;
							}
							else {
								r = r + " sec, without natural gas";
							}
							r = r + "; ";
							goto there;
						}*/

					}
					else {//gasheater OFF, mainpipe > 50C
						if (digitalRead(2) != 1) {
							digitalWrite(2, HIGH);
							if (heatingTime != 0) {
								heaterWorkingTime += time(0) - heatingTime;
								gastime += time(0) - heatingTime;
								heatingTime = 0;
							}
						}
						if (digitalRead(3) != 0) {
							digitalWrite(3, LOW);
						}
					}
				}
				delay(5000);

				if (!temperatureSensors[getSensorNumber("gasheater")].getWorking() == 1) {
					mainLog("Sensor(gasheater) was not working, heating mode set to 1(1178)");
					heatingMode = 1;
					r = "heating finished(mode 2, 1180), it lasted for " + to_string(time(0) - start);
					if (status != 0) {
						r = r + " sec, with natural gas";
						status = 0;
					}
					else {
						r = r + " sec, without natural gas";
					}
					r = r + "; ";
					goto there;
				}

				if (temperatureSensors[mainpipe].getTemp() < mainPipeStartTerm) mainPipeStartTerm = temperatureSensors[mainpipe].getTemp();

				if (digitalRead(2) == 0 && (time(0) - heatingTime) > 600 && temperatureSensors[mainpipe].getTemp() - mainPipeStartTerm <= 1500) {
					sendTopic("alert,Heater is not working!");
					//cout << "in alert, time(0) - heaterTime: " << to_string(time(0) - heatingTime) << endl;
				}

				if (time(0) - start > 420 && temperatureSensors[house].getTemp() > * therm && temperatureSensors[mainpipe].getTemp() > onlyPump) {
					r = "heating finished(mode 2, 1191), it lasted for " + to_string(time(0) - start);
					if (status != 0) {
						r = r + " sec, with natural gas";
						status = 0;
					}
					else {
						r = r + " sec, without natural gas";
					}
					r = r + "; ";
					goto there;
				}
				if (!STOP) goto there1;

			} while (temperatureSensors[house].getTemp() < *therm + whilehouseDiff);//dowhile

			r = "heating finished(mode 2, 1206), it lasted: " + to_string(time(0) - start);
			if (status != 0) {
				r = r + " sec, with natural gas";
				status = 0;
			}
			else {
				r = r + " sec, without natural gas";
			}
			r = r + "; ";
		}
		else {
			heatingMode = 1;
			r = "Sensor(gasheater) was not working(1227), mode 2 can not be started, heating mode set to 1, ";
			sendTopic("alert,Heating mode set to 1!");
		}
	}	//lakas > termosztat

there:

	if (temperatureSensors[heater].getTemp() > heaterMax) {//heatertemp. is higher then allowed
		if (digitalRead(2) != 1) {
			digitalWrite(2, HIGH);
			if (heatingTime != 0) {
				heaterWorkingTime += time(0) - heatingTime;
				heatingTime = 0;
			}
		}
		if (digitalRead(3) != 0) {
			digitalWrite(3, LOW);
		}
		if (heating != 0)	r = r + "heatertemp. is reached the allewed temperature";
		else {
			r = "heatertemp. is reached the allewed temperature";
		}
	}
	else {//heatertemp. < 60C;
		if (digitalRead(2) != 1) {
			digitalWrite(2, HIGH);
			if (heatingTime != 0) {
				heaterWorkingTime += time(0) - heatingTime;
				gastime += time(0) - heatingTime;
				heatingTime = 0;
			}
		}
		if (temperatureSensors[mainpipe].getTemp() > afterCirculation && temperatureSensors[chimney].getTemp() < onlyPumpchimneymin) {
			//aftercirculation
			if (digitalRead(3) == 1) {
				digitalWrite(3, LOW);
				if (heating != 0)	r = r + "aftercirculation ON";
				else {
					r = "aftercirculation ON";
				}
			}
		}
		else {//stop aftercirculation
			if (digitalRead(3) == 0) {
				digitalWrite(3, HIGH);
				if (heating != 0) r = r + "no aftercirculation";
				else {
					r = "aftercirculation OFF";
				}
			}
		}

	}//else kazan > 60
	// lakas > termosztat end

there1:
	if (!r.empty()) heatingFuncreturn = r;
	if (gastime != 0) {
		vector<int> temp = { timeinMin(),gastime };
		//temp[0] = timeinMin();
		//temp[1] = gastime;
		//temp.push_back(timeinMin());
		//temp.push_back(gastime);
		gasheaterWTs.push_back(temp);
	}
	/*there2:
		digitalWrite(2, HIGH);
		r = "Heating finished(mode2), becouse sensor(gasheater[EXT]) was not fresh, it lasted: " + to_string(time(0) - start);
		if (status != 0) {
			r = r + " sec, with natural gas";
			status = 0;
		}
		else {
			r = r + " sec, without natural gas";
		}
		r = r + "; ";
		*/
}

string Core::solarFunc(string deviceName) {

	if (temperatureSensors[solar].getTemp() - solarDiff > temperatureSensors[solarboiler].getTemp()) {
		
		try {
			if (TEST) cout << "Core - solarFuncON- getStart:" << Devices[getDevicesNr(deviceName)].getStart() << endl;
			throw Devices[getDevicesNr(deviceName)].ON();
			if (TEST) cout << "Core - solarFuncON - getStart:" << Devices[getDevicesNr(deviceName)].getStart() << endl;
		}
		catch (int e) {
			if (e == 0) return "Solarpump ON!";
			else { 
				if (TEST) cout << "Core - solarFuncON - 1366" << endl;
				return "0"; 
			}
		}
				
		/*if (Devs[deviceName].ON() == 0) return "Solarpump set to ON!";
		else { return "0"; }*/
	}
	else {
		try {
			if (TEST) cout << "Core - solarFunc OFF - 1376" << endl;
			throw Devices[getDevicesNr(deviceName)].OFF();
		}
		catch (int e) {
			if (e != 0) return "Solarpump OFF, worked " + to_string(secToMin(e)) + " mins.";
			else { return "0"; }
		}

	}
	return "0";
}

string Core::boilerFunc() {
	if (temperatureSensors[mainpipe].getTemp() - boilerDiff > temperatureSensors[boiler2].getTemp()) {
		if (digitalRead(0) != 0) {
			digitalWrite(0, LOW);
			boilerPumpStart = time(0);
			return "boilerpump ON";
		}
	}
	else {
		if (digitalRead(0) != 1) {
			digitalWrite(0, HIGH);
			boilerPumpWT = boilerPumpWT + (time(0) - boilerPumpStart);
			return "boilerpump OFF";
		}
	}
	return "0";
}

string Core::setTimeDiffTemps() {

	string sor;
	ifstream f;
	f.open("/home/pi/projects/thenewone/bin/ARM/Release/timedifftemps.txt");

	if (f.fail()) return "Unable to open timedifftemps.txt, please look at readme.txt, :)";
	else {
		do {
			size_t z = -1;
			getline(f, sor);
			if (sor.find("#") != z) getline(f, sor);
			if (sor.find("-nightstart") != z) nightStarttime = timeinMin(sor.substr(sor.find(' ') + 1, sor.length()));
			if (sor.find("-nightend") != z) nightEndtime = timeinMin(sor.substr(sor.find(' ') + 1, sor.length()));
			if (sor.find("-solardiff") != z) solarDiff = szam(sor.substr(sor.find(' ') + 1, sor.length()));
			if (sor.find("-housediff") != z) houseDiff = szam(sor.substr(sor.find(' ') + 1, sor.length()));
			if (sor.find("-boilerdiff") != z) boilerDiff = szam(sor.substr(sor.find(' ') + 1, sor.length()));
			if (sor.find("-chimneymin") != z) onlyPumpchimneymin = szam(sor.substr(sor.find(' ') + 1, sor.length()));
			if (sor.find("-onlypump") != z) onlyPump = szam(sor.substr(sor.find(' ') + 1, sor.length()));
			if (sor.find("-aftercirc") != z) afterCirculation = szam(sor.substr(sor.find(' ') + 1, sor.length()));
			if (sor.find("-winterstart") != z) winterStart = szam(sor.substr(sor.find(' ') + 1, sor.length()));
			if (sor.find("-winterend") != z) winterEnd = szam(sor.substr(sor.find(' ') + 1, sor.length()));
			if (sor.find("-heatermax") != z) heaterMax = szam(sor.substr(sor.find(' ') + 1, sor.length()));
			if (sor.find("-thermday") != z) thermostatDay = szam(sor.substr(sor.find(' ') + 1, sor.length()));
			if (sor.find("-thermnight") != z) thermostatNight = szam(sor.substr(sor.find(' ') + 1, sor.length()));
			if (sor.find("-whilehodiff") != z) whilehouseDiff = szam(sor.substr(sor.find(' ') + 1, sor.length()));
		} while (!f.eof());
		f.close();
	}

	mainLog("Times, differences and temperatures are uploaded!",1,0);
	mainLog("winterStart -- " + to_string(winterStart), 1, 0);
	mainLog("winterEnd -- " + to_string(winterEnd), 1, 0);
	mainLog("nightStarttime -- " + to_string(nightStarttime), 1, 0);
	mainLog("nightEndtime -- " + to_string(nightEndtime), 1, 0);
	mainLog("solarDiff -- " + to_string(solarDiff), 1, 0);
	mainLog("houseDiff -- " + to_string(houseDiff), 1, 0);
	mainLog("whilehouseDiff -- " + to_string(whilehouseDiff), 1, 0);
	mainLog("boilerDiff -- " + to_string(boilerDiff), 1, 0);
	mainLog("onlyPumpchimneymin -- " + to_string(onlyPumpchimneymin), 1, 0);
	mainLog("onlyPump -- " + to_string(onlyPump), 1, 0);
	mainLog("afterCirculation -- " + to_string(afterCirculation), 1, 0);
	mainLog("thermostatDay -- " + to_string(thermostatDay), 1, 0);
	mainLog("thermostatNight -- " + to_string(thermostatNight), 1, 0);
	mainLog("heaterMax -- " + to_string(heaterMax), 1, 0);
	return "0";
}

//Log functions
int Core::timeinMin() {
	struct tm* theTime;
	int hour, min;
	time_t tim;
	time(&tim);
	theTime = localtime(&tim);
	hour = theTime->tm_hour;
	min = theTime->tm_min;
	return hour * 60 + min;
}

int Core::timeinMin(string x) {
	//string x = "6:00"
	int hour = szam(x.substr(0, x.find(':')));
	int min = szam(x.substr(x.find(':')));
	return hour * 60 + min;
}

string Core::longtime() {
	std::time_t result = std::time(nullptr);
	//std::cout << std::asctime(std::localtime(&result));
	//time_t now = szam(time(nullptr));
	//auto now;
	//auto now = time(NULL);
	//char* dt = asctime(localtime(&now));
	//char* dt = ctime(szam(time(0)));
	//return string(dt);
	string ltime = std::asctime(std::localtime(&result));
	return ltime.substr(0, ltime.length() - 1);
}

string Core::date() {
	string sor = longtime();
	return sor.substr(sor.length() - 21, 6);
}

int Core::month() {
	struct tm* theTime;
	time_t tim;
	time(&tim);
	theTime = localtime(&tim);
	return theTime->tm_mon;
}

int Core::dayOftheweek() {
	struct tm* theTime;
	time_t tim;
	time(&tim);
	theTime = localtime(&tim);
	return theTime->tm_wday;
}

string Core::filename(string path) {
	//string ev, honap, nap, sor;
	//sor = longtime();

	struct tm* theTime;
	char buffer[80];
	time_t tim;
	time(&tim);
	theTime = localtime(&tim);

	strftime(buffer, 80, "%m%d%y.txt", theTime);

	/*honap = theTime->tm_wday;
	nap = theTime->tm_mday;

	//Tue Nov 10 12:37:07 2015   24db 0-23
	ev.append(sor.substr(sor.length() - 3, 2));
	honap.append(sor.substr(sor.length() - 21, 3));
	nap.append(sor.substr(sor.length() - 17, 2));
	if (honap == "Jan")
		honap = "01";
	if (honap == "Feb")
		honap = "01";
	if (honap == "Mar")
		honap = "03";
	if (honap == "Apr")
		honap = "04";
	if (honap == "May")
		honap = "05";
	if (honap == "Jun")
		honap = "06";
	if (honap == "Jul")
		honap = "07";
	if (honap == "Aug")
		honap = "08";
	if (honap == "Sep")
		honap = "09";
	if (honap == "Oct")
		honap = "10";
	if (honap == "Nov")
		honap = "11";
	if (honap == "Dec")
		honap = "12";
	///home/pi/Desktop/cb/log/tempslog/temps" +
	return path + honap + nap + ev + ".txt";*/
	return path + buffer;
}

string Core::tempslogFirstRow()
{
	string x = "date";
	for (size_t i = 0; i < temperatureSensors.size(); i++) {
		x += ";";
		x += temperatureSensors[i].getName();
	}
	x += "\n";
	return x;
}

void Core::logThread() {
	int tempslogI = 1;
	ofstream web, tempslog;
	vector<int> day;

	//web.open(webfilename.c_str(), ios_base::app);
	//"/home/pi/Desktop/cb/log/strekszel.txt"
	//if(web.fail()) mainLog("webfname fail: " + webfilename + "\n");
	//if(web.fail()) cout << "webfname fail: " << webfilename << endl;
	tempsFilename = filename(tempsfilename);
	//cout << tempsFilename << endl;
	tempslog.open(tempsFilename.c_str(), ios_base::app);
	if (tempslog.fail()) {
		mainLog("tempsfilename fail: " + tempsfilename + "\n");
		sendTopic("alert,fail to open tempsfilename");
	}
	else {
		//if(tempslog.fail()) cout << "tempfilename fail: " << tempsfilename << endl;
		//tempslog << "datum ; cso ; bojler teteje ; felso hocserelo ; kazan ; lakas ; kemeny ; napko. ; napk.bojler" << endl;
		tempslog << tempslogFirstRow();
		tempslog.close();
	};


	if (dayOftheweek() != 0 && tempslogI == 1) tempslogI = 0;
	if (dayOftheweek() == 0 && tempslogI == 0) {
		tempslog.open(tempsFilename.c_str(), ios_base::app);
		tempslog << "End: " << longtime() << endl;
		tempslog.close();
		tempsFilename = filename(tempsfilename);
		tempslog.open(tempsFilename.c_str(), ios_base::app);
		tempslog << tempslogFirstRow();
		tempslog.close();
		//tempslog << "datum ; ido ; cso ; bojler teteje ; felso hocserelo ; kazan ; lakas ; kemeny ; napko. ; napk.bojler" << endl;
		//tempslog << longtime() << ";" << mainpipe << ";" << boiler1 << ";" << boiler2 << ";" << heater << ";" << house << ";" << chimney << ";" << solar << ";" << solarboiler << ";" << endl;
		tempslogI = 1;
	}
	else {
		//tempslog << longtime();
		/*for (int i = 0; i < tomb.size(); i++) {
			tempslog << " ;" << tomb[i].getTemp();
		}
		tempslog << endl;*/
		if (dayOftheweek() != 0) tempslogI = 0;
	}
	/*if (fileI == 2) {
		web.close();
		web.open(webfilename.c_str());
		web << longtime();
		for (int i = 0; i < meret; i++) {
			web << "; " << tomb[i].getTemp();
		}
		web << endl;
		fileI = 0;
	}
	else {
		web << longtime();
		for (int i = 0; i < meret; i++) {
			web << "; " << tomb[i].getTemp();
		}
		web << endl;
		fileI = fileI + 1;
	}*/

	delay(45500);

	tempslog.open(tempsFilename.c_str(), ios_base::app);
	tempslog << "End: " << longtime() << endl;
	tempslog.close();
}

void Core::isboilerMax() {
	if (temperatureSensors[boiler1].getTemp() > boilerMax) boilerMax = temperatureSensors[boiler1].getTemp();
}

int Core::getboilerMax()
{
	return boilerMax;
}

int Core::getSensorMax(string sensorName)
{
	return temperatureSensors[getSensorNumber(sensorName)].getMax();
}

int Core::getSensorMin(string sensorName)
{
	return temperatureSensors[getSensorNumber(sensorName)].getMin();
}

string Core::getgasheaterWTs()
{
	string r;
	if (gasheaterWTs.size() > 0) {
		for (size_t i = 0; i < gasheaterWTs.size(); i++) {
			r += to_string(gasheaterWTs[i][0]);
			r += " - ";
			r += to_string(gasheaterWTs[i][1]);
			r += "\n";
		}

	}
	else {
		r = "No data(gasheaterWTs)\n";
	}

	return r;
}

string Core::getDeviceWTs(string deviceName)
{
	return Devices[getDevicesNr(deviceName)].getWTs();
}

int Core::avgHeatingTime()
{
	if (gasheaterWTs.size() > 0) {

		double avg = 0;

		for (size_t i = 0; i < gasheaterWTs.size(); i++) {
			avg += gasheaterWTs[i][1];
		}

		avg = avg / gasheaterWTs.size();

		return secToMin(&avg);
	}
	else {
		return 0;
	}
}

int Core::sqrtAvgHeatingTime()
{
	if (gasheaterWTs.size() > 0) {

		double avg = 0;

		for (size_t i = 0; i < gasheaterWTs.size(); i++) {
			avg += pow(gasheaterWTs[i][1], 2);
		}

		avg = avg / gasheaterWTs.size();

		avg = sqrt(avg);

		return secToMin(&avg);
	}
	else {
		return 0;
	}
}

string Core::commFunc(string mes)
{
	static const size_t npos = -1;
	/*SUSPENDED
	try{
		throw changeInFile(mes, "/home/pi/projects/thenewone/bin/ARM/Release/timedifftemps.txt");
	}
	catch (int z) {
		//NEED REWORK
		if (z == 0) mainLog(mes + " ,has been written in file! :)");
		else if (z == CANTOPENFILE) mainLog("Filename was wrong(changeInFile())! :(");
		else if (z == WRONGFORMAT) mainLog(mes + " ,format was wrong(<=> missing)! :(");
		else if (z == NOMATCHINGLINE) mainLog(mes + " ,could not find matching line! :(");
	}*/

	/*"thermnight","help","thermday","nightstart","nightend","getthermd","getwts",
		"gettdt","winterstart", "winterend","gettemps","solardiff","housediff",*/
	if (mes.find("thermnight") != npos) {
		int neu = szam(mes.substr(mes.find('=') + 1));
		string rep;
		if (neu < 15000 || neu > thermostatDay) {
			rep = "fault new thermNight was invalid value: " + to_string(neu);
		}
		else {
			rep = "thermNight changed from: " + to_string(thermostatNight);
			thermostatNight = neu;
			rep = rep + ", to: " + to_string(thermostatNight);
		}
		return rep;
	}
	else if (mes.find("help") != npos) {
		string rep;
		rep = "Every Celsius value looks like, 21.0 = 21000\n";
		rep += "<getthermd> to get day thermostat\n";
		rep += "<getwts> to get working times(heater, pumps)\n";
		rep += "<gettdt> to get times, diff. end temperatures\n";
		rep += "<gettemps> to get the actual temperatures\n";
		rep += "<thermnight>=<value>, to set night thermostat\n";
		rep += "<thermday>=<value> to set day thermostat\n";
		rep += "<nstarttime>=<time in min> to set night start time\n";
		rep += "<nendtime>=<time in min> to set night end time\n";
		rep += "<winterstart>=<month(0-11)> to set the winter start month\n";
		rep += "<winterend>=<month(0-11)> to set the winter end month\n";
		rep += "<solardiff>=<value> to set the diff. between solar temp. and the boilerbottom temp., when the solarpump starts working\n";
		rep += "<housediff>=<value> to set the diff. between house temp. and the thermostat, when the househeating starts\n";
		rep += "<whilehousediff>=<value> to setwhilehousediff, if house temp. reachs (thermostat+whilehousediff) stop heating\n";
		rep += "<boilerdiff>=<value> to set boilerdiff, if mainpipe temp. higher the (boilermid + boilerdiff), boilerpump start working\n";
		rep += "<onlypumpchimney>=<value> to set onlypumpchimney, if you do not heat with gas, the chimney temp. higher, than this value, the gasheater switchs OFF\n";
		rep += "<onlypump>=<value> to set onlypump, if mainpipe is higher(when heating with gas), than this value, gasheater switch OFF\n";
		rep += "<aftercirculation>=<value> to set aftercirculation value, after heating, if mainpipe temp. higher, than this value the housepump keeps on working\n";
		rep += "<heatermax>=<value> to set the heater max. temp.";
		rep += "<getgpio> to get the gpios\'s state\n";
		rep += "<getsnames> to get the sensor\'s names \n";
		rep += "<Reset> to reset sensors\'s filenames and times,diff., temps\n";
		rep += "<pause> to pause the basicFunc()\n";
		rep += "<continue> to continue the basicFunc()\n";
		rep += "<exit> to close the application\n";
		rep += "<mqttreconnect> to reconnect mqtt client\n";
		rep += "<mqttdisconnect> to disconnect mqtt client\n";
		rep += "<mqttreset> it works like the reconnect should!\n";
		return rep;
	}
	else if (mes.find("thermday") != npos) {
		int neu = szam(mes.substr(mes.find('=') + 1));
		string rep;
		//cout << "cmd(), thermostatDay modified from: " << thermostatDay;
		if (neu < 15000 || neu > 24000) {
			rep = "fault new thermDay was invalid value: " + to_string(neu);
		}
		else {
			rep = "thermDay changed from: " + to_string(thermostatDay);
			thermostatDay = neu;
			rep = rep + ", to: " + to_string(thermostatDay);
		}
		return rep;
	}
	else if (mes.find("nightstart") != npos) {
		int neu = szam(mes.substr(mes.find('=') + 1));
		string rep;
		if (neu < 0 || neu >1439) {
			rep = "fault new nightStartTime was invalid value: " + to_string(neu);
		}
		else {
			rep = "nightStarttime changed from: " + to_string(nightStarttime);
			nightStarttime = neu;
			rep = rep + ", to: " + to_string(nightStarttime);
		}
		return rep;
	}
	else if (mes.find("nightend") != npos) {
		int neu = szam(mes.substr(mes.find('=') + 1));
		//cout << "cmd(), nightEndtime modified from: " << nightEndtime;
		string rep;
		if (neu < 0 || neu > 1439) {
			rep = "fault new nightEndTime was invalid value: " + to_string(neu);
		}
		else {
			rep = "nightStarttime changed from: " + to_string(nightEndtime);
			nightEndtime = neu;
			rep = rep + ", to: " + to_string(nightEndtime);
		}
		return rep;
	}
	else if (mes.find("getthermd") != npos) {
		return getThermd();
	}
	else if (mes.find("getwts") != npos) {
		return getWT();
	}
	else if (mes.find("gettdt") != npos) {
		return getTDT();
	}
	else if (mes.find("gettemps") != npos) {
		return getTemps();
	}
	else if (mes.find("winterstart") != npos) {
		//cout << "cmd(), wS()";
		int neu = szam(mes.substr(mes.find('=') + 1));
		string rep;
		if (neu < 0 || neu > 11) {
			rep = "fault new winterStart was invalid value: " + to_string(neu);
		}
		else {
			rep = "winterStart changed from: " + to_string(winterStart);
			winterStart = neu;
			rep = rep + ", to: " + to_string(winterStart);
		}
		return rep;
	}
	else if (mes.find("winterend") != npos) {
		int neu = szam(mes.substr(mes.find('=') + 1));
		string rep;
		if (neu < 0 || neu > 11) {
			rep = "fault new winterEnd was invalid value: " + to_string(neu);
		}
		else {
			rep = "winterEnd changed from: " + to_string(winterEnd);
			winterEnd = neu;
			rep = rep + ", to: " + to_string(winterEnd);
		}
		return rep;
	}
	else if (mes.find("solardiff") != npos) {
		int neu = szam(mes.substr(mes.find('=') + 1));
		string rep;
		if (neu < 0 || neu > 50000) {
			rep = "fault new solarDiff was invalid value: " + to_string(neu);
		}
		else {
			string rep = "solarDiff changed from: " + to_string(solarDiff);
			solarDiff = neu;
			rep = rep + ", to: " + to_string(solarDiff);
		}
		return rep;
	}
	else if (mes.find("housediff") != npos) {
		int neu = szam(mes.substr(mes.find('=') + 1));
		string rep;
		if (neu < 0 || neu > 5000) {
			rep = "fault new houseDiff was invalid value: " + to_string(neu);
		}
		else {
			rep = "houseDiff changed from: " + to_string(houseDiff);
			houseDiff = neu;
			rep = rep + ", to: " + to_string(houseDiff);
		}
		return rep;
	}
	else if (mes.find("whilehodiff") != npos) {
		int neu = szam(mes.substr(mes.find('=') + 1));
		string rep;
		if (neu < 0 || neu > 2000) {
			rep = "fault new whileHouseDiff was invalid value: " + to_string(neu);
		}
		else {
			rep = "whilehouseDiff changed from: " + to_string(whilehouseDiff);
			whilehouseDiff = neu;
			rep = rep + ", to: " + to_string(whilehouseDiff);
		}
		return rep;
	}
	else if (mes.find("boilerdiff") != npos) {
		int neu = szam(mes.substr(mes.find('=') + 1));
		string rep;
		if (neu < 0 || neu > 20000) {
			rep = "fault new boilerDiff was invalid value: " + to_string(neu);
		}
		else {
			rep = "boilerDiff changed from: " + to_string(boilerDiff);
			boilerDiff = neu;
			rep = rep + ", to: " + to_string(boilerDiff);
		}
		return rep;
	}
	else if (mes.find("onlypumpchimneymin") != npos) {
		int neu = szam(mes.substr(mes.find('=') + 1));
		string rep;
		if (neu < 20000 || neu > 35000) {
			rep = "fault new onlyPumpChimneyminimum was invalid value: " + to_string(neu);
		}
		else {
			rep = "onlyPumpchimneyminimum changed from: " + to_string(onlyPumpchimneymin);
			onlyPumpchimneymin = neu;
			rep = rep + ", to: " + to_string(onlyPumpchimneymin);
		}
		return rep;
	}
	else if (mes.find("onlypump") != npos) {
		int neu = szam(mes.substr(mes.find('=') + 1));
		string rep;
		if (neu < 30000 || neu > 65000) {
			rep = "fault new onlyPump was invalid value: " + to_string(neu);
		}
		else {
			rep = "onlyPump changed from: " + to_string(onlyPump);
			onlyPump = neu;
			rep = rep + ", to: " + to_string(onlyPump);
		}
		return rep;
	}
	else if (mes.find("aftercirc") != npos) {
		int neu = szam(mes.substr(mes.find('=') + 1));
		string rep;
		if (neu < 30000 || neu > 60000) {
			rep = "fault new afterCirculation was invalid value: " + to_string(neu);
		}
		else {
			rep = "afterCirculation changed from: " + to_string(afterCirculation);
			afterCirculation = neu;
			rep = rep + ", to: " + to_string(afterCirculation);
		}
		return rep;
	}
	else if (mes.find("heatermax") != npos) {
		int neu = szam(mes.substr(mes.find('=') + 1));
		string rep;
		if (neu < 40000 || neu > 70000) {
			rep = "fault new heaterMax was invalid value: " + to_string(neu);
		}
		else {
			rep = "heaterMax changed from: " + to_string(heaterMax);
			heaterMax = neu;
			rep = rep + ", to: " + to_string(heaterMax);
		}
		return rep;
	}
	else if (mes.find("getgpio") != npos) {
		return getGPIO();
	}
	else if (mes.find("Reset") != npos) {
		/*RESET = false;
		if (digitalRead(0) != 1) {
			digitalWrite(0, HIGH);
		}
		if (digitalRead(1) != 1) {
			digitalWrite(1, HIGH);
		}
		if (digitalRead(2) != 1) {
			digitalWrite(2, HIGH);
		}
		if (digitalRead(3) != 1) {
			digitalWrite(3, HIGH);
		}*/
		return "Did nothing at the moment, Resetting...\n";
	}
	else if (mes.find("exit") != npos) {
		STOP = false;
		if (digitalRead(0) != 1) {
			digitalWrite(0, HIGH);
		}
		if (digitalRead(1) != 1) {
			digitalWrite(1, HIGH);
		}
		if (digitalRead(2) != 1) {
			digitalWrite(2, HIGH);
		}
		if (digitalRead(3) != 1) {
			digitalWrite(3, HIGH);
		}
		return "Application closing... ";
	}
	else if (mes.find("pause") != npos) {
		OK = false;
		if (digitalRead(0) != 1) {
			digitalWrite(0, HIGH);
		}
		if (digitalRead(1) != 1) {
			digitalWrite(1, HIGH);
		}
		if (digitalRead(2) != 1) {
			digitalWrite(2, HIGH);
		}
		if (digitalRead(3) != 1) {
			digitalWrite(3, HIGH);
		}
		return "mainthread is paused\n";
	}
	else if (mes.find("continue") != npos) {
		OK = true;
		return "mainthread is working\n";
	}
	else if (mes.find("mqttreconnect") != npos) {
		int ret = this->reconnect();
		if (ret == 0) {

			//this->subscribe(clint_id, MQTT_TOPIC, 2);
			//this->subscribe(clint_id, "extsensors", 2);

			mainLog("MQTT client reconnected!\n");
			return "MQTT client reconnected!\n";
		}
		else {
			mainLog("MQTT client reconnect error number: " + ret + '\n');
			return "error number: " + ret + '\n';
		}
	}
	else if (mes.find("mqttdisconnect") != npos) {
		int ret = this->disconnect();
		if (ret == 0) {
			mainLog("MQTT disconnected!");
			return "MQTT disconnected!\n";
		}
		else {
			mainLog("MQTT disconnect error number: " + ret + '\n');
			return "error number: " + ret + '\n';
		}
	}
	else if (mes.find("mqttreset") != npos) {
		int ret = this->mqttReset();
		if (ret == 0) {
			mainLog("MQTT resetted!");
			return "MQTT resetted! :)\n";
		}
		else {
			mainLog("MQTT reset error number: " + to_string(ret));
			return ("error number: " + to_string(ret) + '\n');
		}
	}
	else if (mes.find("getsnames") != npos) {
		return getSensorNames();
	}
	else if (mes.find("addES") != npos) {
		//cout << mes.substr(mes.find('=') + 1) << endl;
		try {
			throw addExtTempSensor(mes.substr(mes.find('=') + 1));
		}
		catch (int x) {
			if (x == 0) return "Sensor added!";
			else {
				return ("Error: " + to_string(x));
			}
		}
	}
	else if (mes.find("delES") != npos) {
		//cout << mes.substr(mes.find('=')+1) << endl;
		try {
			throw delExtTempSensor(mes.substr(mes.find('=') + 1));
		}
		catch (int x) {
			if (x == 0) return "Sensor deleted!";
			else {
				return ("Error: " + to_string(x));
			}
		}
	}
	else if (mes.find("setHM") != npos) {
		int neu = szam(mes.substr(mes.find('=') + 1));
		try {
			throw setHeatingMode(neu);
		}
		catch (int x) {
			if (x == 0) return("Heating mode set to: " + to_string(heatingMode));
			else if (x == 0) return("Heating mode failed to set: " + to_string(heatingMode));
			else {
				return("Something went wrong, heating mode tried to set: " + to_string(neu));
			}
		}
	}
	else if (mes.find("getHM") != npos) {
		return("Heating mode: " + to_string(heatingMode));
	}
	else if (mes.find("getgasWT") != npos) {
		return(getgasheaterWTs());
	}
	else if (mes.find("getdata=") != npos) {
		size_t neu = szam(mes.substr(mes.find('=') + 1));
		if (neu > temperatureSensors.size() || neu < 0) return "Wrong number!";
		else {
			return temperatureSensors[neu].getAllData();
		};
	}
	else if (mes.find("getavght") != npos) {

		return ("Avg: " + to_string(avgHeatingTime()) + ", sqrtAvg: " + to_string(sqrtAvgHeatingTime()));

	}
	else if (mes.find("getvlog") != npos) {

		return getVlog();

	}
	else if (mes.find("addDevice") != npos) {
		string deviceName, gpio;
		int GPIO;

		cout << "Please type the name of the Device: ";
		cin >> deviceName;
		cout << "Please type the GPIO number which used by the Device: ";
		cin >> gpio;
		GPIO = szam(gpio);

		Relay newRelay = Relay(deviceName, GPIO, TEST);
		Devices.push_back(newRelay);

		return "Device uploaded!";
	}
	else if (mes.find("delDevice") != npos) {
		//string deviceName;
		return "Ez nem mukodik!";
		//cout << "Please type the name of the Device, or use ls to show the used Device names: ";
		/*cin >> deviceName;
		if (deviceName == "ls") {
			cout << Devices.getDevicesData() << endl;
			cout << "Please type the name of the Device: ";
			cin >> deviceName;
		}

		try {
			throw Devices.delDevice(deviceName);
		}
		catch (int e) {
			if (e == 0) return "Device is succsesfully deleted!";
			else {
				return "Something went wrong!";
			}
		}*/

	}
	else if (mes.find("lsDevices") != npos) {

		return getDevicesData();

	}
	else if (mes.find("setDevice") != npos) {
		string name;
		int state;
		// setDevice=name,1

		cout << "mes: " << mes << endl;
		name = mes.substr(mes.find_first_of("=") + 1, mes.find(",") - mes.find("="));
		state = szam(mes.substr(mes.find(",") + 1, mes.length() - mes.find(",") - 1));

		//cout << mes.substr(mes.find_first_of("=") + 1, mes.find("=") - mes.find(",")) << endl;
		//cout << mes.find("=") << endl;
		//cout << mes.find(",") << endl;
		cout << "setDevices: " << name << ", state: " << state << endl;

		if (state == 0) {
			//Devs[name].OFF();
			return  name + " device switced OFF, from terminal.";
		}
		else if (state == 1) {
			//Devs[name].ON();
			return  name + " device switced ON, from terminal.";
		}
		else {
			return "You gave wrong state, it can be 0(OFF) or 1(ON)";
		}


	}
	else if (mes.find("getAVGtemps") != npos) {

		for (size_t i = 0; i < temperatureSensors.size(); i++) {

			cout << temperatureSensors[i].getName() << ": " << temperatureSensors[i].getAvgOfDay() << endl;

		}

		return "\n";

	}
	else if (mes.find("setHeatingSensors") != npos) {

		cout << endl;
		cout << "Uploaded sensors:" << endl;
		for (size_t i = 0; i < temperatureSensors.size(); i++) {
			if (i != 0) cout << "\n";
			cout << i + 1 << ". " << temperatureSensors[i].getName();
		}
		cout << "Current heating sensors settings: ";
		for (size_t i = 0; i < heatingSensors.size(); i++) {
			if (i != 0) cout << ",";
			cout << heatingSensors[i];
		}
		cout << endl << "Give the new sensors\' numbers, separated with <,>: ";
		string row;
		cin >> row;
		try
		{
			vectorUploader(row, heatingSensors);
		}
		catch (const int e)
		{
			if (e) { 
				cout << "Sensors updated: "; 
				for (size_t i = 0; i < heatingSensors.size(); i++) {
					if (i != 0) cout << ",";
					cout << heatingSensors[i];
				}
				cout << endl;
			}
			else {
				cout << "Something went wrong" << endl;
			}

		}

		return "\n";
	}
	else if (mes.find("setHeatingDevices") != npos) {

	cout << endl;
	cout << "Uploaded Devices:" << endl;
	cout << getDevicesData();
	cout << "Current heating devices settings: ";
	for (size_t i = 0; i < heatingDevices.size(); i++) {
		if (i != 0) cout << ",";
		cout << heatingSensors[i];
	}
	cout << endl << "Give the new sensors\' numbers, separated with <,>: ";
	string row;
	cin >> row;
	try
	{
		vectorUploader(row, heatingDevices);
	}
	catch (const int e)
	{
		if (e) {
			cout << "Sensors updated: ";
			cout << getDevicesData();
		}
		else {
			cout << "Something went wrong" << endl;
		}

	}
		
		return "\n";
	}
	else if (mes.find("getHeatingSensors") != npos) {
		cout << endl;
		cout << "Current heating sensors settings: ";
		for (size_t i = 0; i < heatingSensors.size(); i++) {
			if (i != 0) cout << ",";
			cout << heatingSensors[i];
		}
		cout << endl;
		cout << "Uploaded sensors:" << endl;
		for (size_t i = 0; i < temperatureSensors.size(); i++) {
			if (i != 0) cout << "\n";
			cout << i + 1 << ". " << temperatureSensors[i].getName();
		}
		return "\n";
	}
	else if (mes.find("getHeatingDevices") != npos) {
		cout << endl;
		cout << "Current heating devices settings: ";
		for (size_t i = 0; i < heatingDevices.size(); i++) {
			if (i != 0) cout << ",";
			cout << heatingDevices[i];
		}
		cout << endl << "Uploaded Devices:" << endl;
		cout <<  getDevicesData();
		return "\n";
	}
	else if (mes.find("TESTON") != npos) {

		TEST = true;
		return "TEST mode ON";

	}
	else if (mes.find("TESTOFF") != npos) {
		
		TEST = false;
		return "TEST mode OFF";

	}

	return "No matching command! type <help> for commands, pls.";
}

string Core::getTemps() {
	//string temps;

	/*for (size_t i = 0; i < temperatureSensors.size(); i++) {
		if (i != 0) temps = temps + "\n";
		temps = temps + temperatureSensors[i].getName() + "=" + to_string(temperatureSensors[i].getTemp());
	}*/

	string temps = "focso =" + to_string(temperatureSensors[0].getTemp()) + "\n";
	temps = temps + "bojler1 =" + to_string(temperatureSensors[1].getTemp()) + "\n";
	temps = temps + "bojler2 =" + to_string(temperatureSensors[2].getTemp()) + "\n";
	temps = temps + "kazan =" + to_string(temperatureSensors[3].getTemp()) + "\n";
	temps = temps + "lakas =" + to_string(temperatureSensors[4].getTemp()) + "\n";
	temps = temps + "kemeny =" + to_string(temperatureSensors[5].getTemp()) + "\n";
	temps = temps + "napkollektor =" + to_string(temperatureSensors[6].getTemp()) + "\n";
	temps = temps + "napkollektorbojler =" + to_string(temperatureSensors[7].getTemp()) + "\n";

	return temps;
}

int Core::mainLog(string str, bool newRow, bool time) {
	ofstream f;
	f.open(logFilename.c_str(), ios_base::app);
	if (time && newRow)	f << longtime() << " --- " << str << endl;
	if (!time && !newRow) f << str;
	if (!time && newRow) f << str << endl;
	if (time && !newRow) f << longtime() << " --- " << str;

	f.close();

	pushBackvLog(&str);

	return 0;
}

int Core::fileNameLog(string name, string kind)
{
	ofstream of;
	ifstream inf;
	string x, y;

	if (kind == "main") {

		inf.open(y.c_str());

		y = MAINLOGFILEPATH;
		do {
			getline(inf, x);
			if (name == x) {
				inf.close();
				return 1;
			}
		} while (!inf.eof());
		inf.close();

		of.open(y.c_str(), ios_base::app);
		of << name << endl;
		of.close();
	}
	else if (kind == "temps") {

		inf.open(y.c_str());

		y = TEMPSLOGFILEPATH;
		do {
			getline(inf, x);
			if (name == x) {
				inf.close();
				return 1;
			}
		} while (!inf.eof());
		inf.close();

		of.open(y.c_str(), ios_base::app);
		of << name << endl;
		of.close();
	}

	return 0;
}

void Core::pushBackvLog(string* x)
{
	if (vlog.size() >= 24) {

		vector<string> v;

		for (size_t i = 1; i < vlog.size(); i++) {
			v.push_back(vlog[i]);
		}

		v.push_back(minToTime(timeinMin()) + " - " + *x + "\n");

		vlog.swap(v);
		v.clear();
	}
	else {
		vlog.push_back(minToTime(timeinMin()) + " - " + *x + "\n");
	}
}

string Core::getVlog()
{
	string x;

	if (!vlog.empty()) {
		for (size_t i = 0; i < vlog.size(); i++) {
			x += vlog[i];
		}

		return x;
	}
	else {
		return "vlog is empty!";
	}
}

void Core::mqttLog(string str) {
	ofstream f;
	f.open("/home/pi/Desktop/log/thenewonemqttlog.txt", ios_base::app);
	f << longtime() << " -- " << str << endl;
	f.close();
}

void Core::tempsLog()
{
	ofstream f;
	f.open(tempsFilename.c_str(), ios_base::app);
	f << endl << longtime();
	for (size_t i = 0; i < temperatureSensors.size(); i++) {
		f << ";" << temperatureSensors[i].getTemp();
	}
	if (TEST) f << "TEST";
	f.close();
}

void Core::AIsamples()
{
	ofstream f;
	f.open("/home/pi/Desktop/log/AI/samples.txt", ios_base::app);
	f << endl << longtime();
	for (size_t i = 0; i < temperatureSensors.size(); i++) {
		f << ";" << temperatureSensors[i].getTemp();
	}

	f << ";";
	f << digitalRead(0) << ";";
	f << digitalRead(1) << ";";
	f << digitalRead(2) << ";";
	f << digitalRead(3) << ";";
	f.close();
}

void Core::tempsLog(string str)
{
	ofstream f;
	f.open(tempsFilename.c_str(), ios_base::app);
	f << longtime() << str;
	f.close();
}

void Core::debugLog(string str) {
	ofstream f;
	f.open("/home/pi/Desktop/log/debug.txt", ios_base::app);
	f << longtime() << " -- " << str << endl;
	f.close();
}

string Core::getTDT() {
	string tdt;
	tdt = "thermostatNight=" + to_string(thermostatNight) + "\n";
	tdt = tdt + "nightStarttime=" + to_string(nightStarttime) + "\n";
	tdt = tdt + "nightEndtime=" + to_string(nightEndtime) + "\n";
	tdt = tdt + "heaterMax=" + to_string(heaterMax) + "\n";
	tdt = tdt + "solarDiff=" + to_string(solarDiff) + "\n";
	tdt = tdt + "houseDiff=" + to_string(houseDiff) + "\n";
	tdt = tdt + "boilerDiff=" + to_string(boilerDiff) + "\n";
	tdt = tdt + "whilehouseDiff=" + to_string(whilehouseDiff) + "\n";
	tdt = tdt + "onlyPumpchimneymin=" + to_string(onlyPumpchimneymin) + "\n";
	tdt = tdt + "onlyPump=" + to_string(onlyPump) + "\n";
	tdt = tdt + "afterCirculation=" + to_string(afterCirculation) + "\n";
	tdt = tdt + "winterStart=" + to_string(winterStart) + "\n";
	tdt = tdt + "winterEnd=" + to_string(winterEnd) + "\n";

	return tdt;
}

string Core::getThermd()
{
	return ("getthermD=" + to_string(thermostatDay));
}

//old getGPIO, will be deleted soon
string Core::getGPIO()
{
	string x;
	x += "gpio,0=";
	x += to_string(digitalRead(0));
	x += ",1=";
	x += to_string(digitalRead(1));
	x += ",2=";
	x += to_string(digitalRead(2));
	x += ",3=";
	x += to_string(digitalRead(3));
	return x;
}

//NEW - gives back the devs state(ON - OFF)
//i should use a better way to handle these lines <dev.name> <state>!!!!
string Core::getGPIOState()
{
	string x;
	x = "gpio,";

	for (size_t i = 0; i < Devices.size(); i++) {
		x += to_string(Devices[i].getPIN()) + "=" + to_string(Devices[i].getState()) + ", ";
	}
	return x;
}

//old getWT, will be deleted soon
string Core::getWT()
{
	string x;
	x = "WTs gasheater=";
	x += minToTime(Devices[getDevicesNr("Gasheater")].getWorkingTime());
	x += ",solar=";
	x += minToTime(Devices[getDevicesNr("Solarpump")].getWorkingTime());
	x += ",boiler=";
	x += minToTime(Devices[getDevicesNr("Boilerpump")].getWorkingTime());
	return x;
}

//NEW - gives back the devices' actual working times(hour:min) for the Android app
//i should use a better way to handle these lines <dev.name> <time>!!!!
string Core::newGetWT() {

	string x;
	x = "WTs ";

	for (size_t i = 0; i < Devices.size(); i++) {
		x += Devices[i].getName() + "=" + to_string(Devices[i].getWorkingTime()) + ", ";
	}
	return x;
}

int Core::getTemp(string name, bool x = false)
{
	for (size_t i = 0; i < temperatureSensors.size(); i++) {
		if (temperatureSensors[i].getName() == name && temperatureSensors[i].getEXT() == x) return temperatureSensors[i].getTemp();
	}
	return 0;
}

int Core::getSensorNumber(string x)
{
	for (size_t i = 0; i < temperatureSensors.size(); i++) {
		if (temperatureSensors[i].getName() == x) return i;
	}

	return -1;
}

int Core::getHeatingMode()
{
	return heatingMode;
}

int Core::setHeatingMode(int x)
{
	if (x == 1 || x == 2) {
		heatingMode = x;
		mainLog("Heating mode set to: " + to_string(heatingMode));
		return SUCCES;
	}
	else {
		mainLog("Heating mode failed set to: " + to_string(heatingMode));
		return WRONGMODE;
	}

}

int Core::changeInFile(string s, const char* fileName) {
	ofstream of;
	ifstream inf;
	vector<string> vec;
	string getLine, name;
	bool ok = false, SUC = false;
	static const size_t npos = -1;

	if (s.find("=") != npos) {
		name = s.substr(0, s.find("="));
		if (name == "nightstart") s.replace(s.find("=") + 1, s.length() - s.find("=") + 1, minToTime(nightStarttime));
		else if (name == "nightend") s.replace(s.find("=") + 1, s.length() - s.find("=") + 1, minToTime(nightEndtime));
		s.insert(0, "-");
		s.replace(s.find("="), 1, " ");
	}
	else {
		return WRONGFORMAT;
	}

	inf.open(fileName);
	if (inf.fail()) {
		return CANTOPENFILE;
	}
	while (!inf.eof()) {
		getline(inf, getLine);
		//if (ok) cout << getLine << endl;
		if (getLine != "\n") vec.push_back(getLine);
	}
	inf.close();
	of.open(fileName);
	//if (ok) cout << vec.size() << endl;
	for (size_t i = 0; i < vec.size(); i++) {
		if (ok)cout << "v[" << i << "]= ";
		if (ok)cout << vec[i] << endl;
		if (getLine == "\n") cout << "found\n";
		else {
			if (vec[i].find(name.c_str()) != npos) {
				vec[i] = s;
				SUC = true;
			}
			of << vec[i] << '\n';
		}
	}
	of.close();
	if (ok) {
		inf.open(fileName);
		while (!inf.eof()) {
			getline(inf, getLine);
			cout << getLine << endl;
		}
		inf.close();
	}
	if (SUC) return 0;
	else {
		return NOMATCHINGLINE;
	}
	vec.clear();
	return 0;
}

string Core::minToTime(int x) {
	int hour = x / 60;
	//cout << hour << endl;
	int minutes = x - (hour * 60);
	//cout << minutes << endl;

	return to_string(hour) + ":" + to_string(minutes);
}

int Core::secToMin(double* x)
{
	return int(*x / 60);
}

int Core::secToMin(int x)
{
	return int(x / 60);
}

void Core::getSmg() {
	cout << nightStarttime << endl;
	cout << nightEndtime << endl;
	cout << solarDiff << endl;
	cout << houseDiff << endl;
	cout << boilerDiff << endl;
	cout << onlyPumpchimneymin << endl;
	cout << onlyPump << endl;
	cout << afterCirculation << endl;
}

//External sensors
int Core::addExtTempSensor(string name)
{
	Temp wen(true);
	wen.setID(name);
	wen.setName(name);
	temperatureSensors.push_back(wen);
	mainLog("EXT sensor added(2183): " + wen.getName());
	return SUCCES;
}

int Core::delExtTempSensor(string name) {
	for (size_t i = 0; i < temperatureSensors.size(); i++) {
		if (temperatureSensors[i].getName() == name) {
			mainLog("EXT sensor deleted(2483): " + temperatureSensors[i].getName());
			temperatureSensors.erase(temperatureSensors.begin() + i);
			return SUCCES;
		}
	}
	return NOMATCHINGSENSOR;
}

string Core::getSensorNames() {
	string x;
	for (size_t i = 0; i < temperatureSensors.size(); i++) {
		if (i != 0) x += "\n";
		x += temperatureSensors[i].getName();
	}
	return x;
}

int Core::setExtTemp(int x) {
	EXTkey = time(0);
	string pl = "temp," + to_string(EXTkey) + "," + temperatureSensors[x].getFilename();
	temperatureSensors[x].setFresh(false);
	//cout << "Last update: " << time(0) << " - " << tomb[x].getLastUpdate() <<" - " << time(0) - getLastUpdate() << endl;
	if (time(0) - temperatureSensors[x].getLastUpdate() > 15) temperatureSensors[x].getBP();
	send("extsensors", pl);
	return SUCCES;
	/*delay(200);
	if (tomb[x].getFresh()) {
		tomb[x].setLastUpdate();
		return SUCCES;
	}
	else{
		tomb[x].getBP();
		debugLog("extsensor not updated in 200ms(2218)");
		debugLog("extsensor blacpoints(2220):" + to_string(tomb[x].showBP()));
		return NOMATCHINGSENSOR;
		//x.setFresh(false);
	}*/
}

int Core::vectorUploader(const string row, vector<int>& uVector)
{
	size_t lastOne = 0;

	if (row.find(',') != size_t(-1)) {

		for (size_t i = 0; i <= row.length() - 1; i++) {

			if (row[i] == ',' && lastOne != row.find_last_of(',')) {

				//cout << row.substr(lastOne, i - lastOne) << endl;

				uVector.push_back(szam(row.substr(lastOne, i - lastOne)));
				lastOne = i + 1;
			}
			else if (i > row.find_last_of(',')) {

				//cout << row.substr(row.find_last_of(',') + 1, row.length() - row.find_last_of(',')) << endl;

				uVector.push_back(szam(row.substr(row.find_last_of(',') + 1, row.length() - row.find_last_of(','))));

				return 1;
			}
		}
	}
	else {
		uVector.push_back(szam(row.substr(row.find_last_of(' ') + 1, row.length() - row.find_last_of(' ') + 1)));
	}
	return 1;
}

//RELAYS
//included in settings function, not used at the moment
/*int Core::setDevices(string filename) {

	string row, name;
	int pin;
	size_t z = -1;
	ifstream f;
	f.open(filename.c_str());

	while (!f.eof()) {
		getline(f, row);
		if (row.find("-d") != z) {
			name = row.substr(row.find(' ') + 1, row.find_last_of(' ') - row.find_first_of(' ') - 1);
			pin = szam(row.substr(row.find_last_of(' ') + 1, row.length() - row.find_last_of(" ")));
			try
			{
				throw Devices.addDevice(name, pin, TEST);
			}
			catch (int e)
			{
				if (e != 0) {
					mainLog("Device got wrong name, could not be added");
				}
				else {
					mainLog("Device added: " + name + ", GPIO: " + to_string(pin));
				}

			}
		}
	}

	f.close();

	return SUCCES;
}*/

int Core::getDevicesNr(string x) {

	for (size_t i = 0; i < Devices.size(); i++) {

		if (Devices[i].getName() == x) return i;
		else { return -1; }

	}
	return -1;
}

string Core::getDevicesData()
{
	string ret;

	for (size_t i = 0; i < Devices.size(); i++) {
		ret += to_string(i + 1) + ". " + Devices[i].getName() + " - GPIO: " + to_string(Devices[i].getPIN()) + "\n";
	}

	return ret;
}