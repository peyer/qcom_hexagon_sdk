/**=============================================================================

@file
   qprintf.c

@brief
   High-level post-processing functions of R and V regs.
   
Copyright (c) 2017 QUALCOMM Technologies Incorporated.
All Rights Reserved Qualcomm Proprietary
=============================================================================**/

#include "qprintf.h"

#ifndef FARF_HIGH
#define FARF_HIGH 1
#endif
#include "HAP_farf.h"
#include "string.h"

unsigned long long qd_print_mask[2] = {-1ull,-1ull};

qprintf_API void qprintf_set_mask(unsigned long long high, unsigned long long low) {
	qd_print_mask[1]=high;
	qd_print_mask[0]=low;
}

int getR(int index, char* stackRegPtr, char *stackArgPtr) {
    if ((index<0)|(index>=32)) {
        printf("Index %d out of range for accessing scalar register.",index);
        return -1;
    }
    if (index==29) {
        return *((int *)(stackArgPtr+12));
    } else if (index == 30) {
        return *((int *)(stackArgPtr+32));
    } else if (index ==31) {
        return *((int *)(stackArgPtr+36));
    } else {
        return *((int *)(stackRegPtr+index*4));
    }
}

int getP(int index, char* stackRegPtr) {
    if ((index<0)|(index>=4)) {
        printf("Index %d out of range for accessing P register.",index);
        return -1;
    }
    return *((int *)(stackRegPtr+112+index*4));
}

#if (__HEXAGON_ARCH__ >= 60)
HVX_Vector getV(int index, char* stackRegPtr) {
    if ((index<0)|(index>=32)) {
        printf("Index %d out of range for accessing V register.",index);
        return Q6_V_vsplat_R(-1);
    }
    HVX_Vector* hvxregPtr = (HVX_Vector*)( (~127)&(int)(stackRegPtr+28*4 + 4*4 + 127) );
    return *(hvxregPtr+index);
}

HVX_Vector getQ(int index, char* stackRegPtr) {
    if ((index<0)|(index>=4)) {
        printf("Index %d out of range for accessing Q register.",index);
        return Q6_V_vsplat_R(-1);
    }
    HVX_Vector* hvxregPtr = (HVX_Vector*)( (~127)&(int)(stackRegPtr+28*4 + 4*4 + 127+128*32) );
    return *(hvxregPtr+index);
}
#endif

// To keep things simpler, we expect V formatter to end after a few known symbols
int getVFormatter(char* formatter, char* string) {
    int count=0;
    while ((*string != '\0')&
           (*string != ' ')&
           (*string != ',')&
           (*string != ';')&
           (*string != '|')&
           (*string != '/')&
           (*string != '\\')&
           (*string != '\n')&
           (*string != '\t')&
           (*string != ':'))  {
       *formatter++=*string++;
       count++;
    }
    *formatter='\0';
    return count;
}
enum Type {R,V,P,Q}; 

typedef struct reg
{
    int index; // -1 means 'all'
    enum Type type;
} reg;

int getRegMapping(char* argv, reg* regs) {
    char* token;
    int index=0;
    while ((token = strsep(&argv, ",")) != NULL) {
        while(*token==' ') token++;
        regs[index].index=atoi(token+1);
        // Make sure that 0 was the result of parsing '0' and not the result of a parsing error
        if (!regs[index].index) {
            regs[index].index=*(token+1)=='0'?0:-1;
        }
        if ((*token=='r')|((*token=='R'))) {
            regs[index].type=R;
        } else if ((*token=='v')|((*token=='V'))) {
            regs[index].type=V;
        } else if ((*token=='p')|((*token=='P'))) {
            regs[index].type=P;
        } else if ((*token=='q')|((*token=='Q'))) {
            regs[index].type=Q;
        }
//        printf("Parsed %s into (%d,%d)\n",token,regs[index].type,regs[index].index);
        index++;
    }
    return index;
}    

// Return number of consecutive 'd' or 'u' at beginning of string
int getNumDU(char* string) {
    int count=0;
    while (*string != '\0')   {
       char c=*string++;
       if ((c=='d')|(c=='u')) {
           count++;
       } else {
           break;
       }
    }
    return count;
}

