#include<string>
#include<fstream>
#include<iostream>
#include<cstdlib>
#include<sstream>
#include<vector>
#include<iterator>
#include<map>
#include<experimental/filesystem>
#include<algorithm>

struct CommandType {
	constexpr static auto C_ARITHMETIC{"C_ARITHMETIC"};
	constexpr static auto C_PUSH{"C_PUHS"};
	constexpr static auto C_POP{ "C_POP" };
	constexpr static auto C_LABEL{ "C_LABEL" };
	constexpr static auto C_GOTO{ "C_GOTO" };
	constexpr static auto C_IF{ "C_IF" };
	constexpr static auto C_FUNCTION{ "C_FUNCTION" };
	constexpr static auto C_RETURN{ "C_RETURN" };
	constexpr static auto C_CALL{"C_CALL"};
};



class Parser {
private:
	std::string m_fileName;
	std::ifstream inf;
	std::string m_currCommand;
	std::string m_commandType;
	std::string m_arg1;
	int m_arg2;
	void advance() {
		//按空格分隔字符串
		std::istringstream iss(m_currCommand);
		std::vector<std::string> results(std::istream_iterator<std::string>{iss},
                                 std::istream_iterator<std::string>());
	
		if (results.size() > 1) {
			m_arg1 = results[1];
		}

		if (results.size() > 2) {
			m_arg2 = std::stoi(results[2]);
		}

		if (results[0] == "push") {
			m_commandType = CommandType::C_PUSH;
		}
		else if (results[0] == "pop") {
			m_commandType = CommandType::C_POP;
		}
		else if (results[0] == "label"){
			m_commandType = CommandType::C_LABEL;
		}
		else if (results[0] == "goto") {
			m_commandType = CommandType::C_GOTO;
		}
		else if (results[0] == "if-goto"){
			m_commandType = CommandType::C_IF;
		}
		else if (results[0] == "call") {
			m_commandType = CommandType::C_CALL;
		}
		else if (results[0] == "function") {
			m_commandType = CommandType::C_FUNCTION;
		}
		else if (results[0] == "return"){
			m_commandType = CommandType::C_RETURN;
		}
		else
		{
			m_commandType = CommandType::C_ARITHMETIC;
			m_arg1 = results[0];
		}
	}
public:
	Parser(std::string path, std::string fileName) :m_fileName{fileName},inf { path + ".vm"}
	{
		if(!inf){
			std::cerr << "Open "+ path +".vm failed!" << std::endl;
			std::exit(1);
		}
		else {
			std::cout << "Open " + path + ".vm successed!" << std::endl;
		}
	};

	~Parser(){
		if(inf)
			inf.close();
	}

	//该方法会将命令中多余的空格和注释清除：掉
	std::string getCommand(std::string command) {
		std::string result;
		bool isPrevWhiteSpace{ false };

		for (char c : command) {
			if (c == ' ' || c == '\t') {
				isPrevWhiteSpace = true;
				continue;
			}
			
			//按照现在的情况如果遇到'/'，可以确定就是注释了
			if (c == '/') {
				result.push_back('\n');
				break;
			}

			if (isPrevWhiteSpace) {
				result.push_back(' ');
				isPrevWhiteSpace = false;
			}
			result.push_back(c);
		}

		return result;
	}

	bool hasMoreCommands() {
		if (!inf)
			return false;
		int line = 1;
		while (inf) {
			getline(inf, m_currCommand);

			if (m_currCommand.empty() || m_currCommand.size() <= 1) {
				continue;
			}

			if (m_currCommand == "\r\n" || m_currCommand == "\n") {
				continue;
			}

			if (m_currCommand[0] == '/' && m_currCommand[1] == '/'){
				continue;
			}

			m_currCommand = getCommand(m_currCommand);

			std::cout << m_currCommand << std::endl;
			advance();
			return true;
		}

		return false;
	}

	std::string commandType() {
		return m_commandType;
	}

	std::string arg1() {
		return m_arg1;
	}

	int arg2() {
		return m_arg2;
	}
};

//==================================================================================================================
class CodeWriter {
private:
	std::string m_fileName;
	std::ofstream outf;
	int retNum;	//函数返回地址后缀,确保每次返回的地址不一样

