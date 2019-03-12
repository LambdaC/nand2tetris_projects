#include<iostream>
#include<string>
#include<fstream>
#include<vector>
#include<sstream>
#include<experimental/filesystem>
#include<ios>
#include<cstdlib>
#include<cctype>
#include<map>
#include<algorithm>
#include<iterator>
#include<list>
#include<tuple>

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

// trim from start (in place)
void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
        return !std::isspace(ch);
    }));
}

// trim from end (in place)
void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}
// trim from both ends (in place)
void trim(std::string &s) {
    ltrim(s);
    rtrim(s);
}

class SymbolTable {
private:
	int staticIndex;
	int fieldIndex;
	int varIndex;
	int argumentIndex;
	std::map<std::string, std::tuple<std::string, std::string, int>> m_classSymbolTable;
	std::map<std::string, std::tuple<std::string, std::string, int>> m_varSymbolTable;
public:
	/*
		Creates a new empty symbol table.
	*/
	SymbolTable() {
		staticIndex = 0;
		fieldIndex = 0;
		varIndex = 0;
		argumentIndex = 0;
	};

	/*
		Starts a new subroutine scope
		(i.e., resets the subroutine's symbol table).
	*/
	void startSubroutine() {
		varIndex = 0;
		argumentIndex = 0;
		m_varSymbolTable.clear();
	};

	/*
		Defines a new identifier of a given name, type, and kind and assigns it a running index.
		STATIC and FIELD identifiers have a class scope, while ARG and VAR identifiers have a subroutine scope.
	*/
	void define(std::string name, std::string type, std::string kind) {
		if (kind == "static") {
			m_classSymbolTable[name] = std::make_tuple(type, kind, staticIndex++);
		}else if(kind == "field"){
			m_classSymbolTable[name] = std::make_tuple(type, kind, fieldIndex++);
		}else if(kind == "argument"){
			m_varSymbolTable[name] = std::make_tuple(type, kind, argumentIndex++);
		}
		else {
			m_varSymbolTable[name] = std::make_tuple(type, kind, varIndex++);
		}
	};

	/*
		Returns the number of variables of the given kind already defined in the current scope.
	*/
	int varCount(std::string kind) {
		int result{ 0 };
		if (kind == "static" || kind == "field") {
			for (auto& kv : m_classSymbolTable) {
				if (std::get<1>(kv.second) == kind) {
					result++;
				}
			}
		}
		else {
			for (auto& kv : m_varSymbolTable) {
				if (std::get<1>(kv.second) == kind) {
					result++;
				}
			}
		}
		return result;
	};

	/*
		Returns the kind of the named identifier in the current scope.
		If the identifier is unknown in the current scope, returns NONE.
		virtual segment
	*/
	std::string kindOf(std::string name) {
		auto search = m_varSymbolTable.find(name);
		if (search != m_varSymbolTable.end()) {
			return std::get<1>(search->second);
		}
		search = m_classSymbolTable.find(name);
		if (search != m_classSymbolTable.end()) {
			return std::get<1>(search->second);
		}
		return "";
	};

	/*
		Returns the type of the named identifier in the current scope.
	*/
	std::string typeOf(std::string name) {
		auto search = m_varSymbolTable.find(name);
		if (search != m_varSymbolTable.end()) {
			return std::get<0>(search->second);
		}
		search = m_classSymbolTable.find(name);
		if (search != m_classSymbolTable.end()) {
			return std::get<0>(search->second);
		}
		return "";
	};

	/*
		Returns the index assigned to the named identifier.
	*/
	int indexOf(std::string name) {
		auto search = m_varSymbolTable.find(name);
		if (search != m_varSymbolTable.end()) {
			return std::get<2>(search->second);
		}
		search = m_classSymbolTable.find(name);
		if (search != m_classSymbolTable.end()) {
			return std::get<2>(search->second);
		}
		return -1;
	}

	void printClassSymbolTable() {
		std::cout << "classSymbolTable:" << std::endl;
		for (auto &kv : m_classSymbolTable) {
			std::cout << kv.first << " " << std::get<0>(kv.second) << " " << std::get<1>(kv.second) << " " << std::get<2>(kv.second) << std::endl;
		}
	}
	
