#include "stdafx.h"
#include "../CustomVariable.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
/*---------helper functions-----------*/
VarType GetTypeFromString(const char *typeString)
{
	if(!strncmp(typeString,"bool",4))
		return VAR_BOOL;
	if(!strncmp(typeString,"string",6))
		return VAR_STRING;
	if(!strncmp(typeString,"float2",6))
		return VAR_FLOAT_2;
	if(!strncmp(typeString,"float3",6))
		return VAR_FLOAT_3;
	if(!strncmp(typeString,"float4",6))
		return VAR_FLOAT_4;
	if(!strncmp(typeString,"int2",4))
		return VAR_INT_2;
	if(!strncmp(typeString,"int3",4))
		return VAR_INT_3;
	if(!strncmp(typeString,"int4",4))
		return VAR_INT_4;
	if(!strncmp(typeString,"int",3))
		return VAR_INT;
	if(!strncmp(typeString,"uint",4))
		return VAR_UINT;
	if(!strncmp(typeString,"float",4))
		return VAR_FLOAT;
	return VAR_UNKWN;
}

int GetFirstWhitespace(const char* string,int startLocation = 0)//get first location of whitespace (' ' '\t') in string
{
	if(!string)
		return 0;
	char c;
	int index =startLocation;
	do
	{
		c=string[index];
		index++;
	}while (c!='\0' && c!=32 && c!='\t' && c!='\r');
	return index-1;
}
int GetFirstNonWhitespace(const char* string,int startLocation = 0)//get location of 1st char not whitespace char (' ' '\t')
{
	if(!string)
		return 0;
	char c;
	int index =startLocation;
	do
	{
		c=string[index];
		if(c=='\0')
			return -1;
		index++;
	}while (c==32 || c=='\t' || c=='\r');
	return index-1;
}

void ParseData(VarType type,const char *buffer,void **ppValue)//parse value from buffer ,this will depend on variable's type
{
	if(type == VAR_STRING)
	{
		int lastIndex = 0;
		char *psrcBuffer;
		if(buffer[0] == '"')//this string enclosed in double quotes
		{
			psrcBuffer =(char*) buffer+1;
			lastIndex = strchr(psrcBuffer,'"')-psrcBuffer;
		}
		else
		{
			lastIndex = GetFirstWhitespace(buffer);
			psrcBuffer =(char*) buffer;
		}
		(*ppValue) = malloc(lastIndex+1);
		strncpy((char*)*ppValue,psrcBuffer,lastIndex);
		((char*)*ppValue)[lastIndex]='\0';
		return;
	}
	if(type == VAR_FLOAT_2)
	{
		*ppValue=malloc(2*sizeof(float));
		if(sscanf(buffer,"%f %f",((float*)*ppValue),((float*)*ppValue)+1)!=2)//failed
		{
			free(*ppValue);
			*ppValue = NULL;
		}
		return;
	}
	if(type == VAR_FLOAT_3)
	{
		*ppValue=malloc(3*sizeof(float));
		if(sscanf(buffer,"%f %f %f",((float*)*ppValue),((float*)*ppValue)+1,((float*)*ppValue)+2)!=3)//failed
		{
			free(*ppValue);
			*ppValue = NULL;
		}
		return;
	}
	if(type == VAR_FLOAT_4)
	{
		*ppValue=malloc(4*sizeof(float));
		if(sscanf(buffer,"%f %f %f %f",((float*)*ppValue),((float*)*ppValue)+1,((float*)*ppValue)+2,((float*)*ppValue)+3)!=4)//failed
		{
			free(*ppValue);
			*ppValue = NULL;
		}
		return;
	}
	if(type == VAR_INT_2)
	{
		*ppValue=malloc(2*sizeof(int));
		if(sscanf(buffer,"%d %d",((int*)*ppValue),((int*)*ppValue)+1)!=2)//failed
		{
			free(*ppValue);
			*ppValue = NULL;
		}
		return;
	}
	if(type == VAR_INT_3)
	{
		*ppValue=malloc(3*sizeof(int));
		if(sscanf(buffer,"%d %d %d",((int*)*ppValue),((int*)*ppValue)+1,((int*)*ppValue)+2)!=3)//failed
		{
			free(*ppValue);
			*ppValue = NULL;
		}
		return;
	}
	if(type == VAR_INT_4)
	{
		*ppValue=malloc(4*sizeof(int));
		if(sscanf(buffer,"%d %d %d %d",((int*)*ppValue),((int*)*ppValue)+1,((int*)*ppValue)+2,((int*)*ppValue)+3)!=4)//failed
		{
			free(*ppValue);
			*ppValue = NULL;
		}
		return;
	}
	if(type == VAR_INT)
	{
		*ppValue=malloc(sizeof(int));
		if(sscanf(buffer,"%d",*ppValue)!=1)//failed
		{
			free(*ppValue);
			*ppValue = NULL;
		}
		return;
	}

	if(type == VAR_UINT)
	{
		*ppValue=malloc(sizeof(unsigned int));
		if(sscanf(buffer,"%u",*ppValue)!=1)//failed
		{
			free(*ppValue);
			*ppValue = NULL;
		}
		return;
	}
	if(type == VAR_FLOAT)
	{
		*ppValue=malloc(sizeof(float));
		if(sscanf(buffer,"%f",*ppValue)!=1)//failed
		{
			free(*ppValue);
			*ppValue = NULL;
		}
		return;
	}
	if(type == VAR_BOOL)
	{
		bool val;
		char sval[5];
		strncpy(sval,buffer,5);
		if(!strncmp(sval,"true",4))
			val=true;
		else if(!strncmp(sval,"false",5))
			val=false;
		else
			return;

		*ppValue=malloc(sizeof(bool));
		*((bool*)*ppValue) = val;
		return;
	}
	return;

}