	std::map<std::string, std::string> label_map;

	int comp = 0; //label index
	/*
	返回逻辑比较的ASM代码
	*/
	std::string getCompCode(std::string command, std::string condition) {
		comp++;
		return "//" + command + "\n" 
			+"@SP\n" 
			+"M=M-1\n" 
			+"A=M\n" 
			+"D=M\n" 
			+"@SP\n"
			+ "M=M-1\n" 
			+"A=M\n" 
			+"D=M-D\n" 
			+"@IF_" + command + std::to_string(comp) + "\n" 
			+"D;" + condition + "\n" 
			+"@SP\n" 
			+"A=M\n" 
			+"M=0\n"
			+"@END_" + command + std::to_string(comp) + "\n"
			+"0;JMP\n"
			+"(IF_" + command + std::to_string(comp) + ")\n"
			+"@SP\n"
			+"A=M\n"
			+"M=-1\n"
			+"(END_" + command + std::to_string(comp) + ")\n"
			+"@SP\n"
			+"M=M+1\n";
	}

	/*
	返回逻辑运算的ASM代码
	*/
	std::string getBoolCode(std::string command, std::string op) {
		return "//" + command + "\n"
			+"@SP\n"
			+"M=M-1\n"
			+"A=M\n"
			+"D=M\n"
			+"@SP\n"
			+"M=M-1\n"
			+"A=M\n"
			+"M=D" + op + "M\n"
			+ "@SP\n"
			+"M=M+1\n";
	}

	/*
	返回LCL、ARG、THIS、THAT的Push ASM代码
	*/
	std::string getNormalPushCode(std::string label, int index) {
		return "@" + std::to_string(index) + "\n"
			+ "D=A\n"
			+ "@" + label + "\n"
			+ "D=D+M\n"
			+ "A=D\n"
			+ "D=M\n"
			+ "@SP\n"
			+ "A=M\n"
			+ "M=D\n"
			+ "@SP\n"
			+ "M=M+1\n";
	}

	std::string getNormalPopCode(std::string label, int index) {
		return "@" + std::to_string(index) + "\n"
			+ "D=A\n"
			+ "@" + label + "\n"
			+ "D=D+M\n"
			+ "@tmp\n"
			+ "M=D\n"
			+ "@SP\n"
			+ "M=M-1\n"
			+ "A=M\n"
			+ "D=M\n"
			+ "@tmp\n"
			+ "A=M\n"
			+ "M=D\n";
	}

public:
	CodeWriter(std::string path) :outf{ path + ".asm" }, retNum{0}
	{
		if(!outf){
			std::cerr << "Open " + path + ".asm failed!" << std::endl;
			std::exit(1);
		}
		else {
			std::cout << "Open " + path + ".asm successed!" << std::endl;
		}
		label_map["local"] = "LCL";
		label_map["argument"] = "ARG";
		label_map["this"] = "THIS";
		label_map["that"] = "THAT";
	}

	~CodeWriter(){
		close();
	}

	void setFileName(std::string fileName) {
		m_fileName = fileName;
	}

	void writeInit() {
		/**
		* Bootstrap code
		* SP=256
		* Call Sys.int
		*/
		outf << "//Bootstrap code\n"
			<< "@256\n"
			<< "D=A\n"
			<< "@SP\n"
			<< "M=D\n";
		writeCall("Sys.init", 0);
	}

	void writeLabel(std::string label) {
		outf << "//label " + label + "\n"
			<< "(" + label + ")\n";
	}

	void writeGoto(std::string label) {
		outf << "//goto " + label + "\n"
			<< "@" + label + "\n"
			<< "0;JMP\n";
	}

	void writeIf(std::string label) {
		outf << "//if-goto " + label + "\n"
			<< "@SP\n"
			<< "M=M-1\n"
			<< "A=M\n"
			<< "D=M\n"
			<< "@" + label + "\n"
			<< "D;JNE\n";
	}

