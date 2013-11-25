#include "stdafx.h"
#include "../Util.h"
Matrix4x4::Matrix4x4()
{
	Identity();
}
Matrix4x4::Matrix4x4(float _11,float _12,float _13,float _14,
			  float _21,float _22,float _23,float _24,
			  float _31,float _32,float _33,float _34,
			  float _41,float _42,float _43,float _44)
{
	Set(_11,_12,_13,_14,_21,_22,_23,_24,_31,_32,_33,_34,_41,_42,_43,_44);
}
void Matrix4x4::Set(float _11, float _12, float _13, float _14, 
					float _21, float _22, float _23, float _24, 
					float _31, float _32, float _33, float _34, 
					float _41, float _42, float _43, float _44)
{
	this->_11=_11;this->_12=_12;this->_13=_13;this->_14=_14;
	this->_21=_21;this->_22=_22;this->_23=_23;this->_24=_24;
	this->_31=_31;this->_32=_32;this->_33=_33;this->_34=_34;
	this->_41=_41;this->_42=_42;this->_43=_43;this->_44=_44;
}

void Matrix4x4::Identity()
{
	Set(1.0f,0.0f,0.0f,0.0f,
		0.0f,1.0f,0.0f,0.0f,
		0.0f,0.0f,1.0f,0.0f,
		0.0f,0.0f,0.0f,1.0f);
}

Matrix4x4& Matrix4x4::operator *=(const Matrix4x4 &m){

	Matrix4x4 result;

	for (unsigned int i=0;i<4;++i)
	{
		for(unsigned int j=0;j<4;++j)
		{
			result(i,j)=0.0f;
			for(unsigned int k=0;k<4;++k)
			{
				result(i,j)+= mt[i][k] * m.mt[k][j];
			}
		}
	}

	*this=result;

	return *this;
}


Matrix4x4 Matrix4x4::operator *(const Matrix4x4 &m) const{
	Matrix4x4 result;

	for (unsigned int i=0;i<4;++i)
	{
		for(unsigned int j=0;j<4;++j)
		{
			result(i,j)=0.0f;
			for(unsigned int k=0;k<4;++k)
			{
				result(i,j)+= mt[i][k] * m.mt[k][j];
			}
		}
	}
	return result;
}



/*--------------------------------------------------------*/


Matrix4x4* Matrix4View(const Vector4* pX,const Vector4* pY,const Vector4* pZ,const Vector4* pPos,Matrix4x4 *out)
{
	out->_44=1.0f;
	out->_11=pX->x;
	out->_22=pY->y;
	out->_33=pZ->z;
	out->_12=pY->x;
	out->_13=pZ->x;
	out->_14=0.0f;

	out->_21=pX->y;
	out->_23=pZ->y;
	out->_24=0.0f;

	out->_31=pX->z;
	out->_32=pY->z;
	out->_34=0.0f;

	out->_41=-((*pX)*(*pPos));
	out->_42=-((*pY)*(*pPos));
	out->_43=-((*pZ)*(*pPos));
	return out;
}

Matrix4x4* Matrix4LookAtRH(const Vector4 *pEye,const Vector4* pAt,const Vector4* pUp,Matrix4x4*out)
{
	Vector4 Xaxis,Yaxis,Zaxis;
	Zaxis=(*pEye)-(*pAt);

	Zaxis.Normalize();

	Xaxis.Cross(*pUp,Zaxis);//Xaxis = up cross Zaxis
	Xaxis.Normalize();

	Yaxis.Cross(Zaxis,Xaxis);//Yaxis = Zaxis cross Zaxis

	return Matrix4View(&Xaxis,&Yaxis,&Zaxis,pEye,out);
}


Matrix4x4* Matrix4OrthoProjRH(const float width,const float height,const float zNear,const float zFar,Matrix4x4*out)
{
	memset(out->m,0,sizeof(Matrix4x4));
	out->_44=1.0f;
	out->_11=2.0f/width;
	out->_22=2.0f/height;

	float f=1.0f/(zNear-zFar);
	out->_33=2.0f*f;
	out->_43=(zNear+zFar)*f;

	return out;
}

Matrix4x4* Matrix4PerspectiveProjRH(const float vFOV,const float aspect,const float zNear,const float zFar,Matrix4x4*out)
{
	memset(out->m,0,sizeof(Matrix4x4));
	float sin,cos;
	float fov_over_2=vFOV/2.0f;
	sin=sinf(fov_over_2);
	cos=cosf(fov_over_2);
	
	out->_22=cos/sin;
	out->_11=out->_22/aspect;
	out->_34=-1.0f;

	float f=1.0f/(zNear-zFar);
	out->_33=(zNear+zFar)*f;
	out->_43=2*zNear*zFar*f;
	return out;
}

Matrix4x4* Matrix4RotateX(float angle, Matrix4x4 *out){
	float fCos,fSin;
	fCos=cosf(angle);
	fSin=sinf(angle);
	out->_11=out->_44=1.0f;
	out->_12=out->_13=out->_14=out->_21=out->_24
		=out->_31=out->_34=out->_41=out->_42=out->_43=0.0f;
	out->_22=fCos;
	out->_23=fSin;
	out->_32=-fSin;
	out->_33=fCos;
	return out;
}

Matrix4x4* Matrix4RotateY(float angle, Matrix4x4 *out ){
	float fCos,fSin;
	fCos=cosf(angle);
	fSin=sinf(angle);
	out->_22=out->_44=1.0f;
	out->_12=out->_14=out->_21=out->_23
		=out->_24=out->_32=out->_34=out->_41=out->_42=out->_43=0.0f;
	out->_11=fCos;
	out->_13=-fSin;
	out->_31=fSin;
	out->_33=fCos;
	return out;
}

Matrix4x4* Matrix4RotateZ(float angle, Matrix4x4 *out){
	float fCos,fSin;
	fCos=cosf(angle);
	fSin=sinf(angle);
	out->_33=out->_44=1.0f;
	out->_13=out->_14=out->_23=out->_24=out->_31
		=out->_32=out->_34=out->_41=out->_42=out->_43=0.0f;
	out->_11=fCos;
	out->_12=fSin;
	out->_21=-fSin;
	out->_22=fCos;
	return out;
}

Matrix4x4* Matrix4Translate(float x, float y, float z, Matrix4x4 *out){
	out->_44=out->_11=out->_22=out->_33=1.0f;
	out->_12=out->_13=out->_21=out->_23=out->_31=out->_32=0.0f;
	out->_41=x;out->_42=y;out->_43=z;
	out->_14=out->_24=out->_34=0.0f;
	return out;
}