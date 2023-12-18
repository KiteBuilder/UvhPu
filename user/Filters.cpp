#include "Filters.h"
#include <math.h>

/**
  * @brief Destructor
  * @param None
  * @retval None
  */
PT1Filter::~PT1Filter()
{

}

/**
  * @brief Compute RC filter parameter
  * @param f_cut: cutoff frequency in Hz
  * @retval RC float value
  */
float PT1Filter::ComputeRC(const float f_cut)
{
    return 1.0f / (2.0f * M_PI * f_cut);
}

/**
  * @brief Initialize filter with RC
  * @param tau : circuit time constant in microseconds
  *        dT : time delta, in microseconds, between two measurements
  * @retval None
  */
void PT1Filter::FilterInitRC(float tau, float dT)
{
    state = 0.0f;
    RC = tau;
    this->dT = dT;
    alpha = dT / (RC + dT);
}

/**
  * @brief Initialize filter
  * @param f_cut: cutoff frequency in Hz
  *        dT : time delta, in microseconds, between two measurements
  * @retval None
  */
void PT1Filter::FilterInit(float f_cut, float dT)
{
    FilterInitRC(ComputeRC(f_cut), dT);
}

/**
  * @brief Apply filter to the measured value
  * @param input: measured value
  * @retval filtered value
  */
float  PT1Filter::FilterApply(float input)
{
    state = state + alpha * (input - state);
    return state;
}

/**
  * @brief Apply filter to the measured value.
  *        Additional dT parameter gives us more room if fT not a constant
  * @param input: measured value
  *        dT: time delta, in microseconds, between two measurements
  * @retval input: measured value
  */
float  PT1Filter::FilterApply(float input, float dT)
{
    this->dT = dT;
    alpha = dT / (RC + dT);
    state = state + alpha * (input - state);
    return state;
}
