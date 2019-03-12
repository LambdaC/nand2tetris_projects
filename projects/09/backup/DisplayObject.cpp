class Aircraft {
	field int row, col;
	field int width, height;
	//field int renderIndex;	// every display object should has this attribute
	field int direction;	// 0-None, 1-Up, 2-Down
	field int objectType;	// 0-Aircraft, 1-Enemy, 2-Bullet

	constructor GameObject new(int type) {
		let objectType = type;
		do initData();
		return this;
	}

	method void initData(){
		let row = 0;
		let col = 0;
		let width = 0;
		let height = 0;
		//let renderIndex = 0;
		let direction = 0;
		return;
	}
	
	/** display the object on the screen */
	method void render() {
		if(objectType = 0) {
			do Render.renderAircraft(row, col);
			return;
		}

		if(objectType = 1) {
			do Render.renderEnemy(row, col);
			return;
		}

		if(objectType = 2) {
			do Render.renderBullet(row, col);
			return;
		}
	}

	method void setPosition(int newRow, int newCol) {
		let row = newRow;
		let col = newCol;
		return;
	}

	method void setSize(int newWidth, int newHeight) {
		let width =  newWidth;
		let height = newHeight;
		return;
	}

	method void setDirection(int newDirection) {
		let direction = newDirection;
		return;
	}

	/** move forward by 1 row depend on direction */
	method void autoMove() {
		if(direction = 0){
			return;
		}
		if(direction = 1) {
			let row = row-1;
			return;
		}
		if(direction = 2) {
			let row = row+1;
			return;
		}
	}

	method int getRow() {
		return row;
	}

	method int getCol() {
		return col;
	}

	method int getWidth() {
		return width;
	}

	method int getHeight() {
		return height;
	}
}
