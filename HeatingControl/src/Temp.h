#ifndef TEMP_H
#define TEMP_H

using namespace std;


class Temp {

public:
	Temp();
	Temp(bool);
	virtual ~Temp();

	int tempGet();
	int getTemp();

	int szam(string);
	void setTemp(int);
	void setName(string);
	int setTempfromfile(bool&);
	void setID(string);
	string getName();
	string getFilename();
	void setEXT(bool);
	bool getEXT();
	void setFresh(int);
	int getFresh();
	void setLastUpdate();
	int getLastUpdate();
	int showBP();
	void getBP();
	int getWorking();
	void setWorking(int);
	int getsameCounter();
	string getAllData();
	string EndOfDay();
	int getAvgOfDay(bool Debug = false);
	int getMax();
	int getMin();


protected:

private:
	string name;
	int number;
	int temp;
	string oneWireID;
	bool EXT;
	int FRESH, WORKING;
	int blackPoint;
	int lastUpdate;
	int sameCounter;
	vector<int> Degrees;
	int maxOfDay;
	int minOfDay;

	int stringFokinteger(string);

};

#endif // TEMP_H

