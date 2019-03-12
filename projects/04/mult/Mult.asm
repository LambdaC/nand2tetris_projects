// This file is part of www.nand2tetris.org
// and the book "The Elements of Computing Systems"
// by Nisan and Schocken, MIT Press.
// File name: projects/04/Mult.asm

// Multiplies R0 and R1 and stores the result in R2.
// (R0, R1, R2 refer to RAM[0], RAM[1], and RAM[2], respectively.)

// Put your code here.

//初始化时R2=0
@R2
M=0
//R1为计数器,如果为0,则直接结束,如果为负数,则R1和R0都取反
@R1
D=M;
@END
D;JEQ
@NEG
D;JLT
@LOOP
D;JGE
(NEG)
@R0
M=-M
@R1
M=-M

//先R2=R2+R0,再判断R1-1是否为0,不为0则继续循环
(LOOP)
@R0
D=M
@R2
M=D+M
@R1
M=M-1
D=M
@LOOP
D;JNE

(END)
@END
0;JMP
