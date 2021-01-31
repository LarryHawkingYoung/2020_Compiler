#include<iostream>
#include<string>
#include<fstream>
#include<map>
#include"SymbolTable.h"
#include"ErrorProcessing.h"

using namespace std;

ErrorProcessing errProc = ErrorProcessing();

ifstream fin("testfile.txt");
//ofstream fout("output.txt");

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

#pragma region Grammar Parse Funcs Declare
void parseProgram();
void parseConstDcrpt();
void parseConstDef();
void parsePerConstDef(int type);
void parseInteger();
void parseChar();
void parseUnsignedInt();
int parseConst();
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
void parseCondition();
int parseExpr();
int parseTerm();
int parseFactor();
void parseFuncCallWithReturn();
void parseFuncCallWithoutReturn();
void parseValueParamTable(FuncInfo funcInfo);
void parseStep();
void parseFuncCall();
void parseCondStmt();
void parseReadStmt();
void parseWriteStmt();
void parseSwitchStmt();
void parseCondTable(int type);
void parseCondChildStmt(int type);
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

	showSymbolTable();

	cout << errProc.getErrNum() << endl;

	cout << "\nfinished" << endl;
	//getchar();

	errProc.outputErrors();

	return 0;
}

void error(int lineID, string errType)
{
	//cout << lineID << " " << errType << endl;
	errProc.insertError(lineID, errType);
}




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

	if (!checkDupDef(wordNow.idenfr, wordNow.lineID)) // 没有标识符重定义
	{
		symTab[++symTabTop] = Symbol{ wordNow.idenfr, CONST, type, levelNow, 0 };
	}

	getSym(true, true); // =
	getSym(true, true); // First(整数 / 字符)
	if (type == INT) parseInteger();
	else parseChar();
}

void parseChar()
{
	if (wordNow.label != "CHARCON")
	{
		printf("Not CHARCON ! In parseChar");
		getchar();
	}
	getSym(true, true);
}

void parseInteger()
{
	if (wordNow.label == "PLUS" || wordNow.label == "MINU")
	{
		getSym(true, true); // first(<无符号整数>)
	}
	parseUnsignedInt();

	cout << "<整数>" << endl;
}

