#ifndef Core_H || SIMPLECLIENT_MQTT_H
#define Core_H
#define SIMPLECLIENT_MQTT_H

#include <iostream>
#include <string>
#include "Temp.h"
#include <wiringPi.h>
#include <thread>
#include <mosquittopp.h>
#include <mosquitto.h>
//#include "Devices.h"
#include "Relay.h"
#include <thread>

//MQTT
//#define CLIENT_ID "HeatingControl"
//#define BROKER_ADDRESS "localhost"
#define MQTT_PORT 1883;
#define MQTT_TOPIC "topic"
#define MAX_PAYLOAD 50
#define DEFAULT_KEEP_ALIVE 60
#define WILL_MESSAGE "alert, HeatingControl disconnected!(will)"
//mqttReset() returns:
#define COULDNOTDISCONNECT 1
#define COULDNOTSTOPLOOP 2
#define COULDNOTCONNECT 3
#define COULDNOTSUBSCRIBE 4
#define COULDNOTSTARTLOOP 5
//changeInFile() returns:
#define CANTOPENFILE 1
#define WRONGFORMAT 2
#define NOMATCHINGLINE 3
//eXTERNAL SENSORS
#define NOMATCHINGSENSOR 1;
#define SUCCES 0;
//HEATING MODE
#define WRONGMODE 1; 
//Other stuff
#define MAINLOGFILEPATH "/home/pi/Desktop/log/mainlogfilenames.txt";
#define TEMPSLOGFILEPATH "/home/pi/Desktop/log/tempslogfilenames.txt";

class Core : public mosqpp::mosquittopp, Temp
{
public:

	//create a Core object
	//if we give it the broker's parameters(user id, broker address, port, username, password), it tries to connect to the broker, only connects
	Core();
	Core(const char* id, const char* host, int port, const char*, const char*);

	//destructor of a Core object
	//clears the <tomb> vector, i should put some other vectors into it
	~Core();

	/*GPIOs - it will be usefull in the future
	std::vector<int> availableGPIO{ 1, 2, 3, 5, 6, 7, 21, 22, 23, 24, 25, 26, 27, 28, 29 };
	const std::vector<int> usableGPIO{ 1, 2, 3, 5, 6, 7, 21, 22, 23, 24, 25, 26, 27, 28, 29, 0 };*/

	/*This function sets the log files' names, and the temperatures sensors' filenames and names
	we have to give it a filename where we store the path to the files
	# - comment lines
	-tlp - temperature log files location
	-log - mainlog file's folder path
	-o - temperature sensors location
	*/
	string settingsFromFile(string);

	/*This function sets the parameters below:
		-nightstart
		-nightend
		-solardiff
		-housediff
		-boilerdiff
		-chimneymin
		-onlypump
		-aftercirc
		-winterstart
		-winterend
		-heatermax
		-thermday
		-thermnight
		-whilehodiff
	*/
	string setTimeDiffTemps();

	//This function gives back the <tomb> vector's size
	int getMeret();


	Temp operator[](int);

	/*This function is the core of the whole program
	every other functions, threads starts from it
	we have to call this as a thread, after we set up the MQTT connection(connect and subscriptions)
	*/
	void basicFunc();

	/*This function gives back the basicFunc() as a thread*/
	thread basicFuncthread();

	/*This function gives back:
	nightStarttime
	nightEndtime
	solarDiff
	houseDiff
	boilerDiff
	onlyPumpchimneymin
	onlyPump
	afterCirculation

	its callable from command line
	*/
	void getSmg();

