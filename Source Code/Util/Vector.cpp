#include "stdafx.h"
#include "../Util.h"

/*----------Vector2------------------*/
Vector2::Vector2()
{
	memset(this,0,sizeof(Vector2));
}

Vector2::Vector2(float x, float y)
{
	Set(x,y);
}

void Vector2::Set(float x, float y)
{
	this->x=x;
	this->y=y;
}

//**********************************************
//multiply vector by a float value
//**********************************************
Vector2 operator*(const float f,const Vector2& v){
	return Vector2(f*v.x,f*v.y);
}
//**********************************************
//negation
//**********************************************
Vector2 Vector2::operator -() const{
	return Vector2(-x,-y);
}

//**********************************************
//multiply vector by a float value
//**********************************************
Vector2& Vector2::operator *=(const float f){
	x*=f;y*=f;
	return *this;
}

Vector2 Vector2::operator *(const float f)const{
	return Vector2(x*f,y*f);
}
//**********************************************
//divide vector by a float value
//**********************************************
Vector2& Vector2::operator /=(const float f){
	x/=f;y/=f;
	return *this;
}
Vector2 Vector2::operator /(const float f)const{
	return Vector2(x/f,y/f);
}
//**********************************************
//sum of 2 vectors
//**********************************************
Vector2& Vector2::operator +=(const Vector2 &v2){
	x+=v2.x;y+=v2.y;
	return *this;
}
Vector2 Vector2::operator +(const Vector2& v2)const{
	return Vector2(x+v2.x,y+v2.y);
}
//**********************************************
//substract operation
//**********************************************
Vector2& Vector2::operator -=(const Vector2 &v2){
	x-=v2.x;y-=v2.y;
	return *this;
}
Vector2 Vector2::operator -(const Vector2& v2)const{
	return Vector2(x-v2.x,y-v2.y);
}
//**********************************************
//dot product
//**********************************************
float Vector2::operator *(const Vector2 &v2) const{
	return x*v2.x+y*v2.y;
}
//**********************************************
//square of length
//**********************************************
float Vector2::LengthSqr()const{
	return x*x+y*y;
}

//**********************************************
//length of vector
//**********************************************
float Vector2::Length(){
	return sqrtf(x*x+y*y);
}

//**********************************************
//normalize
//**********************************************
Vector2& Vector2::Normalize(){
	float rsqrt=1.0f/sqrtf(x*x+y*y); // 1/length
	(*this)*=rsqrt;//multiply by 1/length
	return *this;//return reference to this
}

/*----------Vector3------------------*/

Vector3::Vector3()
{
	memset(this,0,sizeof(Vector3));
}

Vector3::Vector3(float x, float y, float z)
{
	Set(x,y,z);
}

Vector3::Vector3(const Vector2&v, float z)
{
	Set(v,z);
}

void Vector3::Set(float x, float y, float z)
{
	this->x=x;
	this->y=y;
	this->z=z;
}

void Vector3::Set(const Vector2 &v, float z)
{
	this->x=v.x;
	this->y=v.y;
	this->z=z;
}
//**********************************************
//multiply vector by a float value
//**********************************************
Vector3 operator*(const float f,const Vector3& v){
	return Vector3(f*v.x,f*v.y,f*v.z);
}
//**********************************************
//negation
//**********************************************
Vector3 Vector3::operator -() const{
	return Vector3(-x,-y,-z);
}

//**********************************************
//multiply vector by a float value
//**********************************************
Vector3& Vector3::operator *=(const float f){
	x*=f;y*=f;z*=f;
	return *this;
}

Vector3 Vector3::operator *(const float f)const{
	return Vector3(x*f,y*f,z*f);
}
//**********************************************
//divide vector by a float value
//**********************************************
Vector3& Vector3::operator /=(const float f){
	x/=f;y/=f;z/=f;
	return *this;
}
Vector3 Vector3::operator /(const float f)const{
	return Vector3(x/f,y/f,z/f);
}
//**********************************************
//sum of 2 vectors
//**********************************************
Vector3& Vector3::operator +=(const Vector3 &v2){
	x+=v2.x;y+=v2.y;z+=v2.z;
	return *this;
}
Vector3 Vector3::operator +(const Vector3& v2)const{
	return Vector3(x+v2.x,y+v2.y,z+v2.z);
}
//**********************************************
//substract operation
//**********************************************
Vector3& Vector3::operator -=(const Vector3 &v2){
	x-=v2.x;y-=v2.y;z-=v2.z;
	return *this;
}
Vector3 Vector3::operator -(const Vector3& v2)const{
	return Vector3(x-v2.x,y-v2.y,z-v2.z);
}
//**********************************************
//dot product
//**********************************************
float Vector3::operator *(const Vector3 &v2) const{
	return x*v2.x+y*v2.y+z*v2.z;
}
//**********************************************
//square of length
//**********************************************
float Vector3::LengthSqr()const{
	return x*x+y*y+z*z;
}

//**********************************************
//length of vector
//**********************************************
float Vector3::Length(){
	return sqrtf(x*x+y*y+z*z);
}
//**********************************************
//cross product
//**********************************************
Vector3 Vector3::Cross(const Vector3& v2)const{
	Vector3 result;
	result.x= y * v2.z - z * v2.y;
	result.y= z * v2.x - x * v2.z;
	result.z= x * v2.y - y * v2.x;

	return result;
}
Vector3& Vector3::Cross(const Vector3 &v1, const Vector3 &v2){
	x= v1.y * v2.z - v1.z * v2.y;
	y= v1.z * v2.x - v1.x * v2.z;
	z= v1.x * v2.y - v1.y * v2.x;
	return *this;
}

