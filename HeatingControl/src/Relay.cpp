#include "Relay.h"

using namespace std;

Relay::Relay() {

	workingTime = 0;
	START = 0;
	PIN = -1;
	changed = false;

}

Relay::Relay(string namex, int pinNumber, bool &Test, bool &relayDebug) {

	name = namex;
	workingTime = 0;
	START = 0;
	TEST = &Test;
	PIN = pinNumber;
	pinMode(PIN, OUTPUT);
	digitalWrite(PIN, HIGH);
	changed = false;
	debug = &relayDebug;
}

Relay::~Relay(){}

int Relay::getState() {

	return digitalRead(PIN);
}

int Relay::getPIN() {

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

	return 1;
}

string Relay::getName() {

	return name;
}

int Relay::getWorkingTime() {

	return this->workingTime;
}

int Relay::getLastWT() {

	if (WTs.empty()) {
		return 0;
	}
	else { 
		return WTs[WTs.size()-1][1]; 
	}
}

int Relay::ON() {

	if (digitalRead(PIN) != 0 && !*TEST) {
		digitalWrite(PIN, LOW);
		START = time(0);
		
		if (*debug) cout << name << " -- ON" << endl;
		return 0;
	}
	else {
		if (*TEST && START == 0) {
			START = time(0);
			return 0;
		}
		else { return 1; }
	}
	if (*debug) cout << name << " -- it was ON" << endl;
	return 1;
}

int Relay::OFF() {

	if (digitalRead(PIN) != 1 && !*TEST) {
		digitalWrite(PIN, HIGH);
		workingTime = workingTime + time(0) - START;
		vector<time_t> y;
		y.push_back(START);
		y.push_back(time(0) - START);
		WTs.push_back(y);
		START = 0;
		if (*debug) cout << name << " -- OFF" << endl;
		return 0;
	}
	else {
		if (*TEST && START != 0) {
			workingTime = workingTime + time(0) - START;
			START = 0;
			return 0;
		}
	}
	if (*debug) cout << name << " -- it was OFF" << endl;
	return 1;
}

int Relay::getAVGWT() {

	int x = 0;

	if (WTs.size() > 1) {
		for (size_t i = 0; i < WTs.size(); i++) {
			x = x + WTs[i][1];
		}
		return ((x / WTs.size())/60);
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

		return to_string((x / WTs.size())/60);
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

int Relay::getStart()
{
	return START;
}

void Relay::newDay()
{
	workingTime = 0;
	WTs.clear();
}
