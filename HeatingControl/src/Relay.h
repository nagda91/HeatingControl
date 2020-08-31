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
	Relay(string, int, bool&);
	//Relay(string, int, int, bool&);
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
	void setStart(time_t);
	int getStart();


private:

	string name;
	int state, PIN, changedByUserTime, START;
	time_t workingTime = 0;
	bool changed, *TEST;
	vector<vector <time_t>> WTs;
};

