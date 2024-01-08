#include <b57861s103.h>

const float B57861s103::TableTemp[size] = {
	-55.0, -50.0, -45.0, -40.0, -35.0,
	-30.0, -25.0, -20.0, -15.0, -10.0,
	 -5.0,	 0.0,   5.0,  10.0,  15.0,
	 20.0,  25.0,  30.0,  35.0,  40.0,
	 45.0,  50.0,  55.0,  60.0,  65.0,
	 70.0,  75.0,  80.0,  85.0,  90.0,
	 95.0, 100.0, 105.0, 110.0, 115.0,
	120.0, 125.0, 130.0, 135.0, 140.0,
	145.0, 150.0, 155.0
};

const float B57861s103::TableRes[size] = {
	963000.0, 670100.0, 471700.0, 336500.0, 242600.0,
	177000.0, 130400.0,  97070.0,  72930.0,  55330.0,
	 42320.0,  32650.0,  25390.0,  19900.0,  15710.0,
	 12490.0,  10000.0,   8057.0,   6531.0,   5327.0,
	  4369.0,   3603.0,   2986.0,   2488.0,   2083.0,
	  1752.0,   1481.0,   1258.0,   1072.0,    917.7,
	   788.5,    680.0,    588.6,    511.2,    445.4,
	   389.3,    341.7,    300.9,    265.4,    234.8,
	   208.3,    185.3,    165.3
};

B57861s103::B57861s103() {
	// TODO Auto-generated constructor stub

}

B57861s103::~B57861s103() {
	// TODO Auto-generated destructor stub
}

//2490, tempVolt, 3.0
float B57861s103::toTemp(float res_up, float v_res, float v_ref){
	float r_t = res_up * v_res / (v_ref - v_res);
	float temp = 0;
	float persent;
	short indexL = 0;
	short indexH = 0;

	for(short i = 1; i < size; i++){
		if(r_t <= TableRes[i-1] && r_t >= TableRes[i]){
			indexH = i-1;
			indexL = i;
			break;
		}
	}

	if(indexL != indexH){
	  persent = (r_t - TableRes[indexL]) / (TableRes[indexH] - TableRes[indexL]);
	  temp = TableTemp[indexL] - persent*(TableTemp[indexL] - TableTemp[indexH]);
	}
	return temp;
}