	void writeCall(std::string functionName, int numArgs) {
		outf << "// call " + functionName + " "+ std::to_string(numArgs) + "\n"
			//push return-address
			<< "@" + functionName + std::to_string(retNum) + "\n"
			<< "D=A\n"
			<< "@SP\n"
			<< "A=M\n"
			<< "M=D\n"
			<< "@SP\n"
			<< "M=M+1\n"
			//push LCL
			<< "@LCL\n"
			<< "D=M\n"
			<< "@SP\n"
			<< "A=M\n"
			<< "M=D\n"
			<< "@SP\n"
			<< "M=M+1\n"
			//push ARG
			<< "@ARG\n"
			<< "D=M\n"
			<< "@SP\n"
			<< "A=M\n"
			<< "M=D\n"
			<< "@SP\n"
			<< "M=M+1\n"
			//push THIS
			<< "@THIS\n"
			<< "D=M\n"
			<< "@SP\n"
			<< "A=M\n"
			<< "M=D\n"
			<< "@SP\n"
			<< "M=M+1\n"
			//push THAT
			<< "@THAT\n"
			<< "D=M\n"
			<< "@SP\n"
			<< "A=M\n"
			<< "M=D\n"
			<< "@SP\n"
			<< "M=M+1\n"
			// ARG = SP-n-5
			<< "@" + std::to_string(numArgs + 5) + "\n"
			<< "D=A\n"
			<< "@SP\n"
			<< "D=M-D\n"
			<< "@ARG\n"
			<< "M=D\n"
			// LCL = SP
			<< "@SP\n"
			<< "D=M\n"
			<< "@LCL\n"
			<< "M=D\n"
			//goto f
			<< "@" + functionName + "\n"
			<< "0;JMP\n"
			//(return-address)
			<< "(" + functionName + std::to_string(retNum) + ")\n";
		retNum++;
	}

	void writeReturn() {
		outf << "// return\n"
			// FRAME = LCL //FRAME is a temporary variable
			<< "@LCL\n"
			<< "D=M\n"
			<< "@tmp\n"
			<< "M=D\n"
			// RET = *(FRAME-5)	// Put the return-address in a temp.var
			<< "@5\n"
			<< "D=A\n"
			<< "@tmp\n"
			<< "D=M-D\n"
			<< "A=D\n"
			<< "D=M\n"
			<< "@ret\n"
			<< "M=D\n"
			// *ARG = pop()
			<< "@SP\n"
			<< "M=M-1\n"
			<< "A=M\n"
			<< "D=M\n"
			<< "@ARG\n"
			<< "A=M\n"
			<< "M=D\n"
			// SP = ARG + 1
			<< "@ARG\n"
			<< "D=M+1\n"
			<< "@SP\n"
			<< "M=D\n"
			// THAT = *(FRAME-1)
			<< "@tmp\n"
			<< "D=M-1\n"
			<< "A=D\n"
			<< "D=M\n"
			<< "@THAT\n"
			<< "M=D\n"
			// THIS = *(FRAME-2)
			<< "@2\n"
			<< "D=A\n"
			<< "@tmp\n"
			<< "D=M-D\n"
			<< "A=D\n"
			<< "D=M\n"
			<< "@THIS\n"
			<< "M=D\n"
			// ARG = *(FRAME-3)
			<< "@3\n"
			<< "D=A\n"
			<< "@tmp\n"
			<< "D=M-D\n"
			<< "A=D\n"
			<< "D=M\n"
			<< "@ARG\n"
			<< "M=D\n"
			// LCL = *(FRAME-4)
			<< "@4\n"
			<< "D=A\n"
			<< "@tmp\n"
			<< "D=M-D\n"
			<< "A=D\n"
			<< "D=M\n"
			<< "@LCL\n"
			<< "M=D\n"
			// goto RET
			<< "@ret\n"
			<< "A=M\n"
			<< "0;JMP\n";
	}

	void writeFunction(std::string functionName, int numLocals) {
		outf << "// function " + functionName + " " + std::to_string(numLocals) + "\n"
			<< "(" + functionName + ")\n";
		for (int i = 0; i < numLocals; i++) {
			outf << "@" + std::to_string(i) + "\n"
				<< "D=A\n"
				<< "@LCL\n"
				<< "A=D+M\n"
				<< "M=0\n"
				<< "@SP\n"
				<< "M=M+1\n";
		}
	}

