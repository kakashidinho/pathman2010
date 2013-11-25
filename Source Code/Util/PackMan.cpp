#include "stdafx.h"
#include "../Util.h"
#include <stdio.h>

struct BufferData
{
	unsigned char *byteStream;//byte stream loaded from package file
	unsigned int bufferSize;//size of byte stream loaded from package file
	BufferData()
	{
		byteStream = 0;
		bufferSize = 0;
	};
	~BufferData()
	{
		SafeDeleteArray(byteStream);
	}
};

/*-------class PackManImp implements PackManImp interface------*/
class PackManImp:public PackageManager
{
private:
	ItemManager<BufferData> bufferManager;//manager of buffers that created for storing byte stream of package files
	bool readFile(std::string &fileName,unsigned int *pSize,unsigned char** pBytes);
	void Decrypt(unsigned char *&buffer,unsigned int &bufferSize);//for now,only use simple decrypt & encrypt method
	void Encrypt(unsigned char *&byteStream,unsigned int &streamSize);
public:
	PackManImp();
	~PackManImp();
	
	void Clear();//clear all buffer data
	void ClearBuffer(unsigned int bufferID);//clear buffer <bufferID> 
	/*------------------------------------------------------------------------
	this function handles packing multiple files in a package
		<listFiles> - a list of files that will be packed in a package
		<fileName> - name of package file
	-------------------------------------------------------------------*/
	bool Pack(std::list<std::string> &listFiles,const char *fileName);
	/*-----------------------------------------------------------------------
	this method unpacks a package file <fileName> ,stores byte stream and 
	its size in newly created BufferData object
	<pBufferID> - points to ID object of newly created buffer data that loaded from
	package file
	----------------------------------------------------------------------*/
	bool UnPack(const char *fileName,unsigned int *pBufferID);
	bool UnPackToBuffer(const char *fileName,unsigned char *&buffer,unsigned int &bufferSize);

	const unsigned char *GetByteStream(unsigned int bufferID); //get byte stream of buffer <bufferID> that previously loaded from package file
	unsigned int GetBufferSize(unsigned int bufferID); //get size of byte buffer <bufferID> that previously loaded data from package file
	
	unsigned char *GetSubByteStream(unsigned int bufferID,unsigned int elementIndex,unsigned int *pElemSize);//get byte stream of a file that packed in buffer <bufferID>,<elementIndex> is index denoting order of this file in package,<pElemSize> will hold the size of sub stream
	unsigned char *GetSubByteStream(unsigned int bufferID,const char* elementName,unsigned int *pElemSize);//get byte stream of a file that packed in buffer <bufferID>,<elementName> is name of this file ,<pElemSize> will hold the size of sub stream
	
	unsigned char *GetSubByteStreamFromBuffer(const unsigned char* buffer,unsigned int bufferSize,unsigned int elementIndex,unsigned int *pElemSize);
	unsigned char *GetSubByteStreamFromBuffer(const unsigned char* buffer,unsigned int bufferSize,const char* elementName,unsigned int *pElemSize);

	bool ExtractByteStreamFromPackage(const char *packageFileName,const char *fileName,unsigned char *&byteStream,unsigned int& streamSize);
};
/*---------create package manager object--------*/
PackageManager *CreatePackageManager()
{
	return new PackManImp();
}
/*----------------------------------------*/
PackManImp::PackManImp()
{
}

PackManImp::~PackManImp()
{
}

void PackManImp::Clear()
{
	bufferManager.ReleaseAllSlot();
}

void PackManImp::ClearBuffer(unsigned int bufferID)
{
	bufferManager.ReleaseSlot(bufferID);
}

const unsigned char *PackManImp::GetByteStream(unsigned int bufferID)
{
	SharedPtr<BufferData> ptr = bufferManager.GetItemPointer(bufferID);
	if(ptr == NULL)
		return NULL;
	return ptr->byteStream;
}

unsigned int PackManImp::GetBufferSize(unsigned int bufferID) 
{
	SharedPtr<BufferData> ptr = bufferManager.GetItemPointer(bufferID);
	if(ptr == NULL)
		return 0;
	return ptr->bufferSize;
}

bool PackManImp::readFile(std::string& fileName,unsigned int *pSize,unsigned char **ppBytes)
{
	if(pSize == NULL || ppBytes == NULL)
		return false;
	FILE *f;
	f=fopen(fileName.c_str(),"rb");
	if(!f)
		return false;
	
	//get size of file
	fseek(f,0L,SEEK_END);
	*pSize=ftell(f);
	rewind(f);

	unsigned char *pData=new unsigned char[*pSize];

	if(!pData)
	{
		fclose(f);
		return false;
	}

	fread(pData,*pSize,1,f);//copy file data
	
	fclose(f);
	
	*ppBytes=pData;

	return true;
}

