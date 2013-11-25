#ifndef _UTIL_
#define _UTIL_

#ifdef UTIL_EXPORTS
#define UTIL_API __declspec(dllexport)
#else
#define UTIL_API __declspec(dllimport)
#endif

#include "SharedPointer.h"
#include "ItemManager.h"
#include "GrowableArray.h"

#define SafeDelete(p) {if(p!=NULL) {delete p;p=NULL;}}
#define SafeDeleteArray(p) {if(p!=NULL) {delete[] p;p=NULL;}}
#define SafeRelease(p) {if(p!=NULL){p->Release();p=NULL;}}

//pi family
#define _PI    ((float)  3.141592654f) //pi
#define _1OVERPI ((float)  0.318309886f) //1/pi
#define _PIOVER2 ((float)	 1.570796327f) //pi/2
#define _PIOVER3 ((float)	 1.047197551f) //pi/3
#define _PIOVER4 ((float)	 0.785398163f) //pi/4
#define _PIOVER6 ((float)	 0.523598775f) //pi/6
#define _2PI   ((float)	 6.283185307f) //2*pi

/*-----------struct prototype------------*/
struct Matrix4x4;
/*-------------------------------------------
Vector 2D (x,y)
--------------------------------------------*/
struct UTIL_API Vector2
{
	Vector2();
	Vector2(float x,float y);

	void Set(float x,float y);

	Vector2& Normalize();

	float Length();//length
	float LengthSqr()const;//square of length

	Vector2 operator -()const;
	
	Vector2& operator +=(const Vector2& v2);
	Vector2& operator -=(const Vector2& v2);
	Vector2& operator *=(const float f);
	Vector2& operator /=(const float f);

	Vector2 operator +(const Vector2& v2)const;
	Vector2 operator -(const Vector2& v2)const;
	Vector2 operator *(const float f) const;
	Vector2 operator /(const float f) const;

	float operator *(const Vector2& v2)const;//dot product operator

	union{
		struct{float x,y;};
		float v[2];
	};
};
/*-------------------------------------------
Vector 3D (x,y,z)
--------------------------------------------*/
struct UTIL_API Vector3
{
	Vector3();
	Vector3(float x,float y,float z);
	Vector3(const Vector2& v,float z);

	void Set(float x,float y,float z);
	void Set(const Vector2& v,float z);

	Vector3& Normalize();

	float Length();//length
	float LengthSqr()const;//square of length

	Vector3 operator -()const;
	
	Vector3& operator +=(const Vector3& v2);
	Vector3& operator -=(const Vector3& v2);
	Vector3& operator *=(const float f);
	Vector3& operator /=(const float f);

	Vector3 operator +(const Vector3& v2)const;
	Vector3 operator -(const Vector3& v2)const;
	Vector3 operator *(const float f) const;
	Vector3 operator /(const float f) const;

	float operator *(const Vector3& v2)const;//dot product operator
	Vector3 Cross(const Vector3& v2)const;//cross product
	Vector3& Cross(const Vector3& v1,const Vector3& v2);//cross product, result will be stored in this object

	union{
		struct{float x,y,z;};
		float v[3];
	};
};
/*------------------------------------------
Vector (x,y,z,w)
w=0 denotes vector coordinates
w=1 denotes point's position
-------------------------------------------*/
struct UTIL_API Vector4
{
	Vector4();
	Vector4(float x,float y,float z);//create vector object with w=0
	Vector4(float x,float y,float z,float w);
	Vector4(const Vector3& v,float w);

	void Set(float x,float y,float z);
	void Set(float x,float y,float z,float w);
	void Set(const Vector3& v,float w);

	Vector4& Normalize();

	float Length();//length
	float LengthSqr()const;//square of length

	Vector4 operator -()const;
	
	Vector4& operator +=(const Vector4& v2);
	Vector4& operator -=(const Vector4& v2);
	Vector4& operator +=(const Vector3& v2);
	Vector4& operator -=(const Vector3& v2);
	Vector4& operator *=(const float f);
	Vector4& operator /=(const float f);
	Vector4& operator *=(const Matrix4x4& m);