	/*This function handles the commend from the terminal
		Every Celsius value looks like, 21.0�C = 21000
		<getthermd> to get day thermostat
		<getwts> to get working times(heater, pumps)
		<gettdt> to get times, diff. end temperatures
		<gettemps> to get the actual temperatures
		<thermnight>=<value>, to set night thermostat
		<thermday>=<value> to set day thermostat
		<nstarttime>=<time in min> to set night start time
		<nendtime>=<time in min> to set night end time
		<winterstart>=<month(0-11)> to set the winter start month
		<winterend>=<month(0-11)> to set the winter end month
		<solardiff>=<value> to set the diff. between solar temp. and the boilerbottom temp., when the solarpump starts working
		<housediff>=<value> to set the diff. between house temp. and the thermostat, when the househeating starts
		<whilehousediff>=<value> to setwhilehousediff, if house temp. reachs (thermostat+whilehousediff) stop heating
		<boilerdiff>=<value> to set boilerdiff, if mainpipe temp. higher the (boilermid + boilerdiff), boilerpump start working
		<onlypumpchimney>=<value> to set onlypumpchimney, if you do not heat with gas, the chimney temp. higher, than this value, the gasheater switchs OFF
		<onlypump>=<value> to set onlypump, if mainpipe is higher(when heating with gas), than this value, gasheater switch OFF
		<aftercirculation>=<value> to set aftercirculation value, after heating, if mainpipe temp. higher, than this value the housepump keeps on working
		<heatermax>=<value> to set the heater max. temp.
		<getgpio> to get the gpios\'s state
		<getsnames> to get the sensor\'s names
		<Reset> to reset sensors\'s filenames and times,diff., temps
		<pause> to pause the basicFunc()
		<continue> to continue the basicFunc()
		<exit> to close the application
		<mqttreconnect> to reconnect mqtt client
		<mqttdisconnect> to disconnect mqtt client
		<mqttreset> it works like the reconnect should!
	*/
	string commFunc(string);


	/////MQTT//////
	int send(string topicstr, string newPl);
	int sendTopic(string newPl);//send function, topic = "topic"
	int sendReport(const mosquitto_message* msg, string newPl);//topic = "report", everything what happens
	string cmd(const struct mosquitto_message* cmdmsg, std::string);//message "cmd" recieved
	void on_connect(int rc);
	void on_message(const struct mosquitto_message* message);
	void on_disconnect(int rc);
	void on_subscribe(int mid, int qos_count, const int* granted_qos);
	int mqttReset();
	/////MQTT//////

protected:

private:
	int meret, mainpipe, boiler1, boiler2, heater, house, chimney, solar, solarboiler, boilerMax;

	vector<Temp> temperatureSensors;//temperatures vector
	vector<Relay> Devices;//Devices vector

	//Devices Devs;

	//boiler ,ax temperature are collected int thios vector during a whole week
	vector<vector<int>> boilerMaxofWeek;

	//gasheater working times are collected in this vector during a day
	vector<vector<int>> gasheaterWTs;

	//lines which are written into the mainlog file are collected in this vector
	//for MQTT, 25 lines, maybe we can call it from the command line
	vector<string> vlog;

	string webfilename, tempsfilename, logfilename, heatingFuncreturn, tempsFilename, logFilename;
	bool heating = false, OK = true, STOP = true, RESET = false, TEST = false;
	int winterStart, winterEnd, nightStarttime, nightEndtime;
	int solarDiff, houseDiff, whilehouseDiff, boilerDiff;
	int onlyPumpchimneymin, onlyPump, afterCirculation;
	int thermostatDay = 21000, thermostatNight = 19000, *thermostat, heaterMax;
	int heaterWorkingTime = 0, solarPumpWT = 0, housePumpWT = 0, boilerPumpWT = 0, solarPumpStart, boilerPumpStart;
	int heatingMode, EXTkey;

	//Uploading rules for vectors:
	/*vector<int> heatingDevices = {
			0 - gasheater,
			1 - housepump,
			2 - boilerpump}*/
	vector<int> heatingDevices;
	/*vector<int> heatingSensors = {
			0 - house,
			1 - heater,
			2 - mainpipe,
			3 - chimney }*/
	vector<int> heatingSensors;

	//MQTT
	int sadf = 13245;
	int* clint_id = &sadf;

	//Functions

	//give back the temperatures
	string getTemps();

	//give back times, diff. and temps
	string getTDT();

	// give back day thermostat
	string getThermd();

	//old function, will be deleted, NEW -> getGPIOState()
	string getGPIO();

	//NEW - gives back the gpios' state
	string getGPIOState();

	//old function, will be deleted soon
	string getWT();

	//NEW - gives back the working times of the devices(for MQTT, command line)
	string newGetWT();

	//gives back the sensor's temperature, you have to give its name and if the sensor is external a <true> value, start value is <false>
	int getTemp(string, bool);

	//gives back the sensor's number in the tomb vector
	int getSensorNumber(string);

