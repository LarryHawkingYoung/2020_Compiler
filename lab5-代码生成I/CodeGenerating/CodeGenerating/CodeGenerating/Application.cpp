#include<iostream>
#include<string>
#include<fstream>
#include<sstream>
#include<map>
#include<vector>
#include"SymbolTable.h"
#include"ErrorProcessing.h"
#include"IntermediateCode.h"
#include"Function.h"

using namespace std;

ErrorProcessing errProc = ErrorProcessing();
IntermediateCode interCode = IntermediateCode();

ifstream fin("testfile.txt");
//ofstream fout("output.txt");
ofstream mout("mips.txt");

string line;

string strcon;
string idenfr;
string intcon;
char ch;
map<string, string> keptLabelMap;
Symbol symTab[10000];
int symTabTop = -1;
int subProcIndexTable[1000];
int levelNow = 0;

int funcRetFlag = -1;// -1:不在函数里 0：在无返回值函数里 1：在有返回值函数里
int hasRet = 0; // 当前带返回值的函数 是否 有return语句
int retType = -1; // 当前带返回值函数 的 返回值类型

struct FuncInfo
{
	int pos;
	int paramNum;
	int type;
	string funcName;
};

struct QuantityInfo
{
	int kind;
	int type;
};

struct Word
{
	string label;
	string idenfr;
	int lineID;
	bool isAtHead;
};

Word wordList[10000];
int top = 0;// wordList的长度
int wordCnt = 0;
Word wordNow;

int getNewIndex(int start, string str);
string toLowerCase(string in);
void TokenInit();
int getSym(bool forward, bool withOutput);
void insertWordList(int index, string label, string idenfr, int lineID, bool isAtHead);
void levelUp();
void levelDown(bool delAll = false);

void error(int lineID, string errType);
int checkDupDef(string idenfr, int lineID);
int checkUndefined(string idenfr, int lineID);
FuncInfo getFuncInfo(string idenfr);
QuantityInfo getQuantityInfo(string idenfr);

// 代码生成
unsigned int offsetTop = 0;
vector<Symbol> CodeGenSymTab;
struct typeAndValue
{
	int type;
	string value;
};
struct StrDef
{
	string idenfr;
	string content;
	unsigned int start;
	unsigned int len;
};
vector<StrDef> strVec;
unsigned int strOffTop = 0;
int strID = 1;
int mediaID = 1;
int labelID = 1;
string getNewLabel();
string getNewAns(int type, unsigned int size);
unsigned int roundUp(unsigned int offset);
unsigned int getStrSize(string s);
void targetCodeGenerating();
void initStr();
Symbol getSymbolInfo(string idenfr);
map<string, int> ArrCols;
map<string, Function> FuncMap;

Function curFunc;
bool isInFunc = false;
bool isFuncEnd = false;
bool isInMain = false;
bool isMainEnd = false;


string getType(int type)
{
	switch (type)
	{
	case 0: return "CHAR";
	case 1: return "INT";
	case 2: return "VOID";
	default:
		break;
	}
}

string getKind(int kind)
{
	switch (kind)
	{
	case 0: return "CONST";
	case 1: return "VAR";
	case 2: return "FUNC";
	case 3: return "PARAM";
	default:
		break;
	}
}

void showSymbolTable()
{
	cout << "idenfr\t" << "kind\t" << "type\t" << "level\t" << "dim" << endl;
	for (int i = 0; i <= symTabTop; i++) {
		Symbol sym = symTab[i];
		cout << sym.idenfr << "\t" << getKind(sym.kind) << "\t" << getType(sym.type) << "\t" << sym.level << "\t" << sym.dim << endl;
	}
}

void showCodeGenSymTab()
{
	cout << "idenfr\t" << "kind\t" << "type\t" << "level\t" << "offset" << endl;
	for (int i = 0; i < CodeGenSymTab.size(); i++) {
		Symbol sym = CodeGenSymTab[i];
		cout << sym.idenfr << "\t" << getKind(sym.kind) << "\t" << getType(sym.type) << "\t" << sym.level << "\t" << sym.addrOffset << endl;
	}
}

void showStrVec()
{
	cout << "idenfr\t" << "content\t" << "start\t" << "len" << endl;
	for (int i = 0; i < strVec.size(); i++)
	{
		StrDef str = strVec[i];
		cout << str.idenfr << "\t" << str.content << "\t" << str.start << "\t" << str.len << endl;
	}
}

#pragma region Grammar Parse Funcs Declare
void parseProgram();
void parseConstDcrpt();
void parseConstDef();
void parsePerConstDef(int type);
string parseInteger();
string parseChar();
string parseUnsignedInt();
typeAndValue parseConst();
void parseVarDcrpt();
void parseVarDef();
void parsePerVarDefNotInit(int type);
void parseFuncDef();
void parseParamTable();
int parseDeclareHead();
void parseCompStmt();
void parseStmtList();
void parseStmt();
void parseLoopStmt();
string parseCondition();
typeAndValue parseExpr();
typeAndValue parseTerm();
typeAndValue parseFactor();
string parseFuncCallWithReturn();
void parseFuncCallWithoutReturn();
void parseValueParamTable(FuncInfo funcInfo);
string parseStep();
void parseFuncCall();
void parseCondStmt();
void parseReadStmt();
void parseWriteStmt();
void parseSwitchStmt();
void parseCondTable(typeAndValue tav, string endLabel);
string parseCondChildStmt(typeAndValue tav, string endLabel);
void parseDefault();
void parseReturnStmt();
void parseAssignStmt();
void parseMainFunc();
void parseString();
#pragma endregion

int main()
{
	TokenInit();

	int lineID = 0;
	while (getline(fin, line))
	{
		lineID++;
		bool isAtHead = true;
		int ind = 0;
		while ((ind = getNewIndex(ind, line)) < line.length())
		{
			ch = line[ind];

			switch (ch)
			{
			case '\"':
				strcon.clear();
				ind++;
				while ((ch = line[ind]) != '\"')
				{
					if (ch != 32 && ch != 33 && (ch < 35 || ch > 126))
					{
						error(lineID, "a");
					}
					strcon = strcon + ch;
					ind++;
				}
				if (strcon == "") error(lineID, "a");
				insertWordList(top, "STRCON", strcon, lineID, isAtHead);
				isAtHead = false;
				top++;
				break;
			case '\'':
				ind++;
				if (line[ind] != '+' && line[ind] != '-' && line[ind] != '*' && line[ind] != '/' && line[ind] != '_' && !(line[ind] >= 'a' && line[ind] <= 'z' || line[ind] >= 'A' && line[ind] <= 'Z' || line[ind] >= '0' && line[ind] <= '9'))
				{
					error(lineID, "a");
				}
				insertWordList(top, "CHARCON", line.substr(ind, 1), lineID, isAtHead);
				isAtHead = false;
				top++;
				ind++;
				break;
			case '+':
				insertWordList(top, "PLUS", "+", lineID, isAtHead);
				isAtHead = false;
				top++;
				break;
			case '-':
				insertWordList(top, "MINU", "-", lineID, isAtHead);
				isAtHead = false;
				top++;
				break;
			case '*':
				insertWordList(top, "MULT", "*", lineID, isAtHead);
				isAtHead = false;
				top++;
				break;
			case '/':
				insertWordList(top, "DIV", "/", lineID, isAtHead);
				isAtHead = false;
				top++;
				break;
			case '<':
				switch (line[ind + 1])
				{
				case '=':
					insertWordList(top, "LEQ", "<=", lineID, isAtHead);
					isAtHead = false;
					top++;
					ind++;
					break;
				default:
					insertWordList(top, "LSS", "<", lineID, isAtHead);
					isAtHead = false;
					top++;
					break;
				}
				break;
			case '>':
				switch (line[ind + 1])
				{
				case '=':
					insertWordList(top, "GEQ", ">=", lineID, isAtHead);
					isAtHead = false;
					top++;
					ind++;
					break;
				default:
					insertWordList(top, "GRE", ">", lineID, isAtHead);
					isAtHead = false;
					top++;
					break;
				}
				break;
			case '=':
				switch (line[ind + 1])
				{
				case '=':
					insertWordList(top, "EQL", "==", lineID, isAtHead);
					isAtHead = false;
					top++;
					ind++;
					break;
				default:
					insertWordList(top, "ASSIGN", "=", lineID, isAtHead);
					isAtHead = false;
					top++;
					break;
				}
				break;
			case '!':
				insertWordList(top, "NEQ", "!=", lineID, isAtHead);
				isAtHead = false;
				top++;
				ind++;
				break;
			case ':':
				insertWordList(top, "COLON", ":", lineID, isAtHead);
				isAtHead = false;
				top++;
				break;
			case ';':
				insertWordList(top, "SEMICN", ";", lineID, isAtHead);
				isAtHead = false;
				top++;
				break;
			case ',':
				insertWordList(top, "COMMA", ",", lineID, isAtHead);
				isAtHead = false;
				top++;
				break;
			case '(':
				insertWordList(top, "LPARENT", "(", lineID, isAtHead);
				isAtHead = false;
				top++;
				break;
			case ')':
				insertWordList(top, "RPARENT", ")", lineID, isAtHead);
				isAtHead = false;
				top++;
				break;
			case '[':
				insertWordList(top, "LBRACK", "[", lineID, isAtHead);
				isAtHead = false;
				top++;
				break;
			case ']':
				insertWordList(top, "RBRACK", "]", lineID, isAtHead);
				isAtHead = false;
				top++;
				break;
			case '{':
				insertWordList(top, "LBRACE", "{", lineID, isAtHead);
				isAtHead = false;
				top++;
				break;
			case '}':
				insertWordList(top, "RBRACE", "}", lineID, isAtHead);
				isAtHead = false;
				top++;
				break;
			default:
				if (ch == '_' || ch >= 'a' && ch <= 'z' || ch >= 'A' && ch <= 'Z')
				{
					idenfr.clear();
					idenfr += ch;
					ch = line[ind + 1];
					while (ch == '_' || ch >= 'a' && ch <= 'z' || ch >= 'A' && ch <= 'Z' || ch >= '0' && ch <= '9')
					{
						idenfr += ch;
						ind++;
						ch = line[ind + 1];
					}
					map<string, string>::iterator iter = keptLabelMap.find(toLowerCase(idenfr));
					if (iter == keptLabelMap.end())
					{
						insertWordList(top, "IDENFR", idenfr, lineID, isAtHead);
						isAtHead = false;
						top++;
					}
					else
					{
						insertWordList(top, iter->second, idenfr, lineID, isAtHead);
						isAtHead = false;
						top++;
					}
				}
				else if (ch >= '0' && ch <= '9')
				{
					intcon.clear();
					intcon += ch;
					while (line[ind + 1] >= '0' && line[ind + 1] <= '9')
					{
						ind++;
						ch = line[ind];
						intcon += ch;
					}
					insertWordList(top, "INTCON", intcon, lineID, isAtHead);
					isAtHead = false;
					top++;
				}
				break;
			}

			ind++;
		}
	}

	subProcIndexTable[0] = 0;
	if (getSym(true, true))
	{
		parseProgram();
	}

	fin.close();
	//fout.close();

	cout << endl;

	showSymbolTable();

	cout << "\nTotal Error Number : " << errProc.getErrNum() << endl;

	cout << "\nfinished" << endl;

	cout << "\nCodeGenSymTab" << endl;

	showCodeGenSymTab();

	cout << "\nstrVec" << endl;

	showStrVec();

	errProc.outputErrors();
	interCode.outputQuats();

	//targetCodeGenerating();

	getchar();
	return 0;
}


