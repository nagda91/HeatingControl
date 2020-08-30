#include <iostream>
#include "Devices.h"
#include "Relay.h"
#include <vector>
#include <string>

using namespace std;

Devices::Devices()
{

}

Devices::~Devices()
{
	devices.clear();
}

string Devices::getName(int x) {
	return devices[x].getName();
}

int Devices::getDeviceNumber(string name)
{
	for (size_t i = 0; i < devices.size(); i++) {
		if (devices[i].getName() == name) {
			return i;
		}
	}
	return 100;
}

int Devices::getDevicesNumber() {

	return devices.size();

}

int Devices::addDevice(string namex, int pinX) {

	//cout << "pinx: " << to_string(pinX) << endl;

	//cout << "In addDevice(), namex: " << namex << /*", checkPIN(): " << checkPIN(pinX) <<*/ endl;
	//cout << "addDevice, " << namex << " , " << to_string(pinX) << endl;
	Relay z(namex, pinX);

	devices.push_back(z);

	return OKI;

}

int Devices::adddDevice(string namex, int wtX, int pinX) {

	if (checkPIN(pinX)) {
		//Relay z(namex, wtX, pinX);

		//devices.push_back(z);

		return OKI;
	}
	else {
		return WRONG_PIN_NUMBER;
	}
}

int Devices::delDevice(string nameX) {

	try
	{
		throw getDeviceNumber(nameX);
	}
	catch (int e)
	{
		if (e == 100) return NO_MATCHING_NAME;
		else {
			devices.erase(devices.begin() + e);
			return OKI;
		}
	}
}

string Devices::getDevicesData()
{
	string ret;

	//cout << "Devices Size: " << devices.size() << endl;

	for (size_t i = 0; i < devices.size(); i++) {
		ret += to_string(i + 1) + ". " + devices[i].getName() + " - GPIO: " + to_string(devices[i].getPIN()) + "\n";
	}

	return ret;
}

bool Devices::checkPIN(int checkPIN) {

	/*for (size_t i = 0; i < availableGPIO.size(); i++) {

		//cout << "In chackPIN(), AvailableGPIO: " << availableGPIO[i] << ", Pin:" << checkPIN << endl;

		if (devices[i].getPIN() == checkPIN) {

			return true;
		}
	}

	cout << "AvailableGPIO.size(): " << availableGPIO.size() << endl;

	for (size_t i = 0; i < availableGPIO.size(); i++) {

		cout << "AvailableGPIO[" << i << "]:" << availableGPIO[i] << endl;

	}*/

	return false;
}

Relay Devices::operator[](size_t x) {

	if (x > devices.size() || x < 0) return {};
	else {
		return devices[x];
	}
}

Relay Devices::operator[](string x) {
	return devices[getDeviceNumber(x)];
}

void Devices::devicesCheck(bool& STOP)
{
	while (STOP) {

		for (size_t i = 0; i < devices.size(); i++) {

			if (devices[i].getExpState() != devices[i].getState()) {
				devices[i].setChanged();

			}

		}


		delay(1000);
	}
}

size_t Devices::numberOfDevices() {

	return devices.size();

}
