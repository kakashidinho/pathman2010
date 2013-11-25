#ifndef _MIXER_
#define _MIXER_
#define M_OK 1
#define M_FAILED 0
#define M_MAX_VOL 128

class Mixer
{
protected:
	virtual ~Mixer() {}
public:
	Mixer(){}
	virtual int Release()= 0;
	virtual int LoadAudioFromFile(const char *fileName,unsigned int &audioID) = 0;//load audio file and created new audio object.<audioID> will hold id of newly create audio object.That id will then be used for playing audio
	virtual int LoadAudioFromMemory(const unsigned char *byteStream,unsigned streamSize,unsigned int &audioID)=0;
	virtual int ReleaseAudio(unsigned int audioID)=0;
	virtual int Play(unsigned int audioID,int loop)=0;//if <loop> = 0 .audio will play in infinite loops.Return value is the index of channel in which audio is playing.If returned value is -1,this operation has failed
	virtual int Stop(unsigned int channel)=0;
	virtual int Pause(unsigned int channel)=0;
	virtual int Continue(unsigned int channel)=0;
	virtual int SetVolume(unsigned int channel,unsigned int vol)=0;//vol is between 0 and M_MAX_VOL
	virtual int StopAll()=0;
	virtual int PauseAll()=0;
	virtual int ContinueAll()=0;
	virtual int SetVolumeAll(unsigned int vol)=0;//vol is between 0 and M_MAX_VOL
};

extern "C" __declspec(dllexport) Mixer *CreateMixer(unsigned int frequency ,unsigned int channels , unsigned int bufferSize );
extern "C" __declspec(dllexport) Mixer *CreateDefaultMixer();//equals to CreateMixer(22050,2,4096)
typedef Mixer* (*pCreateMixerProc)(unsigned int,unsigned int,unsigned int);
typedef Mixer* (*pCreateDefaultMixerProc)();
#endif