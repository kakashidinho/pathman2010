#include "stdafx.h"
#include "TgaLoader.h"
#include <math.h>

//constructor & destructor
TgaLoader::TgaLoader()
{
	this->height=this->width=this->pSize=0;
	this->bpp=0;
	this->pData=NULL;
	this->pTemp=NULL;
}
TgaLoader::~TgaLoader()
{
	SafeDeleteArray(pData);
	SafeDeleteArray(pTemp);
}


int TgaLoader::LoadFromFile(const char* imgFile)
{
	FILE *f;
	f=fopen(imgFile,"rb");//read file in binary form
	if (!f)
		return IMG_FAIL_FILE_NOT_EXIST;

	//Get file size in bytes
	unsigned int size=0;
	fseek(f,0L,SEEK_END);
	size=ftell(f);
	rewind(f);
	
	//release old temp data stream if existed
	if(pTemp!=NULL)
	{
		delete[] pTemp;
	}

	//allocate memory for temp data stream
	pTemp=new unsigned char[size];
	if(!pTemp)
	{
		fclose(f);
		return IMG_FAIL_MEM_ALLOC;
	}
	fread(pTemp,size,1,f);//store data from file into newly allocated memory block
	//don't need to use file anymore,so close it
	fclose(f);
	
	//check if this is tga file by checking file footer
	unsigned char footer[26];//file footer info  - 26 bytes
	memcpy(footer,pTemp+size-26,26);

	if(memcmp(&footer[8],"TRUEVISION-XFILE",16))//check bytes 9th to 24th of file footer , it must be the string "TRUEVISION-XFILE"
		return IMG_FAIL_BAD_FORMAT;//if not return false

	short x,y,w,h;

	if (pTemp[1]!=0)//we only support non - indexed color map
		return IMG_FAIL_NOT_SUPPORTED;

	unsigned char encode=pTemp[2];//encode type
	if(encode >11)
		return IMG_FAIL_NOT_SUPPORTED;//not supported encode type

	memcpy(&x,&pTemp[8],2);//X origin
	memcpy(&y,&pTemp[10],2);//Y origin
	memcpy(&w,&pTemp[12],2);//width
	memcpy(&h,&pTemp[14],2);//height
	
	if(w<1||h<1)
		return IMG_FAIL_BAD_FORMAT;

	if(pTemp[17]&0xc0)//7th bit and 8th bit of 18th bytes must not zero
		return IMG_FAIL_BAD_FORMAT;

	this->width=(unsigned int)w;
	this->height=(unsigned int)h;

	this->bpp=(short) pTemp[16];//bits per pixel
	this->pSize=(unsigned int)bpp/8*width*height;
	
	int result;
	switch(encode)//encode type
	{
	case 2:case 3://raw data 
		result= LoadRawData();
		break;
	case 10:case 11://RLE 
		result= LoadRLEData();
		break;
	default:
		return IMG_FAIL_NOT_SUPPORTED;
	}
	//check 5th and 6th bits of 18th byte for origin location information
	switch((pTemp[17]&0x30)>>4)
	{
	case 0://bottom left
		break;
	case 1://bottom right
		this->FlipHorizontal();
		break;
	case 2://top left
		this->FlipVertical();
		break;
	case 3://top right
		this->FlipHorizontal();
		this->FlipVertical();
		break;
	}
	return result;
}