	Vector4 operator +(const Vector4& v2)const;
	Vector4 operator -(const Vector4& v2)const;
	Vector4 operator +(const Vector3& v2)const;
	Vector4 operator -(const Vector3& v2)const;
	Vector4 operator *(const float f) const;
	Vector4 operator /(const float f) const;
	Vector4 operator*(const Matrix4x4& m)const;

	float operator *(const Vector4& v2)const;//dot product operator
	Vector4 Cross(const Vector4& v2)const;//cross product
	Vector4& Cross(const Vector4& v1,const Vector4& v2);//cross product, result will be stored in this object

	union{
		struct{float x,y,z,w;};
		float v[4];
	};
};

/*---------------------------------------
Matrix 4x4
---------------------------------------*/
struct UTIL_API Matrix4x4
{
	Matrix4x4();
	Matrix4x4(float _11,float _12,float _13,float _14,
			  float _21,float _22,float _23,float _24,
			  float _31,float _32,float _33,float _34,
			  float _41,float _42,float _43,float _44);


	void Set(float _11,float _12,float _13,float _14,
			  float _21,float _22,float _23,float _24,
			  float _31,float _32,float _33,float _34,
			  float _41,float _42,float _43,float _44);
	
	float& operator() (unsigned int row,unsigned int col)//member access , row & col is zero-based
	{
		if(row > 3 || col > 3) //out of bound
			return m[15];//return last member
		return mt[row][col];
	}	

	void Identity();

	operator float*(){return m;};//casting operator
	operator const float*()const {return m;}

	Matrix4x4 operator *(const Matrix4x4& m)const;
	Matrix4x4& operator *=(const Matrix4x4& m);

	union{
		struct{
			float _11,_12,_13,_14,
				  _21,_22,_23,_24,
				  _31,_32,_33,_34,
				  _41,_42,_43,_44;
		};
		float m[16];
		float mt[4][4];
	};
};
/*----------------matrix operations for openGL---------------------------*/
UTIL_API Matrix4x4* Matrix4View(const Vector4* pX,const Vector4* pY,const Vector4* pZ,const Vector4* pPos,Matrix4x4 *out);//create view matrix,given its axes <pX,pY,pZ> and origin <pPos>
UTIL_API Matrix4x4* Matrix4LookAtRH(const Vector4 *pEye,const Vector4* pAt,const Vector4* pUp,Matrix4x4*out);//create right-handed view matrix ,given camera position<pEye> ,look at postion <pAt>,vector up <pUp>
UTIL_API Matrix4x4* Matrix4OrthoProjRH(const float width,const float height,const float zNear,const float zFar,Matrix4x4*out);//create right-handed ortho projection matrix 
UTIL_API Matrix4x4* Matrix4PerspectiveProjRH(const float vFOV,const float aspect,const float zNear,const float zFar,Matrix4x4*out);//create right-handed perspective projection matrix 

UTIL_API Matrix4x4* Matrix4Translate(float x,float y,float z,Matrix4x4* out);//translation matrix
UTIL_API Matrix4x4* Matrix4RotateX(float angle,Matrix4x4* out);//rotation matrix - counter clockwise around Ox
UTIL_API Matrix4x4* Matrix4RotateY(float angle,Matrix4x4* out);//rotation matrix - counter clockwise around Oy
UTIL_API Matrix4x4* Matrix4RotateZ(float angle,Matrix4x4* out);//rotation matrix - counter clockwise around Oz

/*-----------------------------package handling class-------------------------*/
#include <list>
#include <string>

/*---------------------------------------------------------------------
this class will handle tasks such as packing & unpacking package file

This class is abstract because some problems with compiler warnings 
about template class can't be the base class of dll-exported class
--------------------------------------------------------------------*/
class PackageManager
{
public:
	PackageManager(){}
	virtual ~PackageManager(){};
	