#pragma region Target Code Generation
void targetCodeGenerating()
{
	initStr();
	mout << ".text" << endl;
	mout << "\tli $t0, 0x10010000" << endl;
	for (int i = 0; i < interCode.getQuatNum(); i++)
	{
		Quaternion quat = interCode.getQuaternion(i);
		if (quat.op == "read")
		{
			Symbol sym = getSymbolInfo(quat.ans);
			if (sym.type == INT) mout << "\tli $v0, 5" << endl;
			else if (sym.type == CHAR) mout << "\tli $v0, 12" << endl;
			mout << "\tsyscall" << endl;
			mout << "\tsw $v0, " << sym.addrOffset << "($t0)" << endl;
		}
		else if (quat.op == "print")
		{
			if (quat.x != "") // 有字符串
			{
				mout << "\tli $v0, 4" << endl;
				mout << "\tla $a0, " << quat.x << endl;
				mout << "\tsyscall" << endl;
			}
			if (quat.y != "") // 有表达式
			{
				if (quat.y[0] == '\'') // 字符常量
				{
					mout << "\tli $v0, 11" << endl;
					mout << "\tli $a0, " << quat.y << endl;
				}
				else if (quat.y[0] >= '0' && quat.y[0] <= '9' || quat.y[0] == '-') // 整数常量
				{
					mout << "\tli $v0, 1" << endl;
					mout << "\tli $a0, " << quat.y << endl;
				}
				else
				{
					Symbol sym = getSymbolInfo(quat.y);
					if (sym.type == INT) mout << "\tli $v0, 1" << endl;
					else if (sym.type == CHAR) mout << "\tli $v0, 11" << endl;
					mout << "\tlw $a0, " << sym.addrOffset << "($t0)" << endl;
				}
				mout << "\tsyscall" << endl;
			}

			mout << "\tli $v0, 11" << endl;
			mout << "\tli $a0, \'\\n\'" << endl;
			mout << "\tsyscall" << endl;
		}
		else
		{
			Symbol ans = getSymbolInfo(quat.ans);
			if (quat.op == "")
			{
				if (quat.x[0] == '\'') // 字符常量
				{
					mout << "\tli $t1, " << quat.x << endl;
				}
				else if (quat.x[0] >= '0' && quat.x[0] <= '9' || quat.x[0] == '-') // 整数常量
				{
					mout << "\tli $t1, " << quat.x << endl;
				}
				else
				{
					Symbol sym = getSymbolInfo(quat.x);
					mout << "\tlw $t1, " << sym.addrOffset << "($t0)" << endl;
				}
			}
			else
			{
				if (quat.x == "") mout << "\tli $t2, 0" << endl;
				else
				{
					if (quat.x[0] == '\'') // 字符常量
					{
						mout << "\tli $t2, " << quat.x << endl;
					}
					else if (quat.x[0] >= '0' && quat.x[0] <= '9' || quat.x[0] == '-') // 整数常量
					{
						mout << "\tli $t2, " << quat.x << endl;
					}
					else
					{
						Symbol sym1 = getSymbolInfo(quat.x);
						mout << "\tlw $t2, " << sym1.addrOffset << "($t0)" << endl;
					}
				}
				if (quat.y[0] == '\'') // 字符常量
				{
					mout << "\tli $t3, " << quat.y << endl;
				}
				else if (quat.y[0] >= '0' && quat.y[0] <= '9' || quat.y[0] == '-') // 整数常量
				{
					mout << "\tli $t3, " << quat.y << endl;
				}
				else
				{
					Symbol sym2 = getSymbolInfo(quat.y);
					mout << "\tlw $t3, " << sym2.addrOffset << "($t0)" << endl;
				}

				if (quat.op == "+") mout << "\tadd $t1, $t2, $t3" << endl;
				else if (quat.op == "-") mout << "\tsub $t1, $t2, $t3" << endl;
				if (quat.op == "*")
				{
					mout << "\tmult $t2, $t3" << endl;
					mout << "\tmflo $t1" << endl;
				}
				if (quat.op == "/")
				{
					mout << "\tdiv $t2, $t3" << endl;
					mout << "\tmflo $t1" << endl;
				}
			}
			mout << "\tsw $t1, " << ans.addrOffset << "($t0)" << endl;
		}
	}
}

void initStr()
{
	mout << ".data" << endl;
	for (int i = 0; i < strVec.size(); i++)
	{
		mout << strVec[i].idenfr << ": .asciiz \"" << strVec[i].content << "\"" << endl;
	}
}
#pragma endregion


#pragma region Grammar Parse Funcs Defination
void parseConstDef()
{
	int type = -1;
	if (wordNow.label == "INTTK") type = INT;
	else if (wordNow.label == "CHARTK") type = CHAR;
	if (type == -1)
	{
		printf("What the heck ! In ParseConstDef()");
		getchar();
	}
	parsePerConstDef(type);
	while (wordNow.label == "COMMA")
	{
		parsePerConstDef(type);
	}

	cout << "<常量定义>" << endl;
}

void parsePerConstDef(int type) // ＜标识符＞＝＜整数＞ / ＜标识符＞＝＜字符＞
{ // 没有超前读
	getSym(true, true);//标识符

	if (checkDupDef(wordNow.idenfr, wordNow.lineID))
	{
		getSym(true, true); // =
		getSym(true, true); // First(整数 / 字符)
		if (type == INT) parseInteger();
		else parseChar();
		return;
	}

	symTab[++symTabTop] = Symbol{ wordNow.idenfr, CONST, type, levelNow, 0 };

	string ans = wordNow.idenfr;

	getSym(true, true); // =
	getSym(true, true); // First(整数 / 字符)

	string value;
	if (type == INT) value = parseInteger();
	else value = parseChar();

	if (isInFunc)
	{
		curFunc.insertDef(ans, type);
		interCode.insertQuat(ans, value, "spAssign", "");
	}
	else
	{
		offsetTop = roundUp(offsetTop);
		CodeGenSymTab.push_back(Symbol{ ans, CONST, type, levelNow, 0, offsetTop });
		offsetTop += 4;
		interCode.insertQuat(ans, value, "", "");
	}
}

