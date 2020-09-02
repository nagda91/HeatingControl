#include "Relay.h"
#include <iostream>
#include <string>
#include <time.h>
#include <vector>
#include <wiringPi.h>
#include <cmath>

using namespace std;

Relay::Relay()
{
	state = 0;
	workingTime = 0;
	START = 0;
	PIN = -1;
	changed = false;

}

Relay::Relay(string namex, int pinNumber, bool &Test)
{
	name = namex;
	workingTime = 0;
	START = 0;
	TEST = &Test;
	PIN = pinNumber;
	pinMode(PIN, OUTPUT);
	digitalWrite(PIN, HIGH);
	changed = false;
	state = digitalRead(PIN);

}

/*Relay::Relay(string namex, int pinNumber, int workingTimex = 0)
{
	name = namex;
	workingTime = workingTimex;
	PIN = pinNumber;
	digitalWrite(PIN, HIGH);
	changed = false;
	state = digitalRead(PIN);

}
*/
Relay::~Relay()
{
}

int Relay::getState()
{
	return digitalRead(PIN);
}

int Relay::getExpState()
{
	return state;
}

int Relay::getPIN()
{
	return PIN;
}

int Relay::setPIN(int x, vector<int>& freeGpio) {

	for (size_t i = 0; i < freeGpio.size(); i++) {
		if (freeGpio[i] == x) {
			freeGpio.push_back(PIN);
			PIN = x;
			freeGpio.erase(freeGpio.begin() + i);
			return 0;
		}
	}




	/*if (PIN == -1) {
		for (auto i = 0; i < usedGPIO->size(); i++) {
			if (usedGPIO->at(i) == x) {
				PIN = x;
				usedGPIO[i].erase();
				pinMode(PIN, OUTPUT);
				digitalWrite(PIN, HIGH);
				return 0;
			}
			else {
				return 1;
			}

		}
	}
	else {
		usedGPIO->push_back(PIN);
		for (auto i = 0; i < usedGPIO->size(); i++) {
			if (usedGPIO->at(i) == x) {
				PIN = x;
				usedGPIO[i].erase();
				pinMode(PIN, OUTPUT);
				digitalWrite(PIN, HIGH);
				return 0;
			}
			else {
				return 1;
			}
		}
	}*/
	return 1;
}

string Relay::getName()
{
	return name;
}

int Relay::getWorkingTime()
{
	return this->workingTime;
}

int Relay::ON() {
	cout << "in Relay, ON() - 123 "<< endl;
	if (digitalRead(PIN) != 0 && !*TEST) {
		digitalWrite(PIN, LOW);
		START = time(0);
		return 0;
	}
	else {
		cout << "in Relay, ON() - 131 " << endl;
		if (*TEST && START == 0) {
			cout << "in Relay, ON() - 133 " << endl;state = 0;
			cout << endl << "in Relay, ON() - STARTbefore: " << START << endl;
			START = time(0);
			cout << endl << "in Relay, ON() - STARTafter: " << START << endl;
			return 0;
		}
		else { 
			cout << "in Relay, ON() - 139 " << endl;
			return 1; 
		}
	}
}

int Relay::OFF() {

	if (digitalRead(PIN) != 1 && !*TEST) {
		digitalWrite(PIN, HIGH);state = 1;
		int z = time(0) - START;
		workingTime = workingTime + z;
		vector<time_t> y;
		y.push_back(START);
		y.push_back(z);
		WTs.push_back(y);
		START = 0;
		return z;
	}
	else {

		if (*TEST && START != 0) {
			int z = time(0) - START;
			workingTime = workingTime + z;
			cout << endl << "in Relay, OFF() - START: " << START << endl;
			cout << endl << "in Relay, OFF() - z: " << z << endl;
			START = 0;
			return z;
		}
		else { 
			
			return 0; 
		}
	}
}

/*int Relay::OFF() {
	if (digitalRead(PIN) != 1) {
		digitalWrite(PIN, HIGH);
		state = 1;
		workingTime += (time(0) - switchedON);
		vector<time_t> y;
		y.push_back(switchedON);
		cout << endl << "in Relay, OFF()-time(0): " << time(0) << endl;
		cout << endl << "in Relay, OFF()-switcedON: " << switchedON << endl;
		y.push_back(time(0) - switchedON);
		WTs.push_back(y);
		switchedON = 0;
		cout << endl << "in Relay, OFF(): " << (time(0) - switchedON) << endl;
		cout << endl << "in Relay, OFF() - y[0]: " << y[0] << endl;
		cout << endl << "in Relay, OFF() - y[1]: " << y[1] << endl;
		return (time(0) - switchedON);		
	}
	else {
		return -1;
	}
}*/

int Relay::getAVGWT() {

	int x = 0;

	if (WTs.size() > 1) {
		for (size_t i = 0; i < WTs.size(); i++) {
			x = x + WTs[i][1];
		}
		return (x / WTs.size());
	}
	else {
		return -1;
	}

}

string Relay::getAVGWTstring() {

	int x = 0;

	if (WTs.size() > 1) {
		for (size_t i = 0; i < WTs.size(); i++) {
			x = x + WTs[i][1];
		}

		return to_string(x / WTs.size());
	}
	else {
		return "No data(" + name + "WTs)\n";
	}

}

string Relay::getWTs()
{
	string r;

	if (WTs.size() > 0) {
		for (size_t i = 0; i < WTs.size(); i++) {
			r += to_string(WTs[i][0]);
			r += " - ";
			r += to_string(WTs[i][1]);
			r += "\n";
		}

	}
	else {
		r = "No data(gasheaterWTs)\n";
	}

	return r;
}

void Relay::setChanged()
{
	if (changed) {
		if ((time(0) - changedByUserTime) > 6000) {
			if (state != digitalRead(PIN)) {
				digitalWrite(PIN, state);
				changed = false;
			}
		}
	}
	else {
		changedByUserTime = time(0);
		changed = true;

	}
}

int Relay::getStart()
{
	return START;
}
