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


class JackTokenizer {
private:
	std::string m_path;
	std::ifstream inf;
	std::ofstream outXML;
	std::ifstream inXML;
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
	JackTokenizer(std::string path) :m_path{ path }, inf{ path + ".jack" }, outXML{path + "T.xml"} {
		if(!inf) {
			std::cout << "Open " + path + ".jack Failed!" << std::endl;
			exit(1);
		}
		else {
			std::cout << "Open " + path + ".jack Succeed!" << std::endl;
		}

		if (!outXML) {
			std::cout << "Open " + path + "T.xml Failed!" << std::endl;
			exit(1);
		}
		else {
			std::cout << "Open " + path + "T.xml Succeed!" << std::endl;
		}
		initData();
	};

	~JackTokenizer(){
		if (inf) {
			inf.close();
		}

		if (outXML) {
			outXML.close();
		}

		if (inXML) {
			inXML.close();
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

		outXML.close();
		inXML.open(m_path + "T.xml");
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
		if (outXML) {
			outXML << info + "\n";
		}
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
	std::ofstream outf;
	JackTokenizer &m_tokenizer;
	std::vector<std::string> ops;
	std::vector<std::string> unary_ops;

public:
	/*
		Creates a new compilation engine with the given input and output.
		The next routine called must be compileClass().
	*/
	CompilationEngine(std::string path, JackTokenizer &tokenizer) : m_path{ path }, m_tokenizer{ tokenizer }, outf{path + ".xml"}
	{
		if (!outf) {
			std::cout << "Open " + path + ".xml Failed!" << std::endl;
			exit(1);
		}
		else {
			std::cout << "Open " + path + ".xml Succeed!" << std::endl;
		}
		initData();
	};

	void initData() {
		ops = {"+", "-", "*", "/", "&amp;", "|", "&lt;", "&gt;", "="};
		unary_ops = {"-", "~"};
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
		writeXML("<class>");
		m_tokenizer.advance();
		writeToken();	// 'class'
		m_tokenizer.advance();
		writeToken();	// 'className'
		m_tokenizer.advance();
		writeToken();	// '{'

		while (true) {
			m_tokenizer.advance();
			if (m_tokenizer.tokenValue() == "static" || m_tokenizer.tokenValue() == "field") {
				m_tokenizer.backward();
				compileClassVarDec();
			}else if (m_tokenizer.tokenValue() == "constructor" || m_tokenizer.tokenValue() == "function" || m_tokenizer.tokenValue() == "method") {
				m_tokenizer.backward();
				compileSubroutineDec();
			}
			else {
				m_tokenizer.backward();
				break;
			}
		}
		m_tokenizer.advance();
		writeToken();	// '}'
		writeXML("</class>");
	};

	/*
		Compiles a static declaration or a field declaration.
	*/
	void compileClassVarDec() {
		writeXML("<classVarDec>");
		m_tokenizer.advance();
		writeToken();	// ('static' | 'field')
		m_tokenizer.advance();
		writeToken();	// type
		m_tokenizer.advance();
		writeToken();	// varName
		while (true) {
			m_tokenizer.advance();
			if (m_tokenizer.tokenValue() == ",") {
				writeToken();	// ','
				m_tokenizer.advance();
				writeToken();	// varName
			}
			else {
				break;
			}
		}
		writeToken();	// ';'
		writeXML("</classVarDec>");
	};

	

	/*
		Compiles a (possibly empty) parameter list, not including the enclosing "()".
	*/
	void compileParameterList() {
		writeXML("<parameterList>");
		m_tokenizer.advance();
		if (m_tokenizer.tokenValue() == ")") {
			writeXML("</parameterList>");
			m_tokenizer.backward();
			return;
		}
		writeToken();	// type
		m_tokenizer.advance();
		writeToken();	// varName
		while (true) {
			m_tokenizer.advance();
			if (m_tokenizer.tokenValue() == ",") {
				writeToken();	// ','
				m_tokenizer.advance();
				writeToken();	// type
				m_tokenizer.advance();
				writeToken();	// varName
			}
			else {
				m_tokenizer.backward();
				break;
			}
		}
		writeXML("</parameterList>");
	};

	void compileSubroutineBody() {
		writeXML("<subroutineBody>");
		m_tokenizer.advance();
		writeToken();	// '{'
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
		compileStatements();
		m_tokenizer.advance();
		writeToken();	// '}'
		writeXML("</subroutineBody>");
	};

	/*
	Compiles a complete method, function, or constructor.
	*/
	void compileSubroutineDec() {
		writeXML("<subroutineDec>");
		m_tokenizer.advance();
		writeToken();	// ('constructor' | 'function' | 'method')
		m_tokenizer.advance();
		writeToken();	//('void' | type)
		m_tokenizer.advance();
		writeToken();	// subroutineName
		m_tokenizer.advance();
		writeToken();	// '('
		compileParameterList();
		m_tokenizer.advance();
		writeToken();	// ')'
		compileSubroutineBody();
		writeXML("</subroutineDec>");
	};

	/*
		Compiles a var declaration.
	*/
	void compileVarDec() {
		writeXML("<varDec>");
		m_tokenizer.advance();
		writeToken();	// 'var'
		m_tokenizer.advance();
		writeToken();	// type
		m_tokenizer.advance();
		writeToken();	// varName
		while (true) {
			m_tokenizer.advance();
			if (m_tokenizer.tokenValue() == ",") {
				writeToken();	// ','
				m_tokenizer.advance();
				writeToken();	// varName
			}
			else {
				break;
			}
		}
		writeToken();	// ';'
		writeXML("</varDec>");
	};

	/*
		Compiles a sequence of statements, not including the enclosing "{}".
	*/
	void compileStatements() {
		writeXML("<statements>");
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
		writeXML("</statements>");
	};

	/*
		Compiles a do statement.
	*/
	void compileDo() {
		writeXML("<doStatement>");
		m_tokenizer.advance();
		writeToken();	// 'do'
		compileSubroutineCall();
		m_tokenizer.advance();
		writeToken();	// ';'
		writeXML("</doStatement>");
	};

	/*
		no mark-up
	*/
	void compileSubroutineCall() {
		m_tokenizer.advance();
		writeToken();	// subroutineName | className | varName
		m_tokenizer.advance();
		if (m_tokenizer.tokenValue() == "(") {
			writeToken();	// '('
			compileExpressionList();
			m_tokenizer.advance();
			writeToken();	// ')'
		}
		else if (m_tokenizer.tokenValue() == ".") {
			writeToken();	// '.'
			m_tokenizer.advance();
			writeToken();	// subroutineName
			m_tokenizer.advance();
			writeToken();	// '('
			compileExpressionList();
			m_tokenizer.advance();
			writeToken();	// ')'
		}
	};

	/*
		Compiles a let statement.
	*/
	void compileLet() {
		writeXML("<letStatement>");
		m_tokenizer.advance();
		writeToken();	// 'let'
		m_tokenizer.advance();
		writeToken();	// varName
		m_tokenizer.advance();
		if (m_tokenizer.tokenValue() == "[") {
			writeToken();	// '['
			compileExpression();
			m_tokenizer.advance();
			writeToken();	// ']'
		}
		else {
			m_tokenizer.backward();
		}
		m_tokenizer.advance();
		writeToken();	// '='
		compileExpression();
		m_tokenizer.advance();
		writeToken();	// ';'
		writeXML("</letStatement>");
	};

	/*
		Compiles a while statement.
	*/
	void compileWhile() {
		writeXML("<whileStatement>");
		m_tokenizer.advance();
		writeToken();	// 'while'
		m_tokenizer.advance();
		writeToken();	// '('
		compileExpression();
		m_tokenizer.advance();
		writeToken();	// ')'
		m_tokenizer.advance();
		writeToken();	// '{'
		compileStatements();
		m_tokenizer.advance();
		writeToken();	// '}'
		writeXML("</whileStatement>");
	};

	/*
		Compiles a return statement.
	*/
	void compileReturn() {
		writeXML("<returnStatement>");
		m_tokenizer.advance();
		writeToken();	// 'return'
		m_tokenizer.advance();
		if (m_tokenizer.tokenValue() == ";") {
			writeToken();	// ';'
		}
		else {
			m_tokenizer.backward();
			compileExpression();
			m_tokenizer.advance();
			writeToken();	// ';'
		}
		writeXML("</returnStatement>");
	};

	/*
		Compiles an if statement, possibly with a trailing else clause.
	*/
	void compileIf() {
		writeXML("<ifStatement>");
		m_tokenizer.advance();
		writeToken();	// 'if'
		m_tokenizer.advance();
		writeToken();	// '('
		compileExpression();
		m_tokenizer.advance();
		writeToken();	// ')'
		m_tokenizer.advance();
		writeToken();	// '{'
		compileStatements();
		m_tokenizer.advance();
		writeToken();	// '}'
		m_tokenizer.advance();
		if (m_tokenizer.tokenValue() == "else") {
			writeToken();	// 'else'
			m_tokenizer.advance();
			writeToken();	// '{'
			compileStatements();
			m_tokenizer.advance();
			writeToken();	// '}'
		}
		else {
			m_tokenizer.backward();
		}
		writeXML("</ifStatement>");
	};

	bool is_op(const std::string &s) {
		if (std::find(ops.begin(), ops.end(), s) != ops.end()) {
			return true;
		}
		return false;
	}

	/*
	Compiles a (possibly empty) comma-separated list of expressions.
	*/
	void compileExpressionList() {
		writeXML("<expressionList>");
		m_tokenizer.advance();
		if (m_tokenizer.tokenValue() == ")") {
			m_tokenizer.backward();
			writeXML("</expressionList>");
			return;
		}
		m_tokenizer.backward();
		compileExpression();
		while (true) {
			m_tokenizer.advance();
			if (m_tokenizer.tokenValue() == ",") {
				writeToken();	// ','
				compileExpression();
			}
			else {
				m_tokenizer.backward();
				break;
			}
		}
		writeXML("</expressionList>");
	};

	/*
		Compilers an expression.
	*/
	void compileExpression() {
		writeXML("<expression>");
		compileTerm();
		while (true) {
			m_tokenizer.advance();
			if (is_op(m_tokenizer.tokenValue())) {
				writeToken(); // op
				compileTerm();
			}
			else {
				m_tokenizer.backward();
				break;
			}
		}
		writeXML("</expression>");
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
		writeXML("<term>");
		m_tokenizer.advance();
		//if is expression
		if (m_tokenizer.tokenValue() == "(") {
			writeToken();	// '('
			compileExpression();
			m_tokenizer.advance();
			writeToken();	// ')'
		}else if (is_unaryOp(m_tokenizer.tokenValue())) {
			writeToken();
			compileTerm();
		}
		else {
			if (m_tokenizer.forward() == "[") {
				writeToken();
				m_tokenizer.advance();
				writeToken();	// '['
				compileExpression();
				m_tokenizer.advance();
				writeToken();	// ']'
			}
			else if (m_tokenizer.forward() == "(" || m_tokenizer.forward() == ".") {
				m_tokenizer.backward();
				compileSubroutineCall();
			}
			else {
				writeToken();
			}
		}
		writeXML("</term>");
	};

	void writeToken() {
		writeXML("<" + m_tokenizer.tokenType() + ">" + m_tokenizer.tokenValue() + "</" + m_tokenizer.tokenType() + ">");
	}

	void writeXML(std::string s) {
		if (outf) {
			outf << s << "\n";
		}
	}
};

class JackAnalyzer {
private:
	std::string m_path;
public:
	JackAnalyzer(std::string path) :m_path{ path } {
	};

	void analyzer() {
		//首先判断是一个文件还是一个目录
		std::vector<std::string> results = split(m_path, '.');

		if (results[results.size() - 1] == "jack") {
			//这是一个文件
			m_path.erase(m_path.size() - 5); //erase ".jack"
			//std::cout << m_path << std::endl;
			JackTokenizer tokenizer{ m_path };
			tokenizer.tokenize();
			CompilationEngine analyzer{ m_path, tokenizer };
			analyzer.parse();
		}else{
			//这是一个目录
			analyzeMultFiles();
		}
	};

	void analyzeMultFiles() {
		//std::cout << m_path << std::endl;
		std::experimental::filesystem::path path{m_path};
		if (std::experimental::filesystem::exists(path)) {	//如果路径存在
			for (auto & tmp : std::experimental::filesystem::directory_iterator(path)) {
				//std::cout << tmp.path().string() << std::endl;
				std::vector<std::string> tmpResults = split(tmp.path().string(), '.');
				if (tmpResults[tmpResults.size() - 1] == "jack") {
					std::string pathName = tmp.path().string();
					pathName.erase(pathName.size() - 5);
					JackTokenizer tokenizer{ pathName };
					tokenizer.tokenize();
					CompilationEngine analyzer{ pathName,tokenizer };
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

	JackAnalyzer analyzer{argv[1]};
	analyzer.analyzer();
	return 0;
}
