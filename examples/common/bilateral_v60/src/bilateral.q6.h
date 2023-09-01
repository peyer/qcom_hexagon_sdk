//================================
//   Functions
//================================
void bilateral9x9PerRow(
    unsigned char  *src, 
    int             stride, 
    int             width, 
    unsigned char  *dst
);


//===============================================================
//  DEFINES
//===============================================================
#define   max_t(a, b)       ((a) > (b) ? a : b)
#define   roundup_t(a, m)   (((a)+(m)-1)&(-m))