int isFloatFormat(char* string) {
    while (*string != '\0')   {
       char c=*string++;
       if ((c=='c')|(c=='s')|(c=='d')|(c=='i')|(c=='o')|(c=='x')|(c=='X')|(c=='u')|(c=='n')|(c=='p')) {
           return 0;
       } else if ((c=='f')|(c=='F')|(c=='e')|(c=='E')|(c=='a')|(c=='A')|(c=='g')|(c=='G')) {
           return 1;
       }
    }
    return 0;
}

/*    Format: %[m][(<int>)]<formatter>
 *                    ^    ^
 *        numCol -----|    |--- formatterIdx
 */
typedef struct maskFormat
{
    int isMasked;       // 1 if masking was requested
    int numCol;         // num of columns to be displayed in a given row
    int formatterIdx;   // begin index of standard formating string
} maskFormat;


maskFormat getMaskFormat(char* string) {
    maskFormat mf = {0,-1,1};
    while (*string != '\0')   {
        char c=*string++;
        if ((c=='c')|(c=='s')|(c=='d')|(c=='i')|(c=='o')|(c=='x')|(c=='X')|(c=='u')|(c=='n')|(c=='p')|
           (c=='f')|(c=='F')|(c=='e')|(c=='E')|(c=='a')|(c=='A')|(c=='g')|(c=='G')) {
            return mf;
        } else if ((c=='m')|(c=='(')) {
            if (c=='m') {
                mf.formatterIdx=2;
                mf.isMasked=1;
            } else {
                string--; // re-point to '('
            }
            mf.numCol=-1;
            if (*string++=='(') {
                mf.numCol=atoi(string);
                while(*string++!=')') {
                    mf.formatterIdx+=1;
                }
                mf.formatterIdx+=2;
            }
            return mf;
        }
    }
    return mf;
}

void display32b( char* display, char* string, int value) {
    int numDU=getNumDU(string+1);
    if (numDU>=2) { // new format detected with more than one consecutive d or u               
        char regString[128];
        char* tmpToken = string+1;
        int shftAmt=32;
        int elementSize=32/numDU;
        int elementMask=(1<<elementSize)-1;
        int firstElement=1;
        while (*tmpToken != '\0')   {
            shftAmt-=elementSize;
            char c=*tmpToken++;
            // Right-align element
            int elementValue = value >> shftAmt;
            
            // Sign-extend or zero-extend depending on format type
            if (c=='d') {
                elementValue=(elementValue<<(32-elementSize))>>(32-elementSize);
            } else if (c=='u') {
                elementValue&=elementMask;
            } else {
                //printf("*** BREAK\n");
                break;
            }
            sprintf(regString,numDU==4?"%s%4d":"%s%6d",firstElement?"":"|",elementValue);
            firstElement=0;
            strcat(display,regString);
            //printf("\n*** regString=[%s] and new display=[%s]\n",regString,display);
        }
        strcat(display,string+1+numDU);
    } else { // standard format
        printf("%s",display);
        if (isFloatFormat(string)) {
            float floatValue=*(float*)&value;
            sprintf(display,string,floatValue);
        } else {
            sprintf(display,string,value);
        }
    }
}    

#if (__HEXAGON_ARCH__ >= 60)

/** Append to display the next string segment that contains an HVX register
  *
  * char* display: processed string to be passed on to printf for display
  * char* string: string to process, beginning with and containing only one %
  * char* formatter: formatter to be extracted from string
  * HVX_Vector vect: register to display
  */