	void printVarSymbolTable() {
		std::cout << "varSymbolTabel:" << std::endl;
		for (auto &kv : m_varSymbolTable) {
			std::cout << kv.first << " " << std::get<0>(kv.second) << " " << std::get<1>(kv.second) << " " << std::get<2>(kv.second) << std::endl;
		}
	}
};


class VMWriter {
private:
	std::ofstream outf;
	std::string m_fileName;
	std::string m_path;
public:
	/*
		Creates a new file and prepares if for writing.
	*/
	VMWriter(std::string path, std::string fileName) :m_path{ path }, outf{ path + ".vm" }, m_fileName{ fileName } {
		if (!outf) {
			std::cout << "Open " + path + ".vm Failed!" << std::endl;
			exit(1);
		}
		else {
			std::cout << "Open " + path + ".vm Succeed!" << std::endl;
		}
	};

	/*
		Writes a VM push command.
	*/
	void writePush(std::string segment, int index) {
		outf << "push " + segment + " " + std::to_string(index) + "\n";
	};

	/*
		Writes a VM pop command.
	*/
	void writePop(std::string segment, int index) {
		outf << "pop " + segment + " " + std::to_string(index) + "\n";
	};

	/*
		Writes a VM arithmetic command.
	*/
	void writeArithmetic(std::string command) {
		outf << command << "\n";
	};

	/*
		Writes a VM label command.
	*/
	void writeLabel(std::string label) {
		outf << "label " + label << "\n";
	};

	/*
		Writes a VM goto command.
	*/
	void writeGoto(std::string label) {
		outf << "goto " + label + "\n";
	};

	/*
		Writes a VM if-goto command.
	*/
	void writeIf(std::string label) {
		outf << "if-goto " + label + "\n";
	};

	/*
		Writes a VM call command.
	*/
	void writeCall(std::string name, int nArgs) {
		outf << "call " + name + " " + std::to_string(nArgs) + "\n";
	};

	/*
		Writes a VM function command.
	*/
	void writeFunction(std::string name, int nArgs) {
		outf << "function " + name + " " + std::to_string(nArgs) + "\n";
	};

	/*
		Writes a VM return command.
	*/
	void writeReturn() {
		outf << "return\n";
	};

	/*
		Close the output file.
	*/
	void close() {};
};

class JackTokenizer {
private:
	std::string m_path;
	std::ifstream inf;
	std::map<std::string, std::string> symbol_map;
	std::vector<std::string> keywords;
	std::vector<std::string>  symbols;

	std::list<std::string> m_tokenValues;
	std::list<std::string> m_tokenTypes;
	std::string m_tokenType;
	std::string m_tokenValue;
public:
	/*
		Opens the input file/stream and gets ready to tokenize it.
	*/
	JackTokenizer(std::string path) :m_path{ path }, inf{ path + ".jack" } {
		if(!inf) {
			std::cout << "Open " + path + ".jack Failed!" << std::endl;
			exit(1);
		}
		else {
			std::cout << "Open " + path + ".jack Succeed!" << std::endl;
		}

		initData();
	};

	~JackTokenizer(){
		if (inf) {
			inf.close();
		}
	};

	void initData() {
		symbol_map = { {"<","&lt;"},{">","&gt;"},{"&","&amp;"} };
		keywords = { "class","constructor","function", "method", "field", "static",
					 "var", "int", "char", "boolean", "void", "true", "false",
					 "null", "this", "let", "do", "if", "else", "while", "return"};
		symbols = { "{", "}", "(", ")", "[", "]", ".", "," ,";", "+", "-", "*", 
					"/", "&", "|", "<", ">", "=","~"};
	}

	bool is_number(const std::string& s) {
		return !s.empty() && std::find_if(s.begin(),
			s.end(), [](char c) { return !std::isdigit(c); }) == s.end();
	}

	bool is_keyword(const std::string& s) {
		if (std::find(keywords.begin(), keywords.end(), s) != keywords.end()) {
			return true;
		}
		return false;
	}

	bool is_symbol(char c) {
		std::string s{c};
		if (std::find(symbols.begin(), symbols.end(), s) != symbols.end()) {
			return true;
		}
		return false;
	}

