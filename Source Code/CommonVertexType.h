#ifndef _COMMON_VTYPE_
#define _COMMON_VTYPE_

/*-------common vertex types----------*/
enum VertexType
{
	VT_P3,//only has position (x,y,z) information
	VT_P3_T2,//has position (x,y,z) & texcoord (u,v) informations in that order in struct
	VT_P3_T2_N3//has position (x,y,z) & texcoord (u,v) & normal vector(nx,ny,nz)  informations in that order in struct
};

struct VertexP3
{
	float x,y,z;//position
	const static VertexType vertexType = VT_P3;
};

struct VertexP3T2
{
	float x,y,z;//position
	float u,v;//texcoord
	const static VertexType vertexType = VT_P3_T2;
};

struct VertexP3T2N3
{
	float x,y,z;//position
	float u,v;//texcoord
	float nx,ny,nz;//normal vector
	const static VertexType vertexType = VT_P3_T2_N3;
};

#endif