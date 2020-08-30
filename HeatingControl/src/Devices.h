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
	int getDeviceNumber(string);//gives back the number of the devs name, if it exists
	int getDevicesNumber();
	int addDevice(string, int);
	int adddDevice(string, int, int);
	int delDevice(string);
	string getDevicesData();
	bool checkPIN(int);
	Relay operator[](size_t);
	Relay operator[](string);
	size_t numberOfDevices();

	void devicesCheck(bool&);//thread function for checking ON/OFF

private:

	/*GPIOs
	std::vector<int> availableGPIO{ 1, 2, 3, 5, 6, 7, 21, 22, 23, 24, 25, 26, 27, 28, 29 };
	const std::vector<int> usableGPIO{ 1, 2, 3, 5, 6, 7, 21, 22, 23, 24, 25, 26, 27, 28, 29, 0 };
	//-------------------
	*/

};


