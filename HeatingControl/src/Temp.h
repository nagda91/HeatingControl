#define TEMP_H

#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include <sstream>
#include <vector>
#include <chrono>
#include <wiringPi.h>

using namespace std;

#define TESTDELAY				100
#define CANTOPENTEMPFILE		20210
#define WRONGVALUE				20285
//These should be in private variables, because they should be chenged if someone want it...
#define ONEWIREDEVICESFOLDER	"/sys/bus/w1/devices/"
#define ONEWIRETESTFOLDER		"/home/pi/Desktop/HeatingControl_test/"
#define URLSENSORFOLDER			"/home/pi/Desktop/"
#define ONEWIREFILE				"/w1_slave"

class Temp {

public:
	Temp();
	Temp(bool);
	Temp(string, string, bool, bool);
	virtual ~Temp();

	//Functions for the 1Wire temperature sensors and the others
	int tempGet();
	int getTemp();
	int szam(string);
	void setTemp(int);
	void setName(string);
	int setTempfromfile(bool*);
	string getID();
	void setID(string);
	string getName();
	string getFilename();
	void setLastUpdate();
	int getLastUpdate();
	string getAllData();
	string EndOfDay();
	int getAvgOfDay(bool Debug = false);
	int getMax();
	int getMin();
	string getNote();

	//Functions only for the wireless temperature sensors
	void setEXT(bool);
	bool getEXT();
	void setFresh(int);
	int getFresh();
	int showBP();
	void getBP();
	int getWorking();
	void setWorking(int);
	int getsameCounter();

	//Sensors that values are got from file (not 1wire)
	void setMinedSensor();
	bool isUrl();


protected:

private:

	string			name;
	string			oneWireID;
	string			note; //Specifically for recording the weather
	int				temp;
	vector<int>		Degrees;
	int				maxOfDay;
	int				minOfDay;

	bool			URL;

	bool			EXT;
	int				FRESH, WORKING;
	int				blackPoint;
	int				lastUpdate;
	int				sameCounter;

	int				stringFokinteger(string);
	int				urlTemp(string);

};