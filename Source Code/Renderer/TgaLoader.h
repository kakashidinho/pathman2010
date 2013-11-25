#ifndef _TGA_LOADER_
#define _TGA_LOADER_

#define IMG_OK 1
#define IMG_FAIL_NOT_SUPPORTED 2
#define IMG_FAIL_FILE_NOT_EXIST 3
#define IMG_FAIL_MEM_ALLOC 4
#define IMG_FAIL_BAD_FORMAT 5

/*-------------------------------------------------------------------------------------------
class TgaLoader - takes responsibility for loading and storing pixel data from tga image file
------------------------------------------------------------------------------------*/
class TgaLoader
{
private:
	unsigned int width,height;//width,height of image
	unsigned int pSize;//size of pixel data
	unsigned char *pData;//memory block that holds pixel data
	unsigned char *pTemp;//temporary memory block that holds byte stream loaded from image file
	unsigned short bpp;//bit per pixel
	
	int LoadRawData();//load uncompressed pixel data
	int LoadRLEData();//load run lendth encoded pixel data

	void BilinearFiler24(unsigned char* pPixel,float u,float v,const unsigned int lineSize);//calculate value of pixel <pPixel>,given its arbitrary coordinates (u,v) in 24 bit rgb source pixel data
	void BilinearFiler32(unsigned char* pPixel,float u,float v,const unsigned int lineSize);//calculate value of pixel <pPixel>,given its arbitrary coordinates (u,v) in 32 bit rgba source pixel data
	void BilinearFiler16AL(unsigned char* pPixel,float u,float v,const unsigned int lineSize);//calculate value of pixel <pPixel>,given its arbitrary coordinates (u,v) in 16 bit : 8 bit alpha 8 bit greyscale source pixel data
	void BilinearFiler8(unsigned char* pPixel,float u,float v,const unsigned int lineSize);//calculate value of pixel <pPixel>,given its arbitrary coordinates (u,v) in 8 bit greyscale or alpha source pixel data
public:
	TgaLoader();
	~TgaLoader();
	unsigned int GetWidth(){return width;};//get width value
	unsigned int GetHeight(){return height;};//get height value
	unsigned short GetBpp(){return bpp;};//get bits per pixel
	unsigned int GetSize(){return pSize;}//get size of pixel data
	const unsigned char *GetData(){return pData;}//get pixel data
	int LoadFromFile(const char* imgFile);//load pixel data from image file
	int LoadFromMemory(const unsigned char* byteStream,unsigned int streamSize);//load pixel data from byte stream
	
	void FlipRGB();//flip red and blue value
	int FlipVertical();
	int FlipHorizontal();
	int Scalef(float wFactor,float hFactor);//scale image with width's factor <wFactor> & height's factor <hFactor>
	int Scalei(unsigned int newWidth,unsigned int newHeight);//scale image to desired size : <newWidth> , <newHeight>
};
#endif