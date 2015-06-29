/*
 * endianSwapper.h
 *
 * Created: 6/30/2015 01:34:46
 *  Author: Jimmy Chau
 */ 


#ifndef ENDIANSWAPPER_H_
#define ENDIANSWAPPER_H_

#define endianSwapperU32(num) (((num>>24)&0xff) | ((num<<8)&0xff0000) | ((num>>8)&0xff00) | ((num<<24)&0xff000000)) 



#endif /* ENDIANSWAPPER_H_ */