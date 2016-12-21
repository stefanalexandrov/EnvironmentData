
#include <mysql.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h> 
#include "sketch_dec07b\packetFormat.h"

const int TIMEOUT = 1000;
DCB ComDCM;
 
// just going to input the general details and not the port numbers
struct connection_details
{
    char *server;
    char *user;
    char *password;
    char *database;
};
 
MYSQL* mysql_connection_setup(struct connection_details mysql_details)
{
     // first of all create a mysql instance and initialize the variables within
    MYSQL *connection = mysql_init(NULL);
 
    // connect to the database with the details attached.
    if (!mysql_real_connect(connection,mysql_details.server, mysql_details.user, mysql_details.password, mysql_details.database, 0, NULL, 0)) {
      printf("Conection error : %s\n", mysql_error(connection));
      exit(1);
    }
    return connection;
}
 
MYSQL_RES* mysql_perform_query(MYSQL *connection, char *sql_query)
{
   // send the query to the database
   if (mysql_query(connection, sql_query))
   {
      printf("MySQL query error : %s\n", mysql_error(connection));
      exit(1);
   }
 
   return mysql_use_result(connection);
}

HANDLE connectToSerialPort(char *comport) {
	//Establishind the connection
	printf("Connecting to serial port: %s\n", comport);
	HANDLE serialPort =  CreateFile(
		comport, // name of the com port to open
		GENERIC_READ | GENERIC_WRITE, // read/write access 
		0, // only one handle can be opened at a time
		NULL, //related to security and resource sharing
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);
	
	
	if (serialPort == INVALID_HANDLE_VALUE) { // error checking
		printf("Error encountered while opening serial port!\n");
		//printf("Error encountered while opening cerial port!: %s", GetLastError());
		return INVALID_HANDLE_VALUE;
	} else {
		printf("Serial port connection established!\n");
	
		SetCommMask(serialPort, EV_RXCHAR);
        SetupComm(serialPort, 1500, 1500); //setting up read and write timeouts
 
        COMMTIMEOUTS CommTimeOuts;
        CommTimeOuts.ReadIntervalTimeout = 0xFFFFFFF5;
        CommTimeOuts.ReadTotalTimeoutMultiplier = 0;
        CommTimeOuts.ReadTotalTimeoutConstant = TIMEOUT;
        CommTimeOuts.WriteTotalTimeoutMultiplier = 0;
        CommTimeOuts.WriteTotalTimeoutConstant = TIMEOUT;
 
		SetCommTimeouts(serialPort, &CommTimeOuts);
        
        memset(&ComDCM,0,sizeof(ComDCM));
        ComDCM.DCBlength = sizeof(DCB);
        GetCommState(serialPort, &ComDCM);
		
        ComDCM.BaudRate = DWORD(9600);
        ComDCM.ByteSize = 8;
        ComDCM.Parity = NOPARITY;
        ComDCM.StopBits = ONESTOPBIT;
        ComDCM.fAbortOnError = FALSE;
        ComDCM.fDtrControl = DTR_CONTROL_DISABLE;
        ComDCM.fRtsControl = RTS_CONTROL_TOGGLE;
        ComDCM.fBinary = TRUE;
        ComDCM.fParity = FALSE;
        ComDCM.fInX = FALSE;
        ComDCM.fOutX = FALSE;
        ComDCM.XonChar = 0;
        ComDCM.XoffChar = (unsigned char)0xFF;
        ComDCM.fErrorChar = FALSE;
        ComDCM.fNull = FALSE;
        ComDCM.fOutxCtsFlow = FALSE;
        ComDCM.fOutxDsrFlow = FALSE;
        ComDCM.XonLim = 0;
        ComDCM.XoffLim = 1500;
		SetCommState(serialPort, &ComDCM);
		return serialPort;
	}
}

 
int main(int argc, char **argv)
{
	MYSQL *conn = NULL;		// the connection
	MYSQL_RES *res = NULL;	// the results
	MYSQL_ROW row;	// the results row (line by line)
 
	struct connection_details mysqlD;
	mysqlD.server = "localhost";  // where the mysql database is
	mysqlD.user = "Controller";		// the root user of mysql	
	mysqlD.password = "Gh2US"; // the password of the root user in mysql
	mysqlD.database = "EnvironmentData";	// the databse to pick
 
	// connect to the mysql database
	conn = mysql_connection_setup(mysqlD);
 
	// assign the results return to the MYSQL_RES pointer
	res = mysql_perform_query(conn, "show tables");
 
	printf("MySQL Tables in mysql database:\n");
	while ((row = mysql_fetch_row(res)) !=NULL)
		printf("%s\n", row[0]);
	mysql_free_result(res);
 
	res = mysql_perform_query(conn, "CREATE DATABASE IF NOT EXISTS EnvironmentData");
	if (res)
		while ((row = mysql_fetch_row(res)) !=NULL)
			printf("%s\n", row[0]);
	/* clean up the database result set */
	if (res)
		mysql_free_result(res);
	
	char* createTableSql = "CREATE TABLE IF NOT EXISTS Data (id INT NOT NULL AUTO_INCREMENT, \
																	temperatureCelcius FLOAT, \
																	pressuremmHg FLOAT, \
																	time DATETIME, \
																	PRIMARY KEY (id))";
	res = mysql_perform_query(conn, createTableSql);
	if (res) {
		while ((row = mysql_fetch_row(res)) !=NULL)
			printf("%s\n", row[0]);
		mysql_free_result(res);
	}
		
	char* createTableSql2 = "CREATE TABLE IF NOT EXISTS DataAdditional (id INT NOT NULL AUTO_INCREMENT, \
																	Altitude INT, \
																	AccelerometerAction CHAR(30), \
																	AccelerometerX FLOAT, \
																	AccelerometerY FLOAT, \
																	AccelerometerZ FLOAT, \
																	MagnetometerRawX FLOAT, \
																	MagnetometerRawY FLOAT, \
																	MagnetometerRawZ FLOAT, \
																	MagnetometerScaledX FLOAT, \
																	MagnetometerScaledY FLOAT, \
																	MagnetometerScaledZ FLOAT, \
																	headingDg FLOAT, \
																	time DATETIME, \
																	PRIMARY KEY (id))";
	res = mysql_perform_query(conn, createTableSql2);
		if (res) {
		while ((row = mysql_fetch_row(res)) !=NULL)
			printf("%s\n", row[0]);
		mysql_free_result(res);
	}
		
	char insertDataSql[500];
												
												
	/* clean up the database result set */
	if (res)
		mysql_free_result(res);
	
	HANDLE serialPort = INVALID_HANDLE_VALUE;
	if (argc >= 2) {
		serialPort = connectToSerialPort(argv[1]);
	} else {
		serialPort = connectToSerialPort("COM1");
		if (serialPort == INVALID_HANDLE_VALUE)
			serialPort = connectToSerialPort("COM2");
		if (serialPort == INVALID_HANDLE_VALUE)
			serialPort = connectToSerialPort("COM3");
		if (serialPort == INVALID_HANDLE_VALUE)
			serialPort = connectToSerialPort("COM4");
		if (serialPort == INVALID_HANDLE_VALUE)
			serialPort = connectToSerialPort("COM5");
		if (serialPort == INVALID_HANDLE_VALUE)
			serialPort = connectToSerialPort("COM6");
	}
	
	if (serialPort == INVALID_HANDLE_VALUE) {
		printf("Error! Cannot open serial port connection! Terminating...");
		return 1;
	}
	Packet data;
	char buffer[ sizeof(Packet) ];
	int byteRead = 0;
	bool readSuccess = false;
	while (1) {
	// read from opened serial port
		if (readSuccess = ReadFile(serialPort, buffer, sizeof(Packet), LPDWORD(&byteRead), NULL) && sizeof(Packet) == byteRead) {
			//printf("Data in.\n");
			memcpy( &data, buffer, sizeof(Packet) );
			if (data.header == HEADER && sizeof(Packet) == data.size) {
				sprintf (insertDataSql, "INSERT INTO data (temperatureCelcius, pressuremmHg, time) VALUES \
												(%f , %f , NOW())", data.temperatureCelcius, data.pressuremmHg);
				mysql_perform_query(conn, insertDataSql);
				
				sprintf (insertDataSql, "INSERT INTO DataAdditional (Altitude, \
																	AccelerometerAction, \
																	AccelerometerX, \
																	AccelerometerY, \
																	AccelerometerZ, \
																	MagnetometerRawX, \
																	MagnetometerRawY, \
																	MagnetometerRawZ, \
																	MagnetometerScaledX, \
																	MagnetometerScaledY, \
																	MagnetometerScaledZ, \
																	headingDg, \
																	time) VALUES (%d, '%s', %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, NOW())",
																	data.altitude,
																	data.accelerometerDetect,
																	data.accelerometerX,
																	data.accelerometerY,
																	data.accelerometerZ,
																	data.compassRawX,
																	data.compassRawY,
																	data.compassRawZ,
																	data.compassScaledX,
																	data.compassScaledY,
																	data.compassScaledZ,
																	data.heading);
				mysql_perform_query(conn, insertDataSql);
				printf("Temperature: %.1f *C, pressure: %.1f mmHg\n",data.temperatureCelcius, data.pressuremmHg);
				printf("Altitude: %d m\n", data.altitude);
				printf("Accelerometer X: %f m/s^2, Y %f m/s^2, Z %f m/s^2\n", data.accelerometerX, data.accelerometerY, data.accelerometerZ);
				printf("Accelerometer action detection: %s\n", data.accelerometerDetect);
				printf("Magnetometer raw X: %f mGa, Y %f mGa, Z %f mGa\n", data.compassRawX, data.compassRawY, data.compassRawZ);
				printf("Magnetometer scaled X: %f mGa, Y %f mGa, Z %f mGa\n", data.compassScaledX, data.compassScaledY, data.compassScaledZ);
				printf("Magnetometer heading: %f\n", data.heading);
			}
		} else
			printf("Error!\n");
	}
	CloseHandle(serialPort);
	
	
	/* clean up the database link */
	mysql_close(conn);
 
  return 0;
}