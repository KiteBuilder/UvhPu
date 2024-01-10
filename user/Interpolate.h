#ifndef INTERPOLATE_H_
#define INTERPOLATE_H_

#include <stdint.h>
#include <stdlib.h>

#pragma pack(push, 1)
typedef struct
{
    float x;
    float y;
} xy_t;
#pragma pack(pop)

class NewtonIntrplt {
public:
    NewtonIntrplt() = default;

    NewtonIntrplt(const xy_t *xy_buf, uint32_t n_size);

    virtual ~NewtonIntrplt();

    void Init(const xy_t *xy_buf, uint32_t n_size);
    float GetVal(float x);

private:

    const xy_t *xy = NULL; //array with (x,y) control points
    uint32_t n = 0; //amount of control points
    float **y_forward = NULL;
    float **y_backward = NULL;

    void CreateForwardDiffTbl();
    void CreateBackwardDiffTbl();

    void DeleteForwardDiffTbl();
    void DeleteBackwardDiffTbl();

    float IntrpltNewtForward(float x);
    float IntrpltNewtBackward(float x);
};

class LinearIntrplt {
public:
    LinearIntrplt() = default;

    LinearIntrplt(const xy_t *xy_buf, uint32_t n_size);

    virtual ~LinearIntrplt();

    void Init(const xy_t *xy_buf, uint32_t n_size);
    float GetVal(float x);

private:
    const xy_t *xy = NULL; //array with (x,y) control points
    uint32_t n = 0; //amount of control points

    float a0 = 0;
    float a1 = 0;
    void CalcLine();
};

#endif /* INTERPOLATE_H_ */