string parseChar()
{
	string ret = "\'" + wordNow.idenfr + "\'";
	if (wordNow.label != "CHARCON")
	{
		printf("Not CHARCON ! In parseChar");
		getchar();
	}
	getSym(true, true);

	return ret;
}

string parseInteger()
{
	string ret = "";
	if (wordNow.label == "PLUS" || wordNow.label == "MINU")
	{
		if (wordNow.label == "MINU") ret = "-";
		getSym(true, true); // first(<无符号整数>)
	}
	ret = ret + parseUnsignedInt();

	cout << "<整数>" << endl;

	return ret;
}

string parseUnsignedInt()
{
	string ret = "";
	if (wordNow.label != "INTCON")
	{
		printf("Not INTCON ! In parseUnsignedInt");
		getchar();
	}
	ret = wordNow.idenfr;
	getSym(true, true);
	cout << "<无符号整数>" << endl;

	return ret;
}

void parseConstDcrpt()
{
	// const
	getSym(true, true); // first(<常量定义>)
	parseConstDef();
	if (wordNow.label != "SEMICN")
	{
		if (wordNow.isAtHead)
		{
			getSym(false, false);
			error(wordNow.lineID, "k");
			getSym(true, false);
		}
		else error(wordNow.lineID, "k");
	}
	else
	{
		getSym(true, true); // const
		while (wordNow.label == "CONSTTK")
		{
			getSym(true, true); // first(<常量定义>)
			parseConstDef();
			if (wordNow.label != "SEMICN") // first(<常量定义>)
			{
				if (wordNow.isAtHead)
				{
					getSym(false, false);
					error(wordNow.lineID, "k");
					getSym(true, false);
				}
				else error(wordNow.lineID, "k");
			}
			else getSym(true, true); // first(<常量定义>)
		}
	}

	cout << "<常量说明>" << endl;
}

void parseVarDef()
{
	// 确定变量的类型
	int type = -1;
	if (wordNow.label == "INTTK") type = INT;
	else if (wordNow.label == "CHARTK") type = CHAR;
	if (type == -1)
	{
		printf("What the heck ! In parseVarDef()");
		getchar();
	}

	getSym(true, true);//标识符
	string idenfr = wordNow.idenfr;
	int lineID = wordNow.lineID;
	getSym(true, true);//读一个符号
	int dim = 0, D1 = -1, D2 = -1;
	// 确定变量的维数
	if (wordNow.label == "LBRACK")// [ 一维数组
	{
		dim++;
		getSym(true, true);// first（<无符号整数>）
		D1 = stoi(parseUnsignedInt());// ]
		if (wordNow.label != "RBRACK")
		{
			if (wordNow.isAtHead)
			{
				getSym(false, false);
				error(wordNow.lineID, "m");
				getSym(true, false);
			}
			else error(wordNow.lineID, "m");
		}
		else getSym(true, true);
		if (wordNow.label == "LBRACK")// [ 二维数组
		{
			dim++;
			getSym(true, true);// first（<无符号整数>）
			D2 = stoi(parseUnsignedInt());// ]
			if (wordNow.label != "RBRACK")
			{
				if (wordNow.isAtHead)
				{
					getSym(false, false);
					error(wordNow.lineID, "m");
					getSym(true, false);
				}
				else error(wordNow.lineID, "m");
			}
			else getSym(true, true);
		}
	}

	// = 变量定义及初始化
	if (wordNow.label == "ASSIGN")
	{
		if (checkDupDef(idenfr, lineID)) // 如果重复定义
		{
			while (wordNow.label != "SEMICN")
			{
				getSym(true, true);
			}
			return; // 退出 <变量定义>
		}

		// 没有重复定义
		symTab[++symTabTop] = Symbol{ idenfr, VAR, type, levelNow, dim };

		int size = (D2 > 0) ? (4 * D1 * D2) :
			((D1 > 0) ? (4 * D1) : 4);
		if (isInFunc)
		{
			curFunc.insertDef(idenfr, size, type);
		}
		else
		{
			// 将数组标识符存代码生成符号表CodeGenSymTab
			offsetTop = roundUp(offsetTop);
			CodeGenSymTab.push_back(Symbol{ idenfr, VAR, type, levelNow, dim, offsetTop });
			offsetTop += size;
		}


		// 二维
		if (D2 >= 0)
		{
			ArrCols[idenfr] = D2;
			int m = D1, n = D2;
			int deltaOff = 0;
			getSym(true, true); // {
			if (wordNow.label != "LBRACE") error(wordNow.lineID, "n");
			while (m--)
			{
				getSym(true, true);
				if (wordNow.label != "LBRACE") error(wordNow.lineID, "n");
				while (n--)
				{
					getSym(true, true);
					if (wordNow.label != "PLUS" && wordNow.label != "MINU" && wordNow.label != "INTCON" && wordNow.label != "CHARCON")
					{
						error(wordNow.lineID, "n");
					}
					typeAndValue tav = parseConst();
					if (type != tav.type) error(wordNow.lineID, "o");
					if (n != 0 && wordNow.label != "COMMA") { // 参数少了
						error(wordNow.lineID, "n");
						while (wordNow.label != "SEMICN") getSym(true, true);
						cout << "<变量定义及初始化>" << endl;
						return;
					}

					// 若没出现错误，则 ↓
					if (isInFunc)
					{
						interCode.insertQuat(tav.value, idenfr, "spARRSET", to_string(deltaOff));
						deltaOff += 4;
					}
					else
					{
						interCode.insertQuat(tav.value, idenfr, "ARRSET", to_string(deltaOff));
						deltaOff += 4;
					}
				}
				n = D2;
				if (wordNow.label != "RBRACE") { // 参数多了
					error(wordNow.lineID, "n");
					while (wordNow.label != "SEMICN") getSym(true, true);
					cout << "<变量定义及初始化>" << endl;
					return;
				}
				else getSym(true, true);
				if (m != 0 && wordNow.label != "COMMA") { // 参数少了
					error(wordNow.lineID, "n");
					while (wordNow.label != "SEMICN") getSym(true, true);
					cout << "<变量定义及初始化>" << endl;
					return;
				}
			}
			if (wordNow.label != "RBRACE") { // 参数多了
				error(wordNow.lineID, "n");
				while (wordNow.label != "SEMICN") getSym(true, true);
				cout << "<变量定义及初始化>" << endl;
				return;
			}
			else getSym(true, true);
		}
		// 一维
		else if (D1 >= 0)
		{
			int deltaOff = 0;
			getSym(true, true);
			if (wordNow.label != "LBRACE") error(wordNow.lineID, "n");
			while (D1--)
			{
				getSym(true, true);
				if (wordNow.label != "PLUS" && wordNow.label != "MINU" && wordNow.label != "INTCON" && wordNow.label != "CHARCON")
				{
					error(wordNow.lineID, "n");
				}
				typeAndValue tav = parseConst();
				if (type != tav.type) error(wordNow.lineID, "o");
				if (D1 != 0 && wordNow.label != "COMMA") {
					error(wordNow.lineID, "n");
					while (wordNow.label != "SEMICN") getSym(true, true);
					cout << "<变量定义及初始化>" << endl;
					return;
				}
				// 若没出现错误，则 ↓
				if (isInFunc)
				{
					interCode.insertQuat(tav.value, idenfr, "spARRSET", to_string(deltaOff));
					deltaOff += 4;
				}
				else
				{
					interCode.insertQuat(tav.value, idenfr, "ARRSET", to_string(deltaOff));
					deltaOff += 4;
				}
			}
			if (wordNow.label != "RBRACE") {
				error(wordNow.lineID, "n");
				while (wordNow.label != "SEMICN") getSym(true, true);
				cout << "<变量定义及初始化>" << endl;
				return;
			}
			else getSym(true, true);
		}
		// 零维
		else
		{
			getSym(true, true);
			typeAndValue tav = parseConst();
			if (tav.type != type) error(wordNow.lineID, "o");
			else
			{
				if (isInFunc)
				{
					interCode.insertQuat(idenfr, tav.value, "spAssign", "");
				}
				else
				{
					interCode.insertQuat(idenfr, tav.value, "", "");
				}
			}
		}

		cout << "<变量定义及初始化>" << endl;
	}
	// 变量定义无初始化
	else
	{// 此时已经读到了 , 或 ;
		if (!checkDupDef(idenfr, lineID))
		{
			symTab[++symTabTop] = Symbol{ idenfr, VAR, type, levelNow, dim };

			int size = (D2 > 0) ? (4 * D1 * D2) :
				((D1 > 0) ? (4 * D1) : 4);
			if (isInFunc)
			{
				curFunc.insertDef(idenfr, size, type);
			}
			else
			{
				// 将数组标识符存代码生成符号表CodeGenSymTab
				offsetTop = roundUp(offsetTop);
				CodeGenSymTab.push_back(Symbol{ idenfr, VAR, type, levelNow, dim, offsetTop });
				offsetTop += size;
			}
		}

		while (wordNow.label == "COMMA")
		{
			getSym(true, true);
			parsePerVarDefNotInit(type);
		}//若退出了循环，则表明遇到了 非',' 可能为';' 如果不是';' 则在退出后会报错

		cout << "<变量定义无初始化>" << endl;
	}

	cout << "<变量定义>" << endl;
}

