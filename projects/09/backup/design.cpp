GameObject/DisplayObject {
private:
	int row,col;
	int width,height;
	int renderIndex;	//every display object should has this attribute
	int direction;  // 0-None,1-Up,2-Down
	int objectType 	//0-Aircarfa, 1-Enemy, 2-Bullet
public:
	// display the object on the screen
	// if objectType == 0, it should call Render.renderAircraft(int row, int col)
	// if objectType == 1, it should call Render.renderEnemy(int row, int col)
	// if objectType == 2, it should call Render.renderBullet(int row, int col)
	void render();
	// set the gameObjetc's position
	void setPosition(int row, int col);
	void setSize(int width, int height);
	void setDirection(int direction);
	void setRenderIndex(int renderIndex);
	int getRow();
	int getCol();
	int getWidth();
	int getHeight();
}

class Render {
public:
	// draw the Aircarfa Object on the screen
	static void renderAircraft(int row, int col);
	// draw the Enemy Object on the screen
	static void renderEnemy(int row, int col);
	// draw the Bullet Object on the screen
	static void renderBullet(int row, int col);
	
	static void renderObjects(Array displayObjects, int objectsNum){
		for( int i = 0; i < objectsNum; i ++){
			displayObjects[i].draw();
		}
	};
}

class InputController {
private:
	GameObject myAircraft;
	int keyPressed;
	int getInput() {
		keyPressed = Keyboard.keyPressed();
	};
public:
	int handleInput(int gameState){
		let keyPressed = getInput();
		if(keyPressed == "ESC"){
			gameState = GameState.Over();
			return gameState;
		}
		
		if(keyPressed == "p"){
			if(gameState == GameState.Running()){
				gameState = GameState.Pause();
			}
			if(gameState == GameState.Pause()){
				gameState = GameState.Running();
			}
			return gameState;
		}
		
		return gameState;
	};
}

class GameState{
	int Init(){
		return 0;
	}
	
	int Running(){
		return 1;
	}
	
	int Pause(){
		return 2;
	}
	
	int Over(){
		return 3;
	}
}

class GameObjectFactory {
public:
	static GameObject creatGameObject(int objectType);
}

class GameController {
private:
	InputController inputController;
	Render render;
	Array gameObjects;
	int objectsNum = 0;
	int gameState;	// 0-init, 1-running, 2-pause, 3-over
	
	int handleInput(gameState);
	void handleObjects();
	void render(){
		Render.renderObjects(displayObjects,objectsNum);
	};
public:
	void init(){
		// set gameState = 0
		// create myAircraft
		// creat inputController and refer to myAircraft
	};
	bool isGameOver();
	void over();
	void handleGame(){
		// if gamestate == 0
		// set myAircraft to initPosition
		// render init infomation
		// if gameState == 1
		// detect 
		// if gameState == 3
		// do nothing
	};
}



class Game {
private:
	GameController gameController;
public:
	// run the game;
	void run() {
		gameController.init();
		while(!gameController.isGameOver()){
			gameController.handleGame();
		}
		gameController.over();
		return;
	};	
}
