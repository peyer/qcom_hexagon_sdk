#include "hexagon_types.h"

#define  CreateL2pfParam(stride, w, h, dir)   (unsigned long long)HEXAGON_V64_CREATE_H((dir), (stride), (w), (h)) 


static void L2fetch(unsigned int addr, unsigned long long param)
{
    __asm__ __volatile__ ("l2fetch(%0,%1)" : : "r"(addr), "r"(param));
}