int TgaLoader::LoadFromMemory(const unsigned char* byteStream,unsigned int size)
{
	if (!byteStream)
		return IMG_FAIL_FILE_NOT_EXIST;

	SafeDeleteArray(pTemp);
	
	pTemp=(unsigned char*)byteStream;//copy address of byte stream
	
	//check if this is tga file by checking file footer
	unsigned char footer[26];//file footer info  - 26 bytes
	memcpy(footer,pTemp+size-26,26);

	if(memcmp(&footer[8],"TRUEVISION-XFILE",16))//check bytes 9th to 24th of file footer , it must be the string "TRUEVISION-XFILE"
	{
		pTemp=NULL;
		return IMG_FAIL_BAD_FORMAT;//if not return false
	}

	short x,y,w,h;

	if (pTemp[1]!=0)//we only support non - indexed color map
	{
		pTemp=NULL;
		return IMG_FAIL_NOT_SUPPORTED;
	}

	unsigned char encode=pTemp[2];//encode type
	if(encode >11)
	{
		pTemp=NULL;
		return IMG_FAIL_NOT_SUPPORTED;//not supported encode type
	}

	memcpy(&x,&pTemp[8],2);//X origin
	memcpy(&y,&pTemp[10],2);//Y origin
	memcpy(&w,&pTemp[12],2);//width
	memcpy(&h,&pTemp[14],2);//height
	
	if(w<1||h<1)
	{
		pTemp=NULL;
		return IMG_FAIL_BAD_FORMAT;
	}

	if(pTemp[17]&0xc0)//7th bit and 8th bit of 18th bytes must not zero
	{
		pTemp=NULL;
		return IMG_FAIL_BAD_FORMAT;
	}

	this->width=(unsigned int)w;
	this->height=(unsigned int)h;

	this->bpp=(short) pTemp[16];//bits per pixel
	this->pSize=(unsigned int)bpp/8*width*height;
	
	int result=IMG_OK;
	switch(encode)//encode type
	{
	case 2:case 3://raw data 
		result= LoadRawData();
		break;
	case 10:case 11://RLE 
		result= LoadRLEData();
		break;
	default:
		pTemp=NULL;
		return IMG_FAIL_NOT_SUPPORTED;
	}
	//check 5th and 6th bits of 18th byte for origin location information
	switch((pTemp[17]&0x30)>>4)
	{
	case 0://bottom left
		break;
	case 1://bottom right
		this->FlipHorizontal();
		break;
	case 2://top left
		this->FlipVertical();
		break;
	case 3://top right
		this->FlipHorizontal();
		this->FlipVertical();
		break;
	}

	pTemp=NULL;//avoid mistake memmory release
	return result;
}

int TgaLoader::LoadRawData()
{
	//release existed pixel data
	if(pData!=NULL)
		delete[] pData;
	pData=new unsigned char[pSize];//alloc mem for new pixel data
	if(pData==NULL)
		return IMG_FAIL_MEM_ALLOC;

	int offset=(int)pTemp[0]+18;//offset to 1st byte of pixel data in byte stream,it's the sum of value of 1st bype and size of header of byte stream
	memcpy(pData,&pTemp[offset],pSize);//copy pSize bytes from byte stream
	return IMG_OK;
}
int TgaLoader::LoadRLEData()
{
	int pixelSize=(int)bpp/8;//size of 1 pixel (in bytes)
	unsigned char *cur;

	//release existed pixel data
	if(pData)
		delete[] pData;
	pData=new unsigned char[pSize];//alloc mem for pixel data
	if(pData==NULL)
		return IMG_FAIL_MEM_ALLOC;

	int offset=(int)pTemp[0]+18;//offset to 1st byte of pixel data in byte stream
	cur=(unsigned char*)pTemp+offset;//point to pixel data's 1st byte in byte stream
	
	unsigned int index=0;//index of byte in decompressed pixel data
	//decompress pixel data from byte stream <pTeamp> and store into <pData>
	while (index<pSize)//still in bound
	{
		if(*cur&0x80)//most significant bit is 1=>run length chunk
		{
			int length=((*cur)-127);//length that pixel will repeat
			cur++;//move to pixel value
			//copy pixel "length" times
			for(int j=0;j<length;++j)
			{
				memcpy(pData+index,cur,pixelSize);
				index+=pixelSize;
			}
			cur+=pixelSize;//next chunk
		}
		else {//raw chunk

			int num=*cur+1;//number of next pixels
			
			cur++;//move to pixel value
			//copy "num" next pixels directly
			for(int j=0;j<num;++j)
			{
				memcpy(pData+index,cur,pixelSize);
				index+=pixelSize;
				cur+=pixelSize;//next pixel
			}
		}
	}
	return IMG_OK;
}

void TgaLoader::FlipRGB()
{
	if(!pData || bpp!=24 && bpp!=32)
		return ;

	unsigned int pixelSize=bpp/8;//pixel size in byte

	unsigned char *cur=pData;//starting address of pixel data
	unsigned char* pEnd=pData+pSize;//ending address of pixel data

	for(;cur<pEnd;cur+=pixelSize)
	{
		//flip red and blue value (1st byte and 3th byte)
		cur[0]=cur[0] ^ cur[2];
		cur[2]=cur[0] ^ cur[2];
		cur[0]=cur[0] ^ cur[2];
	}
}