//**********************************************
//normalize
//**********************************************
Vector3& Vector3::Normalize(){
	float rsqrt=1.0f/sqrtf(x*x+y*y+z*z); // 1/length
	(*this)*=rsqrt;//multiply by 1/length
	return *this;//return reference to this
}

/*----------Vector4------------------*/
Vector4::Vector4()
{
	memset(this,0,sizeof(Vector4));
}

Vector4::Vector4(float x, float y, float z)
{
	Set(x,y,z);
}

Vector4::Vector4(float x, float y, float z, float w)
{
	Set(x,y,z,w);
}

Vector4::Vector4(const Vector3& v, float w)
{
	Set(v,w);
}

void Vector4::Set(float x, float y, float z)
{
	Set(x,y,z,0.0f);
}

void Vector4::Set(float x, float y, float z, float w)
{
	this->x=x;
	this->y=y;
	this->z=z;
	this->w=w;
}

void Vector4::Set(const Vector3& v, float w)
{
	this->x=v.x;
	this->y=v.y;
	this->z=v.z;
	this->w=w;
}

//**********************************************
//multiply vector by a float value
//**********************************************
Vector4 operator*(const float f,const Vector4& v){
	return Vector4(f*v.x,f*v.y,f*v.z,v.w);
}
//**********************************************
//negation
//**********************************************
Vector4 Vector4::operator -() const{
	return Vector4(-x,-y,-z,w);
}

//**********************************************
//multiply vector by a float value
//**********************************************
Vector4& Vector4::operator *=(const float f){
	x*=f;y*=f;z*=f;
	return *this;
}

Vector4 Vector4::operator *(const float f)const{
	return Vector4(x*f,y*f,z*f,w);
}
//**********************************************
//divide vector by a float value
//**********************************************
Vector4& Vector4::operator /=(const float f){
	x/=f;y/=f;z/=f;
	return *this;
}
Vector4 Vector4::operator /(const float f)const{
	return Vector4(x/f,y/f,z/f,w);
}
//**********************************************
//sum of 2 vectors
//**********************************************
Vector4& Vector4::operator +=(const Vector4 &v2){
	x+=v2.x;y+=v2.y;z+=v2.z;
	return *this;
}
Vector4 Vector4::operator +(const Vector4& v2)const{
	return Vector4(x+v2.x,y+v2.y,z+v2.z,w);
}
Vector4& Vector4::operator +=(const Vector3 &v2){
	x+=v2.x;y+=v2.y;z+=v2.z;
	return *this;
}
Vector4 Vector4::operator +(const Vector3& v2)const{
	return Vector4(x+v2.x,y+v2.y,z+v2.z,w);
}
//**********************************************
//substract operation
//**********************************************
Vector4& Vector4::operator -=(const Vector4 &v2){
	x-=v2.x;y-=v2.y;z-=v2.z;
	return *this;
}
Vector4 Vector4::operator -(const Vector4& v2)const{
	return Vector4(x-v2.x,y-v2.y,z-v2.z,w);
}
Vector4& Vector4::operator -=(const Vector3 &v2){
	x-=v2.x;y-=v2.y;z-=v2.z;
	return *this;
}
Vector4 Vector4::operator -(const Vector3& v2)const{
	return Vector4(x-v2.x,y-v2.y,z-v2.z,w);
}
//**********************************************
//dot product
//**********************************************
float Vector4::operator *(const Vector4 &v2) const{
	return x*v2.x+y*v2.y+z*v2.z;
}
//**********************************************
//square of length
//**********************************************
float Vector4::LengthSqr()const{
	return x*x+y*y+z*z;
}

//**********************************************
//length of vector
//**********************************************
float Vector4::Length(){
	return sqrtf(x*x+y*y+z*z);
}
//**********************************************
//cross product
//**********************************************
Vector4 Vector4::Cross(const Vector4& v2)const{
	Vector4 result;
	result.x= y * v2.z - z * v2.y;
	result.y= z * v2.x - x * v2.z;
	result.z= x * v2.y - y * v2.x;
	result.w=0;

	return result;
}
Vector4& Vector4::Cross(const Vector4 &v1, const Vector4 &v2){
	x= v1.y * v2.z - v1.z * v2.y;
	y= v1.z * v2.x - v1.x * v2.z;
	z= v1.x * v2.y - v1.y * v2.x;
	w=0;
	return *this;
}

//**********************************************
//normalize
//**********************************************
Vector4& Vector4::Normalize(){
	float rsqrt=1.0f/sqrtf(x*x+y*y+z*z); // 1/length
	(*this)*=rsqrt;//multiply by 1/length
	return *this;//return reference to this
}

//**********************************************************
//multiply vector with a matrix
//**********************************************************
Vector4& Vector4::operator *=(const Matrix4x4 &m){
	float X = x * m._11 + y * m._21 + z * m._31 + w * m._41;
	float Y = x * m._12 + y * m._22 + z * m._32 + w * m._42;
	z = x * m._13 + y * m._23 + z * m._33 + w * m._43;
	Set(X,Y,z);
	
	return *this;
}

Vector4 Vector4::operator *(const Matrix4x4 &m)const{
	Vector4 result;
	result.x = x * m._11 + y * m._21 + z * m._31 + w * m._41;
	result.y = x * m._12 + y * m._22 + z * m._32 + w * m._42;
	result.z = x * m._13 + y * m._23 + z * m._33 + w * m._43;
	return result;
}