#ifndef _GAME_H
#define _GAME_H
#include "Board.h"
#include "Object.h"
#include "AI.h"
#include "../Renderer.h"
#include <list>
#include <map>
struct GraphicSetup
{
	int numLights;
	DirectionLight *dLights;//directional light info
	float fov;//camera field of view
	Vector3 cameraPos;//camera position
	Vector3	cameraLookAt;//camera's look at point
	GraphicSetup()
	{
		dLights = NULL;
	}
	~GraphicSetup()
	{
		SafeDeleteArray(dLights);
	}
};

struct GameInfo
{
	int section;//0 denotes current section ,no change. 1 denotes changing to menu and 2 denotes retry current section
	Board *board;
	GraphicSetup gsetup;//setup light & camera info
	unsigned int musicID;//music audio ID in mixer
	int musicChannel;
	unsigned int hurtSoundID;//hurt sound ID in mixer
	unsigned int coinSoundID;//collect coin sound ID in mixer
	unsigned int loseMusicID;//lose music ID in mixer
	unsigned int winMusicID;//win music ID in mixer
	unsigned int homeButtonID;//home button ID
	unsigned int bgTexID;//background texture id
	unsigned int soundButtonID[2];//enable / disable sound button id
	unsigned int crossImgID[2];
	unsigned int boardTexID;//texture for rendering board
	unsigned int meshIDs[4];//list of mesh ids for rendering player,ghost,box,coin
	unsigned int meshPackageBufferID;//id of package buffer that holds bytes stream of mesh package
	std::map<int,StaticObject> coins;
	std::list<StaticObject> boxes;
	std::list<AIController *> ghosts;
	MovableObject *player;
	float coinRotAngle;//rotation angle per frame of coins
	float coinCurrentAngle;
	Matrix4x4 coinRotMatrix;//rotation matrix for transforming coin
	int life;
	float lifeTextX,lifeTextY;//starting location of life indicating text line in screen
	float coinTextX,coinTextY;//starting location of coins indicating text line in screen
	float levelTextX,levelTextY;//starting location of current level indicating text line in screen
	float fpsTextX,fpsTextY;//starting location of fps indicating text line in screen
	char lifeTextPrefix[128];//prefix of life indicating text
	char coinTextPrefix[128];//prefix of coins indicating text
	char lifeText[256];//life indicating text
	char coinText[256];//coins indicating text
	char levelText[256];//current level indicating text
	unsigned int lifeTextFontID;//id of font object for rendering life indicating text
	unsigned int coinTextFontID;//id of font object for rendering coins indicating text
	unsigned int levelTextFontID;//id of font object for rendering current level indicating text
	Rect mapRect;//rectangle area of map in window screen
	Matrix4x4 eagleOrthoProj;//ortho projection matrix for rendering map
	Matrix4x4 eagleViewMatrix;//view matrix for rendering map
	Matrix4x4 ProjMatrix;//main projection matrix
	Matrix4x4 ViewMatrix;//main view matrix
	Matrix4x4 rotationMatrix;
	float anglePerFrame;
	float currentXAngle;//current board'rotation angle via X-axis
	float currentYAngle;//current board'rotation angle via Y-axis
	float maxXAngle,minXAngle;
	float maxYAngle,minYAngle;
	int nextLevel;//next level
	unsigned int maxInviFrames;//max invisible frames
	AnimController<bool> inviAnimCtrl;//invisible animation controller
	/*-----win/lose animation info------*/
	unsigned int winLoseTexID;//texture for rendering win/lose text
	AnimController<int> winLoseAnim;//win/lose text image animation controller
	float endSceneFrames;//how many frames that the win/lose text will keep showing up on screen
	int wX[4],lX[4];
	int wlH;
	Rect wlImgRect[4];
	/*----------------------------------*/
	bool havePopup ;//is there a popup on screen,if so, we need to disable all buttons
	bool paused;
	bool gameStarted;
	bool invisible;
	bool win;
	GameInfo();
	~GameInfo();
};

extern GameInfo *gameInfo;
extern int level ; //current level
namespace Game
{
	void OnExit5();
	void DeActiveAllButtons();
	void ActiveAllButtons();
	void ActiveAllButtons();
	void GameUpdate();
	void GameRender();
	void KeyPressed(unsigned char key);
};
#endif