void display128B(char* display, char* string, char* formatter, HVX_Vector vect) {
    int value, formatterSize, numDU, numCol, colIdx=0, i;  
    maskFormat mf = getMaskFormat(string);
    numDU=getNumDU(string+mf.formatterIdx);
    formatterSize=getVFormatter(formatter+1, string+mf.formatterIdx);  
    if (!mf.isMasked) { 
        numCol=mf.numCol==-1?(numDU==4?4:8):mf.numCol;
        for (i=124;i>=0;i-=4) {
            if (!(colIdx%numCol)) {
                strcat(display,"\n  ");
            }
            value = Q6_R_vextract_VR(vect,i);
            display32b(display, formatter, value);
            colIdx++;
            if (colIdx%numCol) {
                strcat(display,",");
            }
        }
    } else {
        numCol=mf.numCol==-1?8:mf.numCol;
        if (numDU==1) {
            for (i=124;i>=0;i-=4) {
                if (qd_print_mask[(i&64)>>6]&(1ull<<(i&63))) { 
                    if (!(colIdx%numCol)) {
                        strcat(display,"\n  ");
                    }
                    char index[8];
                    sprintf(index,"[%02x]=",i); 
                    strcat(display,index);
                    value = Q6_R_vextract_VR(vect,i);
                    display32b(display, formatter, value);
                    colIdx++;
                    if (colIdx%numCol) {
                        strcat(display,",");
                    }
                }
            }
        } else {
            int startIndex=numDU==2?126:127;
            int elementSize=32/numDU;
            int elementMask=(1<<elementSize)-1;
            for (i=startIndex;i>=0;i-=3-(numDU>>1)) {
                if (qd_print_mask[(i&64)>>6]&(1ull<<(i&63))) { 
                    if (!(colIdx%numCol)) {
                        strcat(display,"\n  ");
                    }
                    char index[8];
                    sprintf(index,"[%02x]=",i); 
                    strcat(display,index);
                    value = Q6_R_vextract_VR(vect,i);
                    // Assess sign by looking at last char in formatter
                    if (*(string+mf.formatterIdx+formatterSize-1)=='d') {
                        value=(value<<(32-elementSize))>>(32-elementSize);
                    } else {
                        value&=elementMask;
                    }
                    char regString[128];
                    sprintf(regString,numDU==2?"%6d":"%4d",value);
                    strcat(display,regString);
                    colIdx++;
                    if (colIdx%numCol) {
                        strcat(display,",");
                    }
                }
            }
        }
    }
}

void displayQ(char* display, HVX_Vector V) 
{
    int i; 
    unsigned long long highQ = 0;
    for (i=127;i>=64;i-=1) { 
        highQ<<=1;
        int a= Q6_R_vextract_VR(V,i);
        a<<=(24-8*(i&3)); 
        a>>=24; 
        highQ|=a&1;
    } 
    unsigned long long lowQ = 0;
    for (i=63;i>=0;i-=1) { 
        lowQ<<=1;
        int a= Q6_R_vextract_VR(V,i);
        a<<=(24-8*(i&3)); 
        a>>=24; 
        lowQ|=a&1;
    } 
    char name[56];
    sprintf(name,"0x%08x|%08x|%08x|%08x",(int)(highQ>>32),(int)(highQ&-1),(int)(lowQ>>32),(int)(lowQ&-1)); 
    strcat(display,name); 
}
#endif