	//gives back which heating mode we use at the moment
	int getHeatingMode();

	//we can set which heating mode de we want to use(1 or 2), usable only in command line
	int setHeatingMode(int);

	//Log functions

	/*This function is called every time we want to write sometinhg into the mainlog file
	you can call it wit/without new row or start the line with the current time or not
	*/
	int mainLog(string, bool newRow = true, bool time = true);

	/*i dont know what this function for...
	*/
	int fileNameLog(string, string);

	/*This function controls the vLog vectors size, it cant be more than 25 row
	Android app is set to handle 25 lines*/
	void pushBackvLog(string*);

	/*This function gives back the 25 lines of vLog vector*/
	string getVlog();

	/*This function is used to logging the messages sent to and get from the MQTT brokere*/
	void mqttLog(string str);

	/*This function writes a row into the tempslog file after every update(6.5 - 7sec)*/
	void tempsLog();

	// makes samples for Neural Network
	void AIsamples();

	/*This function writes into the tempslog file the given string parameter*/
	void tempsLog(string);

	/*This function can be called if we want to write something into a debugfile
	I shuold put a bool variable into the parameters, it can be easier to handle when i want to use it*/
	void debugLog(string str);

	//This function gives back which day of the week is at the moment(Sunday = 0)
	int dayOftheweek();

	//actual month
	int month();

	//actual time in min
	int timeinMin();

	//given time("6:30") convert to min
	int timeinMin(string);

	/* minToTime() function:
		minute of a day(int) converts to digit form time(string), 650->"10:50"*/
	string minToTime(int x);

	/*This function converts sec.-s to minute, parameter can be double or integer*/
	int secToMin(double*);
	int secToMin(int);

	/*This function gives back long timer format
	*/
	string longtime();

	/*This function cuts the date part from longtime()*/
	string date();

	/*boiler heating function
		using two temperatures sensors value, and a different value between them
		and a relay for the pump*/
	string boilerFunc();

	/*solarpanel function
	Totally same like boilerfunc(), but using different values in it*/
	/*it using the new Devices obj. to control the pumps*/
	string solarFunc(string);

	//This function create a new filename with actual date, its called every startup, and every Sunday 0:00	
	string filename(string);

	//two old function, will be deleted
	void isboilerMax();//not necessary
	int getboilerMax();//not necessary

	/*These two functions look at is the new value is the lowest or the biggest every day, and every temperature update*/
	int getSensorMax(string);
	int getSensorMin(string);


	string getgasheaterWTs();//should be deleted

	/*NEW function, it can give back the given device's working times as string in sec.*/
	string getDeviceWTs(string);

	/*This function gives back the average working times of the gasheater
	its needed when changing night to day part of the day*/
	int avgHeatingTime();//average heating time in minutes
	int sqrtAvgHeatingTime();//square average heating time in minutes

	/* changeInFile() function:
		if one of the TDTs change, it can be called to change it in the file too*/
	int changeInFile(string, const char*);

	/*This function is called every time it creates a new temperature log file
	every temperature sensor's name is written into the first line, separated with <;>*/
	string tempslogFirstRow();

	//########## Functions for threads ##############

	//Temperatures are written into a file
	void logThread();

	//refreshing the temperatures ~10sec
	void setTempsthread();

	/*house heating - mode 1
	It works in a separated thread*/
	void heaterFunc(int*);

	/*house heating - mode 2
	It works in a separated thread*/
	void heaterFunc_mode2(int*);

	//External temperature sensors

	/*This function adds a new external sensor to the vector
	It works only from terminal
	<name> is the sensors name which will be sent in the update request message*/
	int addExtTempSensor(string name);

	/*Delete the <name> external sensor from the vector*/
	int delExtTempSensor(string name);

	/*Make list of the temperature sensors*/
	string getSensorNames();

	/*This function can modify an external sensor's temperature value*/
	int setExtTemp(int);

	/*vector uploader with integers
	heatingSensors and heatingDevices*/
	int vectorUploader(const string, vector<int>&);

	//############ RELAYS ################

	/*This function sets the Devices in a vector
	The devices' parametrs are read from file, parameter is the file's path*/
	int setDevices(string);

	int getDevicesNr(string);

	string getDevicesData();
};

#endif // SENSOR_H