//MyCraft
//left_top
function void draw(int location) {
	let memAddress = 16384+location;
	do Memory.poke(memAddress+0, -32768);
	do Memory.poke(memAddress+32, 16384);
	do Memory.poke(memAddress+64, 16384);
	do Memory.poke(memAddress+96, 16384);
	do Memory.poke(memAddress+128, 16384);
	do Memory.poke(memAddress+160, 16384);
	do Memory.poke(memAddress+192, -8192);
	do Memory.poke(memAddress+224, 24576);
	do Memory.poke(memAddress+256, 28672);
	do Memory.poke(memAddress+288, 6656);
	do Memory.poke(memAddress+320, -8576);
	do Memory.poke(memAddress+352, -18560);
	do Memory.poke(memAddress+384, -22592);
	do Memory.poke(memAddress+416, -19984);
	do Memory.poke(memAddress+448, -26624);
	do Memory.poke(memAddress+480, -29696);
	return;
}

//right_top
function void draw(int location) {
	let memAddress = 16384+location;
	do Memory.poke(memAddress+0, 0);
	do Memory.poke(memAddress+32, 1);
	do Memory.poke(memAddress+64, 1);
	do Memory.poke(memAddress+96, 1);
	do Memory.poke(memAddress+128, 1);
	do Memory.poke(memAddress+160, 1);
	do Memory.poke(memAddress+192, 3);
	do Memory.poke(memAddress+224, 3);
	do Memory.poke(memAddress+256, 23);
	do Memory.poke(memAddress+288, 92);
	do Memory.poke(memAddress+320, 253);
	do Memory.poke(memAddress+352, 502);
	do Memory.poke(memAddress+384, 1986);
	do Memory.poke(memAddress+416, 6);
	do Memory.poke(memAddress+448, 12);
	do Memory.poke(memAddress+480, 8);
	return;
}

//left_bottom
function void draw(int location) {
	let memAddress = 16384+location;
	do Memory.poke(memAddress+0, -29184);
	do Memory.poke(memAddress+32, -256);
	do Memory.poke(memAddress+64, 25600);
	do Memory.poke(memAddress+96, 16384);
	return;
}

//right_bottom
function void draw(int location) {
	let memAddress = 16384+location;
	do Memory.poke(memAddress+0, 56);
	do Memory.poke(memAddress+32, 127);
	do Memory.poke(memAddress+64, 19);
	do Memory.poke(memAddress+96, 1);
	return;
}

//enemy craft
//left_top
function void draw(int location) {
	let memAddress = 16384+location;
	do Memory.poke(memAddress+0, -32768);
	do Memory.poke(memAddress+32, -16384);
	do Memory.poke(memAddress+64, -1024);
	do Memory.poke(memAddress+96, 14336);
	return;
}
//right_top
function void draw(int location) {
	let memAddress = 16384+location;
	do Memory.poke(memAddress+0, 2);
	do Memory.poke(memAddress+32, 6);
	do Memory.poke(memAddress+64, 127);
	do Memory.poke(memAddress+96, 57);
	return;
}
//left_bottom
function void draw(int location) {
	let memAddress = 16384+location;
	do Memory.poke(memAddress+0, 12288);
	do Memory.poke(memAddress+32, 25472);
	do Memory.poke(memAddress+64, -12800);
	do Memory.poke(memAddress+96, -25600);
	do Memory.poke(memAddress+128, -18432);
	do Memory.poke(memAddress+160, -4096);
	do Memory.poke(memAddress+192, -4096);
	do Memory.poke(memAddress+224, -8192);
	do Memory.poke(memAddress+256, -16384);
	do Memory.poke(memAddress+288, -32768);
	do Memory.poke(memAddress+320, -32768);
	do Memory.poke(memAddress+352, -32768);
	do Memory.poke(memAddress+384, -32768);
	do Memory.poke(memAddress+416, 0);
	do Memory.poke(memAddress+448, 0);
	do Memory.poke(memAddress+480, 0);
	return;
}
//right_bottom
function void draw(int location) {
	let memAddress = 16384+location;
	do Memory.poke(memAddress+0, 25);
	do Memory.poke(memAddress+32, 13);
	do Memory.poke(memAddress+64, 902);
	do Memory.poke(memAddress+96, 242);
	do Memory.poke(memAddress+128, 122);
	do Memory.poke(memAddress+160, 62);
	do Memory.poke(memAddress+192, 30);
	do Memory.poke(memAddress+224, 14);
	do Memory.poke(memAddress+256, 7);
	do Memory.poke(memAddress+288, 2);
	do Memory.poke(memAddress+320, 2);
	do Memory.poke(memAddress+352, 2);
	do Memory.poke(memAddress+384, 1);
	do Memory.poke(memAddress+416, 1);
	do Memory.poke(memAddress+448, 1);
	do Memory.poke(memAddress+480, 1);
	return;
}