void parsePerVarDefNotInit(int type)
{
	int D1 = 0, D2 = 0;
	string idenfr = wordNow.idenfr;
	int lineID = wordNow.lineID;
	if (checkDupDef(idenfr, lineID))
	{
		while (wordNow.label != "COMMA" && wordNow.label != "SEMICN")
			getSym(true, true);
		return;
	}
	int dim = 0;
	getSym(true, true);
	if (wordNow.label == "LBRACK")
	{
		dim++;
		getSym(true, true);
		D1 = stoi(parseUnsignedInt());// ]
		if (wordNow.label != "RBRACK")
		{
			if (wordNow.isAtHead)
			{
				getSym(false, false);
				error(wordNow.lineID, "m");
				getSym(true, false);
			}
			else error(wordNow.lineID, "m");
		}
		else getSym(true, true);
		if (wordNow.label == "LBRACK")
		{
			dim++;
			getSym(true, true);
			D2 = stoi(parseUnsignedInt());// ]
			ArrCols[idenfr] = D2;
			if (wordNow.label != "RBRACK")
			{
				if (wordNow.isAtHead)
				{
					getSym(false, false);
					error(wordNow.lineID, "m");
					getSym(true, false);
				}
				else error(wordNow.lineID, "m");
			}
			else getSym(true, true);
		}
	}
	symTab[++symTabTop] = Symbol{ idenfr, VAR, type, levelNow, dim };

	int size = (D2 > 0) ? (4 * D1 * D2) :
		((D1 > 0) ? (4 * D1) : 4);
	if (isInFunc)
	{
		curFunc.insertDef(idenfr, size, type);
	}
	else
	{
		// 将数组标识符存代码生成符号表CodeGenSymTab
		offsetTop = roundUp(offsetTop);
		CodeGenSymTab.push_back(Symbol{ idenfr, VAR, type, levelNow, dim, offsetTop });
		offsetTop += size;
	}
}

typeAndValue parseConst()
{
	typeAndValue ret;
	if (wordNow.label == "INTCON" || wordNow.label == "PLUS" || wordNow.label == "MINU")
	{
		ret.value = parseInteger();
		cout << "<常量>" << endl;
		ret.type = INT;
		return ret;
	}
	else if (wordNow.label == "CHARCON")
	{
		ret.value = parseChar();
		cout << "<常量>" << endl;
		ret.type = CHAR;
		return ret;
	}
	else
	{
		printf("What the heck in parseConst() !");
		getchar();
	}
}

void parseVarDcrpt()
{
	// first(<变量定义>)
	parseVarDef(); // ;
	getSym(true, true); // first(<变量定义>) / first({＜有返回值函数定义＞|＜无返回值函数定义＞}＜主函数＞) / first(＜语句列＞)
	while (true)
	{
		if (wordNow.label != "INTTK" && wordNow.label != "CHARTK") break; // 排除 <语句列> 和 无返回值函数 的可能性
		getSym(true, false); // 偷看 × 1  函数名？
		getSym(true, false); // 偷看 × 2  左括号？
		if (wordNow.label == "LPARENT") // 左括号 
		{
			getSym(false, false); // 回退
			getSym(false, false); // 回退
			break; // 排除有返回值函数的可能性
		}
		getSym(false, false); // 回退
		getSym(false, false); // 回退
		parseVarDef(); // 变量定义 // ;
		if (wordNow.label != "SEMICN")
		{
			if (wordNow.isAtHead)
			{
				getSym(false, false);
				error(wordNow.lineID, "k");
				getSym(true, false);
			}
			else error(wordNow.lineID, "k");
		}
		else getSym(true, true); // 多读一个开启下一轮循环
	}

	cout << "<变量说明>" << endl;
}

void parseParamTable()
{
	// int or char
	int type = -1;
	if (wordNow.label == "INTTK") type = INT;
	else if (wordNow.label == "CHARTK") type = CHAR;
	else // 说明为空参数表
	{
		cout << "<参数表>" << endl;
		return;
	}
	getSym(true, true);// 标识符
	curFunc.insertPara(wordNow.idenfr, type);
	checkDupDef(wordNow.idenfr, wordNow.lineID);
	symTab[++symTabTop] = Symbol{ wordNow.idenfr, PARAM, type, levelNow, 0 };
	getSym(true, true);
	while (wordNow.label == "COMMA")
	{
		getSym(true, true);// int or char
		if (wordNow.label == "INTTK") type = INT;
		else if (wordNow.label == "CHARTK") type = CHAR;
		getSym(true, true);// 标识符
		curFunc.insertPara(wordNow.idenfr, type);
		checkDupDef(wordNow.idenfr, wordNow.lineID);
		symTab[++symTabTop] = Symbol{ wordNow.idenfr, PARAM, type, levelNow, 0 };
		getSym(true, true);
	}

	FuncMap[curFunc.getFuncName()] = curFunc;

	cout << "<参数表>" << endl;

}

int parseDeclareHead()
{
	int type = -1;
	if (wordNow.label == "INTTK") type = INT;
	else if (wordNow.label == "CHARTK") type = CHAR;
	if (type == -1)
	{
		printf("What the heck ! In parseDeclareHead()");
		getchar();
	}
	retType = type;
	getSym(true, true);// 标识符
	if (checkDupDef(wordNow.idenfr, wordNow.lineID))
	{
		symTab[++symTabTop] = Symbol{ wordNow.idenfr, FUNC, type, levelNow, 0 }; // 不能提出去
		getSym(true, true);
		cout << "<声明头部>" << endl;
		return 1;
	}

	curFunc = Function(wordNow.idenfr, type);
	interCode.insertQuat(wordNow.idenfr, "", ":", ""); // 函数名

	symTab[++symTabTop] = Symbol{ wordNow.idenfr, FUNC, type, levelNow, 0 };
	keptLabelMap.insert(map<string, string>::value_type(wordNow.idenfr, "RETURNABLE"));
	getSym(true, true);// (

	cout << "<声明头部>" << endl;

	return 0;
}

void parseLoopStmt()
{
	if (wordNow.label == "WHILETK")
	{
		getSym(true, true); // '('
		getSym(true, true); // first(<条件>)

		string startLoopLabel = getNewLabel();
		interCode.insertQuat(startLoopLabel, "", ":", "");
		string endLoopLabel = parseCondition();

		// ')'

		if (wordNow.label != "RPARENT")
		{
			if (wordNow.isAtHead)
			{
				getSym(false, false);
				error(wordNow.lineID, "l");
				getSym(true, false);
			}
			else error(wordNow.lineID, "l");
		}
		else getSym(true, true);
		parseStmt();

		interCode.insertQuat(startLoopLabel, "", "j", "");
		interCode.insertQuat(endLoopLabel, "", ":", "");
	}
	else if (wordNow.label == "FORTK")
	{
		getSym(true, true); // (
		getSym(true, true); // 标识符
		string i = wordNow.idenfr;
		checkUndefined(i, wordNow.lineID);
		getSym(true, true); // =
		getSym(true, true); // first(表达式)
		typeAndValue i0 = parseExpr(); // ;
		interCode.insertQuat(i, i0.value, "", ""); // 初始化循环变量

		// 错误处理
		if (wordNow.label != "SEMICN")
		{
			if (wordNow.isAtHead)
			{
				getSym(false, false);
				error(wordNow.lineID, "k");
				getSym(true, false);
			}
			else error(wordNow.lineID, "k");
		}
		else getSym(true, true); // first(条件)

		string startLoopLabel = getNewLabel();
		interCode.insertQuat(startLoopLabel, "", ":", ""); // 循环开始标签
		string endLoopLabel = parseCondition(); // ; 根据判断条件决定是否跳转到循环结束

		// 错误处理
		if (wordNow.label != "SEMICN")
		{
			if (wordNow.isAtHead)
			{
				getSym(false, false);
				error(wordNow.lineID, "k");
				getSym(true, false);
			}
			else error(wordNow.lineID, "k");
		}
		else getSym(true, true); // 标识符
		checkUndefined(wordNow.idenfr, wordNow.lineID);

		getSym(true, true); // =
		getSym(true, true); // 标识符
		string x = wordNow.idenfr;
		checkUndefined(x, wordNow.lineID);
		getSym(true, true); // + / -
		string op = wordNow.idenfr;
		getSym(true, true); // first(步长)
		string y = parseStep(); // )
		if (wordNow.label != "RPARENT")
		{
			if (wordNow.isAtHead)
			{
				getSym(false, false);
				error(wordNow.lineID, "l");
				getSym(true, false);
			}
			else error(wordNow.lineID, "l");
		}
		else getSym(true, true);
		parseStmt();

		interCode.insertQuat(i, x, op, y); // 循环变量递增
		interCode.insertQuat(startLoopLabel, "", "j", ""); // 调回循环开头
		interCode.insertQuat(endLoopLabel, "", ":", ""); // 循环结束标签
	}

	cout << "<循环语句>" << endl;
}