int TgaLoader::FlipVertical()
{
	if(!pData)
		return IMG_FAIL_FILE_NOT_EXIST;
	
	unsigned int w=width;
	unsigned int h=height;
	
	unsigned char *pLine1,*pLine2;
	unsigned int lineSize;

	lineSize=w*bpp/8;//size (bytes) of 1 pixel line in source image

	pLine1=pData;//first line
	pLine2=pData+lineSize*(h-1);//last line

	//flip 2 lines
	for(;pLine1<pLine2;pLine2-=2*lineSize)
	{
		for(unsigned int index=0;index<lineSize;++pLine1,++pLine2,++index)//flip each pixels in 2 lines
		{
			*pLine1 = *pLine1 ^ *pLine2;
			*pLine2 = *pLine1 ^ *pLine2;
			*pLine1 = *pLine1 ^ *pLine2;
		}
	}

	
	return IMG_OK;
}

int TgaLoader::FlipHorizontal()
{
	if(!pData)
		return IMG_FAIL_FILE_NOT_EXIST;

	unsigned char *pIterator1,*pIterator2,*pLine;
	unsigned char *temp;
	unsigned short pixelSize=bpp/8;
	temp=new unsigned char[pixelSize];

	unsigned int lineSize=width*pixelSize;//size (bytes) of 1 pixel line in source image
	unsigned char *pEnd=pData + lineSize*height;//ending address of source byte stream

	pLine=pData;//first line

	while(pLine<pEnd)
	{
		//flip left and right pixels in line
		pIterator1=pLine;//first pixel
		pIterator2=pLine+lineSize-pixelSize;//last pixel
		for (;pIterator1<pIterator2;pIterator1+=pixelSize,pIterator2-=pixelSize){
			memcpy(temp,pIterator1,pixelSize);
			memcpy(pIterator1,pIterator2,pixelSize);
			memcpy(pIterator2,temp,pixelSize);
		}
		pLine+=lineSize;//next line
	}
	delete[] temp;
	

	return IMG_OK;
}

int TgaLoader::Scalef(float wFactor, float hFactor)
{
	if(!pData)
		return IMG_FAIL_FILE_NOT_EXIST;

	if(wFactor==1.0f && hFactor==1.0f)//no change
		return IMG_OK;


	unsigned int newW=(unsigned int)(width*wFactor);
	unsigned int newH=(unsigned int)(height*hFactor);

	unsigned short pixelSize=bpp/8;
	unsigned int newSize=newW*newH*pixelSize;
	//bilinear filtering
	
	unsigned char * newData=new unsigned char[newSize];
	unsigned char * pPixel=newData;
	if(!newData)
		return IMG_FAIL_MEM_ALLOC;

	float wRatio=1.0f/wFactor;// old width / new width
	float hRatio=1.0f/hFactor;// old height / new height
	
	unsigned int lineSize=pixelSize*width;//size (bytes) of 1 pixel line in source image
	
	for(unsigned int row=0;row<newH;++row)
		for(unsigned int col=0;col<newW;++col)
		{
			switch(bpp)
			{
			case 24:
				this->BilinearFiler24(pPixel,col*wRatio,row*hRatio,lineSize);
				break;
			case 32:
				this->BilinearFiler32(pPixel,col*wRatio,row*hRatio,lineSize);
				break;
			case 8:
				this->BilinearFiler8(pPixel,col*wRatio,row*hRatio,lineSize);
				break;
			case 16:
				this->BilinearFiler16AL(pPixel,col*wRatio,row*hRatio,lineSize);
				break;
			default:
				delete[] newData;
				return IMG_FAIL_NOT_SUPPORTED;
			}
			pPixel+=pixelSize;
		}
	
	delete[] pData;
	pData=newData;
	
	width=newW;
	height=newH;

	pSize=newSize;

	return IMG_OK;
}


int TgaLoader::Scalei(unsigned int newW, unsigned int newH)
{
	if(!pData)
		return IMG_FAIL_FILE_NOT_EXIST;
	if(newW==width && newH==height)//no change
		return IMG_OK;
	
	unsigned short pixelSize=bpp/8;
	unsigned int newSize=newW*newH*pixelSize;
	//bilinear filtering
	
	unsigned char * newData=new unsigned char[newSize];
	unsigned char * pPixel=newData;
	if(!newData)
		return IMG_FAIL_MEM_ALLOC;

	float wRatio=(float)width/newW;// old width / new width
	float hRatio=(float)height/newH;// old height / new height
	
	unsigned int lineSize=pixelSize*width;//size (bytes) of 1 pixel line in source image

	for(unsigned int row=0;row<newH;++row)
		for(unsigned int col=0;col<newW;++col)
		{
			switch(bpp)
			{
			case 24:
				this->BilinearFiler24(pPixel,col*wRatio,row*hRatio,lineSize);
				break;
			case 32:
				this->BilinearFiler32(pPixel,col*wRatio,row*hRatio,lineSize);
				break;
			case 8:
				this->BilinearFiler8(pPixel,col*wRatio,row*hRatio,lineSize);
				break;
			case 16:
				this->BilinearFiler16AL(pPixel,col*wRatio,row*hRatio,lineSize);
				break;
			default:
				delete[] newData;
				return IMG_FAIL_NOT_SUPPORTED;
			}
			pPixel+=pixelSize;
		}
	
	delete[] pData;
	pData=newData;

	width=newW;
	height=newH;

	pSize=newSize;

	return IMG_OK;
}

