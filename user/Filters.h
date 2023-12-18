
#ifndef FILTERS_H_
#define FILTERS_H_

class PT1Filter {
public:
    PT1Filter(): state(0), RC(0), dT(0), alpha(0)
    {

    }

    virtual ~PT1Filter();

    void FilterInit(float f_cut, float dT);
    float FilterApply(float input);
    float FilterApply(float input, float dT);

private:
    float state;
    float RC;
    float dT;
    float alpha;

    float ComputeRC(const float f_cut);
    void FilterInitRC(float tau, float dT);
};

#endif /* FILTERS_H_ */
