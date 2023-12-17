#include "port.h"

port::port() {
	// TODO Auto-generated constructor stub

}

port::~port() {
	// TODO Auto-generated destructor stub
}

bool port::status(HAL_StatusTypeDef status){
	if(status == HAL_OK) return true;
	else return false;
}