string  parseStep()
{
	string step = parseUnsignedInt();

	cout << "<步长>" << endl;

	return step;
}

void parseValueParamTable(FuncInfo funcInfo)
{
	if (wordNow.label == "RPARENT")
	{
		if (funcInfo.paramNum != 0) {
			error(wordNow.lineID, "d");
		}
		cout << "<值参数表>" << endl;
		return;
	}
	else if (wordNow.label == "SEMICN")
	{
		cout << "<值参数表>" << endl;
		return;
	}
	else
	{
		int cnt = 0;
		int offset = 0;
		Function func = FuncMap[funcInfo.funcName];
		typeAndValue paraValue = parseExpr();
		func.insertPara(paraValue.value, paraValue.type);
		interCode.insertQuat("", paraValue.value, "spPush", to_string(offset));
		offset += 4;

		cnt++;
		if (paraValue.type != symTab[cnt + funcInfo.pos].type) error(wordNow.lineID, "e");
		while (wordNow.label == "COMMA")
		{
			getSym(true, true);
			paraValue = parseExpr();
			func.insertPara(paraValue.value, paraValue.type);
			interCode.insertQuat("", paraValue.value, "spPush", to_string(offset));
			offset += 4;

			cnt++;
			if (paraValue.type != symTab[cnt + funcInfo.pos].type) error(wordNow.lineID, "e");
		}
		if (cnt != funcInfo.paramNum) error(wordNow.lineID, "d");

		FuncMap[funcInfo.funcName] = func;

		interCode.insertQuat(funcInfo.funcName, "", "jal", "");
	}

	cout << "<值参数表>" << endl;
}

string parseFuncCallWithReturn()
{
	if (checkUndefined(wordNow.idenfr, wordNow.lineID))
	{
		while (wordNow.label != "SEMICN") getSym(true, true);
		return;
	}

	FuncInfo funcInfo = getFuncInfo(wordNow.idenfr);
	getSym(true, true); // (
	getSym(true, true); // first(值参数表)
	parseValueParamTable(funcInfo); // )

	if (wordNow.label != "RPARENT")
	{
		if (wordNow.isAtHead)
		{
			getSym(false, false);
			error(wordNow.lineID, "l");
			getSym(true, false);
		}
		else error(wordNow.lineID, "l");
	}
	else getSym(true, true);

	string ret = getNewAns(funcInfo.type, 4);
	interCode.insertQuat(ret, "", "$v0", "");

	cout << "<有返回值函数调用语句>" << endl;

	return ret;
}

void parseFuncCallWithoutReturn()
{
	if (checkUndefined(wordNow.idenfr, wordNow.lineID))
	{
		while (wordNow.label != "SEMICN") getSym(true, true);
		return;
	}
	FuncInfo funcInfo = getFuncInfo(wordNow.idenfr);
	getSym(true, true); // (
	getSym(true, true); // first(值参数表)
	parseValueParamTable(funcInfo); // )
	if (wordNow.label != "RPARENT")
	{
		if (wordNow.isAtHead)
		{
			getSym(false, false);
			error(wordNow.lineID, "l");
			getSym(true, false);
		}
		else error(wordNow.lineID, "l");
	}
	else getSym(true, true);

	cout << "<无返回值函数调用语句>" << endl;
}

typeAndValue parseFactor()
{
	typeAndValue ret;
	ret.type = -1;
	// 整数
	if (wordNow.label == "PLUS" || wordNow.label == "MINU" || wordNow.label == "INTCON")
	{
		ret.type = INT;
		ret.value = parseInteger();
	}
	// 字符
	else if (wordNow.label == "CHARCON")
	{
		ret.type = CHAR;
		ret.value = parseChar();
	}
	// 有返回值函数调用
	else if (keptLabelMap.find(wordNow.idenfr) != keptLabelMap.end() && keptLabelMap[wordNow.idenfr] == "RETURNABLE")
	{
		ret.type = getFuncInfo(wordNow.idenfr).type;
		ret.value = parseFuncCallWithReturn();
	}
	// 表达式
	else if (wordNow.label == "LPARENT")
	{
		ret.type = INT;
		getSym(true, true);
		ret.value = parseExpr().value;
		if (wordNow.label != "RPARENT")
		{
			if (wordNow.isAtHead)
			{
				getSym(false, false);
				error(wordNow.lineID, "l");
				getSym(true, false);
			}
			else error(wordNow.lineID, "l");
		}
		else getSym(true, true);
	}
	// 变量（包含数组）、常量
	else
	{
		string idenfr = wordNow.idenfr;
		// 标识符
		int isUndefined = checkUndefined(idenfr, wordNow.lineID);
		if (!isUndefined && !isInFunc)
		{
			ret.type = getQuantityInfo(idenfr).type;
			ret.value = idenfr;
		}
		getSym(true, true);

		if (wordNow.label == "LBRACK")
		{
			getSym(true, true);
			typeAndValue expr1 = parseExpr();
			// 下标是字符 出错
			if (!isUndefined && expr1.type == CHAR) error(wordNow.lineID, "i");
			// 缺 ']' 出错
			if (wordNow.label != "RBRACK")
			{
				if (wordNow.isAtHead)
				{
					getSym(false, false);
					error(wordNow.lineID, "m");
					getSym(true, false);
				}
				else error(wordNow.lineID, "m");
			}
			else getSym(true, true);

			// 数组第一维索引
			string index1 = getNewAns(INT, 4);
			string x = expr1.value;
			interCode.insertQuat(index1, x, "*", "4");

			if (wordNow.label == "LBRACK")
			{
				getSym(true, true);
				typeAndValue expr2 = parseExpr();
				// 下标是字符 出错
				if (!isUndefined && expr2.type == CHAR) error(wordNow.lineID, "i");
				// 缺 ']' 出错
				if (wordNow.label != "RBRACK")
				{
					if (wordNow.isAtHead)
					{
						getSym(false, false);
						error(wordNow.lineID, "m");
						getSym(true, false);
					}
					else error(wordNow.lineID, "m");
				}
				else getSym(true, true);

				// 数组第二维索引
				interCode.insertQuat(index1, index1, "*", to_string(ArrCols[idenfr]));
				string index2 = getNewAns(INT, 4);
				string x = expr2.value;
				interCode.insertQuat(index2, x, "*", "4");
				interCode.insertQuat(index1, index1, "+", index2);
			}

			string ans = getNewAns(ret.type, 4);
			if (isInFunc && curFunc.hasDef(idenfr))
			{
				interCode.insertQuat(index1, index1, "+", to_string(curFunc.getOffset(idenfr)));
				interCode.insertQuat(ans, idenfr, "spARRGET", index1);
			}
			else
			{
				interCode.insertQuat(ans, idenfr, "ARRGET", index1);
			}
			ret.value = ans;

		}
		else
		{
			if (isInFunc && curFunc.hasCheck(idenfr))
			{
				ret.type = curFunc.getType(idenfr);
				string ans = getNewAns(ret.type, 4);
				interCode.insertQuat(ans, to_string(curFunc.getOffset(idenfr)), "spGET", "");
				ret.value = ans;
			}
		}
	}

	cout << "<因子>" << endl;
	return ret;
}

typeAndValue parseTerm()
{
	typeAndValue ret;
	ret.type = -1;
	ret = parseFactor();

	while (wordNow.label == "MULT" || wordNow.label == "DIV")
	{
		string ans = getNewAns(ret.type, 4);

		string x = ret.value;

		string op;
		if (wordNow.label == "MULT") op = "*";
		else if (wordNow.label == "DIV") op = "/";

		ret.type = INT;
		getSym(true, true);
		string y = parseFactor().value;

		interCode.insertQuat(ans, x, op, y);

		ret.value = ans;
	}

	cout << "<项>" << endl;
	return ret;
}

