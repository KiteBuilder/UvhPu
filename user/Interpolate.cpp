#include "interpolate.h"

/**
  * @brief Constructor
  * @param None
  * @retval None
  */
NewtonIntrplt::NewtonIntrplt(const xy_t *xy_buf, uint32_t n_size): xy(xy_buf), n(n_size)
{
    if (xy != NULL && n > 0)
    {
        CreateForwardDiffTbl();
        CreateBackwardDiffTbl();
    }
}

/**
  * @brief Destructor
  * @param None
  * @retval None
  */
NewtonIntrplt::~NewtonIntrplt()
{
    DeleteForwardDiffTbl();
    DeleteBackwardDiffTbl();
}

/**
  * @brief Inititalize interpolation method
  * @param None
  * @retval None
  */
void NewtonIntrplt::Init(const xy_t *xy_buf, uint32_t n_size)
{
    xy = xy_buf;
    n = n_size;

    if (xy != NULL && n > 0)
    {
        if (y_forward != NULL)
        {
            DeleteForwardDiffTbl();
        }
        CreateForwardDiffTbl();

        if (y_backward != NULL)
        {
            DeleteBackwardDiffTbl();
        }
        CreateBackwardDiffTbl();
    }
}

/**
  * @brief Allocate memory, create and calculate the forward difference table
  * @param None
  * @retval None
  */
void NewtonIntrplt::CreateForwardDiffTbl()
{
    //Allocat memory for two dimensions array
    y_forward  = new float*[n];
    for (uint32_t i = 0; i < n; i++)
    {
        y_forward[i] = new float[n];
    }

    //To load allocated array with a f(x) values
    for (uint32_t i = 0; i < n; i++)
    {
        y_forward[i][0] = xy[i].y;
    }

    //Calculate the forward difference table
    for (uint32_t i = 1; i < n; i++)
    {
        for (uint32_t j = 0; j < n-i; j++)
        {
            y_forward[j][i] = y_forward[j + 1][i - 1] - y_forward[j][i - 1];
        }
    }
}

/**
  * @brief Allocate memory, create and calculate the backward difference table
  * @param None
  * @retval None
  */
void NewtonIntrplt::CreateBackwardDiffTbl()
{
    //Allocat memory for two dimensions array
    y_backward = new float* [n];
    for (uint32_t i = 0; i < n; i++)
    {
        y_backward[i] = new float[n];
    }

    //To load allocated array with a f(x) values
    for (uint32_t i = 0; i < n; i++)
    {
        y_backward[i][0] = xy[i].y;
    }

    //Calculate the backward difference table
    for (uint32_t i = 1; i < n; i++)
    {
        for (uint32_t j = i; j < n; j++)
        {
            y_backward[j][i] = y_backward[j][i - 1] - y_backward[j - 1][i - 1];
        }
    }
}

/**
  * @brief To free allocated memory for the forward difference table
  * @param None
  * @retval None
  */
void NewtonIntrplt::DeleteForwardDiffTbl()
{

    if (y_forward == NULL)
    {
        return;
    }

    //To free allocated memory
    for (uint32_t i = 0; i < n; i++) {
        delete[] y_forward[i];
    }
    delete[] y_forward;

    y_forward = NULL;
}

/**
  * @brief To free allocated memory for the backward difference table
  * @param None
  * @retval None
  */
void NewtonIntrplt::DeleteBackwardDiffTbl()
{
    if (y_backward == NULL)
    {
        return;
    }

    //To free allocated memory
    for (uint32_t i = 0; i < n; i++) {
        delete[] y_backward[i];
    }
    delete[] y_backward;

    y_backward = NULL;
}

/**
  * @brief Forward Newton Interpolation method
  * @param x: x-axis value for which we have to find teh y value
  *
  * @retval float: Calculated y value
  */
