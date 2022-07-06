#include <iostream>
#include <string>
#include <time.h>
#include <vector>
#include <cmath>
#include <fstream>
#include <gpiod.h>

#pragma once

#define SUCCESS					0
#define WRONGMODE				1

using namespace std;

class Relay
{
public:
	Relay();
	Relay(struct gpiod_chip*, string, int, bool&, bool&);
	~Relay();

	int getState();
	string getOnOff();
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
	//pinMode: in or out
	int pinMode(string);
	//pin value 1 == 3.3V; 0 == 0V
	int setPinValue(int);

private:

	string name;
	int PIN, changedByUserTime, START;
	time_t workingTime = 0;
	bool changed, *TEST, *debug;
	vector<vector <time_t>> WTs;

	struct gpiod_line* pinLine;
};