//new Aircraft
//left_top
function void draw(int location) {
	let memAddress = 16384+location;
	do Memory.poke(memAddress+0, 2048);
	do Memory.poke(memAddress+32, 5120);
	do Memory.poke(memAddress+64, 5120);
	do Memory.poke(memAddress+96, 5120);
	do Memory.poke(memAddress+128, 5120);
	do Memory.poke(memAddress+160, 5120);
	do Memory.poke(memAddress+192, 13824);
	do Memory.poke(memAddress+224, 13824);
	do Memory.poke(memAddress+256, 30464);
	do Memory.poke(memAddress+288, -15968);
	do Memory.poke(memAddress+320, -8728);
	do Memory.poke(memAddress+352, 27512);
	do Memory.poke(memAddress+384, 10876);
	do Memory.poke(memAddress+416, 27423);
	do Memory.poke(memAddress+448, -13952);
	do Memory.poke(memAddress+480, -30528);
	return;
}

//right_top
function void draw(int location) {
	let memAddress = 16384+location;
	var int value;
	let value = Memory.peek(memAddress+288);
	let value = value & -256;
	do Memory.poke(memAddress+288, value + 2);
	let value = Memory.peek(memAddress+288);
	let value = value & -256;
	do Memory.poke(memAddress+320, value + 11);
	let value = Memory.peek(memAddress+288);
	let value = value & -256;
	do Memory.poke(memAddress+352, value + 31);
	let value = Memory.peek(memAddress+288);
	let value = value & -256;
	do Memory.poke(memAddress+384, value + 62);
	let value = Memory.peek(memAddress+288);
	let value = value & -256;
	do Memory.poke(memAddress+416, 248);
	let value = Memory.peek(memAddress+288);
	let value = value & -256;
	do Memory.poke(memAddress+480, value + 1);
	return;
}

//left_bottom
function void draw(int location) {
	let memAddress = 16384+location;
	do Memory.poke(memAddress+0, -30496);
	do Memory.poke(memAddress+32, -16);
	do Memory.poke(memAddress+64, 13888);
	do Memory.poke(memAddress+96, 5120);
	return;
}

//right_bottom
function void draw(int location) {
	let memAddress = 16384+location;
	var int value;
	let value = Memory.peek(memAddress+0);
	let value = value & -256;
	do Memory.poke(memAddress+0, value+3);
	let value = Memory.peek(memAddress+32);
	let value = value & -256;
	do Memory.poke(memAddress+32, value+7);
	let value = Memory.peek(memAddress+64);
	let value = value & -256;
	do Memory.poke(memAddress+64, value+1);
	return;
}

// new enemy
// left_top
function void draw(int location) {
	let memAddress = 16384+location;
	do Memory.poke(memAddress+0, 5120);
	do Memory.poke(memAddress+32, 13824);
	do Memory.poke(memAddress+64, -32);
	do Memory.poke(memAddress+96, -13888);
	do Memory.poke(memAddress+128, -13952);
	do Memory.poke(memAddress+160, 27392);
	do Memory.poke(memAddress+192, 13855);
	do Memory.poke(memAddress+224, 5244);
	do Memory.poke(memAddress+256, -10760);
	do Memory.poke(memAddress+288, -2072);
	do Memory.poke(memAddress+320, -2144);
	do Memory.poke(memAddress+352, 30464);
	do Memory.poke(memAddress+384, 15872);
	do Memory.poke(memAddress+416, 5120);
	do Memory.poke(memAddress+448, 5120);
	do Memory.poke(memAddress+480, 5120);
	return;
}
// right_top
function void draw(int location) {
	var int value;
	let memAddress = 16384+location;
	let value = Memory.peek(memAddress+64);
	let value = value & -256;
	do Memory.poke(memAddress+64, value+3);
	
	let value = Memory.peek(memAddress+96);
	let value = value & -256;
	do Memory.poke(memAddress+96, value+1);
	
	let value = Memory.peek(memAddress+192);
	let value = value & -256;
	do Memory.poke(memAddress+192, value+124);
	
	let value = Memory.peek(memAddress+224);
	let value = value & -256;
	do Memory.poke(memAddress+224, value+31);
	
	let value = Memory.peek(memAddress+256);
	let value = value & -256;
	do Memory.poke(memAddress+256, value+15);
	
	let value = Memory.peek(memAddress+288);
	let value = value & -256;
	do Memory.poke(memAddress+288, value+11);
	
	let value = Memory.peek(memAddress+320);
	let value = value & -256;
	do Memory.poke(memAddress+320, value+2);
	return;
}
// left_bottom
function void draw(int location) {
	let memAddress = 16384+location;
	do Memory.poke(memAddress+0, 7168);
	do Memory.poke(memAddress+32, 2048);
	do Memory.poke(memAddress+64, 2048);
	do Memory.poke(memAddress+96, 2048);
	return;
}
// right_bottom