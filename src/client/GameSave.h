//
//  GameSave.h
//  The Powder Toy
//
//  Created by Simon Robertshaw on 04/06/2012.
//

#ifndef The_Powder_Toy_GameSave_h
#define The_Powder_Toy_GameSave_h

#include <vector>
#include <string>
#include "Config.h"
#include "Misc.h"
#include "simulation/Sign.h"
#include "simulation/Particle.h"

using namespace std;

struct ParseException: public exception {
	enum ParseResult { OK = 0, Corrupt, WrongVersion, InvalidDimensions, InternalError, MissingElement };
	string message;
	ParseResult result;
public:
	ParseException(ParseResult result, string message_): message(message_), result(result) {}
	const char * what() const throw()
	{
		return message.c_str();
	}
	~ParseException() throw() {};
};

class GameSave
{
public:
	
	int width, height;

	//Simulation data
	//int ** particleMap;
	int particlesCount;
	Particle * particles;
	unsigned char ** blockMap;
	float ** fanVelX;
	float ** fanVelY;
	
	//Simulation Options
	bool waterEEnabled;
	bool legacyEnable;
	bool gravityEnable;
	bool paused;
	int gravityMode;
	int airMode;
	
	//Signs
	std::vector<sign> signs;
	
	GameSave();
	GameSave(GameSave & save);
	GameSave(int width, int height);
	GameSave(char * data, int dataSize);
	~GameSave();
	void setSize(int width, int height);
	char * Serialise(int & dataSize);
	void Transform(matrix2d transform, vector2d translate);
	
	inline GameSave& operator << (Particle v)
	{
		if(particlesCount<NPART && v.type)
		{
			particles[particlesCount++] = v;
		}
	}
	
	inline GameSave& operator << (sign v)
	{
		if(signs.size()<MAXSIGNS && v.text.length())
			signs.push_back(v);
	}
		
private:
	float * fanVelXPtr;
	float * fanVelYPtr;
	unsigned char * blockMapPtr;

	void readOPS(char * data, int dataLength);
	void readPSv(char * data, int dataLength);
	char * serialiseOPS(int & dataSize);
	//serialisePSv();
};

#endif
