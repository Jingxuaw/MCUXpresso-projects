/*
 * move.h
 *
 *  Created on: Oct 2, 2018
 *      Author: ASUS
 */

#include "DigitalIoPin.h"
#include <stdlib.h>
#include "chip.h"
#ifndef MOVE_H_
#define MOVE_H_
struct FloatPoint {
	int x;
	int y;
};

class move{

public:
	move();
	bool direction();
	int desired_move(int x, int y);
	void initialize();
	void change_dir(int axle,bool dir);
	int xmove,ymove,x_currentcoor,y_currentcoor,max_x,max_y;
	int ini, step_x, step_y, error, yi;
	bool dirx,diry;
private:

	FloatPoint delta;
	DigitalIoPin dir_x;
	DigitalIoPin dir_y;


};





#endif /* MOVE_H_ */