bool PackManImp::Pack(std::list<std::string> &listFiles, const char *packageFileName)
{
	unsigned char *pData=NULL;
	unsigned int size;

	FILE * f;
	f=fopen(packageFileName,"wb");
	fprintf(f,"Q PACK ");//magic string
	std::list<std::string>::iterator ite;
	for (ite = listFiles.begin(); ite!=listFiles.end();++ite)
	{
		readFile(*ite,&size,&pData);
		if(size==0)
		{
			SafeDeleteArray(pData);
			continue;
		}
		if(!pData)
			continue;
		
		unsigned int pos = ite->find_last_of('\\');
		unsigned int nameLength=ite->length() - pos -1;//length of filename,not including file path
		char *fName = (char*)ite->c_str() + pos + 1;//file name ,not including file path
		fwrite(&size,sizeof(unsigned int),1,f);//copy size of this file to output package
		fwrite(&nameLength,sizeof(unsigned int),1,f);//length of filename
		fwrite(fName,nameLength,1,f);//name of file,not including file path
		this->Encrypt(pData,size);
		fwrite(pData,size,1,f);//write byte stream that read from file to output package

		SafeDeleteArray(pData);
	}
	fclose(f);
	return true;
}

bool PackManImp::UnPack(const char *fileName,unsigned int *pBufferID)
{
	FILE *f;
	f=fopen(fileName,"rb");
	if(!f)
		return false;
	unsigned char signature[7] ;//signature string
	fread(signature,7,1,f);
	
	//it must be "Q PACK "
	if (memcmp(signature , "Q PACK ",7))
	{
		fclose(f);
		return false;
	}
	

	//get size of file,excluding signature string
	fseek(f,0L,SEEK_END);
	unsigned bufferSize=ftell(f) - 7;
	rewind(f);
	fseek(f,7,SEEK_SET);
	


	unsigned char *byteStream=new unsigned char[bufferSize];

	if(!byteStream)//can't alloc new memory block
	{
		fclose(f);
		return false;
	}

	fread(byteStream,bufferSize,1,f);//copy file data

	fclose(f);
	
	this->Decrypt(byteStream,bufferSize);

	BufferData *buffer = new BufferData();
	buffer->byteStream = byteStream;
	buffer->bufferSize = bufferSize;

	if(!bufferManager.AddItem(buffer,pBufferID))
	{
		delete buffer;
		return false;
	}

	return true;
}


bool PackManImp::UnPackToBuffer(const char *fileName,unsigned char *&buffer,unsigned int &bufferSize)
{
	FILE *f;
	f=fopen(fileName,"rb");
	if(!f)
		return false;
	unsigned char signature[7] ;//signature string
	fread(signature,7,1,f);
	
	//it must be "Q PACK "
	if (memcmp(signature , "Q PACK ",7))
	{
		fclose(f);
		return false;
	}
	

	//get size of file,excluding signature string
	fseek(f,0L,SEEK_END);
	bufferSize=ftell(f) - 7;
	rewind(f);
	fseek(f,7,SEEK_SET);
	


	buffer=new unsigned char[bufferSize];

	if(!buffer)//can't alloc new memory block
	{
		fclose(f);
		return false;
	}

	fread(buffer,bufferSize,1,f);//copy file data

	this->Decrypt(buffer,bufferSize);

	fclose(f);

	return true;
}


unsigned char* PackManImp::GetSubByteStream(unsigned int bufferID,unsigned int index,unsigned int *pElemSize)
{
	SharedPtr<BufferData> ptr = bufferManager.GetItemPointer(bufferID);
	if(ptr == NULL || ptr->byteStream == NULL || ptr->bufferSize == 0)//failed
		return NULL;
	return this->GetSubByteStreamFromBuffer(ptr->byteStream,ptr->bufferSize,index,pElemSize);
}

