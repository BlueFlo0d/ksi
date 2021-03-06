#ifndef __data_h__
#define __data_h__
#include <assert.h>
typedef union{int32_t i;float f;} KsiData;
#define ksiNodePortTypeFloat 0x0
#define ksiNodePortTypeGate 0x1
#define ksiNodePortTypeInt32 0x2
#define ksiNodePortTypeEventFlag 0x8
#define ksiNodePortTypeEventFloat (0x0|0x8)
#define ksiNodePortTypeEventGate (0x1|0x8)
#define ksiNodePortTypeEventInt32 (0x2|0x8)
#define ksiNodePortTypeEventString (0x3|0x8)
#define ksiNodePortTypeEventList (0x4|0x8)
#define ksiNodePortTypeMask 0xF
#define ksiNodePortTypeDataMask 0x7
static char ksiDataTypeToCharacter(int8_t typeFlag){
        switch(typeFlag&ksiNodePortTypeMask){
        case ksiNodePortTypeEventFloat:
                return 'F';
        case ksiNodePortTypeEventGate:
                return 'G';
        case ksiNodePortTypeEventInt32:
                return 'I';
        case ksiNodePortTypeEventString:
                return 'S';//dummy
        case ksiNodePortTypeEventList:
                return 'L';//dummy
        case ksiNodePortTypeFloat:
                return 'f';
        case ksiNodePortTypeGate:
                return 'g';
        case ksiNodePortTypeInt32:
                return 'i';
        }
        printf("%d\n",(int)typeFlag);
        assert(0);
        return 0;
}
static inline void ksiDataIncrease(KsiData* s,KsiData i,int8_t type){
        switch(type&ksiNodePortTypeMask){
        case ksiNodePortTypeFloat:
                s->f += i.f;
                break;
        case ksiNodePortTypeInt32:
                s->i += i.i;
                break;
        case ksiNodePortTypeGate:
                s->i |= i.i;
                break;
        }
}
static inline int ksiDataIsUnit(KsiData d,int8_t type){
        switch(type&ksiNodePortTypeMask){
        case ksiNodePortTypeFloat:
                return d.f==1.0f;
        case ksiNodePortTypeInt32:
                return d.i==1;
        case ksiNodePortTypeGate:
                return d.i==0;
        default:
                return 0;
        }
}
static inline void ksiDataWeightedIncrease(KsiData* s,KsiData i,KsiData c,int8_t type){
        switch(type&ksiNodePortTypeMask){
        case ksiNodePortTypeFloat:
                s->f += i.f*c.f;
                break;
        case ksiNodePortTypeInt32:
                s->i += i.i*c.i;
                break;
        case ksiNodePortTypeGate:
                s->i |= i.i|c.i;
                break;
        }
}

static inline void ksiDataBufferWeightedIncrease(KsiData *des,KsiData *src,KsiData coeff,int32_t size,int8_t type){
        switch(type&ksiNodePortTypeMask){
        case ksiNodePortTypeFloat:
                for(int32_t i=0;i<size;i++){
                        des[i].f += src[i].f*coeff.f;
                }
                break;
        case ksiNodePortTypeInt32:
                for(int32_t i=0;i<size;i++){
                        des[i].i += src[i].i*coeff.i;
                }
                break;
        case ksiNodePortTypeGate:
                for(int32_t i=0;i<size;i++){
                        des[i].i |= src[i].i|coeff.i;
                }
                break;
        }

}
#endif