typeAndValue parseExpr()
{
	typeAndValue ret;
	string ans = "";
	ret.type = -1;
	if (wordNow.label == "PLUS" || wordNow.label == "MINU")
	{
		ret.type = INT;
		if (wordNow.label == "MINU")
		{
			ans = getNewAns(ret.type, 4);
		}
		getSym(true, true);
	}
	typeAndValue tav = parseTerm();
	if (ret.type < 0) ret.type = tav.type;

	if (ans == "") ret.value = tav.value;
	else
	{
		ret.value = ans;

		interCode.insertQuat(ans, "", "-", tav.value);
	}

	while (wordNow.label == "PLUS" || wordNow.label == "MINU")
	{
		ans = getNewAns(ret.type, 4);
		string x = ret.value;
		string op = "";
		if (wordNow.label == "PLUS") op = "+";
		else if (wordNow.label == "MINU") op = "-";
		ret.type = INT;
		getSym(true, true);
		string y = parseTerm().value;

		interCode.insertQuat(ans, x, op, y);

		ret.value = ans;
	}

	cout << "<表达式>" << endl;

	return ret;
}

string parseCondition()
{
	int type;
	typeAndValue leftTav, rightTav;
	leftTav = parseExpr();
	if (leftTav.type != INT) error(wordNow.lineID, "f");

	// <比较运算符>
	string label = getNewLabel();
	interCode.insertQuat(label, leftTav.value, wordNow.idenfr, rightTav.value);

	getSym(true, true);
	rightTav = parseExpr();
	if (rightTav.type != INT) error(wordNow.lineID, "f");

	cout << "<条件>" << endl;

	return label;
}

void parseCondStmt()
{
	// if
	getSym(true, true); // (
	getSym(true, true); // first(条件)
	string elseLabel = parseCondition();

	// 错误处理
	if (wordNow.label != "RPARENT")
	{
		if (wordNow.isAtHead)
		{
			getSym(false, false);
			error(wordNow.lineID, "l");
			getSym(true, false);
		}
		else error(wordNow.lineID, "l");
	}
	else getSym(true, true);

	parseStmt();

	string endLabel = getNewLabel();
	if (wordNow.label == "ELSETK")
	{
		//跳转语句 跳到 endLabel
		interCode.insertQuat(endLabel, "", "j", "");
		//插入标签于此 elseLabel
		interCode.insertQuat(elseLabel, "", ":", "");
		getSym(true, true);
		parseStmt();
	}
	else {
		interCode.insertQuat(elseLabel, "", ":", "");
	}

	cout << "<条件语句>" << endl;
}

void parseReadStmt()
{
	// scanf
	getSym(true, true);// (
	getSym(true, true);// 标识符
	int isUndefined = checkUndefined(wordNow.idenfr, wordNow.lineID);
	if (!isUndefined && getQuantityInfo(wordNow.idenfr).kind == CONST) error(wordNow.lineID, "j");
	else if (!isUndefined) interCode.insertQuat(wordNow.idenfr, "", "read", "");
	getSym(true, true);// )
	if (wordNow.label != "RPARENT")
	{
		if (wordNow.isAtHead)
		{
			getSym(false, false);
			error(wordNow.lineID, "l");
			getSym(true, false);
		}
		else error(wordNow.lineID, "l");
	}
	else getSym(true, true);

	cout << "<读语句>" << endl;
}

void parseString()
{
	getSym(true, true);

	cout << "<字符串>" << endl;
}

void parseWriteStmt()
{
	getSym(true, true);// (
	getSym(true, true);
	if (wordNow.label == "STRCON")
	{
		unsigned int strSize = getStrSize(wordNow.idenfr);
		strVec.push_back(StrDef{ "str" + to_string(strID), wordNow.idenfr, strOffTop, strSize });
		string x = "str" + to_string(strID);
		strID++;
		strOffTop += strSize;

		// 更新之前定义的变、常量的偏移
		unsigned int usedOffTop = offsetTop;
		offsetTop = roundUp(strOffTop);
		unsigned int delta = 0;
		for (int i = 0; i < CodeGenSymTab.size(); i++)
		{
			if (i + 1 < CodeGenSymTab.size()) delta = CodeGenSymTab[i + 1].addrOffset - CodeGenSymTab[i].addrOffset;
			else delta = usedOffTop - CodeGenSymTab[i].addrOffset;

			CodeGenSymTab[i].addrOffset = offsetTop;
			offsetTop += delta;
		}

		parseString();
		if (wordNow.label == "COMMA")
		{
			getSym(true, true);
			string y = parseExpr().value;
			if (wordNow.label != "RPARENT")
			{
				if (wordNow.isAtHead)
				{
					getSym(false, false);
					error(wordNow.lineID, "l");
					getSym(true, false);
				}
				else error(wordNow.lineID, "l");
			}
			else
			{
				interCode.insertQuat("", x, "print", y);
				getSym(true, true);
			}
		}
		else
		{
			if (wordNow.label != "RPARENT")
			{
				if (wordNow.isAtHead)
				{
					getSym(false, false);
					error(wordNow.lineID, "l");
					getSym(true, false);
				}
				else error(wordNow.lineID, "l");
			}
			else
			{
				interCode.insertQuat("", x, "print", "");
				getSym(true, true);
			}
		}
	}
	else
	{
		string y = parseExpr().value;
		if (wordNow.label != "RPARENT")
		{
			if (wordNow.isAtHead)
			{
				getSym(false, false);
				error(wordNow.lineID, "l");
				getSym(true, false);
			}
			else error(wordNow.lineID, "l");
		}
		else
		{
			interCode.insertQuat("", "", "print", y);
			getSym(true, true);
		}
	}

	cout << "<写语句>" << endl;
}

string parseCondChildStmt(typeAndValue tavIn, string endLabel)
{
	getSym(true, true); // first(<常量>)
	typeAndValue tav = parseConst();

	string nextLabel = getNewLabel();
	interCode.insertQuat(nextLabel, tavIn.value, "==", tav.value);

	// 错误处理
	if (tavIn.type != tav.type) error(wordNow.lineID, "o");
	getSym(true, true);

	parseStmt();

	interCode.insertQuat(endLabel, "", "j", "");

	cout << "<情况子语句>" << endl;

	return nextLabel;
}

void parseCondTable(typeAndValue tav, string endLabel)
{
	while (wordNow.label == "CASETK")
	{
		string nextLabel = parseCondChildStmt(tav, endLabel);
		interCode.insertQuat(nextLabel, "", ":", "");
	}

	cout << "<情况表>" << endl;
}

void parseDefault()
{
	getSym(true, true);
	getSym(true, true);
	parseStmt();

	cout << "<缺省>" << endl;
}

void parseSwitchStmt()
{
	getSym(true, true);// (
	getSym(true, true);// first(表达式)
	typeAndValue tav = parseExpr(); // )

	// 错误处理
	if (wordNow.label != "RPARENT")
	{
		if (wordNow.isAtHead)
		{
			getSym(false, false);
			error(wordNow.lineID, "l");
			getSym(true, false);
		}
		else error(wordNow.lineID, "l");
	}
	else getSym(true, true); // {

	string endLabel = getNewLabel();
	getSym(true, true); // first(情况表)
	parseCondTable(tav, endLabel); // first(<缺省>)

	if (wordNow.label != "DEFAULTTK") error(wordNow.lineID, "p");
	else parseDefault(); // }

	interCode.insertQuat(endLabel, "", ":", "");

	getSym(true, true);

	cout << "<情况语句>" << endl;
}

void parseReturnStmt()
{
	// return
	getSym(true, true);
	if (wordNow.label == "LPARENT")
	{
		if (funcRetFlag == 0) error(wordNow.lineID, "g"); // void函数里 出现 return(
		getSym(true, true);
		if (funcRetFlag == 1 && wordNow.label == "RPARENT") error(wordNow.lineID, "h");// return();
		typeAndValue retValue = parseExpr();
		if (isInFunc && funcRetFlag == 1)
		{
			interCode.insertQuat("", retValue.value, "return", "");
		}
		if (funcRetFlag == 1 && retValue.type != retType) error(wordNow.lineID, "h");// 表达式类型与返回值类型不一致
		if (wordNow.label != "RPARENT")
		{
			if (wordNow.isAtHead)
			{
				getSym(false, false);
				error(wordNow.lineID, "l");
				getSym(true, false);
			}
			else error(wordNow.lineID, "l");
		}
		else getSym(true, true);
	}
	else
	{
		if (funcRetFlag == 1) error(wordNow.lineID, "h");// return;
		if (isInMain) isMainEnd = true;
	}

	cout << "<返回语句>" << endl;
}

