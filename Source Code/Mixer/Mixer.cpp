// Mixer.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "MixerImp.h"
#include "../SharedPointer.h"
Mixer *CreateMixer(unsigned int frequency,unsigned int channels, unsigned int bufferSize)
{
	MixerImp * mixer = new MixerImp(frequency,channels,bufferSize);
	if(!mixer->GetStatus())
	{
		mixer->Release();
		return NULL;
	}
	return mixer;
}

Mixer *CreateDefaultMixer()
{
	MixerImp * mixer = new MixerImp(22050,2,4096);
	if(!mixer->GetStatus())
	{
		mixer->Release();
		return NULL;
	}
	return mixer;
}

/*-------MixerImp---------------*/
MixerImp::MixerImp(unsigned int frequency,unsigned int channels, unsigned int bufferSize)
{
	if(Mix_OpenAudio(frequency, MIX_DEFAULT_FORMAT, channels, bufferSize)== -1)
		status = false;
	else
		status = true;
}
MixerImp::~MixerImp()
{
	Mix_CloseAudio();
}
int MixerImp::Release()
{
	delete this;
	return M_OK;
}
int MixerImp::LoadAudioFromFile(const char *fileName,unsigned int &audioID)
{
	Mix_Chunk* chunk = Mix_LoadWAV(fileName);
	if(chunk == NULL)
		return M_FAILED;
	Audio * audio = new Audio(chunk);
	this->AddItem(audio,&audioID);
	return M_OK;
}
int MixerImp::LoadAudioFromMemory(const unsigned char *byteStream,unsigned streamSize,unsigned int &audioID)
{
	SDL_RWops* ops = SDL_RWFromConstMem(byteStream,streamSize);
	Mix_Chunk* chunk=Mix_LoadWAV_RW(ops,1);

	if(chunk == NULL)
		return M_FAILED;
	Audio * audio = new Audio(chunk);
	this->AddItem(audio,&audioID);
	return M_OK;
}

int MixerImp::ReleaseAudio(unsigned int audioID)
{
	this->ReleaseSlot(audioID);

	return M_OK;
}
int MixerImp::Play(unsigned int audioID,int loop)
{
	SharedPtr<Audio> ptr = this->GetItemPointer(audioID);
	if(ptr == NULL)
		return -1;
	return Mix_PlayChannel(-1,ptr->soundChunk,loop - 1);
}
int MixerImp::Stop(unsigned int channel)
{
	Mix_HaltChannel(channel);
	return M_OK;
}
int MixerImp::Pause(unsigned int channel)
{
	Mix_Pause(channel);
	return M_OK;
}
int MixerImp::Continue(unsigned int channel)
{
	Mix_Resume(channel);
	return M_OK;
}
int MixerImp::SetVolume(unsigned int channel,unsigned int vol)
{
	Mix_Volume(channel,vol);
	return M_OK;
}
int MixerImp::StopAll()
{
	Mix_HaltChannel(-1);
	return M_OK;
}
int MixerImp::PauseAll()
{
	Mix_Pause(-1);
	return M_OK;
}
int MixerImp::ContinueAll()
{
	Mix_Resume(-1);
	return M_OK;
}
int MixerImp::SetVolumeAll(unsigned int vol)
{
	Mix_Volume(-1,vol);
	return M_OK;
}
