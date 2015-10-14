/* 
 * File:   stringutils.h
 * Author: banshee
 *
 * Created on January 28, 2014, 11:35 PM
 */

#ifndef STRINGUTILS_H
#define	STRINGUTILS_H


#include <string.h>

void cleanLine(char *line);
char **splitLine(char *line, char *delim);
void compactLine(char* dest, char* line);

#endif	/* STRINGUTILS_H */