	bool is_identifier(const std::string &s){
		if(!std::isalpha(s[0]) && ! s[0] != '_') {
			return false;
		}
		for(auto c:s) {
			if(!std::isalnum(c) && c != '_') {
				return false;
			}
		}
		return true;
	}
	void tokenize() {
		writeXML("<tokens>");
		std::string line;
		bool isMultComment {false};		//用来记录是否正在多行注释中
		bool isStringing {false};		//用来记录是否正拼接字符串当中
		std::string token;
		while (inf) {
			//按行读取
			getline(inf, line);
			
			//判断是否是空行，是空行就跳过
			bool isAllSpace{ true };
			for (char c : line) {
				if (!std::isspace(c)) {
					isAllSpace = false;
					break;
				}
			}
			if (isAllSpace)
				continue;

			trim(line);

			//是不是单行注释
			if (line[0] == '/' && line[1] == '/') {
				continue;
			}

			//是否是多行注释
			if (line[0] == '/' && line[1] == '*' && !isMultComment) {
				isMultComment = true;
				//这里不continue是因为有可能多行注释在同一行就结束了
			}
			
			//简单处理多行注释
			if (isMultComment && !(line[line.size() - 2] == '*' && line[line.size() - 1] == '/')) {
				continue;
			}

			//先简单处理多行注释已结束吧
			if (isMultComment && line[line.size() - 2] == '*' && line[line.size() - 1] == '/') {
				isMultComment = false;
				continue;
			}

			//std::cout << line << std::endl;
			
			//一个字符一个字符地处理
			for(int i = 0; i < line.size(); i++){
				char c = line[i];
				//判断是否是字符串开始了
				if(c == '"' && !isStringing) {
					handleToken(token);
					isStringing = true;
					continue;
				}

				if(isStringing && c != '"'){
					token.push_back(c);
					continue;
				}

				if(isStringing && c == '"') {
					writeString(token);
					token.clear();
					isStringing = false;
					continue;
				}

				if (c == '/' && line[i+1] == '/') {
						handleToken(token);
						break;
				}

				if(is_symbol(c)){
					handleToken(token);
					writeSymbol(c);
					continue;
				}

				if(std::isspace(c)){
					handleToken(token);
					continue;
				}else{
					token.push_back(c);
				}
			}
		}
		writeXML("</tokens>");

	};

	void handleToken(std::string &token) {
		if(token.empty()){
			return;
		}
		if (is_keyword(token)) {
			writeKeyword(token);
		}else if (is_number(token)) {
			writeInteger(token);
		}else if (is_identifier(token)) {
			writeIdentifier(token);
		}
		token.clear();
	};

	void writeKeyword(std::string &keyWord) {
		m_tokenTypes.push_back("keyword");
		m_tokenValues.push_back(keyWord);
		writeXML("<keyword>" + keyWord + "</keyword>");
	};

	void writeSymbol(char c) {
		std::string symbol {c};
		auto search = symbol_map.find(symbol);
		if (search != symbol_map.end()) {
			m_tokenTypes.push_back("symbol");
			m_tokenValues.push_back(search->second);
			writeXML("<symbol>" + search->second + "</symbol>");
		}
		else {
			m_tokenTypes.push_back("symbol");
			m_tokenValues.push_back(symbol);
			writeXML("<symbol>" + symbol + "</symbol>");
		}
	};

	void writeInteger(std::string &integer) {
		m_tokenTypes.push_back("integerConstant");
		m_tokenValues.push_back(integer);
		writeXML("<integerConstant>" + integer + "</integerConstant>");
	};
	
	void writeString(std::string &s) {
		m_tokenTypes.push_back("stringConstant");
		m_tokenValues.push_back(s);
		writeXML("<stringConstant>" + s + "</stringConstant>");
	};

	void writeIdentifier(std::string &identifier) {
		m_tokenTypes.push_back("identifier");
		m_tokenValues.push_back(identifier);
		writeXML("<identifier>"+ identifier +"</identifier>");
	};

	void writeXML(std::string info) {
		//if (outXML) {
		//	outXML << info + "\n";
		//}
	};

	/*
		Do we have more tokens in the input?
	*/
	bool hasMoreTokens() {
	};