	virtual void Clear()=0;//clear all buffer data that used for storing byte stream of unpacked package file
	virtual void ClearBuffer(unsigned int bufferID)=0;//clear buffer <bufferID> 
	/*------------------------------------------------------------------------
	this function handles packing multiple files into a package file
		<listFiles> - a list of files that will be packed in a package
		<fileName> - name of package file
	-------------------------------------------------------------------*/
	virtual bool Pack(std::list<std::string> &listFiles,const char *fileName)=0;

	/*-----------------------------------------------------------------------
	this method unpacks a package file <fileName> ,create new buffer ,stores byte stream and 
	its size in that buffer.
	<pBufferID> - points to ID object that stores ID of newly created buffer data that holds byte stream loaded from
	package file
	----------------------------------------------------------------------*/
	virtual bool UnPack(const char *fileName,unsigned int *pBufferID)=0;
	
	/*-----------------------------------------------------------------------
	this method unpacks a package file <fileName> ,stores byte stream and 
	its size in memmory block pointed by <buffer>.
	Note: you need to manually free memory pointed by <buffer> by using operator delete[]
	----------------------------------------------------------------------*/
	virtual bool UnPackToBuffer(const char *fileName,unsigned char *&buffer,unsigned int &bufferSize)=0;

	virtual const unsigned char *GetByteStream(unsigned int bufferID)=0; //get byte stream of buffer <bufferID> that previously loaded from package file
	virtual unsigned int GetBufferSize(unsigned int bufferID)=0; //get size of byte buffer <bufferID> that previously loaded data from package file
	
	virtual unsigned char *GetSubByteStreamFromBuffer(const unsigned char* buffer,unsigned int bufferSize,
											  unsigned int elementIndex,unsigned int *pElemSize)=0;
	virtual unsigned char *GetSubByteStreamFromBuffer(const unsigned char* buffer,unsigned int bufferSize,
											  const char* elementName,unsigned int *pElemSize)=0;

	virtual unsigned char *GetSubByteStream(unsigned int bufferID,unsigned int elementIndex,unsigned int *pElemSize)=0;//get byte stream of a file that packed in buffer <bufferID>,<elementIndex> is index denoting order of this file in package,<pElemSize> will hold the size of this file's byte stream
	virtual unsigned char *GetSubByteStream(unsigned int bufferID,const char* elementName,unsigned int *pElemSize)=0;//get byte stream of a file that packed in buffer <bufferID>,<elementName> is name of this file ,<pElemSize> will hold the size of this file's byte stream
	//extract byte stream of file <fileName> from a package file and store it in memory block pointed by <byteStream> , also the byte stream's size will be stored in <streamSize>.
	//Note:you need to free memory pointed by <byteStream> after use it by using operator delete[]
	virtual bool ExtractByteStreamFromPackage(const char *packageFileName,const char *fileName,unsigned char *&byteStream,unsigned int& streamSize)=0;
};

UTIL_API PackageManager *CreatePackageManager();

/*---------string stream handler - handles some tasks on C string but not modifies it-----------*/
#define END_OF_STREAM 0
#define OUT_OF_CHAR -1

class UTIL_API CStringStream
{
private:
	char* str;//C string associated with this stream,read only
	unsigned int curPos;//current indicator associated with this stream
	unsigned int terminatePos;//terminated characrer position
public:
	CStringStream();
	CStringStream(const char* str);
	CStringStream(const char* str,unsigned int terminatePos);//create stream associated with a string,<terminatePos> is the position of terminated character in string stream
	
	void Set(const char *str);//set another string associated with stream
	void Set(const char* str,unsigned int terminatePos);//set another string associated with stream
	
	bool isAtEnd() {return (curPos == terminatePos);};//

	int GetLine(char *lineOut,unsigned int maxChar);//<maxChar> includes null terminated char
	void Rewind();//reset indicator to the beginning of the stream
};
#endif