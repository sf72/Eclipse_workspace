/*
 * Cobra36.h
 *
 *  Created on: 24/set/2014
 *      Author: pluto
 *  prototipi rubati da labview
 */

#ifndef COBRA36_H_
#define COBRA36_H_

 extern "C" __declspec(dllexport) void setFrame(uint8_t *Frame);
 extern "C" __declspec(dllexport) void  aceDecrypt(void );
 extern "C" __declspec(dllexport) void  getFrame(uint8_t *Frame);

#endif /* COBRA36_H_ */
