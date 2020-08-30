#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include <sstream>
#include <vector>
#include <chrono>
#include "Temp.h"
#include <wiringPi.h>

using namespace std;

Temp::Temp()
{
	temp = 15000;
	temp2 = 13000;
	EXT = false;
	FRESH = false;
	sameCounter = 0;
	maxOfDay = -20000;
	minOfDay = 100000;
	if (EXT) oneWireID = name;
	else {
		oneWireID = " ";
	}
}

Temp::Temp(bool ext) {
	EXT = true;
	WORKING = true;
	blackPoint = 0;
	sameCounter = 0;
	temp = 15000;
	maxOfDay = -20000;
	minOfDay = 100000;
	if (EXT) oneWireID = name;
}

Temp::~Temp()
{
	//dtor
}

int Temp::getTemp() {
	return temp;
}

string Temp::getName() {
	return name;
}

string Temp::getFilename() {
	return oneWireID;
}

void Temp::setTemp(int a)
{
	if (temp == a) {
		sameCounter++;
		if (sameCounter > 6) WORKING = false;
	}
	else {
		temp = a;
		sameCounter = 0;
		if (blackPoint > 3)	blackPoint = 0;
	}
	if (EXT && !WORKING) {
		WORKING = true;
		if (blackPoint > 1)	blackPoint = 0;
	}
}

void Temp::setName(string x) {
	name = x;
}

int Temp::setTempfromfile() {
	ifstream f;
	string row;
	//string filePath = "/sys/bus/w1/devices/" + oneWireID + "/w1_slave";
	f.open(("/sys/bus/w1/devices/" + oneWireID + "/w1_slave").c_str());
	if (f.fail()) {
		temp = 20201;
		return 1;
	}
	else {
		getline(f, row);
		getline(f, row);
		f.close();
		temp = stringFokinteger(row);

		if (temp == 85000) {
			f.open(("/sys/bus/w1/devices/" + oneWireID + "/w1_slave").c_str());
			if (f.fail()) {
				temp = 20201;
				return 1;
			}
			else {
				getline(f, row);
				getline(f, row);
				f.close();
				temp = stringFokinteger(row);
				if (temp == 85000) {
					temp = 20285;
					f.close();
					return 2;
				}
			}
		}
		f.close();

		Degrees.push_back(temp);
		if (temp > maxOfDay) maxOfDay = temp;
		if (temp < minOfDay) minOfDay = temp;

		return 0;
	}
}

int Temp::stringFokinteger(string szor) {
	string t;
	int h;

	h = szor.length();

	switch (h) {
	case 35:
		h = h - 6;
		t.append(szor.substr(h, 6));
		break;
	case 34:
		h = h - 5;
		t.append(szor.substr(h, 5));
		break;
	case 33:
		h = h - 4;
		t.append(szor.substr(h, 4));
		break;
	case 32:
		h = h - 3;
		t.append(szor.substr(h, 3));
		break;
	case 31:
		h = h - 2;
		t.append(szor.substr(h, 2));
		break;
	case 30:
		h = h - 1;
		t.append(szor.substr(h, 1));
		break;
	};

	return szam(t);
}

int Temp::szam(string szo) {
	stringstream s;
	int x;

	s << szo;
	s >> x;

	return x;
}

void Temp::setID(string s) {
	const char* c;
	c = s.c_str();
	oneWireID = c;
}

void Temp::setEXT(bool x) {
	EXT = x;
}

bool Temp::getEXT() {
	return EXT;
}

int Temp::getFresh() {
	return FRESH;
}

void Temp::setFresh(int x) {
	if (x == 1 || x == 0) FRESH = x;
}

void Temp::setLastUpdate() {
	if (time(0) - lastUpdate > 2) getBP();
	lastUpdate = time(0);
}

int Temp::getLastUpdate() {
	return lastUpdate;
}

int Temp::showBP() {
	return blackPoint;
}

void Temp::getBP() {
	blackPoint++;
	if (blackPoint > 10) WORKING = false;
}

int Temp::getWorking() {
	return WORKING;
}

void Temp::setWorking(int x)
{
	WORKING = x;
}

int Temp::getsameCounter()
{
	return sameCounter;
}

string Temp::getAllData()
{
	string ret;

	ret = "Name = " + name + "\n";
	ret += "Filename: " + oneWireID + "\n";
	ret += "Number: " + to_string(number) + "\n";
	ret += "Temperature: " + to_string(temp) + "\n";
	if (EXT) {
		if (WORKING) {
			ret += "It is working.\n";
			const time_t lu = lastUpdate;
			ret += "Last update: " + string(std::asctime(std::localtime(&lu))) + "\n";
			ret += "Blacpoints: " + to_string(blackPoint) + "\n";
			ret += "Samecounter: " + to_string(sameCounter);
		}
		else {
			ret += "It is not working right now!";
		}
	}
	else {
		ret += "Not wireless sensor.";
	}

	return ret;
}

string Temp::EndOfDay()
{
	string r = name + ": MAX: " + to_string(maxOfDay) + ", MIN: " + to_string(minOfDay) + ", AVG: " + to_string(getAvgOfDay());

	Degrees.clear();
	maxOfDay = -100000;
	minOfDay = 100000;

	return r;
}

int Temp::getAvgOfDay(bool Debug)
{

	if (Debug) cout << "Size of Degrees: " << Degrees.size() << endl;

	if (Degrees.size() > 0) {
		int sum = 0;
		for (size_t i = 0; i < Degrees.size(); i++) {

			if (Debug) cout << "Degrees[" << i << "]: " << Degrees[i] << endl;

			sum += Degrees[i];

			if (Debug) cout << "For sum:" << sum << endl;
		}

		if (Debug) cout << "Sum: " << sum << endl;

		return (sum / Degrees.size());
	}
	else {

		if (Debug) cout << "In else return is going to be 0." << endl;

		return 0;
	}
}

int Temp::getMax()
{
	return maxOfDay;
}

int Temp::getMin()
{
	return minOfDay;
}
