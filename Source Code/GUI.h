#ifndef _GUI_SYSYEM_
#define _GUI_SYSTEM_

#include "2DAnimationController.h"
#include "ItemManager.h"

/*-----------Button animation pose info struct----------------------*/
struct ButtonAnimPose
{
	Rect rect;//rectangle area of icon image in image file
	unsigned int textureID;//id of texture that loaded from image file contains icon
};
/*-----------Button class----------------------*/
class Button
{
	friend class GUImanager;//only GUImanager object can create this object
private:
	bool isActive;
	Renderer *pRenderer;
	Rect rect;//rectangle area in window space of button
	void(*ClickedHandler1)(void);//callback function pointer that will be triggered when button is clicked
	void(*ClickedHandler2)(void);//callback function pointer that will be triggered when clicking animation is finished
	AnimController<ButtonAnimPose> clickAnimCtrl;//clicking animation controller
	unsigned int animFrames;//total number of frames that clicking animation will last
	/*
	create button object
		<clickedHandler1> - call back function triggered when button is clicked
		<clickedHandler2> - call back function triggered when clicking animation is finished
		<animFrames> - total frames that clicking animation will last
		<framePerPose> - number of frames that each pose in animation will last
		<textureID1> - //id of texture that contains icon1 of button - this is default icon of button,when button is not clicked
		<rect1> - rectangle area of button's icon1 in image space 
		<textureID2> - //id of texture that contains icon2 of button - icon2 is used in clicking animation 
		<rect2> - rectangle area of button's icon2 in image space
		<buttonRect> - button's rectangle in window space
		<pRenderer> - pointer to renderer
	*/
	Button(void (*clickedHandler1)(void),void (*clickedHandler2)(void),
		   unsigned int animFrames,unsigned int framePerPose,
		   unsigned int textureID1,Rect& rect1,
		   unsigned int textureID2,Rect& rect2,
		   Rect& buttonRect,Renderer *pRenderer);

	bool IsActive(){return isActive;}
	void Active() {isActive = true;}
	void DeActive() {isActive = false;}
	bool IsClicked(int mouseX , int mouseY);//is this button clicked? This will implicitly start clicking animation if return true.If button is still in clicking animation loop,this will return false
	void UpdateStatus();//update current status of button,needed for animation
	void Draw();//draw button
public:
	~Button();
};


struct DisplayText
{
	char* String;//string that will be displayed on screen
	unsigned int FontID;//id of font object that will be used in rendering text
	float X,Y;//location of text's line in window screen
	~DisplayText()
	{
		if(String)
			delete[] String;
	}
};

/*---------------GUImanager - manages list of GUI items-----------------*/
/*-------------------
GUI items:
-background
-button
-image
-text
-------------------*/
struct Image
{
	unsigned int textureID;//id of texture used for drawing image to screen
	Rect imageRect;//rectangle area in image space that you want to display on screen
	Rect windowRect;//rectangle area in window space that you want to draw image to
};

class GUImanager:private ItemManager<Image>
{
private:
	Renderer *pRenderer;
	Image backGround;
	ItemManager<DisplayText> textMan;
	ItemManager<Button> buttonMan;
public:
	GUImanager(Renderer *pRenderer){this->pRenderer = pRenderer;};

	//set background
	//<textureID> - id of texture that will used for draw background
	//<pImgRect> - pointer to rectangle structure that represents area in image space that contains background,NULL denotes entire image
	//<pWinRect> - pointer to rectangle structure that represents area in window space that background will be drawn to,NULL denotes entire window
	//notes : image space has origin at top left, and window space has origin at bottom left
	int SetBackGround(unsigned int textureID,Rect *pImgRect=NULL,Rect *pWinRect=NULL);
	
	int AddImage(unsigned int textureID,Rect *pImgRect,Rect *pWinRect,unsigned int*pImageID);//add image you want to display on screen to GUI group
	
	/*
	create button object and add to GUI manager
		<clickedHandler1> - call back function triggered when button clicked
		<clickedHandler2> - call back function triggered when clicking animation is finished
		<animFrames> - total frames that clicking animation will last
		<framePerPose> - number of frames that each pose in animation will last
		<textureID1> - //id of texture that contains icon1 of button - this is default icon of button,when button is not clicked
		<rect1> - rectangle area of button's icon1 in image space 
		<textureID2> - //id of texture that contains icon2 of button - icon2 is used in clicking animation 
		<rect2> - rectangle area of button's icon2 in image space
		<buttonRect> - button's rectangle in window space
		<pButtonID> - pointer to id of newly added button
	*/
	int AddButton(void (*clickedHandler1)(void),void (*clickedHandler2)(void),
		   unsigned int animFrames,unsigned int framePerPose,//total clicking animation frames & frames per pose
		   unsigned int textureID1,Rect& rect1,
		   unsigned int textureID2,Rect& rect2,
		   Rect& buttonRect,
		   unsigned int* pButtonID);
	int AddText(const char * text,float x,float y,unsigned int fontID,unsigned int *pTextID);

	int RemoveImage(unsigned int imageID);
	int RemoveButton(unsigned int buttonID);
	int RemoveText(unsigned int textID);

	void RemoveAllItems();//remove all GUI items 
	
	void UpdateButtonStatus();//update all buttons' statuses , needed for animation
	bool IsButtonClicked(unsigned int buttonID,int mouseX,int mouseY);//is button <buttonID> clicked? This will implicitly start clicking animation if return true.If button is still in clicking animation loop,this will return false
	bool IsAnyButtonClicked(int mouseX,int mouseY);//is any button clicked? This will implicitly start clicking animation if return true.If button is still in clicking animation loop,this will return false
	void ActiveButton(unsigned int buttonID);//active button for accepting clicked event
	void DeActiveButton(unsigned int buttonID);//deactive button for not accepting clicked event

	void DrawImage(unsigned int imageID);//draw image <imageID> to screen
	void DrawImages();//draw all image
	void DrawButton(unsigned int buttonID);
	void DrawButtons();//draw all button
	void Draw_Text(unsigned int textID);
	void DrawTexts();//draw all text
	void DrawBackground();//draw background
};

#endif