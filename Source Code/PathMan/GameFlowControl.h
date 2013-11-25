#ifndef _FLOW_CTRL_
#define _FLOW_CTRL_

#include "../Framework.h"
#include "../Util.h"


#if defined (_DEBUG)||defined(DEBUG)
#pragma comment(lib,"../Debug/FrameWork.lib")
#pragma comment(lib,"../Debug/Util.lib")
#else
#pragma comment(lib,"../Release/FrameWork.lib")
#pragma comment(lib,"../Release/Util.lib")
#endif

struct Icon{
	unsigned int texID;//texture id used for display texture
	Rect rect;//area in image of icon
};

extern AppFrameWork* AppFW;
extern Renderer * pRenderer;//renderer
extern Mixer *pMixer;//sound mixer
extern SharedPtr<GUImanager> pGUI ;//GUI manager
extern SharedPtr<VariableManager> pVar;//variable manager
extern SharedPtr<MeshManager> pMeshMan ;//mesh manager
extern StopWatch timer;
extern PackageManager *packMan;//for handling package file
extern unsigned int textBufferID;//id of buffer that stored data from package file contains info about texts in game
extern unsigned int audioBufferID;//id of buffer that stored data from package file contains info about audio in game
extern unsigned int *font;//list of font objects' ID used in this game
extern float fadingTime ;//fading duration (in seconds)
extern unsigned int mutexID;//mutex used in multithread
extern Icon* icons;//list of icon
extern bool sound;//is sound enabled?
extern bool drawFps;//draw frames per second

void PlayButtonClickSound();
/*----------intro stuffs---------------------*/
namespace Intro
{
	void InitIntro();//loading images,icons,texts ... and initialize some stuffs for displaying intro logo & splash screen
	void IntroRendering(void);//display Gameloft Logo
	void SplashScreen(void);//display splashscreen
};
/*----------questioning pop-up stuffs-----------------------*/
namespace PopUp
{
	enum PopUpType
	{
		P_EXIT = 0,//popup dialog for asking if user want to quit or not
		P_MENU = 1,//popup dialog for asking if user to go back to main menu or not
		P_SOUND = 2,//popup dialog for asking if user want to enable sound or not
		P_RETRY = 3
	};
	extern bool isPopping;
	void InitPopUp();//loading images,icons,texts ... and initialize some stuffs for display pop-up dialog
	void BeginPopUp(PopUpType type);//begin popup
	void PopUpRendering();//pop-up dialog render function
};
/*----------loading stuffs----------------------*/
namespace LoadingProgress
{
	extern bool waitingForCont;//load progress is completed , waiting for continue
	extern bool userCont;//user pressed key to continue
	void IncreaseProgressVal(unsigned int percentage);//increase progress percentage value by <percentage> amount,progress value can't exceed 100 which denotes loading progress is at 100%(finished)
	void SetProgressVal(unsigned int percentage);
	void InitLoadingStuff();
	void Updating();
	void BeginLoading(CallbackFunc progressTask,CallbackFunc doneTask);//<progressTask> is call back function used when loading screen is displaying . <doneTask> is call back function used when loading progress finishes
};
/*----------menu stuffs-------------------------*/
namespace Menu
{
	void InitMenu();
	void BeginDisplayMenu();
};

/*----------in game stuffs----------------------*/
namespace Game
{
	void InitGame();
	void BeginGame();
	//section = 0 - stay at current section.
	//section = 1 - to menu .
	//section = 2 - reloading game (either current level or to next level)
	void ChangeSection(int section);
};
#endif