	void writeArithmetic(std::string command) {
		if (command == "add") {
			outf << "// add\n"
				<< "@SP\n"
				<< "M=M-1\n"
				<< "A=M\n"
				<< "D=M\n"
				<< "@SP\n"
				<< "M=M-1\n"
				<< "A=M\n"
				<< "M=D+M\n"
				<< "@SP\n"
				<< "M=M+1\n";
			return;
		}

		if (command == "sub") {
			outf << "// sub\n"
				<< "@SP\n"
				<< "M=M-1\n"
				<< "A=M\n"
				<< "D=M\n"
				<< "@SP\n"
				<< "M=M-1\n"
				<< "A=M\n"
				<< "M=M-D\n"
				<< "@SP\n"
				<< "M=M+1\n";
			return;
		}

		if (command == "neg") {
			outf << "// neg\n"
				<< "@SP\n"
				<< "M=M-1\n"
				<< "A=M\n"
				<< "M=-M\n"
				<< "@SP\n"
				<< "M=M+1\n";
			return;
		}

		if (command == "eq") {
			outf << getCompCode(command, "JEQ");
			return;
		}

		if (command == "gt") {
			outf << getCompCode(command, "JGT");
			return;
		}

		if (command =="lt") {
			outf << getCompCode(command,"JLT");
			return;
		}

		if (command == "and") {
			outf << getBoolCode(command, "&");
			return;
		}

		if (command == "or") {
			outf << getBoolCode(command, "|");
			return;
		}

		if (command == "not") {
			outf << "// not\n"
				<< "@SP\n"
				<< "M=M-1\n"
				<< "A=M\n"
				<< "M=!M\n"
				<< "@SP\n"
				<< "M=M+1\n";
			return;
		}
	}

	void writerPushPop(std::string command, std::string segment, int index){

		if (command == CommandType::C_PUSH) {
			outf << "//push " + segment + " " + std::to_string(index) + "\n";
		}

		if (command == CommandType::C_POP) {
			outf << "//pop " + segment + " " + std::to_string(index) + "\n";
		}

		if (segment == "local" || segment == "argument" || segment == "this" || segment == "that") {
			std::string label = label_map[segment];
			if (command == CommandType::C_PUSH) {
				outf << getNormalPushCode(label,index);
				return;
			}

			if (command == CommandType::C_POP) {
				outf << getNormalPopCode(label, index);
				return;
			}
		}

		if (segment == "static") {
			if (command == CommandType::C_PUSH) {
				outf << "@" + m_fileName + "." + std::to_string(index) + "\n"
					<< "D=M\n"
					<< "@SP\n"
					<< "A=M\n"
					<< "M=D\n"
					<< "@SP\n"
					<< "M=M+1\n";
			}
			else {
				outf << "@SP\n"
					<< "M=M-1\n"
					<< "A=M\n"
					<< "D=M\n"
					<< "@" + m_fileName + "." + std::to_string(index) + "\n"
					<< "M=D\n";
			}
		}

		if (segment == "constant") {
			if (command == CommandType::C_PUSH) {
				outf << "@" + std::to_string(index) + "\n"
					<< "D=A\n"
					<< "@SP\n"
					<< "A=M\n"
					<< "M=D\n"
					<< "@SP\n"
					<< "M=M+1\n";
			}
		}

		if (segment == "pointer") {
			if (command == CommandType::C_PUSH) {
				outf << std::string("@") + (index == 0 ? "THIS" : "THAT") + "\n"
					<< "D=M\n"
					<< "@SP\n"
					<< "A=M\n"
					<< "M=D\n"
					<< "@SP\n"
					<< "M=M+1\n";
			}
			else {
				outf << "@SP\n"
					<< "M=M-1\n"
					<< "A=M\n"
					<< "D=M\n"
					<< std::string("@") + (index == 0 ? "THIS" : "THAT") + "\n"
					<< "M=D\n";
			}
		}

		if (segment == "temp") {
			if (command == CommandType::C_PUSH) {
				outf << "@" + std::to_string(index) + "\n"
					<< "D=A\n"
					<< "@5\n"
					<< "D=D+A\n"
					<< "A=D\n"
					<< "D=M\n"
					<< "@SP\n"
					<< "A=M\n"
					<< "M=D\n"
					<< "@SP\n"
					<< "M=M+1\n";
			}
			else {
				outf << "@" + std::to_string(index) + "\n"
					<< "D=A\n"
					<< "@5\n"
					<< "D=D+A\n"
					<< "@tmp\n"
					<< "M=D\n"
					<< "@SP\n"
					<< "M=M-1\n"
					<< "A=M\n"
					<< "D=M\n"
					<< "@tmp\n"
					<< "A=M\n"
					<< "M=D\n";
			}
		}
	}