void parseStmt()
{
	// 循环语句
	if (wordNow.label == "WHILETK" || wordNow.label == "FORTK")
	{
		parseLoopStmt();
	}
	// 条件语句
	else if (wordNow.label == "IFTK")
	{
		parseCondStmt();
	}
	// 读语句
	else if (wordNow.label == "SCANFTK")
	{
		parseReadStmt();
		if (wordNow.label != "SEMICN")
		{
			if (wordNow.isAtHead)
			{
				getSym(false, false);
				error(wordNow.lineID, "k");
				getSym(true, false);
			}
			else error(wordNow.lineID, "k");
		}
		else getSym(true, true);
	}
	// 写语句
	else if (wordNow.label == "PRINTFTK")
	{
		parseWriteStmt();
		if (wordNow.label != "SEMICN")
		{
			if (wordNow.isAtHead)
			{
				getSym(false, false);
				error(wordNow.lineID, "k");
				getSym(true, false);
			}
			else error(wordNow.lineID, "k");
		}
		else getSym(true, true);
	}
	// 情况语句
	else if (wordNow.label == "SWITCHTK")
	{
		parseSwitchStmt();
	}
	// 空语句
	else if (wordNow.label == "SEMICN")
	{
		getSym(true, true);
	}
	// 返回语句
	else if (wordNow.label == "RETURNTK")
	{
		if (funcRetFlag) hasRet = 1;
		parseReturnStmt();
		if (wordNow.label != "SEMICN")
		{
			if (wordNow.isAtHead)
			{
				getSym(false, false);
				error(wordNow.lineID, "k");
				getSym(true, false);
			}
			else error(wordNow.lineID, "k");
		}
		else getSym(true, true);
		if (isMainEnd) return;
	}
	// 语句列
	else if (wordNow.label == "LBRACE")
	{
		levelUp();
		getSym(true, true);
		parseStmtList();
		levelDown();
		getSym(true, true);
	}
	else
	{
		getSym(true, false);
		// 函数调用语句
		if (wordNow.label == "LPARENT")
		{
			getSym(false, false);
			if (checkUndefined(wordNow.idenfr, wordNow.lineID))
			{
				while (wordNow.label != "SEMICN") getSym(true, true);
			}
			else parseFuncCall();// 函数调用

			if (wordNow.label != "SEMICN")
			{
				if (wordNow.isAtHead)
				{
					getSym(false, false);
					error(wordNow.lineID, "k");
					getSym(true, false);
				}
				else error(wordNow.lineID, "k");
			}
			else getSym(true, true);
		}
		// 赋值语句
		else
		{
			getSym(false, false);
			parseAssignStmt();// 赋值语句
			if (wordNow.label != "SEMICN")
			{
				if (wordNow.isAtHead)
				{
					getSym(false, false);
					error(wordNow.lineID, "k");
					getSym(true, false);
				}
				else error(wordNow.lineID, "k");
			}
			else getSym(true, true);
		}
	}

	cout << "<语句>" << endl;
}

void parseFuncCall()
{
	Function func = FuncMap[wordNow.idenfr];
	int size = func.getTotalSize();
	interCode.insertQuat("", to_string(size), "spAlloc", "");

	// 函数的参数压栈
	for (int i = 0; i < curFunc.getParaNum(); i += 1)
	{
		interCode.insertQuat("", curFunc.getParaName(i), "spPush", to_string(i));
	}

	if (keptLabelMap.find(wordNow.idenfr) != keptLabelMap.end() && keptLabelMap.find(wordNow.idenfr)->second == "RETURNABLE")
	{
		parseFuncCallWithReturn();
	}
	else if (keptLabelMap.find(wordNow.idenfr) != keptLabelMap.end() && keptLabelMap.find(wordNow.idenfr)->second == "UNRETURNABLE")
	{
		parseFuncCallWithoutReturn();
	}

	interCode.insertQuat("", to_string(size), "spFree", "");
}

void parseAssignStmt()
{
	string idenfr = wordNow.idenfr;
	int isUndefined = checkUndefined(wordNow.idenfr, wordNow.lineID);
	if (!isUndefined && getQuantityInfo(wordNow.idenfr).kind == CONST) error(wordNow.lineID, "j");
	getSym(true, true);
	if (wordNow.label == "ASSIGN")
	{
		getSym(true, true);
		string x = parseExpr().value;
		interCode.insertQuat(idenfr, x, "", "");
	}
	else if (wordNow.label == "LBRACK")
	{
		getSym(true, true);
		typeAndValue index1 = parseExpr();
		if (index1.type == CHAR) error(wordNow.lineID, "i");
		if (wordNow.label != "RBRACK")
		{
			if (wordNow.isAtHead)
			{
				getSym(false, false);
				error(wordNow.lineID, "m");
				getSym(true, false);
			}
			else error(wordNow.lineID, "m");
		}
		else getSym(true, true);

		if (wordNow.label == "ASSIGN")
		{
			getSym(true, true);
			typeAndValue value = parseExpr();

			string offset = getNewAns(INT, 4);
			interCode.insertQuat(offset, index1.value, "*", "4");
			interCode.insertQuat(value.value, idenfr, "ARRSET", offset);
		}
		else if (wordNow.label == "LBRACK")
		{
			getSym(true, true);
			typeAndValue index2 = parseExpr();
			if (index2.type == CHAR) error(wordNow.lineID, "i");
			if (wordNow.label != "RBRACK")
			{
				if (wordNow.isAtHead)
				{
					getSym(false, false);
					error(wordNow.lineID, "m");
					getSym(true, false);
				}
				else error(wordNow.lineID, "m");
			}
			else getSym(true, true);
			getSym(true, true);

			typeAndValue value = parseExpr();

			string offset = getNewAns(INT, 4);
			interCode.insertQuat(offset, index1.value, "*", "4");
			string tmp = getNewAns(INT, 4);
			interCode.insertQuat(tmp, index2.value, "*", "4");
			interCode.insertQuat(offset, offset, "*", to_string(ArrCols[idenfr]));
			interCode.insertQuat(offset, offset, "+", tmp);
			interCode.insertQuat(value.value, idenfr, "ARRSET", offset);
		}
	}

	cout << "<赋值语句>" << endl;
}

void parseStmtList()
{
	while (wordNow.label != "RBRACE")
	{
		parseStmt();
		if (isFuncEnd) return;
		if (isMainEnd) return;
	}

	cout << "<语句列>" << endl;
}

void parseCompStmt()
{
	if (wordNow.label == "CONSTTK")
	{
		parseConstDcrpt();
	}

	if (wordNow.label == "INTTK" || wordNow.label == "CHARTK")
	{
		parseVarDcrpt();
	}

	parseStmtList();

	cout << "<复合语句>" << endl;
}

void parseFuncDef()
{
	interCode.insertQuat("main", "", "j", "");
	isInFunc = true;

	string str = wordNow.label;// 返回值类型 void/int/char
	getSym(true, false);// idenfr 函数名
	while (str != "VOIDTK" || wordNow.label != "MAINTK")
	{
		getSym(false, false);// 返回值类型 void/int/char  
		if (wordNow.label == "VOIDTK")
		{
			funcRetFlag = 0; // 进入void函数
			getSym(true, true);// void的函数名

			curFunc = Function(wordNow.idenfr, VOID);
			interCode.insertQuat(wordNow.idenfr, "", ":", ""); // 函数名

			int isDupDef = checkDupDef(wordNow.idenfr, wordNow.lineID);
			if (!isDupDef)
			{
				keptLabelMap.insert(map<string, string>::value_type(wordNow.idenfr, "UNRETURNABLE"));
			}

			symTab[++symTabTop] = Symbol{ wordNow.idenfr, FUNC, VOID, levelNow, 0 };
			getSym(true, true);// (
			levelUp();
			getSym(true, true);
			if (wordNow.label != "RPARENT") {
				parseParamTable();
			}
			else cout << "<参数表>" << endl;
			// )
			if (wordNow.label != "RPARENT")
			{
				if (wordNow.isAtHead)
				{
					getSym(false, false);
					error(wordNow.lineID, "l");
					getSym(true, false);
				}
				else error(wordNow.lineID, "l");
			}
			else getSym(true, true);// {
			getSym(true, true);// first(<复合语句>)
			parseCompStmt();// }

			funcRetFlag = -1;// 退出函数
			/*if (isDupDef) levelDown(true);
			else levelDown();*/
			levelDown(isDupDef); // 如果重复定义就把函数名也删了，否则留下函数名和param
			getSym(true, true);
			cout << "<无返回值函数定义>" << endl;
		}
		else
		{
			funcRetFlag = 1; // 进入有返回值的函数
			int isDupDef = parseDeclareHead(); //（
			levelUp();
			getSym(true, true);
			if (wordNow.label != "RPARENT") {
				parseParamTable();
			}
			else cout << "<参数表>" << endl;
			// )
			if (wordNow.label != "RPARENT")
			{
				if (wordNow.isAtHead)
				{
					getSym(false, false);
					error(wordNow.lineID, "l");
					getSym(true, false);
				}
				else error(wordNow.lineID, "l");
			}
			else getSym(true, true);// {

			getSym(true, true); // first(<复合语句>)
			parseCompStmt(); // }
			if (!hasRet) error(wordNow.lineID, "h");
			funcRetFlag = -1;
			levelDown(isDupDef); // 如果重复定义就把函数名也删了，否则留下函数名和param
			getSym(true, true);
			cout << "<有返回值函数定义>" << endl;

			hasRet = 0;
		}
		str = wordNow.label;
		getSym(true, false);

		interCode.insertQuat("", "", "jr", ""); // 函数跳转返回
	}
	getSym(false, false);

	isInFunc = false;
}