qprintf_API void qprintf_c(char* stackRegPtr, char* stackArgPtr) {
    char* msg_orig = *((char**)(stackArgPtr+16));
    char* args = *((char**)(stackArgPtr+20));
    
    char msg_array[512];  // Max size for the line to be displayed
    strcpy(msg_array, msg_orig);
    char *msg = (char*)msg_array;
    
    reg regMap[32];  // No more that 32 registers can be specified in any given assembly qprintf command 
    
    // Populate register map and identify how many registers were passed in
    int numRegs = getRegMapping(args,regMap);
 
    char* token;
    int count=0;
    int wasPreviousCharPercent=0;
    char display[4000];
    strcpy(display,"");
    int regIndex=0;

    int lineNumber=*((int *)(stackArgPtr+24));
    char* filePath=*((char**)(stackArgPtr+28));
    // strip path to keep filename concise
    char* fileName=filePath;
    char* c=filePath;
    while(*c++!='\0') {
        if (*c=='/') {
            fileName=c+1;
        }
    }
    if (lineNumber>0) {  // Only print file[line] if line number is valid. Otherwise we are calling from C via qprintf_V_all
      printf("%s[%d]: ",fileName,lineNumber);
    }

    while (1) {
        
        // recipient for string beginning with and containing exactly one formatted symbol: %[format]<some txt>
        char string[4000];

        // Get next token. If we reach end of the string, exit
        token = strsep(&msg, "%");
        //printf("Processing [%s] ; regIndex=%d\n",token,regIndex);
        if (!(token)) {
            break;
        }
        
        // Append token into string, preceding with % first unless we process first segment
        // If first token and non-empty, just print token and move to next
        if (count++) {
            sprintf(string,"%%%s",token);
        } else {
            printf("%s",token);
            continue;
        }

        // If current token is empty and not first token, flag as sequence of two consecutive %
        // Move to process next token after appending a % in string if needed
        char c=*token;
        if (c=='\0') {
            if (wasPreviousCharPercent) {
               strcat(display,"%");
            }
            wasPreviousCharPercent^=1;
            continue;
        }

        // string begins with and contains exactly one formatted symbol: %[format]<some txt>
        if (wasPreviousCharPercent) {
            strcat(display,string);
        } else {
            if (regIndex>=numRegs) {
                printf("\nNot enough registers (%d vs. %d) specified in string\n  %s\n  to display string %s\n",
                       regIndex,numRegs,args,*((char**)(stackArgPtr+16)));
            }
            char formatter[16];
            *formatter='%';
            int value, i;

#if (__HEXAGON_ARCH__ >= 60)
            char txt[8];
            maskFormat mf;
            int formatterSize;
#endif

            switch(regMap[regIndex].type) {
                case R:
                if (regMap[regIndex].index==-1) {
                    for (i=0;i<32;i+=1) {
                        if (i%8) {
                            strcat(display,", ");
                        } else {
                            strcat(display,"\n  ");
                        }
                        value = getR(i,stackRegPtr,stackArgPtr);
                        char txt[8];
                        sprintf(txt,"R%-2d: ",i); 
                        strcat(display,txt);
                        display32b(display, string, value);
                    }
                    strcat(display,"\n  ");
                    for (i=0;i<4;i+=1) {
                        if (i%4) {
                            strcat(display,", ");
                        }
                        value = getP(i,stackRegPtr);
                        char txt[8];
                        sprintf(txt,"P%d:  ",i); 
                        strcat(display,txt);
                        display32b(display, "%08x", value);
                    }
                } else {
                    value = getR(regMap[regIndex].index,stackRegPtr,stackArgPtr);
                    display32b(display, string, value);
                }
                break;
                
                case P:
                value = getP(regMap[regIndex].index,stackRegPtr);
                display32b(display, string, value);
                break;
                
#if (__HEXAGON_ARCH__ >= 60)
                case V:
                if (regMap[regIndex].index==-1) {
                    strcat(display,"\n");
                    for (i=0;i<32;i+=1) {
                        char txt[8];
                        sprintf(txt,"V%-2d:",i); 
                        strcat(display,txt);
                        display128B(display, string, formatter, getV(i,stackRegPtr));
                        printf("%s\n",display);
                        strcpy(display,"");
                    }
                    for (i=0;i<4;i+=1) {
                        char txt[8];
                        sprintf(txt,"Q%d: ",i); 
                        strcat(display,txt);
                        displayQ(display, getQ(i,stackRegPtr));
                        strcat(display,i&1?"\n":", ");
                    }
                } else {
                    display128B(display, string, formatter, getV(regMap[regIndex].index,stackRegPtr));
                }
                mf = getMaskFormat(string);
                formatterSize=getVFormatter(formatter+1, string+mf.formatterIdx);  
                strcat(display,string+mf.formatterIdx+formatterSize);
                break;

                case Q:
                sprintf(txt,"Q%d: ",regMap[regIndex].index); 
                strcat(display,txt);
                displayQ(display, getQ(regMap[regIndex].index,stackRegPtr));
                break;                
#endif

                default:
                printf("Register type %d not supported for display\n",regMap[regIndex].type);
            }
            regIndex++;
        } 
        wasPreviousCharPercent=0;
    }
    printf("%s\n",display);  
    
}

#if (__HEXAGON_ARCH__ >= 60)
/* A simple function to display a string with exactly one HVX register
 * and its formatter */
qprintf_API void qprintf_V(char* msg, HVX_Vector V) {
    char display[2000];
    int count=0;
    while(msg[count]!='%') {
        display[count]=msg[count];
        count++;
    }
    display[count]='\0';
    char* string = msg+count;
    char formatter[16];
    *formatter='%';
    display128B(display, string, formatter, V);
    maskFormat mf = getMaskFormat(formatter);
    int formatterSize=getVFormatter(formatter+1, string+mf.formatterIdx);     
    strcat(display,string+mf.formatterIdx+formatterSize);
    printf("%s",display);
}

qprintf_API void qprintf_Q(char* msg, HVX_VectorPred Q) {
    HVX_Vector V = Q6_V_vand_QR(Q,-1);

    char display[2000];
    int count=0;
    while(msg[count]!='%') {
        display[count]=msg[count];
        count++;
    }
    display[count]='\0';

    char* string = msg+count;
    maskFormat mf = getMaskFormat(string);
    displayQ(display, V);
    char tmp[16];
    int formatterSize=getVFormatter(tmp, string+mf.formatterIdx);     
    strcat(display,string+mf.formatterIdx+formatterSize);
    printf("%s",display);    
}
#endif
