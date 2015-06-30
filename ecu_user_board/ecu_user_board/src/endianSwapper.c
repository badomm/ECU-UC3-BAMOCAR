/*
 * endianSwapper.c
 *
 * Created: 6/30/2015 16:30:30
 *  Author: Jimmy Chau
 */ 
#include <asf.h>
#include "endianSwapper.h"

float endianSwapperF(float num){
	Union32 orginalEndian;
	Union32 newEndian;
	orginalEndian.f = num;
	newEndian.u32 = endianSwapperU32(orginalEndian.u32);
	return newEndian.f;
}