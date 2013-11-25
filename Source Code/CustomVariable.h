#ifndef _CUSTOM_VAR_
#define _CUSTOM_VAR_
#include "ItemManager.h"
#include "SharedPointer.h"
/*-----------------------------------------------------------------------
this file declares some classes used in handling custom variable that are
not hard - coded ,in fact they are read from external sources such as 
files stored in hard disk
-----------------------------------------------------------------------*/
enum VarType
{
	VAR_BOOL,//boolean
	VAR_INT,//integer
	VAR_UINT,//unsigned int
	VAR_FLOAT,//float
	VAR_STRING,//string
	VAR_INT_2,//array of 2 ints
	VAR_INT_3,//array of 3 ints
	VAR_INT_4,//array of 4 ints
	VAR_FLOAT_2,//array of 2 floats
	VAR_FLOAT_3,//array of 3 floats
	VAR_FLOAT_4,//array of 4 floats
	VAR_UNKWN//unknown
};

class Variable
{
public:
	Variable(const char* buffer);//create a Variable object from a line of string stream <buffer> that contains name , type , value of variable
	~Variable();
	VarType GetType() {return type;}
	void * GetValue() {return pValue;}
	const char* GetName() {return name;}
private:
	VarType type;
	void *pValue;
	char* name;
};

/*--------------------manages a group of variables,variables in same group can't have same name---------------------------*/
class VariableGroup:private ItemManager<Variable>
{
private:
	void *GetVariableValue(const char *varName,VarType typeOfVar);
public:
	int ParseVariablesFromFile(const char *fileName);//parse variables from text file source , return number of variables parsed
	int ParseVariablesFromMemory(const unsigned char* byteStream,unsigned int streamSize);//parse variables from byte stream source , return number of variables parsed
	void DeleteVariable(const char *variableName);
	void DeleteVariables(const char *variableNames);//delete a list of variables which has name in string <variableNames>.Names in <variableNames> is separated by space ,or tab or newline char
	bool* GetBool(const char *variableName);//return pointer to value,NULL if variable not exists
	float* GetFloat(const char *variableName);//return pointer to value,NULL if variable not exists
	int* GetInt(const char *variableName);//return pointer to value,NULL if variable not exists
	unsigned int* GetUint(const char *variableName);//return pointer to value,NULL if variable not exists
	int* GetInt2(const char *variableName);//return array of 2 integers
	int* GetInt3(const char *variableName);//return array of 3 integers
	int* GetInt4(const char *variableName);//return array of 4 integers
	float* GetFloat2(const char *variableName);//return array of 2 floats
	float* GetFloat3(const char *variableName);//return array of 3 floats
	float* GetFloat4(const char *variableName);//return array of 4 floats
	char * GetString(const char *variableName);
};

class VariableManager:private ItemManager<VariableGroup>
{
public:
	void DeleteAllVariableGroup();
	int CreateVariableGroup(unsigned int *pGroupID);
	void RemoveVariableGroup(unsigned int groupID);
	SharedPtr<VariableGroup> GetVariableGroup(unsigned int groupID);
};
#endif