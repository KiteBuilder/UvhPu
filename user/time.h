
#ifndef TIME_H_
#define TIME_H_

#include <stdint.h>

typedef uint32_t timeMs_t;
typedef uint64_t timeUs_t;
typedef int32_t timeDelta_t;

extern uint32_t usTicks;

timeUs_t micros(void);
void delayUs(timeUs_t us);
void delayMs(timeMs_t ms);

#endif /* TIME_H_ */
