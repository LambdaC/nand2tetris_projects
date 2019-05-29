import sys


MAX=2**16
SymbolTable={}
JumpTable={
	"null":"000",
	"JGT":"001",
	"JEQ":"010",
	"JGE":"011",
	"JLT":"100",
	"JNE":"101",
	"JLE":"110",
	"JMP":"111",
}
DestTable={
	"null":"000",
	"M":"001",
	"D":"010",
	"MD":"011",
	"A":"100",
	"AM":"101",
	"AD":"110",
	"AMD":"111",
}
CompTable={
	"0":"0101010",
	"1":"0111111",
	"-1":"0111010",
	"D":"0001100",
	"A":"0110000",
	"!D":"0001101",
	"!A":"0110001",
	"-D":"0001111",
	"-A":"0110011",
	"D+1":"0011111",
	"A+1":"0110111",
	"D-1":"0001110",
	"A-1":"0110010",
	"D+A":"0000010",
	"D-A":"0010011",
	"A-D":"0000111",
	"D&A":"0000000",
	"D|A":"0010101",
	"M":"1110000",
	"!M":"1110001",
	"-M":"1110011",
	"M+1":"1110111",
	"M-1":"1110010",
	"D+M":"1000010",
	"D-M":"1010011",
	"M-D":"1000111",
	"D&M":"1000000",
	"D|M":"1010101",
}
LineNum=0
Index=16

def Init():
	global SymbolTable
	SymbolTable["R0"]=0
	SymbolTable["R1"]=1
	SymbolTable["R2"]=2
	SymbolTable["R3"]=3
	SymbolTable["R4"]=4
	SymbolTable["R5"]=5
	SymbolTable["R6"]=6
	SymbolTable["R7"]=7
	SymbolTable["R8"]=8
	SymbolTable["R9"]=9
	SymbolTable["R10"]=10
	SymbolTable["R11"]=11
	SymbolTable["R12"]=12
	SymbolTable["R13"]=13
	SymbolTable["R14"]=14
	SymbolTable["R15"]=15
	SymbolTable["SCREEN"]=16384
	SymbolTable["KBD"]=24576
	SymbolTable["SP"]=0
	SymbolTable["LCL"]=1
	SymbolTable["ARG"]=2
	SymbolTable["THIS"]=3
	SymbolTable["THAT"]=4

def ReadFile(path):
	global LineNum
	global SymbolTable
	global Index
	iTable=[]
	fo=open(path,"r")
        //第一次循环,找出所有的label,并保存他们对应的位置
	for line in fo:
		line=line.replace(" ","")
		if "\r\n" in line:
			line=line.split("\r\n")[0]
		else:
			line=line.split("\n")[0]
		if not line:
			continue
		line=line.split("//")[0]
		if line=="":
			continue
		if line[0]=="(":
			label=line[1:-1]
			if not label in SymbolTable.keys():
				SymbolTable[label]=LineNum
			continue
		LineNum+=1
	fo.close()

	fo=open(path,"r")
        //第二次循环，找出所有的汇编指令
	for line in fo:
		line=line.replace(" ","")
		if "\r\n" in line:
			line=line.split("\r\n")[0]
		else:
			line=line.split("\n")[0]
		if not line:
			continue
		line=line.split("//")[0]
		if line=="":
			continue
		if line[0]=="(":
			continue
		if line[0]=="@":
			if line[1] not in ["0","1","2","3","4","5","6","7","8","9"]:
				label=line[1:]
				if not label in SymbolTable.keys():
					print "%s:%s"%(label,Index)
					SymbolTable[label]=Index
					Index+=1
		if not line=="":
			iTable.append(line)
	fo.close()
	return iTable

def Decode(iTable,path):
	path=path.split(".a")[0]
	path=path+".hack"
	fo=open(path,"w")
	for code in iTable:
                // A指令
		if code[0]=="@":
			if code[1] not in ["0","1","2","3","4","5","6","7","8","9",]:
				label=code[1:]
				addr=SymbolTable.get(label,None)
				print "%s:%s"%(label,addr)
				addr=addr|MAX
				bAddr=bin(addr)[3:]
				bAddr=bAddr+"\n"
				fo.write(bAddr)
			else:
				addr=int(code[1:])
				addr=addr|MAX
				bAddr=bin(addr)[3:]
				bAddr=bAddr+"\n"
				fo.write(bAddr)
                // C指令
		else:
                        // dest=comp;jump格式
			if "=" in code and ";" in code:
				dest=code.split("=")[0]
				comp=code.split("=")[1].split(";")[0]
				jump=code.split("=")[1].split(":")[1]
				bDest=DestTable.get(dest,None)
				bComp=CompTable.get(comp,None)
				bJump=JumpTable.get(jump,None)
				bAddr="111"+bComp+bDest+bJump+"\n"
				fo.write(bAddr)
                        // dest=comp格式
			elif "=" in code and ";" not in code:
				dest=code.split("=")[0]
				comp=code.split("=")[1]
				bDest=DestTable.get(dest,None)
				bComp=CompTable.get(comp,None)
				bAddr="111"+bComp+bDest+"000"+"\n"
				fo.write(bAddr)
                        // comp;jump格式
			elif "=" not in code and ";" in code:
				comp=code.split(";")[0]
				jump=code.split(";")[1]
				bComp=CompTable.get(comp,None)
				bJump=JumpTable.get(jump,None)
				bAddr="111"+bComp+"000"+bJump+"\n"
				fo.write(bAddr)
                        // comp格式
			else:
				comp=code
				bComp=CompTable.get(comp,None)
				bAddr="111"+bComp+"000"+"000"+"\n"
				fo.write(bAddr)
	fo.close()

if __name__=="__main__":
	Init()
	iTable=ReadFile(sys.argv[1])
	Decode(iTable,sys.argv[1])
