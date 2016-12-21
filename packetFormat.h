#pragma once

const int HEADER = 0x00000501;

struct Packet {
	int header;
	float temperatureCelcius;
	float pressuremmHg;
	int altitude;
	float accelerometerX;
	float accelerometerY;
	float accelerometerZ;
	float compassRawX;
	float compassRawY;
	float compassRawZ;
	float compassScaledX;
	float compassScaledY;
	float compassScaledZ;
	float heading;
	char accelerometerDetect[20];
	int size;
};