	/*
		Gets the next token from the input and makes it the current token.
		This method should only be called if hasMoreTokens() is true.
		Initially there is no current token.
	*/
	void advance() {
		if(!m_tokenValues.empty()) {
			m_tokenType = m_tokenTypes.front();
			m_tokenTypes.pop_front();
			m_tokenValue = m_tokenValues.front();
			m_tokenValues.pop_front();
		}
	};

	void backward() {
		m_tokenTypes.push_front(m_tokenType);
		m_tokenValues.push_front(m_tokenValue);
	}

	/*
		access the next token value, it won't change the currentToken
	*/
	std::string forward() {
		return m_tokenValues.front();
	}

	/*
		Returns the type of the current token.
	*/
	std::string tokenType() {
		return m_tokenType;
	};

	std::string tokenValue() {
		return m_tokenValue;
	}
	
	/*
		Returns the keyword which is the current token.
		Should be called only when tokenType() is KEYWORD.
	*/
	std::string keyWord() {
		return m_tokenValue;
	};
	
	/*
		Returns the character which is the current token.
		Should be called only when tokenType() is SYMBOL.
	*/
	std::string symbol() {
		return m_tokenValue;
	};

	/*
		Returns the identifier which is the current token.
		Should be called only when tokenType() is IDENTIFIER.
	*/
	std::string identifier() {
		return m_tokenValue;
	};

	/*
		Returns the integer value of the current token.
		Should be called only when tokenType() is INT_CONST.
	*/
	std::string intVal() {
		return m_tokenValue;
	};

	/*
		Returns the string value of the current token, without the double quotes.
		Should be called only when tokenType() is STRING_CONST.
	*/
	std::string stringVal() {
		return m_tokenValue;
	};

};

class CompilationEngine {
private:
	std::string m_path;
	std::string m_fileName;
	JackTokenizer &m_tokenizer;
	std::vector<std::string> ops;
	std::vector<std::string> unary_ops;
	SymbolTable m_symbolTable;
	VMWriter m_writer;
	std::map<std::string, std::string> op_map;
	std::map<std::string, std::string> unaryop_map;
	int m_labelIndex;

public:
	/*
		Creates a new compilation engine with the given input and output.
		The next routine called must be compileClass().
	*/
	CompilationEngine(std::string fileName, std::string path, JackTokenizer &tokenizer) : m_fileName{ fileName }, m_path{ path }, m_tokenizer{ tokenizer }, m_writer{path, fileName}
	{
		initData();
	};

	void initData() {
		m_labelIndex = 0;
		ops = {"+", "-", "*", "/", "&amp;", "|", "&lt;", "&gt;", "="};
		unary_ops = {"-", "~"};
		op_map["+"] = "add";
		op_map["-"] = "sub";
		op_map["*"] = "call Math.multiply 2";
		op_map["/"] = "call Math.divide 2";
		op_map["&amp;"] = "and";
		op_map["|"] = "or";
		op_map["&lt;"] = "lt";
		op_map["&gt;"] = "gt";
		op_map["="] = "eq";
		unaryop_map["-"] = "neg";
		unaryop_map["~"] = "not";
	}

	/*
		Start the compile process.
	*/
	void parse() {
		compileClass();
	}

	/*
		Compiles a complete class.
	*/
	void compileClass() { 
		m_tokenizer.advance();	// 'class'
		m_tokenizer.advance();	// 'className'
		m_tokenizer.advance();	// '{'

		while (true) {
			m_tokenizer.advance();
			if (m_tokenizer.tokenValue() == "static" || m_tokenizer.tokenValue() == "field") {
				m_tokenizer.backward();
				compileClassVarDec();
				//m_symbolTable.printClassSymbolTable();
			}else if (m_tokenizer.tokenValue() == "constructor" || m_tokenizer.tokenValue() == "function" || m_tokenizer.tokenValue() == "method") {
				m_tokenizer.backward();
				compileSubroutineDec();
			}
			else {
				m_tokenizer.backward();
				break;
			}
		}
		m_tokenizer.advance();	// '}'
	};