/*------------------------------------*/
Variable::Variable(const char *buffer)
{
	name = NULL;
	pValue = NULL;
	if(buffer == NULL)
		return;
	int index[2] = {0,0};
	//get name
	if(buffer[0] == 32 || buffer[0] == '\t' || buffer[0] == '\r')//the first char is whitespace char
	{
		index[1] = GetFirstNonWhitespace(buffer);
		index[0] = GetFirstWhitespace(buffer,index[1]);
	}
	else
		index[0] = GetFirstWhitespace(buffer);
	if(index[0] == 0)
		return;//invalid buffer 
	name = (char*)malloc((index[0] - index[1] +1)*sizeof(char));
	strncpy(name,buffer+index[1],index[0] - index[1]);
	name[index[0] - index[1]] = '\0';
	
	//get type
	index[1] = GetFirstNonWhitespace(buffer,index[0]);
	if(index[1] == -1)
		return;//invalid buffer
	
	index[0] = GetFirstWhitespace(buffer,index[1]);

	char * ctype = (char*)malloc(index[0] - index[1]+1);
	strncpy(ctype,buffer + index[1],index[0] - index[1]);
	ctype[index[0] - index[1]] = '\0';
	this->type = GetTypeFromString(ctype);

	free(ctype);

	//get value
	index[1] = GetFirstNonWhitespace(buffer,index[0]);
	ParseData(type,buffer + index[1],&pValue);
	
}

Variable::~Variable()
{
	if(pValue)
	{
		free(pValue);
	}
	if(name)
		free(name);
}

/*--------------------------------------------*/
int VariableGroup::ParseVariablesFromFile(const char *fileName)
{
	std::ifstream stream;
	stream.open(fileName);
	int numVars=0;
	char line[1024];
	bool begin = false;
	while(stream.good())
	{
		stream.getline(line,1024);
		if(!begin)
		{
			if(!strncmp(line,"Begin Variable Declaration",22))//begin
				begin = true;
		}
		else
		{
			if(!strncmp(line,"End Variable Declaration",22))//end
			{
				begin = false;
				continue;
			}
			char keyword[2];
			int index = GetFirstNonWhitespace(line,0);
			if(index == -1)//blank line
				continue;
			strncpy(keyword,line + index,2);

			if(!strncmp(keyword,"//",2))//comment line,so ignore
				continue;
			Variable* var = new Variable(line);
			if(var->GetValue() == NULL || !this->AddItem(var,0))
			{
				delete var;
				continue;
			}
			numVars++;
		}
	}
	stream.close();
	return numVars;
}
int VariableGroup::ParseVariablesFromMemory(const unsigned char* byteStream,unsigned int streamSize)
{
	if(!byteStream)
		return 0;
	CStringStream stream((char*)byteStream,streamSize);
	int numVars=0;
	char line[1024];
	bool begin = false;
	while(stream.GetLine(line,1024)!=END_OF_STREAM)
	{
		if(!begin)
		{
			if(!strncmp(line,"Begin Variable Declaration",22))//begin
				begin = true;
		}
		else
		{
			if(!strncmp(line,"End Variable Declaration",22))//end
			{
				begin = false;
				continue;
			}
			char keyword[2];
			int index = GetFirstNonWhitespace(line,0);
			if(index == -1)//blank line
				continue;
			strncpy(keyword,line + index,2);

			if(!strncmp(keyword,"//",2))//comment line,so ignore
				continue;
			Variable* var = new Variable(line);
			if(var->GetValue() == NULL || !this->AddItem(var,0))
			{
				delete var;
				continue;
			}
			numVars++;
		}
	}
	return numVars;
}
void VariableGroup::DeleteVariable(const char *variableName)
{
	if(slots)
	{
		for(unsigned int i=0;i<allocSlots;++i)
		{
			SharedPtr<Variable> ptr=this->GetItemPointer(i);
			if(ptr!=NULL)
			{
				if(!strcmp(ptr->GetName(),variableName))
				{
					this->ReleaseSlot(i);
					break;
				}
			}
		}
	}
}

