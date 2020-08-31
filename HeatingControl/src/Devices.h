#pragma once

#include "Relay.h"
#include <vector>

//ERROR
#define WRONG_PIN_NUMBER 1
#define NO_MATCHING_NAME 2
#define OKI 0

using namespace std;

class Devices
{
public:
	Devices();
	~Devices();

	vector<Relay> devices;

	string getName(int);
	int getDeviceNumber(string&);//gives back the number of the devs name, if it exists
	int getDevicesNumber();
	int addDevice(string, int, bool&);
	//int adddDevice(string, int, int);
	int delDevice(string);
	string getDevicesData();
	bool checkPIN(int);
	Relay operator[](size_t);
	Relay operator[](string);
	size_t numberOfDevices();

	void devicesCheck(bool&);//thread function for checking ON/OFF

private:

	

};