	void close(){
		if (outf) {
			outf.close();
		}
	}
};

std::vector<std::string> split(const std::string& s, char delimiter)
{
	std::vector<std::string> tokens;
	std::string token;
	std::istringstream tokenStream(s);
	while (std::getline(tokenStream, token, delimiter))
	{
		tokens.push_back(token);
	}
	return tokens;
}

/*
现在路径解析放在translate中做
path有可能是一个文件名（包含路径），该文件名不一定包含文件后缀,当不包含后缀时就当作文件夹路径处理
	也有可能是一个文件夹路径,如果路径不存在，则说明这是一个不含后缀名的文件
*/
void translate(std::string path) {

	std::vector<std::string> paths;		//这个是要解析的vm的文件名列表

	std::vector<std::string> results = split(path, '.');

	bool isPath{ false };

	//只是一个文件而已
	if (results.size() == 2) {
		std::cout << "这是一个文件" << std::endl;
		path = results[0];
		paths.push_back(path);
	}else if(results.size() == 1){
		std::cout << "这是一个路径" << std::endl;
		std::experimental::filesystem::path p{path};
		if (std::experimental::filesystem::exists(p)) {	//如果路径存在
			isPath = true;
			std::cout << "路劲存在" << std::endl;
			for (auto & tmp : std::experimental::filesystem::directory_iterator(p)) {
				std::vector<std::string> tmpResults = split(tmp.path().string(), '.');
				if (tmpResults[1] == "vm") {
					paths.push_back(tmpResults[0]);
				}
			}
		}
		else {
			std::cout << "路径不存在,当作文件处理" << std::endl;
			path = results[0];
			paths.push_back(path);
		}
	}
	else {
		std::cout << "出现了错误" << std::endl;
		exit(0);
	}


	std::string finalPath{path};
	if (isPath) {
		results = split(path, '\\');
		finalPath = path + "/" + results[results.size() - 1];
	}
	CodeWriter cw(finalPath);

	if (isPath) {
		cw.writeInit();
	}

	for (std::string filePath : paths) {
		std::vector <std::string> results{ split(filePath, '/') };
		std::string fileName = results[results.size()-1];
		cw.setFileName(fileName);
		Parser p(filePath, fileName);
		while (p.hasMoreCommands()){
			std::string commandType = p.commandType();
			std::string arg1;
			int arg2;

			if (commandType == CommandType::C_ARITHMETIC) {
				arg1 = p.arg1();
				cw.writeArithmetic(arg1);
			}

			if (commandType == CommandType::C_PUSH || commandType == CommandType::C_POP) {
				arg1 = p.arg1();
				arg2 = p.arg2();
				cw.writerPushPop(commandType, arg1, arg2);
			}

			if (commandType == CommandType::C_LABEL) {
				arg1 = p.arg1();
				cw.writeLabel(arg1);
			}

			if (commandType == CommandType::C_GOTO) {
				arg1 = p.arg1();
				cw.writeGoto(arg1);
			}

			if (commandType == CommandType::C_IF) {
				arg1 = p.arg1();
				cw.writeIf(arg1);
			}

			if (commandType == CommandType::C_CALL) {
				arg1 = p.arg1();
				arg2 = p.arg2();
				cw.writeCall(arg1, arg2);
			}

			if (commandType == CommandType::C_FUNCTION) {
				arg1 = p.arg1();
				arg2 = p.arg2();
				cw.writeFunction(arg1, arg2);
			}

			if (commandType == CommandType::C_RETURN) {
				cw.writeReturn();
			}
		}
	}
}


int main(int argc, char* argv[]) {
	if(argc != 2) {
		std::cerr << "Params Error" << std::endl;
		exit(1);
	}

	std::string path = argv[1];
	std::cout << path << std::endl;
	translate(path);

	return 0;
}