void parseMainFunc()
{
	// void
	interCode.insertQuat("main", "", ":", "");
	funcRetFlag = 0; // 进入void函数
	isInMain = true;
	getSym(true, true);// main
	getSym(true, true);// (
	getSym(true, true);// )

	if (wordNow.label != "RPARENT")
	{
		if (wordNow.isAtHead)
		{
			getSym(false, false);
			error(wordNow.lineID, "l");
			getSym(true, false);
		}
		else error(wordNow.lineID, "l");
	}
	else getSym(true, true);// {

	levelUp();
	getSym(true, true);// first(＜复合语句＞)
	parseCompStmt();
	// }
	funcRetFlag = -1;//退出函数
	isInMain = false;
	//levelDown();
	getSym(true, true);

	cout << "<主函数>" << endl;
}

void parseProgram()
{
	if (wordNow.label == "CONSTTK")
	{
		parseConstDcrpt();
	}

	if (wordNow.label == "VOIDTK")
	{
		getSym(true, false);// 偷看一个
		if (wordNow.label == "MAINTK")
		{
			getSym(false, false);// 回退到 void

			parseMainFunc();
		}
		else
		{
			getSym(false, false);// 回退到 void

			parseFuncDef();
			parseMainFunc();
		}
	}
	else if (wordNow.label == "INTTK" || wordNow.label == "CHARTK")
	{
		getSym(true, false); // 偷看 × 1
		getSym(true, false); // 偷看 × 2
		if (wordNow.label != "LPARENT") // != <变量说明>
		{
			getSym(false, false);
			getSym(false, false);

			parseVarDcrpt();
		}
		else
		{
			getSym(false, false);
			getSym(false, false);
		}
		parseFuncDef();
		parseMainFunc();
	}

	cout << "<程序>" << endl;
}
#pragma endregion


#pragma region Util Funcs

string getNewLabel()
{
	string label = "label" + to_string(labelID);
	labelID++;
	return label;
}

string getNewAns(int type, unsigned int size)
{
	string ans = "~t" + to_string(mediaID);
	mediaID++;

	offsetTop = roundUp(offsetTop);
	CodeGenSymTab.push_back(Symbol{ ans, VAR, type, levelNow, 0, offsetTop });
	offsetTop += size;

	return ans;
}

unsigned int roundUp(unsigned int offset)
{
	while (offset % 4 != 0) offset++;
	return offset;
}

unsigned int getStrSize(string s)
{
	return s.length() + 1;
}

void error(int lineID, string errType)
{
	//cout << lineID << " " << errType << endl;
	errProc.insertError(lineID, errType);
}

Symbol getSymbolInfo(string idenfr)
{
	for (int i = CodeGenSymTab.size() - 1; i >= 0; i--)
	{
		if (CodeGenSymTab[i].idenfr == idenfr) return CodeGenSymTab[i];
	}
}

QuantityInfo getQuantityInfo(string idenfr)
{
	QuantityInfo quantityInfo = { -1, -1 };
	for (int i = symTabTop; i >= 0; i--)
	{
		if (toLowerCase(symTab[i].idenfr) == toLowerCase(idenfr))
		{
			quantityInfo.kind = symTab[i].kind;
			quantityInfo.type = symTab[i].type;
			return quantityInfo;
		}
	}
	return quantityInfo;
}

FuncInfo getFuncInfo(string idenfr)
{
	FuncInfo funcInfo = { -1, -1, -1, idenfr };// [0]:func pos [1]:param num [2]:return type
	for (int i = symTabTop; i >= 0; i--)
	{
		if (toLowerCase(symTab[i].idenfr) == toLowerCase(idenfr) && symTab[i].kind == FUNC)
		{
			funcInfo.type = symTab[i].type;
			funcInfo.pos = i;
			int cnt = 0, ind = i + 1;
			while (symTab[ind].kind == PARAM)
			{
				ind++;
				cnt++;
			}
			funcInfo.paramNum = cnt;
		}
	}
	return funcInfo;
}

int checkUndefined(string idenfr, int lineID)
{
	for (int i = symTabTop; i >= 0; i--)
	{
		if (toLowerCase(idenfr) == toLowerCase(symTab[i].idenfr))
		{
			return 0;
		}
	}
	error(lineID, "c");
	return 1;
}

int checkDupDef(string idenfr, int lineID)
{
	for (int i = symTabTop; i >= subProcIndexTable[levelNow]; i--)
	{
		if (toLowerCase(idenfr) == toLowerCase(symTab[i].idenfr))
		{
			error(lineID, "b");
			return 1;
		}
	} return 0;
}

void levelUp()
{
	levelNow++;
	subProcIndexTable[levelNow] = symTabTop + 1;
}

void levelDown(bool delAll)
{
	symTabTop = subProcIndexTable[levelNow] - 1;
	levelNow--;

	if (delAll)
	{
		symTabTop--; // 把函数名也删了
		return;
	}
	//函数参数保留
	for (int i = symTabTop + 1; symTab[i].kind == PARAM; i++) {
		symTab[i].idenfr = "";
		//symTab[i].level--;
		symTabTop++;
	}
}

void insertWordList(int index, string label, string idenfr, int lineID, bool isAtHead)
{
	wordList[index].label = label;
	wordList[index].idenfr = idenfr;
	wordList[index].lineID = lineID;
	wordList[index].isAtHead = isAtHead;
}

int getSym(bool forward, bool withOutput = false)
{
	if (forward)
	{
		if (withOutput && wordNow.label != "" && wordNow.idenfr != "")
			cout << wordNow.label << " " << wordNow.idenfr << endl;

		if (wordCnt >= top)
		{
			wordNow.label = wordNow.idenfr = "";
			return 0;
		}
		else
		{
			wordNow = wordList[wordCnt];
			wordCnt++;
			return 1;
		}
	}
	else
	{
		wordNow = wordList[wordCnt - 2];
		wordCnt--;
		return 1;
	}
}

void TokenInit()
{
	keptLabelMap.insert(map<string, string>::value_type("const", "CONSTTK"));
	keptLabelMap.insert(map<string, string>::value_type("int", "INTTK"));
	keptLabelMap.insert(map<string, string>::value_type("char", "CHARTK"));
	keptLabelMap.insert(map<string, string>::value_type("void", "VOIDTK"));
	keptLabelMap.insert(map<string, string>::value_type("main", "MAINTK"));
	keptLabelMap.insert(map<string, string>::value_type("if", "IFTK"));
	keptLabelMap.insert(map<string, string>::value_type("else", "ELSETK"));
	keptLabelMap.insert(map<string, string>::value_type("switch", "SWITCHTK"));
	keptLabelMap.insert(map<string, string>::value_type("case", "CASETK"));
	keptLabelMap.insert(map<string, string>::value_type("default", "DEFAULTTK"));
	keptLabelMap.insert(map<string, string>::value_type("while", "WHILETK"));
	keptLabelMap.insert(map<string, string>::value_type("for", "FORTK"));
	keptLabelMap.insert(map<string, string>::value_type("scanf", "SCANFTK"));
	keptLabelMap.insert(map<string, string>::value_type("printf", "PRINTFTK"));
	keptLabelMap.insert(map<string, string>::value_type("return", "RETURNTK"));
}

string toLowerCase(string in)
{
	string out = "";
	for (int i = 0; i < in.length(); i++)
	{
		out += in[i] >= 'a' && in[i] <= 'z' ? in[i] : in[i] - ('A' - 'a');
	}
	return out;
}

int getNewIndex(int start, string str)
{
	int i = start;
	while (str[i] == '\t' || str[i] == ' ') i++;
	return i;
}
#pragma endregion