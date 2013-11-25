#ifndef _MIXER_IMP_
#define _MIXER_IMP_

#include "../Mixer.h"
#include "../ItemManager.h"
#include <SDL.h>
#include <SDL_mixer.h>
#pragma comment(lib,"SDL.lib")
#pragma comment(lib,"SDLmain.lib")
#pragma comment(lib,"SDL_mixer.lib")

/*----------------class MixerImp-implements Mixer----------*/
struct Audio
{
	Mix_Chunk* soundChunk;
	Audio(Mix_Chunk* chunk){soundChunk = chunk;}
	~Audio() {Mix_FreeChunk(soundChunk);}
};

class MixerImp:public Mixer,public ItemManager<Audio>
{
protected:
	bool status;
	~MixerImp();
public:
	MixerImp(unsigned int frequency,unsigned int channels, unsigned int bufferSize);
	int Release();
	int LoadAudioFromFile(const char *fileName,unsigned int &audioID);//load audio file and created new audio object.<audioID> will hold id of newly create audio object.That id will then be used for playing audio
	int LoadAudioFromMemory(const unsigned char *byteStream,unsigned streamSize,unsigned int &audioID);//load audio file and created new audio object.<audioID> will hold id of newly create audio object.That id will then be used for playing audio
	int ReleaseAudio(unsigned int audioID);
	int Play(unsigned int audioID,int loop);//if <loop> = 0 .audio will play in infinite loops.Return value is the channel in which audio is playing.If returned value is -1,this operation has failed
	int Stop(unsigned int channel);
	int Pause(unsigned int channel);
	int Continue(unsigned int channel);
	int SetVolume(unsigned int channel,unsigned int vol);
	int StopAll();
	int PauseAll();
	int ContinueAll();
	int SetVolumeAll(unsigned int vol);//vol is between 0 and M_MAX_VOL
	bool GetStatus() {return status;}
};
#endif