float NewtonIntrplt::IntrpltNewtForward(float x)
{
    if (y_forward == NULL)
    {
        return 0;
    }

    //Interpolate
    float sum = y_forward[0][0];
    float u = (x - xy[0].x) / (xy[1].x - xy[0].x);
    float p = 1.0;
    for (uint32_t i = 1; i < n; i++)
    {
        p *= (u - i + 1) / i;
        sum = sum + p * y_forward[0][i];
    }

    return sum;
}

/**
  * @brief Backward Newton Interpolation method
  * @param x: x-axis value for which we have to find teh y value
  *
  * @retval float: Calculated y value
  */
float NewtonIntrplt::IntrpltNewtBackward(float x)
{
    if (y_backward == NULL)
    {
        return 0;
    }

    //Interpolate
    float sum = y_backward[n-1][0];
    float u = (x - xy[n-1].x) / (xy[1].x - xy[0].x);
    float p = 1.0;
    for (uint32_t i = 1; i < n; i++)
    {
        p *= (u + i - 1) / i;
        sum = sum + p * y_backward[n - 1][i];
    }

    return sum;
}

/**
  * @brief Newton interpolation method
  * @param x: x-axis value for which we have to find the y value
  *
  * @retval float: Calculated y value
  */
float NewtonIntrplt::GetVal(float x)
{
    if (xy == NULL)
    {
        return 1;
    }

    uint32_t k = 0;
    while (k < n)
    {
        if (x < xy[k].x)
        {
            break;
        }
        ++k;
    }

    float result = 0.0;

    if (k < (n >> 1))
    {
        result = IntrpltNewtForward(x);
    }
    else
    {
        result = IntrpltNewtBackward(x);
    }

    return result;
}

/**
  * @brief Constructor
  * @param None
  * @retval None
  */
LinearIntrplt::LinearIntrplt(const xy_t *xy_buf, uint32_t n_size): xy(xy_buf), n(n_size)
{
    if (xy != NULL && n > 0)
    {
        CalcLine();
    }
}

/**
  * @brief Destructor
  * @param None
  * @retval None
  */
LinearIntrplt::~LinearIntrplt()
{

}

/**
  * @brief Inititalize interpolation method
  * @param None
  * @retval None
  */
void LinearIntrplt::Init(const xy_t *xy_buf, uint32_t n_size)
{
    xy = xy_buf;
    n = n_size;

    if (xy != NULL && n > 0)
    {
        CalcLine();
    }
}

/**
  * @brief Calculate the linear interpolation coefficients
  * @param
  *
  * @retval
  */
void LinearIntrplt::CalcLine()
{
    float S1, S2, S3, S4;
    a0 = a1 = 0;

    for (uint32_t i = 0; i < n; i++)
    {
        S1 += xy[i].x;
        S2 += xy[i].y;
        S3 += xy[i].x * xy[i].x;
        S4 += xy[i].x * xy[i].y;
    }

    a0 = ((S2 * S3) - (S1 * S4)) / ((n * S3) - (S1 * S1));
    a1 = ((n * S4) - (S1 * S2)) / ((n * S3) - (S1 * S1));
}

/**
  * @brief Linear interpolation method
  * @param x: x-axis value for which we have to find the y value
  *
  * @retval float: Calculated y value
  */
float LinearIntrplt::GetVal(float x)
{
    return x * a1 + a0;
}

/**
  * @brief Linear interpolation on the interval
  * @param x: x-axis value for which we have to find the y value
  *
  * @retval float: Calculated y value
  */
float LinearIntrplt::GetValOnInterval(float x)
{
    float a = 0, b = 0;

    if (xy == NULL)
    {
        return 1;
    }

    for (uint32_t i = 1; i < n; i++)
    {
        if (x >= xy[i - 1].x && x <= xy[i].x)
        {
            a = (xy[i].y - xy[i - 1].y) / (xy[i].x - xy[i - 1].x);
            b = xy[i - 1].y - a * xy[i - 1].x;

            break;
        }
    }

    return x * a + b;
}
