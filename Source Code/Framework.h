#ifndef _FRAME_WORK_
#define _FRAME_WORK_

#include <list>
#include "Renderer.h"
#include "Mixer.h"
#include "Timer.h"

#include <GL/glut.h>
#pragma comment(lib,"glut32.lib")

#include "2DAnimationController.h"
#include "GUI.h"
#include "CustomVariable.h"
#include "Mesh.h"

typedef void (*CallbackFunc)(void);
typedef void (*KeyCallBack)(unsigned char key);

#define LEFT_MOUSE_CLICK 0x1
#define RIGHT_MOUSE_CLICK 0x2
struct MouseState
{
	int X,Y;
	int dX,dY;
	int flags;
};
/*---------------------------------------------
Application framework - handles rendering task,
input,audio , and threads managment
----------------------------------------------*/
class AppFrameWork
{
private:
	static AppFrameWork instance;//singleton instance
	static bool ready;//ready to use?
	/*-----call back function pointers-------------------------*/
	CallbackFunc updateFunc;//pointer to update function
	CallbackFunc displayFunc;//pointer to display/render function 
	KeyCallBack keyPressFunc;//pointer to keypress event handling function
	std::list<CallbackFunc> exitTasks;//list of tasks that will be triggerd when exitting application
	/*--------------------------------------------------------*/

	HMODULE pRendererDll,pMixerDll;//
	Renderer* pRenderer;//interface pointer to renderer
	Mixer * pMixer;//interface pointer to mixer
	SharedPtr<GUImanager> pGui;//shared pointer to GUI manager object
	SharedPtr<VariableManager> pVarMan;//shared pointer to custom varible manager object
	SharedPtr<MeshManager> pMeshMan;//shared pointer to mesh manager

	unsigned int numMutex;//number of mutexes
	HANDLE* mutexes;//list of mutexes

	MouseState mouseState;

	bool exit;//exit application ?
	int exitCode;
	/*-------timing stuffs----------*/
	double elapsedTime,overSleepTime,sleepTime,excessTime,totalTime;//(in seconds)
	double period;//desired period (in seconds) of an game loop's iteration
	double fpsUpdatePeriod;//period (in seconds) that varerage fps will be updated
	StopWatch timer;
	unsigned int frames;//total frames rendered
	float averageFps;//average fps
	
	bool calFps;//calculate fps?
	/*--------------------*/
	void InitWindow(unsigned int width,unsigned int height);
	
	/*--------friend functions------------------*/
	friend void GameLoop(void);//game loop
	friend void GameRender(void);//game rendering stage
	friend void Keyboard(unsigned char key,int x,int y);//keyboard callback function
	friend void Mouse(int button, int state,int x, int y);//mouse pressed,released event handler
	friend void MouseMotion(int x,int y);

	AppFrameWork();//prevent user from creating other objects
	~AppFrameWork();
public:
	static AppFrameWork* GetInstancePointer();//get pointer to instance ,if it not ready to use,NULL will be returned
	Renderer* GetRenderer() { return pRenderer;};
	Mixer *GetMixer() {return pMixer;};
	SharedPtr<GUImanager> GetGUImanager() { return pGui;};
	SharedPtr<VariableManager> GetVariableManager() {return pVarMan;}
	SharedPtr<MeshManager> GetMeshManager() {return pMeshMan;}
	const MouseState& GetMouseState() {return mouseState;}

	void SetUpdateFunc(void (*func)(void));//set updating function
	void SetRenderFunc(void (*func)(void));//set rendering function
	void SetKeyPressFunc(void (*func)(unsigned char));//set key pressed event handling function
	void AddExitTask(void (*func)(void));//append call back function to the end of exit tasks list.
	void Start();//start application,the calling thread will run in infinite loop and will not return after this method is called
	
	void LockMutex(unsigned int mutexID);//lock mutex that has ID <mutexID>
	void UnlockMutex(unsigned int mutexID);//unlock mutex that has ID <mutexID>
	bool CreateNewMutex(unsigned int *mutexID);//create new mutex
	void StartThread(void (*func)(void*), void *argList = NULL);//start a thread with subroutine <func> and pass argument list <argList> to thread
	
	void EnableCalFps();//begin calculating fps
	void DisableCalFps();//end calculating fps
	void DrawFps(float x,float y,unsigned int fontID);//draw text that show frames per second to screen

	unsigned int QueryFramesByTime(float time);//get number of frames will be rendered in <time> (seconds)

	void ExitApp(int exitCode);//exit application
};



#endif