unsigned char* PackManImp::GetSubByteStream(unsigned int bufferID,const char *name,unsigned int *pElemSize)
{
	SharedPtr<BufferData> ptr = bufferManager.GetItemPointer(bufferID);
	if(ptr == NULL || ptr->byteStream == NULL || ptr->bufferSize == 0)//failed
		return NULL;
	return this->GetSubByteStreamFromBuffer(ptr->byteStream,ptr->bufferSize,name,pElemSize);
}
unsigned char* PackManImp::GetSubByteStreamFromBuffer(const unsigned char* buffer,unsigned int bufferSize,unsigned int index,unsigned int *pElemSize)
{
	if(buffer == NULL)
		return NULL;
	unsigned char *pEnd = (unsigned char *)buffer + bufferSize;//ending address of buffer bytestream
	unsigned char *pCur = (unsigned char *)buffer;
	
	unsigned int curIndex=0;
	unsigned int elementSize;//size of file that packed in this package
	unsigned int nameSize;//size of name of file that packed in this package
	while(pCur != pEnd)
	{
		elementSize = *(unsigned int*)pCur;
		nameSize= *(unsigned int*)(pCur + sizeof(unsigned int));
	
		if(curIndex == index)//we found the file
		{
			if(pElemSize)
				*pElemSize = elementSize;
			return pCur + 2*sizeof(unsigned int) + nameSize;
		}

		curIndex++;
		pCur += 2*sizeof(unsigned int) + elementSize + nameSize;//move to next file in this package
	}
	//file not found in this package
	return NULL;
}

unsigned char* PackManImp::GetSubByteStreamFromBuffer(const unsigned char* buffer,unsigned int bufferSize,const char *name,unsigned int *pElemSize)
{
	if(buffer == NULL)
		return NULL;

	unsigned int fnameLength=strlen(name);//length of file's name we need to file

	unsigned char *pEnd = (unsigned char *)buffer + bufferSize;//ending address of buffer bytestream
	unsigned char *pCur = (unsigned char *)buffer;

	unsigned int elementSize;//size of file that packed in this package
	unsigned int nameSize;//size of name of file that packed in this package
	
	while(pCur != pEnd)
	{
		elementSize = *(unsigned int*)pCur;
		nameSize= *(unsigned int*)(pCur + sizeof(unsigned int));

		pCur += 2*sizeof(unsigned int);//move to file name chunk

		if(fnameLength==nameSize && !strncmp(name,(char*)pCur,nameSize))//we found the file
		{
			if(pElemSize)
				*pElemSize = elementSize;
			return pCur + nameSize;
		}
		pCur += elementSize + nameSize;//move to next file in this package
	}
	//file not found in this package
	return NULL;
}

bool PackManImp::ExtractByteStreamFromPackage(const char *packageFileName,const char *fileName,unsigned char *&byteStream,unsigned int& streamSize)
{
	FILE *f;
	f=fopen(packageFileName,"rb");
	if(!f)
		return false;
	unsigned char signature[7] ;//signature string
	fread(signature,7,1,f);
	
	//it must be "Q PACK "
	if (memcmp(signature , "Q PACK ",7))
	{
		fclose(f);
		return false;
	}
	

	//get size of file,excluding signature string
	fseek(f,0L,SEEK_END);
	unsigned bufferSize=ftell(f) - 7;
	rewind(f);
	fseek(f,7,SEEK_SET);
	


	unsigned char *byteStreamBuffer=new unsigned char[bufferSize];

	if(!byteStreamBuffer)//can't alloc new memory block
	{
		fclose(f);
		return false;
	}

	fread(byteStreamBuffer,bufferSize,1,f);//copy file data

	fclose(f);
	
	this->Decrypt(byteStreamBuffer,bufferSize);

	unsigned char *subByteStream = this->GetSubByteStreamFromBuffer(byteStreamBuffer,bufferSize,fileName,&streamSize);
	
	if(subByteStream == NULL)
	{
		byteStream = NULL;
		delete[] byteStreamBuffer;
		return false;
	}
	byteStream = new unsigned char[streamSize];
	memcpy(byteStream,subByteStream,streamSize);

	delete[] byteStreamBuffer;
	

	
	return true;
}

void PackManImp::Decrypt(unsigned char *&buffer,unsigned int &bufferSize)
{
	unsigned char *pEnd = buffer + bufferSize;//ending address of buffer bytestream
	unsigned char *pCur = buffer;

	unsigned char *decryptBytes = NULL;
	unsigned int elementSize;//size of file that packed in this package
	unsigned int nameSize;//size of name of file that packed in this package
	while(pCur != pEnd)
	{
		elementSize = *(unsigned int*)pCur;
		nameSize= *(unsigned int*)(pCur + sizeof(unsigned int));

		decryptBytes = pCur + 2*sizeof(unsigned int) + nameSize;

		for(unsigned int i =0;i < elementSize;++i)
			decryptBytes[i] = 255 - decryptBytes[i];
		
		pCur += 2*sizeof(unsigned int) + elementSize + nameSize;//move to next file in this package
	}
}
void PackManImp::Encrypt(unsigned char *&byteStream,unsigned int &streamSize)
{
	for(unsigned int i =0;i < streamSize;++i)
		byteStream[i] = 255 - byteStream[i];
}