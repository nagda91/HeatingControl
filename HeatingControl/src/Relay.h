#include <iostream>
#include <string>
#include <time.h>
#include <vector>
#include <cmath>
#include <fstream>

#pragma once

#define GPIOEXPORT				"/sys/class/gpio/export"
#define GPIOUNEXPORT			"/sys/class/gpio/unexport"
#define GPIOPATH				"/sys/class/gpio/gpio"
#define GPIOMODE				"/direction"
#define GPIOVALUE				"/value"
#define SUCCESS					0
#define WRONGMODE				1
#define CANTOPENEXPORTFILE		2
#define CANTOPENUNEXPORTFILE	3
#define CANTOPENDIRECTIONFILE	4
#define CANTOPENVALUEFILE		5

using namespace std;

class Relay
{
public:
	Relay();
	Relay(string, int, bool&, bool&);
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
	int openExport(ofstream&);
	int openUnexport(ofstream&);
};