float Lerp(float a,float b,float t)
{
	return (1-t)*a+t*b;
}

void TgaLoader::BilinearFiler24(unsigned char *pPixel, float u, float v,const unsigned int lineSize)
{
	//----------------------
	//c0(x0,y0)------A------------ c1(x1,y0)
	//		|		 |				|
	//		|--------p(u,v)			|
	//		|		 |				|
	//		|		 |				|
	//c2(x0,y1)------B------------ c3(x1,y1)
	//----------------------
	unsigned int x0,y0,x1,y1;
	float t1,t2;
	unsigned char *c0,*c1,*c2,*c3;


	x0=(unsigned int)floorf(u);
	y0=(unsigned int)floorf(v);

	x1=x0+1;
	y1=y0+1;

	t1=(u-x0);
	t2=(v-y0);

	bool outOfWidth=(x1>=width);
	bool outOfHeight=(y1>=height);
	
	//pixel at (x0,y0)
	c0=pData+lineSize*y0+x0*3;
	//pixel at (x1,y0)
	if(!outOfWidth)
		c1=pData+lineSize*y0+x1*3;
	else c1=c0;
	//pixel at (x0,y1)
	if(!outOfHeight)
		c2=pData+lineSize*y1+x0*3;
	else c2=c0;
	//pixel at (x1,y1)
	if(outOfHeight)
		c3=c1;
	else if(outOfWidth)
		c3=c2;
	else
		c3=pData+lineSize*y1+x1*3;
	
	float A,B;
	//channel 1
	A=Lerp(c0[0]/255.0f,c1[0]/255.0f,t1);
	B=Lerp(c2[0]/255.0f,c3[0]/255.0f,t1);
	
	pPixel[0]=(unsigned char)(255*Lerp(A,B,t2));

	//channel 2
	A=Lerp(c0[1]/255.0f,c1[1]/255.0f,t1);
	B=Lerp(c2[1]/255.0f,c3[1]/255.0f,t1);
	
	pPixel[1]=(unsigned char)(255*Lerp(A,B,t2));

	//channel 3
	A=Lerp(c0[2]/255.0f,c1[2]/255.0f,t1);
	B=Lerp(c2[2]/255.0f,c3[2]/255.0f,t1);
	
	pPixel[2]=(unsigned char)(255*Lerp(A,B,t2));

}

void TgaLoader::BilinearFiler32(unsigned char *pPixel, float u, float v,const unsigned int lineSize)
{
	//----------------------
	//c0(x0,y0)------A------------ c1(x1,y0)
	//		|		 |				|
	//		|--------p(u,v)			|
	//		|		 |				|
	//		|		 |				|
	//c2(x0,y1)------B------------ c3(x1,y1)
	//----------------------
	unsigned int x0,y0,x1,y1;
	float t1,t2;
	unsigned char *c0,*c1,*c2,*c3;


	x0=(unsigned int)floorf(u);
	y0=(unsigned int)floorf(v);

	x1=x0+1;
	y1=y0+1;

	t1=(u-x0);
	t2=(v-y0);

	bool outOfWidth=(x1>=width);
	bool outOfHeight=(y1>=height);
	
	//pixel at (x0,y0)
	c0=pData+lineSize*y0+x0*4;
	//pixel at (x1,y0)
	if(!outOfWidth)
		c1=pData+lineSize*y0+x1*4;
	else c1=c0;
	//pixel at (x0,y1)
	if(!outOfHeight)
		c2=pData+lineSize*y1+x0*4;
	else c2=c0;
	//pixel at (x1,y1)
	if(outOfHeight)
		c3=c1;
	else if(outOfWidth)
		c3=c2;
	else
		c3=pData+lineSize*y1+x1*4;
	
	float A,B;
	//channel 1
	A=Lerp(c0[0]/255.0f,c1[0]/255.0f,t1);
	B=Lerp(c2[0]/255.0f,c3[0]/255.0f,t1);
	
	pPixel[0]=(unsigned char)(255*Lerp(A,B,t2));

	//channel 2
	A=Lerp(c0[1]/255.0f,c1[1]/255.0f,t1);
	B=Lerp(c2[1]/255.0f,c3[1]/255.0f,t1);
	
	pPixel[1]=(unsigned char)(255*Lerp(A,B,t2));

	//channel 3
	A=Lerp(c0[2]/255.0f,c1[2]/255.0f,t1);
	B=Lerp(c2[2]/255.0f,c3[2]/255.0f,t1);
	
	pPixel[2]=(unsigned char)(255*Lerp(A,B,t2));

	//channel 4
	A=Lerp(c0[3]/255.0f,c1[3]/255.0f,t1);
	B=Lerp(c2[3]/255.0f,c3[3]/255.0f,t1);
	
	pPixel[3]=(unsigned char)(255*Lerp(A,B,t2));

}