void parseUnsignedInt()
{
	if (wordNow.label != "INTCON")
	{
		printf("Not INTCON ! In parseUnsignedInt");
		getchar();
	}
	getSym(true, true);
	cout << "<无符号整数>" << endl;
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
		D1 = stoi(wordNow.idenfr);
		parseUnsignedInt();// ]
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
			D2 = stoi(wordNow.idenfr);
			parseUnsignedInt();// ]
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

		// 二维
		if (D2 >= 0)
		{
			int m = D1, n = D2;
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
					if (type != parseConst()) error(wordNow.lineID, "o");
					if (n != 0 && wordNow.label != "COMMA") { // 参数少了
						error(wordNow.lineID, "n");
						while (wordNow.label != "SEMICN") getSym(true, true);
						cout << "<变量定义及初始化>" << endl;
						return;
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
			getSym(true, true);
			if (wordNow.label != "LBRACE") error(wordNow.lineID, "n");
			while (D1--)
			{
				getSym(true, true);
				if (wordNow.label != "PLUS" && wordNow.label != "MINU" && wordNow.label != "INTCON" && wordNow.label != "CHARCON")
				{
					error(wordNow.lineID, "n");
				}
				if (type != parseConst()) error(wordNow.lineID, "o");
				if (D1 != 0 && wordNow.label != "COMMA") {
					error(wordNow.lineID, "n");
					while (wordNow.label != "SEMICN") getSym(true, true);
					cout << "<变量定义及初始化>" << endl;
					return;
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
			if (type != parseConst()) error(wordNow.lineID, "o");
		}

		cout << "<变量定义及初始化>" << endl;
	}
	// 变量定义无初始化
	else
	{// 此时已经读到了 , 或 ;
		if (!checkDupDef(idenfr, lineID))
			symTab[++symTabTop] = Symbol{ idenfr, VAR, type, levelNow, dim };

		while (wordNow.label == "COMMA")
		{
			getSym(true, true);
			parsePerVarDefNotInit(type);
		}//若退出了循环，则表明遇到了 非, 可能为; 如果不是; 则在退出后会报错

		cout << "<变量定义无初始化>" << endl;
	}

	cout << "<变量定义>" << endl;
}

void parsePerVarDefNotInit(int type)
{
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
		parseUnsignedInt();
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
			parseUnsignedInt();
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
}

int parseConst()
{
	if (wordNow.label == "INTCON" || wordNow.label == "PLUS" || wordNow.label == "MINU")
	{
		parseInteger();
		cout << "<常量>" << endl;
		return INT;
	}
	else if (wordNow.label == "CHARCON")
	{
		getSym(true, true);
		cout << "<常量>" << endl;
		return CHAR;
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
	checkDupDef(wordNow.idenfr, wordNow.lineID);
	symTab[++symTabTop] = Symbol{ wordNow.idenfr, PARAM, type, levelNow, 0 };
	getSym(true, true);
	while (wordNow.label == "COMMA")
	{
		getSym(true, true);// int or char
		if (wordNow.label == "INTTK") type = INT;
		else if (wordNow.label == "CHARTK") type = CHAR;
		getSym(true, true);// 标识符
		checkDupDef(wordNow.idenfr, wordNow.lineID);
		symTab[++symTabTop] = Symbol{ wordNow.idenfr, PARAM, type, levelNow, 0 };
		getSym(true, true);
	}

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
		getSym(true, true);
		getSym(true, true);
		parseCondition();
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
	}
	else if (wordNow.label == "FORTK")
	{
		getSym(true, true); // (
		getSym(true, true); // 标识符
		checkUndefined(wordNow.idenfr, wordNow.lineID);
		getSym(true, true); // =
		getSym(true, true); // first(表达式)
		parseExpr(); // ;
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
		parseCondition(); // ;
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
		checkUndefined(wordNow.idenfr, wordNow.lineID);
		getSym(true, true); // + / -
		getSym(true, true); // first(步长)
		parseStep(); // )
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
	}

	cout << "<循环语句>" << endl;
}

void parseStep()
{
	parseUnsignedInt();

	cout << "<步长>" << endl;
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
		int type = parseExpr();
		cnt++;
		if (type != symTab[cnt + funcInfo.pos].type) error(wordNow.lineID, "e");
		while (wordNow.label == "COMMA")
		{
			getSym(true, true);
			type = parseExpr();
			cnt++;
			if (type != symTab[cnt + funcInfo.pos].type) error(wordNow.lineID, "e");
		}
		if (cnt != funcInfo.paramNum) error(wordNow.lineID, "d");
	}

	cout << "<值参数表>" << endl;
}

void parseFuncCallWithReturn()
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

	cout << "<有返回值函数调用语句>" << endl;
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

int parseFactor()
{
	int type = -1;
	// 整数
	if (wordNow.label == "PLUS" || wordNow.label == "MINU" || wordNow.label == "INTCON")
	{
		type = INT;
		parseInteger();
	}
	// 字符
	else if (wordNow.label == "CHARCON")
	{
		type = CHAR;
		getSym(true, true);
	}
	// 有返回值函数调用
	else if (keptLabelMap.find(wordNow.idenfr) != keptLabelMap.end() && keptLabelMap.find(wordNow.idenfr)->second == "RETURNABLE")
	{
		type = getFuncInfo(wordNow.idenfr).type;
		parseFuncCallWithReturn();
	}
	// 表达式
	else if (wordNow.label == "LPARENT")
	{
		type = INT;
		getSym(true, true);
		parseExpr();
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
		// 标识符
		int isUndefined = checkUndefined(wordNow.idenfr, wordNow.lineID);
		if (!isUndefined) type = getQuantityInfo(wordNow.idenfr).type;
		getSym(true, true);
		if (wordNow.label == "LBRACK")
		{
			getSym(true, true);
			if (!isUndefined && parseExpr() == CHAR) error(wordNow.lineID, "i");
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
				getSym(true, true);
				if (!isUndefined && parseExpr() == CHAR) error(wordNow.lineID, "i");
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
	}

	cout << "<因子>" << endl;
	return type;
}

int parseTerm()
{
	int type = -1;
	type = parseFactor();
	while (wordNow.label == "MULT" || wordNow.label == "DIV")
	{
		type = INT;
		getSym(true, true);
		parseFactor();
	}

	cout << "<项>" << endl;
	return type;
}

int parseExpr()
{
	int type = -1;
	if (wordNow.label == "PLUS" || wordNow.label == "MINU")
	{
		type = INT;
		getSym(true, true);
	}
	int tmp = parseTerm();
	if (type < 0) type = tmp;
	while (wordNow.label == "PLUS" || wordNow.label == "MINU")
	{
		type = INT;
		getSym(true, true);
		parseTerm();
	}

	cout << "<表达式>" << endl;

	return type;
}

void parseCondition()
{
	int type;
	type = parseExpr();
	if (type != INT) error(wordNow.lineID, "f");
	getSym(true, true);
	type = parseExpr();
	if (type != INT) error(wordNow.lineID, "f");

	cout << "<条件>" << endl;
}

void parseCondStmt()
{
	// if
	getSym(true, true); // (
	getSym(true, true); // first(条件)
	parseCondition();
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
	if (wordNow.label == "ELSETK")
	{
		getSym(true, true);
		parseStmt();
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
	getSym(true, true);
	getSym(true, true);
	if (wordNow.label == "STRCON")
	{
		parseString();
		if (wordNow.label == "COMMA")
		{
			getSym(true, true);
			parseExpr();
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
	}
	else
	{
		parseExpr();
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

	cout << "<写语句>" << endl;
}

void parseCondChildStmt(int type)
{
	getSym(true, true);
	if (type != parseConst()) error(wordNow.lineID, "o");
	getSym(true, true);
	parseStmt();

	cout << "<情况子语句>" << endl;
}

void parseCondTable(int type)
{
	while (wordNow.label == "CASETK")
	{
		parseCondChildStmt(type);
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
	int type = parseExpr(); // )
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
	getSym(true, true); // first(情况表)
	parseCondTable(type); // first(缺省)
	if (wordNow.label != "DEFAULTTK") error(wordNow.lineID, "p");
	else parseDefault(); // }
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
		int type = parseExpr();
		if (funcRetFlag == 1 && type != retType) error(wordNow.lineID, "h");// 表达式类型与返回值类型不一致
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
	if (keptLabelMap.find(wordNow.idenfr) != keptLabelMap.end() && keptLabelMap.find(wordNow.idenfr)->second == "RETURNABLE")
	{
		parseFuncCallWithReturn();
	}
	else if (keptLabelMap.find(wordNow.idenfr) != keptLabelMap.end() && keptLabelMap.find(wordNow.idenfr)->second == "UNRETURNABLE")
	{
		parseFuncCallWithoutReturn();
	}
}

void parseAssignStmt()
{
	int isUndefined = checkUndefined(wordNow.idenfr, wordNow.lineID);
	if (!isUndefined && getQuantityInfo(wordNow.idenfr).kind == CONST) error(wordNow.lineID, "j");
	getSym(true, true);
	if (wordNow.label == "ASSIGN")
	{
		getSym(true, true);
		parseExpr();
	}
	else if (wordNow.label == "LBRACK")
	{
		getSym(true, true);
		if (parseExpr() == CHAR) error(wordNow.lineID, "i");
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
			parseExpr();
		}
		else if (wordNow.label == "LBRACK")
		{
			getSym(true, true);
			if (parseExpr() == CHAR) error(wordNow.lineID, "i");
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
			parseExpr();
		}
	}

	cout << "<赋值语句>" << endl;
}

void parseStmtList()
{
	while (wordNow.label != "RBRACE")
	{
		parseStmt();
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
	string str = wordNow.label;// 返回值类型 void/int/char
	getSym(true, false);// idenfr 函数名
	while (str != "VOIDTK" || wordNow.label != "MAINTK")
	{
		getSym(false, false);// 返回值类型 void/int/char
		if (wordNow.label == "VOIDTK")
		{
			funcRetFlag = 0; // 进入void函数
			getSym(true, true);// void的函数名
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
	}
	getSym(false, false);
}

void parseMainFunc()
{
	// void
	funcRetFlag = 0; // 进入void函数
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
	levelDown();
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
	FuncInfo funcInfo = { -1, -1, -1 };// [0]:func pos [1]:param num [2]:return type
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