void VariableGroup::DeleteVariables(const char *variableNames)
{
	char *name;
	int len = strlen(variableNames)+1;
	char *nameList = (char*)malloc(len);
	memcpy(nameList,variableNames,len);
	name = strtok(nameList," \t\n");
	while(name!=NULL)
	{
		this->DeleteVariable(name);
		name = strtok(NULL," \t\n");
	}
	free(nameList);

}
bool* VariableGroup::GetBool(const char *variableName)
{
	return (bool*)this->GetVariableValue(variableName,VAR_BOOL);
}
float* VariableGroup::GetFloat(const char *variableName)
{
	return (float*)this->GetVariableValue(variableName,VAR_FLOAT);
}
int* VariableGroup::GetInt(const char *variableName)
{
	return (int*)this->GetVariableValue(variableName,VAR_INT);
}
unsigned int* VariableGroup::GetUint(const char *variableName)
{
	return (unsigned int*)this->GetVariableValue(variableName,VAR_UINT);
}
int* VariableGroup::GetInt2(const char *variableName)
{
	return (int*)this->GetVariableValue(variableName,VAR_INT_2);
}
int* VariableGroup::GetInt3(const char *variableName)
{
	return (int*)this->GetVariableValue(variableName,VAR_INT_3);
}
int* VariableGroup::GetInt4(const char *variableName)
{
	return (int*)this->GetVariableValue(variableName,VAR_INT_4);
}
float* VariableGroup::GetFloat2(const char *variableName)
{
	return (float*)this->GetVariableValue(variableName,VAR_FLOAT_2);
}
float* VariableGroup::GetFloat3(const char *variableName)
{
	return (float*)this->GetVariableValue(variableName,VAR_FLOAT_3);
}
float* VariableGroup::GetFloat4(const char *variableName)
{
	return (float*)this->GetVariableValue(variableName,VAR_FLOAT_4);
}
char * VariableGroup::GetString(const char *variableName)
{
	return (char*)this->GetVariableValue(variableName,VAR_STRING);
}

void *VariableGroup::GetVariableValue(const char *varName,VarType typeOfVar)
{
	if(slots)
	{
		for(unsigned int i=0;i<allocSlots;++i)
		{
			SharedPtr<Variable> ptr=this->GetItemPointer(i);
			if(ptr!=NULL)
			{
				if(!strcmp(ptr->GetName(),varName))
				{
					if(typeOfVar != ptr->GetType())
						return NULL;
					return ptr->GetValue();
				}
			}
		}
	}
	return NULL;
}

/*--------------VariableManager----------------*/
void VariableManager::DeleteAllVariableGroup()
{
	ReleaseAllSlot();
}
int VariableManager::CreateVariableGroup(unsigned int *pGroupID)
{
	VariableGroup *pGroup = new VariableGroup();
	if(!AddItem(pGroup,pGroupID))
	{
		delete pGroup;
		return 0;
	}
	return 1;
}
void VariableManager::RemoveVariableGroup(unsigned int groupID)
{
	ReleaseSlot(groupID);
}
SharedPtr<VariableGroup> VariableManager::GetVariableGroup(unsigned int groupID)
{
	return GetItemPointer(groupID);
}