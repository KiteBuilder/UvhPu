#ifndef B57861S103_H_
#define B57861S103_H_

class B57861s103 {
public:
	B57861s103();
	virtual ~B57861s103();

	static float toTemp(float res_up, float v_res, float v_ref);

private:
	static const int size = 43;
	static const float TableTemp[size];
	static const float TableRes[size];
};


#endif /* B57861S103_H_ */