	/*
		Compiles a static declaration or a field declaration.
	*/
	void compileClassVarDec() {
		m_tokenizer.advance();	// ('static' | 'field')
		std::string kind{ m_tokenizer.tokenValue() };
		m_tokenizer.advance();	// type
		std::string type{ m_tokenizer.tokenValue() };
		m_tokenizer.advance();	// varName
		std::string name{ m_tokenizer.tokenValue() };
		m_symbolTable.define(name, type, kind);
		while (true) {
			m_tokenizer.advance();
			if (m_tokenizer.tokenValue() == ",") {	// ','
				m_tokenizer.advance();	// varName
				name = m_tokenizer.tokenValue();
				m_symbolTable.define(name, type, kind);
			}
			else {
				break;
			}
		}
		//writeToken();	// ';'
	};

	/*
	Compiles a complete method, function, or constructor.
	*/
	void compileSubroutineDec() {
		m_tokenizer.advance();	// ('constructor' | 'function' | 'method')
		std::string functionType{ m_tokenizer.tokenValue() };
		m_tokenizer.advance();	//('void' | type)
		m_tokenizer.advance();	// subroutineName
		std::string functionName{ m_tokenizer.tokenValue() };
		m_tokenizer.advance();	// '('
		compileParameterList(functionType);
		m_tokenizer.advance();	// ')'
		compileSubroutineBody(functionType, functionName);
	};

	/*
	Compiles a (possibly empty) parameter list, not including the enclosing "()".
	*/
	void compileParameterList(std::string functionType) {
		m_symbolTable.startSubroutine();
		m_tokenizer.advance();	// maybe ')' or type
		if (m_tokenizer.tokenValue() == ")") {
			m_tokenizer.backward();
			return;
		}
		if (functionType == "method") {
			m_symbolTable.define("this", m_fileName, "argument");
		}
		std::string kind{ "argument" };
		std::string type{ m_tokenizer.tokenValue() };
		m_tokenizer.advance();	// varName
		std::string name{ m_tokenizer.tokenValue() };
		m_symbolTable.define(name, type, kind);
		while (true) {
			m_tokenizer.advance();	// maybe ','
			if (m_tokenizer.tokenValue() == ",") {
				m_tokenizer.advance();	// type
				type = m_tokenizer.tokenValue();
				m_tokenizer.advance();	// varName
				name = m_tokenizer.tokenValue();
				m_symbolTable.define(name, type, kind);
			}
			else {
				m_tokenizer.backward();
				break;
			}
		}
	};

	void compileSubroutineBody(std::string functionType, std::string functionName) {
		m_tokenizer.advance();	// '{'
		while (true) {
			m_tokenizer.advance();
			if (m_tokenizer.tokenValue() == "var") {
				m_tokenizer.backward();
				compileVarDec();
			}
			else {
				m_tokenizer.backward();
				break;
			}
		}
		//m_symbolTable.printVarSymbolTable();
		m_writer.writeFunction(m_fileName + "." + functionName, m_symbolTable.varCount("local"));
		if (functionType == "constructor") {
			m_writer.writePush("constant", m_symbolTable.varCount("field"));
			m_writer.writeCall("Memory.alloc", 1);
			m_writer.writePop("pointer", 0);
		}
		else if (functionType == "method") {
			m_writer.writePush("argument", 0);
			m_writer.writePop("pointer", 0);
		}
		compileStatements();
		m_tokenizer.advance();	// '}'
	};
	
	/*
	Compiles a var declaration.
	*/
	void compileVarDec() {
		m_tokenizer.advance();	// 'var'
		std::string kind{ "local" };
		m_tokenizer.advance();	// type
		std::string type{ m_tokenizer.tokenValue() };
		m_tokenizer.advance();	// varName
		std::string name{ m_tokenizer.tokenValue() };
		m_symbolTable.define(name, type, kind);
		while (true) {
			m_tokenizer.advance();	// maybe ',' or ';':
			if (m_tokenizer.tokenValue() == ",") {
				m_tokenizer.advance();	// varName
				name = m_tokenizer.tokenValue();
				m_symbolTable.define(name, type, kind);
			}
			else {
				// ';'
				break;
			}
		}
	};

	/*
		Compiles a sequence of statements, not including the enclosing "{}".
	*/
	void compileStatements() {
		while (true) {
			m_tokenizer.advance();
			if (m_tokenizer.tokenValue() == "let") {
				m_tokenizer.backward();
				compileLet();
			}
			else if (m_tokenizer.tokenValue() == "if") {
				m_tokenizer.backward();
				compileIf();
			}
			else if (m_tokenizer.tokenValue() == "while") {
				m_tokenizer.backward();
				compileWhile();
			}
			else if (m_tokenizer.tokenValue() == "do") {
				m_tokenizer.backward();
				compileDo();
			}
			else if (m_tokenizer.tokenValue() == "return") {
				m_tokenizer.backward();
				compileReturn();
			}
			else {
				m_tokenizer.backward();
				break;
			}
		}
	};