void TgaLoader::BilinearFiler16AL(unsigned char *pPixel, float u, float v,const unsigned int lineSize)
{
	//----------------------
	//c0(x0,y0)------A------------ c1(x1,y0)
	//		|		 |				|
	//		|--------p(u,v)			|
	//		|		 |				|
	//		|		 |				|
	//c2(x0,y1)------B------------ c3(x1,y1)
	//----------------------
	unsigned int x0,y0,x1,y1;
	float t1,t2;
	unsigned char *c0,*c1,*c2,*c3;


	x0=(unsigned int)floorf(u);
	y0=(unsigned int)floorf(v);

	x1=x0+1;
	y1=y0+1;

	t1=(u-x0);
	t2=(v-y0);

	bool outOfWidth=(x1>=width);
	bool outOfHeight=(y1>=height);
	
	//pixel at (x0,y0)
	c0=pData+lineSize*y0+x0*2;
	//pixel at (x1,y0)
	if(!outOfWidth)
		c1=pData+lineSize*y0+x1*2;
	else c1=c0;
	//pixel at (x0,y1)
	if(!outOfHeight)
		c2=pData+lineSize*y1+x0*2;
	else c2=c0;
	//pixel at (x1,y1)
	if(outOfHeight)
		c3=c1;
	else if(outOfWidth)
		c3=c2;
	else
		c3=pData+lineSize*y1+x1*2;
	
	float A,B;
	//channel 1
	A=Lerp(c0[0]/255.0f,c1[0]/255.0f,t1);
	B=Lerp(c2[0]/255.0f,c3[0]/255.0f,t1);
	
	pPixel[0]=(unsigned char)(255*Lerp(A,B,t2));

	//channel 2
	A=Lerp(c0[1]/255.0f,c1[1]/255.0f,t1);
	B=Lerp(c2[1]/255.0f,c3[1]/255.0f,t1);
	
	pPixel[1]=(unsigned char)(255*Lerp(A,B,t2));

}

void TgaLoader::BilinearFiler8(unsigned char *pPixel, float u, float v,const unsigned int lineSize)
{
	//----------------------
	//c0(x0,y0)------A------------ c1(x1,y0)
	//		|		 |				|
	//		|--------p(u,v)			|
	//		|		 |				|
	//		|		 |				|
	//c2(x0,y1)------B------------ c3(x1,y1)
	//----------------------
	unsigned int x0,y0,x1,y1;
	float t1,t2;
	unsigned char *c0,*c1,*c2,*c3;


	x0=(unsigned int)floorf(u);
	y0=(unsigned int)floorf(v);

	x1=x0+1;
	y1=y0+1;

	t1=(u-x0);
	t2=(v-y0);

	bool outOfWidth=(x1>=width);
	bool outOfHeight=(y1>=height);
	
	//pixel at (x0,y0)
	c0=pData+lineSize*y0+x0;
	//pixel at (x1,y0)
	if(!outOfWidth)
		c1=pData+lineSize*y0+x1;
	else c1=c0;
	//pixel at (x0,y1)
	if(!outOfHeight)
		c2=pData+lineSize*y1+x0;
	else c2=c0;
	//pixel at (x1,y1)
	if(outOfHeight)
		c3=c1;
	else if(outOfWidth)
		c3=c2;
	else
		c3=pData+lineSize*y1+x1;
	
	float A,B;
	//channel 1
	A=Lerp(c0[0]/255.0f,c1[0]/255.0f,t1);
	B=Lerp(c2[0]/255.0f,c3[0]/255.0f,t1);
	
	pPixel[0]=(unsigned char)(255*Lerp(A,B,t2));

}