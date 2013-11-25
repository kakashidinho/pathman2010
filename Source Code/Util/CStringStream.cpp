#include "stdafx.h"
#include "../Util.h"
#include <string.h>

CStringStream::CStringStream()
{
	this->terminatePos=0;
	this->curPos=0;
	this->str=0;
}
CStringStream::CStringStream(const char* str)
{
	this->terminatePos=strlen(str);
	this->curPos=0;
	this->str=(char*)str;
}

CStringStream::CStringStream(const char* str,unsigned int terminatePos)
{
	this->terminatePos=terminatePos;
	this->curPos=0;
	this->str=(char*)str;
}

void CStringStream::Set(const char *str)
{
	this->terminatePos=strlen(str);
	this->curPos=0;
	this->str=(char*)str;
}
void CStringStream::Set(const char*str ,unsigned int terminatePos)
{
	this->terminatePos=terminatePos;
	this->curPos=0;
	this->str=(char*)str;
}

void CStringStream::Rewind()
{
	this->curPos=0;
}

int CStringStream::GetLine(char *lineOut,unsigned int maxChar)
{
	if(curPos == terminatePos)
		return END_OF_STREAM;

	unsigned int numReadedChar=0;
	while(curPos != terminatePos)
	{
		if(numReadedChar == maxChar)
			return OUT_OF_CHAR;
		if(str[curPos] == '\n' )
		{
			lineOut[numReadedChar]='\0';
			curPos++;
			return 1;
		}
		lineOut[numReadedChar] = str[curPos];
		numReadedChar++;
		curPos++;
	}
	if(numReadedChar == maxChar)
		return OUT_OF_CHAR;
	lineOut[numReadedChar]='\0';
	return 1;
}