	/*
		Compiles a do statement.
	*/
	void compileDo() {
		m_tokenizer.advance();	// 'do'
		compileSubroutineCall();
		m_writer.writePop("temp", 0);
		m_tokenizer.advance();	// ';'
	};

	/*
		Compiles a let statement.
	*/
	void compileLet() {
		bool isArr{ false };
		m_tokenizer.advance();	// 'let'
		m_tokenizer.advance();	// varName
		std::string name{ m_tokenizer.tokenValue() };
		m_tokenizer.advance();
		if (m_tokenizer.tokenValue() == "[") {	// '['
			if (m_symbolTable.kindOf(name) == "field") {
				m_writer.writePush("this", m_symbolTable.indexOf(name));
			}
			else {
				m_writer.writePush(m_symbolTable.kindOf(name), m_symbolTable.indexOf(name));
			}
			compileExpression();
			m_writer.writeArithmetic("add");
			m_tokenizer.advance();	// ']'
			m_tokenizer.advance();	// '='
			compileExpression();
			m_writer.writePop("temp", 0);
			m_writer.writePop("pointer", 1);
			m_writer.writePush("temp", 0);
			m_writer.writePop("that", 0);
			m_tokenizer.advance();	// ';'
		}
		else {
			m_tokenizer.backward();
			m_tokenizer.advance();	// '='
			compileExpression();
			if(m_symbolTable.kindOf(name) == "field"){
				m_writer.writePop("this", m_symbolTable.indexOf(name));
			}
			else {
				m_writer.writePop(m_symbolTable.kindOf(name), m_symbolTable.indexOf(name));
			}
			m_tokenizer.advance();	// ';'
		}
	};

	/*
		Compiles a while statement.
	*/
	void compileWhile() {
		int labelIndex1 = m_labelIndex++;
		int labelIndex2 = m_labelIndex++;
		m_tokenizer.advance();	// 'while'
		m_tokenizer.advance();	// '('
		m_writer.writeLabel("L" + std::to_string(labelIndex1));
		compileExpression();
		m_writer.writeArithmetic("not");
		m_writer.writeIf("L" + std::to_string(labelIndex2));
		m_tokenizer.advance();	// ')'
		m_tokenizer.advance();	// '{'
		compileStatements();
		m_writer.writeGoto("L" + std::to_string(labelIndex1));
		m_writer.writeLabel("L" + std::to_string(labelIndex2));
		m_tokenizer.advance();	// '}'
	};

	/*
		Compiles a return statement.
	*/
	void compileReturn() {
		m_tokenizer.advance();	// 'return'
		m_tokenizer.advance();	// maybe';'
		if (m_tokenizer.tokenValue() == ";") {
			m_writer.writePush("constant", 0);
			m_writer.writeReturn();
		}
		else {
			m_tokenizer.backward();
			compileExpression();
			m_writer.writeReturn();
			m_tokenizer.advance();	// ';'
		}
	};

	/*
		Compiles an if statement, possibly with a trailing else clause.
	*/
	void compileIf() {
		int labelIndex1 = m_labelIndex++;
		int labelIndex2 = m_labelIndex++;
		m_tokenizer.advance();	//'if'
		m_tokenizer.advance();	// '('
		compileExpression();
		m_tokenizer.advance();	// ')'
		m_writer.writeArithmetic("not");
		m_writer.writeIf("L" + std::to_string(labelIndex1));
		m_tokenizer.advance();	// '{'
		compileStatements();
		m_tokenizer.advance();	// '}'
		m_tokenizer.advance();	// maybe 'else'
		if (m_tokenizer.tokenValue() == "else") {
			m_writer.writeGoto("L" + std::to_string(labelIndex2));
			m_writer.writeLabel("L" + std::to_string(labelIndex1));
			m_tokenizer.advance();	// '{'
			compileStatements();
			m_tokenizer.advance();	// '}'
			m_writer.writeLabel("L" + std::to_string(labelIndex2));
		}
		else {
			m_writer.writeLabel("L" + std::to_string(labelIndex1));
			m_tokenizer.backward();
		}
	};

