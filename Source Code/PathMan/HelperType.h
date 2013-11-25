#ifndef _HELPER_TYPE_
#define _HELPER_TYPE_

struct TextureImageSrcInfo//contains info about which image's byte stream will be used to upload to texture
{
	unsigned int bufferID;//buffer contains byte stream of image
	char imageName[256];//name of image file packed in package
};



#endif