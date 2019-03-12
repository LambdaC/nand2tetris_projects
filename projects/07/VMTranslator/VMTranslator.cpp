#include<string>
#include<fstream>
#include<iostream>
#include<cstdlib>
#include<sstream>
#include<vector>
#include<iterator>
#include<map>

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

	bool hasMoreCommands() {
		if (!inf)
			return false;

		while (inf) {
			getline(inf, m_currCommand);
			if (m_currCommand.empty()) {
				continue;
			}
			if (m_currCommand[0] == '/' && m_currCommand[1] == '/'){
				continue;
			}
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
	CodeWriter(std::string path, std::string fileName) :m_fileName{fileName},outf { path + ".asm" }
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
		close();
		m_fileName = fileName;
		outf.open(fileName + ".asm", std::ios::app);
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

void translate(std::string path, std::string fileName) {
	Parser p(path, fileName);
	CodeWriter cw(path, fileName);

	while (p.hasMoreCommands()) {
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
	}
}

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
int main(int argc, char* argv[]) {
	if(argc != 2) {
		std::cerr << "Params Error" << std::endl;
		exit(1);
	}

	std::string path = argv[1];
	std::cout << path << std::endl;
	std::vector<std::string> results = split(path, '.');
	path = results[0];
	results = split(path, '/');
	std::string fileName{results[results.size()-1]};
	translate(path, fileName);

	return 0;
}