	bool is_op(const std::string &s) {
		if (std::find(ops.begin(), ops.end(), s) != ops.end()) {
			return true;
		}
		return false;
	}

	/*
		Compilers an expression.
	*/
	void compileExpression() {
		compileTerm();
		while (true) {
			m_tokenizer.advance();	// maybe "op"
			if (is_op(m_tokenizer.tokenValue())) {
				std::string op{ m_tokenizer.tokenValue() };
				compileTerm();
				m_writer.writeArithmetic(op_map[op]);
			}
			else {
				m_tokenizer.backward();
				break;
			}
		}
	};

	bool is_unaryOp(const std::string &s) {
		if (std::find(unary_ops.begin(), unary_ops.end(), s) != unary_ops.end()) {
			return true;
		}
		return false;
	}

	/*
		Compiles a term. This routine is faced with a slight difficulty when trying to decide between some of the alternative parsing rules.
		Specifically, if the current token is an identifier, the routine must distinguish between a variable, an array entry, and a subrontine call.
		A single look-ahead token, which may be one of "[", "(", or "." suffices to distinguish between the three possibilities.
		Any other token is not part of this term and should not be advanced over.
	*/
	void compileTerm() {
		m_tokenizer.advance();
		// ( expression )
		if (m_tokenizer.tokenValue() == "(") {	// '('
			compileExpression();
			m_tokenizer.advance();	// ')'
		// unaryOp term
		}else if (is_unaryOp(m_tokenizer.tokenValue())) {
			std::string op{ m_tokenizer.tokenValue() };
			compileTerm();
			m_writer.writeArithmetic(unaryop_map[op]);
		}
		else {
			// varName [ expression ] 
			if (m_tokenizer.forward() == "[") {
				std::string name{ m_tokenizer.tokenValue() };
				if (m_symbolTable.kindOf(name) == "field") {
					m_writer.writePush("this", m_symbolTable.indexOf(name));
				}
				else {
					m_writer.writePush(m_symbolTable.kindOf(name), m_symbolTable.indexOf(name));
				}
				m_tokenizer.advance();	// '['
				compileExpression();
				m_writer.writeArithmetic("add");
				m_writer.writePop("pointer", 1);
				m_writer.writePush("that", 0);
				m_tokenizer.advance(); // ']'
			}
			// subroutineCall
			else if (m_tokenizer.forward() == "(" || m_tokenizer.forward() == ".") {
				m_tokenizer.backward();
				compileSubroutineCall();
			}
			else {
				if (m_tokenizer.tokenType() == "integerConstant") {
					m_writer.writePush("constant", std::stoi(m_tokenizer.tokenValue()));
				}else if (m_tokenizer.tokenType() == "stringConstant") {
					std::string str{ m_tokenizer.tokenValue() };
					compileString(str);
				}else if (m_tokenizer.tokenValue() == "this") {
					m_writer.writePush("pointer", 0);
				}else if (m_tokenizer.tokenValue() == "true") {
					m_writer.writePush("constant", 1);
					m_writer.writeArithmetic("neg");
				}else if (m_tokenizer.tokenValue() == "false" || m_tokenizer.tokenValue() == "null") {
					m_writer.writePush("constant", 0);
				}
				else {
					std::string name{ m_tokenizer.tokenValue() };
					if (m_symbolTable.kindOf(name) == "field") {
						m_writer.writePush("this", m_symbolTable.indexOf(name));
					}
					else {
						m_writer.writePush(m_symbolTable.kindOf(name), m_symbolTable.indexOf(name));
					}
				}
			}
		}
	};

	void compileString(std::string str) {
		m_writer.writePush("constant", str.size());
		m_writer.writeCall("String.new", 1);
		for (auto c : str) {
			m_writer.writePush("constant", c);
			m_writer.writeCall("String.appendChar", 2);
		}
	};

