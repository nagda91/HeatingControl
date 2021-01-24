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
	Relay(string, int, bool&, bool&);
	~Relay();

	int getState();
	int getPIN();
	int getWorkingTime();
	int getLastWT();
	int ON();
	int OFF();
	int setPIN(int, vector<int>&);
	string getName();
	int getAVGWT();
	string getAVGWTstring();
	string getWTs();
	int getStart();
	void newDay();


private:

	string name;
	int PIN, changedByUserTime, START;
	time_t workingTime = 0;
	bool changed, *TEST, *debug;
	vector<vector <time_t>> WTs;
};

