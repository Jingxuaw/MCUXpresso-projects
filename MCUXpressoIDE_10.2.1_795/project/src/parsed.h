#pragma once
#include <string>
#include "board.h"
#include <cstring>
#include "user_vcom.h"
#ifdef UNIT
#include "stdafx.h"
#endif

class parsed
{
public:
	parsed();
	~parsed();
	int parser();
	bool has_key(std::string str, char key);
	double get_number(std::string str, char c);
	std::string readUART();
	std::string buf;
	char command;
	int command_index;
	int x, y,power;
	bool down;
private:
	
};