	/*
	no mark-up
	*/
	void compileSubroutineCall() {
		m_tokenizer.advance();
		std::string callName{ m_tokenizer.tokenValue() };
		//writeToken();	// subroutineName | className | varName
		m_tokenizer.advance();	// maybe '(' or '.'
		if (m_tokenizer.tokenValue() == "(") {
			m_writer.writePush("pointer", 0);
			int nArgs = compileExpressionList();
			m_writer.writeCall(m_fileName + "." + callName, nArgs+1);
			m_tokenizer.advance();	// ')'
		}
		else if (m_tokenizer.tokenValue() == ".") {
			m_tokenizer.advance();	// subroutineName
			std::string functionName{ m_tokenizer.tokenValue() };
			m_tokenizer.advance();	//'('
			if (!m_symbolTable.kindOf(callName).empty()) {
				if (m_symbolTable.kindOf(callName) == "field") {
					m_writer.writePush("this", m_symbolTable.indexOf(callName));
				}
				else {
					m_writer.writePush(m_symbolTable.kindOf(callName), m_symbolTable.indexOf(callName));
				}
				int nArgs = compileExpressionList();
				m_writer.writeCall(m_symbolTable.typeOf(callName) + "." + functionName, nArgs+1);
			}
			else {
				int nArgs = compileExpressionList();
				m_writer.writeCall(callName + "." + functionName, nArgs);
			}
			m_tokenizer.advance();	// ')'
		}
	};

	/*
	Compiles a (possibly empty) comma-separated list of expressions.
	*/
	int compileExpressionList() {
		int nArgs{ 0 };
		m_tokenizer.advance();	// maybe ')'
		if (m_tokenizer.tokenValue() == ")") {
			m_tokenizer.backward();
			return nArgs;
		}
		m_tokenizer.backward();
		compileExpression();
		nArgs++;
		while (true) {
			m_tokenizer.advance();	// maybe ','
			if (m_tokenizer.tokenValue() == ",") {
				compileExpression();
				nArgs++;
			}
			else {
				m_tokenizer.backward();
				break;
			}
		}
		return nArgs;
	};

	void writeToken() {
		//writeXML("<" + m_tokenizer.tokenType() + ">" + m_tokenizer.tokenValue() + "</" + m_tokenizer.tokenType() + ">");
	}

	void writeXML(std::string s) {
		//if (outf) {
		//	outf << s << "\n";
		//}
	}
};

class JackCompiler {
private:
	std::string m_path;
public:
	JackCompiler(std::string path) :m_path{ path } {
	};

	void compile() {
		//首先判断是一个文件还是一个目录
		std::vector<std::string> results = split(m_path, '.');

		if (results[results.size() - 1] == "jack") {
			//这是一个文件
			m_path.erase(m_path.size() - 5); //erase ".jack"
			std::vector<std::string> tmp = split(m_path, '/');
			std::string fileName{ tmp[tmp.size() - 1] };
			//std::cout << m_path << std::endl;
			//std::cout << fileName << std::endl;
			JackTokenizer tokenizer{ m_path };
			tokenizer.tokenize();
			CompilationEngine analyzer{fileName, m_path, tokenizer };
			analyzer.parse();
		}else{
			//这是一个目录
			compileMultFiles();
		}
	};

	void compileMultFiles() {
		//std::cout << m_path << std::endl;
		std::experimental::filesystem::path path{m_path};
		if (std::experimental::filesystem::exists(path)) {	//如果路径存在
			for (auto & tmp : std::experimental::filesystem::directory_iterator(path)) {
				//std::cout << tmp.path().string() << std::endl;
				std::vector<std::string> tmpResults = split(tmp.path().string(), '.');
				if (tmpResults[tmpResults.size() - 1] == "jack") {
					std::string pathName = tmp.path().string();
					pathName.erase(pathName.size() - 5);
					std::vector<std::string> tmp = split(pathName, '/');
					std::string fileName{ tmp[tmp.size() - 1] };
					//std::cout << fileName << std::endl;
					JackTokenizer tokenizer{ pathName };
					tokenizer.tokenize();
					CompilationEngine analyzer{fileName, pathName,tokenizer };
					analyzer.parse();
				}
			}
		}
	};
};

int main(int argc, char* argv[]) {
	if (argc != 2) {
		std::cerr << "Params Error" << std::endl;
		exit(1);
	}

	JackCompiler compiler{argv[1]};
	compiler.compile();
	return 0;
}
