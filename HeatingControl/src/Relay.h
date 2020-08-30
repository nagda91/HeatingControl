#include <iostream>
#include <iostream>
#include <string>
#include <time.h>
#include <vector>
#include <wiringPi.h>

#pragma once

using namespace std;

class Relay
{
public:
	Relay();
	Relay(string, int);
	Relay(string, int, int);
	~Relay();

	int getState();
	int getExpState();
	int getPIN();
	int getWorkingTime();
	int ON();
	int OFF();
	int setPIN(int, vector<int>&);
	string getName();
	int getAVGWT();
	string getAVGWTstring();
	string getWTs();
	void setChanged();


private:

	string name;
	int workingTime, state, PIN, switchedON, changedByUserTime;
	bool changed;
	vector<vector <int>> WTs;

};

