#include<iostream>
#include<string>
#include<fstream>
#include<sstream>
#include<map>
#include<vector>
#include"ErrorProcessing.h"
#include"IntermediateCode.h"
#include"Function.h"
#include"Utils.h"

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
	int type = INT;
	string value = "";
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
vector<string> FuncVec;
Function curFunc;
vector<string> curFuncInterQuat;
bool isInFunc = false;
bool isInMain = false;
//bool raChanged = false;

//bool printLock = false;


void showSymbolTable()
{
	cout << "idenfr\t" << "kind\t" << "type\t" << "level\t" << "dim" << endl;
	for (int i = 0; i <= symTabTop; i++) {
		Symbol sym = symTab[i];
		cout << sym.idenfr << "\t" << numToKind(sym.kind) << "\t" << numToType(sym.type) << "\t" << sym.level << "\t" << sym.dim << endl;
	}
}

void showCodeGenSymTab()
{
	cout << "[idenfr]\t" << "[kind]\t" << "[type]\t" << "[level]\t" << "[offset]\t" << "[dim]" << endl;
	for (int i = 0; i < CodeGenSymTab.size(); i++) {
		Symbol sym = CodeGenSymTab[i];
		cout << sym.idenfr << "\t" << numToKind(sym.kind) << "\t" << numToType(sym.type) << "\t" << sym.level << "\t" << sym.addrOffset << "\t" << sym.dim << endl;
	}
}

void regOptimize()
{
	map<string, int> indexMap;
	for (int i = 0; i < CodeGenSymTab.size(); i++) {
		Symbol sym = CodeGenSymTab[i];
		if (sym.idenfr[0] != '~' && sym.dim == 0)
		{
			indexMap[sym.idenfr] = i;
			interCode.processSym(sym.idenfr);
		}
	}
	vector<SymAndCnt> SortedSymCnt = interCode.getSortedSymCnt();
	for (int i = 0; i < SortedSymCnt.size() && i < getRegPoolSize(); i++)
	{
		string idenfr = SortedSymCnt[i].idenfr;
		int index = indexMap[idenfr];
		CodeGenSymTab[index].addrOffset = regPool[i];
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
void parseNewCondition(string label);
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
					if (ch == '\\') strcon = strcon + ch + ch;
					else strcon = strcon + ch;
					ind++;
				}
				if (strcon == "") error(lineID, "a");
				insertWordList(top, "STRCON", strcon, lineID, isAtHead);
				isAtHead = false;
				top++;
				break;
			case '\'':
				ind++;
				if (line[ind] != '+' && line[ind] != '-' && line[ind] != '*' && line[ind] != '/' && line[ind] != '_' && !((line[ind] >= 'a' && line[ind] <= 'z') || (line[ind] >= 'A' && line[ind] <= 'Z') || (line[ind] >= '0' && line[ind] <= '9')))
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
				if (ch == '_' || (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z'))
				{
					idenfr.clear();
					idenfr += ch;
					ch = line[ind + 1];
					while (ch == '_' || (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9'))
					{
						idenfr += ch;
						ind++;
						ch = line[ind + 1];
					}
					map<string, string>::iterator iter = keptLabelMap.find(toLowerCase(idenfr));
					if (iter == keptLabelMap.end())
					{
						insertWordList(top, "IDENFR", toLowerCase(idenfr), lineID, isAtHead);
						isAtHead = false;
						top++;
					}
					else
					{
						insertWordList(top, iter->second, toLowerCase(idenfr), lineID, isAtHead);
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
		interCode.insertQuat("EndMain", "", ":", "");
	}

	fin.close();
	//fout.close();

	cout << endl;

	errProc.outputErrors();

	showSymbolTable();

	cout << "\nTotal Error Number : " << errProc.getErrNum() << endl;

	cout << "\nfinished" << endl;

	cout << "\nstrVec" << endl;

	showStrVec();

	cout << "\nfuncInfo" << endl;
	for (int i = 0; i < FuncVec.size(); i++)
	{
		cout << "+++++++++++++++[" << FuncVec[i] << "]++++++++++++++++++" << endl;
		FuncMap[FuncVec[i]].showFuncInfo();
		cout << "\n" << endl;
	}

	cout << "\nCodeGenSymbol_Before" << endl;
	showCodeGenSymTab();
	regOptimize();
	cout << "\nCodeGenSymbol_After" << endl;
	showCodeGenSymTab();


	interCode.OptimizeCode();
	interCode.outputQuats();


	targetCodeGenerating();

	//getchar();
	return 0;
}


#pragma region Target Code Generation
void targetCodeGenerating()
{
	initStr();
	mout << ".text" << endl;
	mout << "\tli $t0 0x10010000" << endl;
	for (int i = 0; i < interCode.getQuatNum(); i++)
	{
		Quaternion quat = interCode.getQuaternion(i);
		mout << "############################### [" << quat.ans << "]\t[" << quat.x << "]\t[" << quat.op << "]\t[" << quat.y << "]" << endl;
		if (quat.op == "read")
		{
			Symbol sym = getSymbolInfo(quat.ans);
			if (sym.type == INT) mout << "\tli $v0 5" << endl;
			else if (sym.type == CHAR) mout << "\tli $v0 12" << endl;
			mout << "\tsyscall" << endl;
			if (sym.addrOffset[0] == '$')
				mout << "\tmove " << sym.addrOffset << " $v0" << endl;
			else
				mout << "\tsw $v0 " << sym.addrOffset << "($t0)" << endl;
		}
		else if (quat.op == "ReadByType")
		{
			if (quat.x == "INT") mout << "\tli $v0 5" << endl;
			else if (quat.x == "CHAR") mout << "\tli $v0 12" << endl;
			mout << "\tsyscall" << endl;
		}
		else if (quat.op == "print")
		{
			if (quat.x != "") // 有字符串
			{
				mout << "\tli $v0 4" << endl;
				mout << "\tla $a0 " << quat.x << endl;
				mout << "\tsyscall" << endl;
			}
			if (quat.y != "") // 有表达式
			{
				if (quat.y[0] == '\'') // 字符常量
				{
					mout << "\tli $v0 11" << endl;
					mout << "\tli $a0 " << quat.y << endl;
				}
				else if ((quat.y[0] >= '0' && quat.y[0] <= '9') || quat.y[0] == '-') // 整数常量
				{
					mout << "\tli $v0 1" << endl;
					mout << "\tli $a0 " << quat.y << endl;
				}
				else
				{
					Symbol sym = getSymbolInfo(quat.y);
					if (sym.type == INT) mout << "\tli $v0 1" << endl;
					else if (sym.type == CHAR) mout << "\tli $v0 11" << endl;
					if (sym.addrOffset[0] == '$')
						mout << "\tmove $a0 " << sym.addrOffset << endl;
					else
						mout << "\tlw $a0 " << sym.addrOffset << "($t0)" << endl;
				}
				mout << "\tsyscall" << endl;
			}

			mout << "\tli $v0 11" << endl;
			mout << "\tli $a0 \'\\n\'" << endl;
			mout << "\tsyscall" << endl;
		}
		else if (quat.op == "ARRSETbyVAR")
		{
			if (isDigit(quat.ans) || isChar(quat.ans)) mout << "\tli $t1 " << quat.ans << endl;
			else
			{
				Symbol ans = getSymbolInfo(quat.ans);
				if (ans.addrOffset[0] == '$')
					mout << "\tmove $t1 " << ans.addrOffset << endl;
				else
					mout << "\tlw $t1 " << ans.addrOffset << "($t0)" << endl;
			}
			Symbol arr = getSymbolInfo(quat.x);
			Symbol offset = getSymbolInfo(quat.y);
			if (offset.addrOffset[0] == '$')
			{
				mout << "\taddi $t2 " << offset.addrOffset << " " << arr.addrOffset << endl;
			}
			else
			{
				mout << "\tlw $t2 " << offset.addrOffset << "($t0)" << endl;
				mout << "\taddi $t2 $t2 " << arr.addrOffset << endl;
			}

			///////////////////////////////////////////////mout << "\tadd $t2 $t2 $t0" << endl;
			mout << "\taddu $t2 $t2 $t0" << endl;

			mout << "\tsw $t1 0($t2)" << endl;
		}
		else if (quat.op == "ARRSETbyCONST")
		{
			if (isDigit(quat.ans) || isChar(quat.ans)) mout << "\tli $t1 " << quat.ans << endl;
			else
			{
				Symbol ans = getSymbolInfo(quat.ans);
				if (ans.addrOffset[0] == '$')
					mout << "\tmove $t1 " << ans.addrOffset << endl;
				else
					mout << "\tlw $t1 " << ans.addrOffset << "($t0)" << endl;
			}
			Symbol arr = getSymbolInfo(quat.x);
			mout << "\tsw $t1 " << (stoi(arr.addrOffset) + stoi(quat.y)) << "($t0)" << endl;
		}
		else if (quat.op == "ARRGET")
		{
			Symbol ans = getSymbolInfo(quat.ans);
			Symbol arr = getSymbolInfo(quat.x);
			Symbol offset = getSymbolInfo(quat.y);
			if (offset.addrOffset[0] == '$')
			{
				mout << "\taddi $t1 " << offset.addrOffset << " " << arr.addrOffset << endl;
			}
			else
			{
				mout << "\tlw $t1 " << offset.addrOffset << "($t0)" << endl;
				mout << "\taddi $t1 $t1 " << arr.addrOffset << endl;
			}

			//////////////////////////////////////////////////mout << "\tadd $t1 $t1 $t0" << endl;
			mout << "\taddu $t1 $t1 $t0" << endl;

			if (ans.addrOffset[0] == '$')
				mout << "\tlw " << ans.addrOffset << " 0($t1)" << endl;
			else
			{
				mout << "\tlw $t1 " << "0($t1)" << endl;
				mout << "\tsw $t1 " << ans.addrOffset << "($t0)" << endl;
			}
		}
		else if (quat.op == "spARRGET")
		{
			Symbol ans = getSymbolInfo(quat.ans);
			Symbol offset = getSymbolInfo(quat.x);
			if (offset.addrOffset[0] == '$')
				mout << "\taddu $t1 " << offset.addrOffset << " $sp" << endl;
			else
			{
				mout << "\tlw $t1 " << offset.addrOffset << "($t0)" << endl;
				mout << "\taddu $t1 $t1 $sp" << endl;
			}

			if (ans.addrOffset[0] == '$')
				mout << "\tlw " << ans.addrOffset << " 0($t1)" << endl;
			else
			{
				mout << "\tlw $t1 0($t1)" << endl;
				mout << "\tsw $t1 " << ans.addrOffset << "($t0)" << endl;
			}
		}
		else if (quat.op == "spGET")
		{
			Symbol ans = getSymbolInfo(quat.ans);
			string offset = quat.x;
			if (ans.addrOffset[0] == '$')
			{
				mout << "\tlw " << ans.addrOffset << " " << offset << "($sp)" << endl;
			}
			else
			{
				mout << "\tlw $t1 " << offset << "($sp)" << endl;
				mout << "\tsw $t1 " << ans.addrOffset << "($t0)" << endl;
			}
		}
		else if (quat.op == "spARRSET" || quat.op == "spSET" || quat.op == "spPush")
		{
			Symbol ans = getSymbolInfo(quat.ans);
			if (isDigit(quat.ans) || isChar(quat.ans)) mout << "\tli $t1 " << quat.ans << endl;
			else
			{
				if (ans.addrOffset[0] != '$')
					mout << "\tlw $t1 " << ans.addrOffset << "($t0)" << endl;
			}
			
			if (isDigit(quat.x) || isChar(quat.x))
			{
				if (ans.addrOffset[0] != '$')
					mout << "\tsw $t1 " << quat.x << "($sp)" << endl;
				else
					mout << "\tsw " << ans.addrOffset << " " << quat.x << "($sp)" << endl;
			}
			else
			{
				Symbol x = getSymbolInfo(quat.x);
				if (x.addrOffset[0] == '$')
				{
					mout << "\taddu $t2 $sp " << x.addrOffset << endl;
				}
				else
				{
					mout << "\tlw $t2 " << x.addrOffset << "($t0)" << endl;
					mout << "\taddu $t2 $t2 $sp" << endl;
				}

				if (ans.addrOffset[0] != '$')
					mout << "\tsw $t1 0($t2)" << endl;
				else
					mout << "\tsw " << ans.addrOffset << " 0($t2)" << endl;
			}
		}
		else if (quat.op == "spSET$v0")
		{
			string offset = quat.x;
			mout << "\tsw $v0 " << offset << "($sp)" << endl;
		}
		else if (quat.op == "spFree")
		{
			mout << "\taddi $sp $sp " + quat.x << endl;
		}
		else if (quat.op == "spAlloc")
		{
			mout << "\taddi $sp $sp -" + quat.x << endl;
		}
		else if (quat.op == "jr")
		{
			mout << "\tjr $ra" << endl;
		}
		else if (quat.op == "$v0")
		{
			Symbol ans = getSymbolInfo(quat.ans);
			if (ans.addrOffset[0] == '$')
				mout << "\tmove " << ans.addrOffset << " $v0" << endl;
			else
				mout << "\tsw $v0 " << ans.addrOffset << "($t0)" << endl;
		}
		else if (quat.op == "return")
		{
			if (isDigit(quat.ans) || isChar(quat.ans)) mout << "\tli $v0 " << quat.ans << endl;
			else
			{
				Symbol ans = getSymbolInfo(quat.ans);
				if (ans.addrOffset[0] == '$')
					mout << "\tmove $v0 " << ans.addrOffset << endl;
				else
					mout << "\tlw $v0 " << ans.addrOffset << "($t0)" << endl;
			}
		}
		else if (quat.op == "jal")
		{
			mout << "\tjal " << quat.ans << endl;
		}
		else if (quat.op == "j")
		{
			mout << "\tj " << quat.ans << endl;
		}
		else if (quat.op == ":")
		{
			mout << quat.ans << " :" << endl;
		}
		else if (isCmpOp(quat.op))
		{
			string x = "$t1";
			if (isDigit(quat.x) || isChar(quat.x)) mout << "\tli $t1 " << quat.x << endl;
			else
			{
				Symbol quatX = getSymbolInfo(quat.x);
				if (quatX.addrOffset[0] == '$')
					x = quatX.addrOffset;
				else
					mout << "\tlw $t1 " << quatX.addrOffset << "($t0)" << endl;
			}
			
			string y = "$t2";
			if (isDigit(quat.y) || isChar(quat.y))
			{
				y = quat.y;
			}
			else
			{
				Symbol quatY = getSymbolInfo(quat.y);
				if (quatY.addrOffset[0] == '$')
					y = quatY.addrOffset;
				else
					mout << "\tlw $t2 " << quatY.addrOffset << "($t0)" << endl;
			}
			
			string label = quat.ans;
			if (quat.op == "==") mout << "\tbeq " << x << " " << y << " " << label << endl;
			else if (quat.op == "!=") mout << "\tbne " << x << " " << y << " " << label << endl;
			else if (quat.op == "<") mout << "\tblt " << x << " " << y << " " << label << endl;
			else if (quat.op == "<=") mout << "\tble " << x << " " << y << " " << label << endl;
			else if (quat.op == ">") mout << "\tbgt " << x << " " << y << " " << label << endl;
			else if (quat.op == ">=") mout << "\tbge " << x << " " << y << " " << label << endl;
		}
		else if (quat.op == "addi")
		{
			Symbol ans = getSymbolInfo(quat.ans);
			Symbol x = getSymbolInfo(quat.x);
			if (x.addrOffset[0] == '$' && ans.addrOffset[0] == '$')
			{
				mout << "\taddi " << ans.addrOffset << " " << x.addrOffset << " " << quat.y << endl;
			}
			else if (x.addrOffset[0] == '$')
			{
				mout << "\taddi $t1 " << x.addrOffset << " " << quat.y << endl;
				mout << "\tsw $t1 " << ans.addrOffset << "($t0)" << endl;
			}
			else if (ans.addrOffset[0] == '$')
			{
				mout << "\tlw $t1 " << x.addrOffset << "($t0)" << endl;
				mout << "\taddi " << ans.addrOffset << " $t1 " << quat.y << endl;
			}
			else
			{
				mout << "\tlw $t1 " << x.addrOffset << "($t0)" << endl;
				mout << "\taddi $t1 $t1 " << quat.y << endl;
				mout << "\tsw $t1 " << ans.addrOffset << "($t0)" << endl;
			}
		}
		else if (quat.op == "raPush") mout << "\tsw $ra 0($sp)" << endl;
		else if (quat.op == "raPop") mout << "\tlw $ra 0($sp)" << endl;
		// 赋值语句
		else
		{
			Symbol ans = getSymbolInfo(quat.ans);
			if (quat.op == "")
			{
				if (isChar(quat.x) || isDigit(quat.x)) // 字符常量 / 整数常量
				{
					if (ans.addrOffset[0] == '$') mout << "\tli " << ans.addrOffset << " " << quat.x << endl;
					else
					{
						mout << "\tli $t1 " << quat.x << endl;
						mout << "\tsw $t1 " << ans.addrOffset << "($t0)" << endl;
					}
				}
				else
				{
					Symbol sym = getSymbolInfo(quat.x);
					if (sym.addrOffset[0] == '$')
					{
						if (ans.addrOffset[0] == '$') mout << "\tmove " << ans.addrOffset << " " << sym.addrOffset << endl;
						else mout << "\tsw " << sym.addrOffset << " " << ans.addrOffset << "($t0)" << endl;
					}
					else
					{
						if (ans.addrOffset[0] == '$') mout << "\tlw " << ans.addrOffset << " " << sym.addrOffset << "($t0)" << endl;
						else
						{
							mout << "\tlw $t1 " << sym.addrOffset << "($t0)" << endl;
							mout << "\tsw $t1 " << ans.addrOffset << "($t0)" << endl;
						}
					}
				}
			}
			else if (quat.x == "" && quat.op == "-")
			{
				if (isDigit(quat.y) || isChar(quat.y))
				{
					if (ans.addrOffset[0] == '$') mout << "\tli " << ans.addrOffset << " " << calc("0", quat.y, SUB) << endl;
					else
					{
						mout << "\tli $t1 " << calc("0", quat.y, SUB) << endl;
						mout << "\tsw $t1 " << ans.addrOffset << "($t0)" << endl;
					}
				}
				else
				{
					Symbol sym = getSymbolInfo(quat.y);
					if (sym.addrOffset[0] == '$')
					{
						if (ans.addrOffset[0] == '$') mout << "\tsub " << ans.addrOffset << " $0 " << sym.addrOffset << endl;
						else
						{
							mout << "\tsub $t1 $0 " << sym.addrOffset << endl;
							mout << "\tsw $t1 " << ans.addrOffset << "($t0)" << endl;
						}
					}
					else
					{
						if (ans.addrOffset[0] == '$')
						{
							mout << "\tlw " << ans.addrOffset << " " << sym.addrOffset << "($t0)" << endl;
							mout << "\tsub " << ans.addrOffset << " $0 " << ans.addrOffset << endl;
						}
						else
						{
							mout << "\tlw $t1 " << sym.addrOffset << "($t0)" << endl;
							mout << "\tsub $t1 $0 $t1" << endl;
							mout << "\tsw $t1 " << ans.addrOffset << "($t0)" << endl;
						}
					}
				}
			}
			else
			{
				if (quat.op == "+")
				{
					if (isDigit(quat.x) || isChar(quat.x))
					{
						if (isDigit(quat.y) || isChar(quat.y))
						{
							if (ans.addrOffset[0] == '$') mout << "\tli " << ans.addrOffset << " " << calc(quat.x, quat.y, ADD) << endl;
							else
							{
								mout << "\tli $t1 " << calc(quat.x, quat.y, ADD) << endl;
								mout << "\tsw $t1 " << ans.addrOffset << "($t0)" << endl;
							}
						}
						else
						{
							if (getValue(quat.x) == 0)
							{
								Symbol sym = getSymbolInfo(quat.y);
								if (sym.addrOffset[0] == '$')
								{
									if (ans.addrOffset[0] == '$') mout << "\tmove " << ans.addrOffset << " " << sym.addrOffset << endl;
									else mout << "\tsw " << sym.addrOffset << " " << ans.addrOffset << "($t0)" << endl;
								}
								else
								{
									if (ans.addrOffset[0] == '$') mout << "\tlw " << ans.addrOffset << " " << sym.addrOffset << "($t0)" << endl;
									else
									{
										mout << "\tlw $t1 " << sym.addrOffset << "($t0)" << endl;
										mout << "\tsw $t1 " << ans.addrOffset << "($t0)" << endl;
									}
								}
							}
							else
							{
								Symbol sym = getSymbolInfo(quat.y);
								if (sym.addrOffset[0] == '$')
								{
									if (ans.addrOffset[0] == '$') mout << "\taddiu " << ans.addrOffset << " " << sym.addrOffset << " " << quat.x << endl;
									else
									{
										mout << "\taddiu $t1 " << sym.addrOffset << " " << quat.x << endl;
										mout << "\tsw $t1 " << ans.addrOffset << "($t0)" << endl;
									}
								}
								else
								{
									if (ans.addrOffset[0] == '$')
									{
										mout << "\tlw $t1 " << sym.addrOffset << "($t0)" << endl;
										mout << "\taddiu " << ans.addrOffset << " $t1 " << quat.x << endl;
									}
									else
									{
										mout << "\tlw $t1 " << sym.addrOffset << "($t0)" << endl;
										mout << "\taddiu $t1 $t1 " << quat.x << endl;
										mout << "\tsw $t1 " << ans.addrOffset << "($t0)" << endl;
									}
								}
							}
						}
					}
					else {
						if (isDigit(quat.y) || isChar(quat.y))
						{
							if (getValue(quat.y) == 0)
							{
								Symbol sym = getSymbolInfo(quat.x);
								if (ans.addrOffset[0] == '$' && sym.addrOffset[0] == '$')
								{
									mout << "\tmove " << ans.addrOffset << " " << sym.addrOffset << endl;
								}
								else if (ans.addrOffset[0] == '$')
								{
									mout << "\tlw " << ans.addrOffset << " " << sym.addrOffset << "($t0)" << endl;
								}
								else if (sym.addrOffset[0] == '$')
								{
									mout << "\tsw " << sym.addrOffset << " " << ans.addrOffset << "($t0)" << endl;
								}
								else
								{
									mout << "\tlw $t1 " << sym.addrOffset << "($t0)" << endl;
									mout << "\tsw $t1 " << ans.addrOffset << "($t0)" << endl;
								}
							}
							else
							{
								string tt1 = "$t1";
								Symbol sym = getSymbolInfo(quat.x);
								if (sym.addrOffset[0] == '$') tt1 = sym.addrOffset;
								else mout << "\tlw $t1 " << sym.addrOffset << "($t0)" << endl;

								if (ans.addrOffset[0] == '$')
								{
									mout << "\taddiu " << ans.addrOffset << " " << tt1 << " " << quat.y << endl;
								}
								else
								{
									mout << "\taddiu $t1 " << tt1 << " " << quat.y << endl;
									mout << "\tsw $t1 " << ans.addrOffset << "($t0)" << endl;
								}
							}
						}
						else
						{
							string tt1 = "$t1", tt2 = "$t2";
							Symbol sym1 = getSymbolInfo(quat.x);
							Symbol sym2 = getSymbolInfo(quat.y);
							if (sym1.addrOffset[0] == '$') tt1 = sym1.addrOffset;
							else mout << "\tlw $t1 " << sym1.addrOffset << "($t0)" << endl;
							if (sym2.addrOffset[0] == '$') tt2 = sym2.addrOffset;
							else mout << "\tlw $t2 " << sym2.addrOffset << "($t0)" << endl;

							if (ans.addrOffset[0] == '$')
							{
								mout << "\taddu " << ans.addrOffset << " " << tt1 << " " << tt2 << endl;
							}
							else
							{
								mout << "\taddu $t1 " << tt1 << " " << tt2 << endl;
								mout << "\tsw $t1 " << ans.addrOffset << "($t0)" << endl;
							}
						}
					}
				}
				else if (quat.op == "-")
				{
					if (isDigit(quat.x) || isChar(quat.x))
					{
						if (isDigit(quat.y) || isChar(quat.y))
						{
							if (ans.addrOffset[0] == '$') mout << "\tli " << ans.addrOffset << " " << calc(quat.x, quat.y, SUB) << endl;
							else
							{
								mout << "\tli $t1 " << calc(quat.x, quat.y, SUB) << endl;
								mout << "\tsw $t1 " << ans.addrOffset << "($t0)" << endl;
							}
						}
						else
						{
							if (getValue(quat.x) == 0)
							{
								string tt1 = "$t1";
								Symbol sym = getSymbolInfo(quat.y);
								if (sym.addrOffset[0] == '$') tt1 = sym.addrOffset;
								else mout << "\tlw $t1 " << sym.addrOffset << "($t0)" << endl;

								if (ans.addrOffset[0] == '$')
								{
									mout << "\tsub " << ans.addrOffset << " $0 " << tt1 << endl;
								}
								else
								{
									mout << "\tsub $t1 $0 " << tt1 << endl;
									mout << "\tsw $t1 " << ans.addrOffset << "($t0)" << endl;
								}
							}
							else
							{
								string tt1 = "$t1";
								Symbol sym = getSymbolInfo(quat.y);
								if (sym.addrOffset[0] == '$') tt1 = sym.addrOffset;
								else mout << "\tlw $t1 " << sym.addrOffset << "($t0)" << endl;

								if (ans.addrOffset[0] == '$')
								{
									mout << "\tsub " << ans.addrOffset << " $0 " << tt1 << endl;
									mout << "\taddiu " << ans.addrOffset << " " << ans.addrOffset << " " << quat.x << endl;
								}
								else
								{
									mout << "\tsub $t1 $0 " << tt1 << endl;
									mout << "\taddiu $t1 $t1 " << quat.x << endl;
									mout << "\tsw $t1 " << ans.addrOffset << "($t0)" << endl;
								}
							}
						}
					}
					else {
						if (isDigit(quat.y) || isChar(quat.y))
						{
							if (getValue(quat.y) == 0)
							{
								Symbol sym = getSymbolInfo(quat.x);

								if (ans.addrOffset[0] == '$' && sym.addrOffset[0] == '$')
								{
									mout << "\tmove " << ans.addrOffset << " " << sym.addrOffset << endl;
								}
								else if (ans.addrOffset[0] == '$')
								{
									mout << "\tlw " << ans.addrOffset << " " << sym.addrOffset << "($t0)" << endl;
								}
								else if (sym.addrOffset[0] == '$')
								{
									mout << "\tsw " << sym.addrOffset << " " << ans.addrOffset << "($t0)" << endl;
								}
								else
								{
									mout << "\tlw $t1 " << sym.addrOffset << "($t0)" << endl;
									mout << "\tsw $t1 " << ans.addrOffset << "($t0)" << endl;
								}
							}
							else
							{
								string tt1 = "$t1";
								Symbol sym = getSymbolInfo(quat.x);
								if (sym.addrOffset[0] == '$') tt1 = sym.addrOffset;
								else mout << "\tlw $t1 " << sym.addrOffset << "($t0)" << endl;

								if (isChar(quat.y)) quat.y = "-" + to_string((int)quat.y[1]);
								else if (isDigit(quat.y))
								{
									if (quat.y[0] == '-') quat.y = quat.y.substr(1);
									else quat.y = "-" + quat.y;
								}

								if (ans.addrOffset[0] == '$')
								{
									mout << "\taddiu " << ans.addrOffset << " " << tt1 << " " << quat.y << endl;
								}
								else
								{
									mout << "\taddiu $t1 " << tt1 << " " << quat.y << endl;
									mout << "\tsw $t1 " << ans.addrOffset << "($t0)" << endl;
								}
							}
						}
						else
						{
							string tt1 = "$t1", tt2 = "$t2";
							Symbol sym1 = getSymbolInfo(quat.x);
							Symbol sym2 = getSymbolInfo(quat.y);
							if (sym1.addrOffset[0] == '$') tt1 = sym1.addrOffset;
							else mout << "\tlw $t1 " << sym1.addrOffset << "($t0)" << endl;
							if (sym2.addrOffset[0] == '$') tt2 = sym2.addrOffset;
							else mout << "\tlw $t2 " << sym2.addrOffset << "($t0)" << endl;

							if (ans.addrOffset[0] == '$')
							{
								mout << "\tsub " << ans.addrOffset << " " << tt1 << " " << tt2 << endl;
							}
							else
							{
								mout << "\tsub $t1 " << tt1 << " " << tt2 << endl;
								mout << "\tsw $t1 " << ans.addrOffset << "($t0)" << endl;
							}
						}
					}
				}
				else if (quat.op == "*")
				{
					if (isDigit(quat.x) || isChar(quat.x))
					{
						if (isDigit(quat.y) || isChar(quat.y))
						{
							if (ans.addrOffset[0] == '$')
							{
								mout << "\tli " << ans.addrOffset << " " << calc(quat.x, quat.y, MUL) << endl;
							}
							else
							{
								mout << "\tli $t1 " << calc(quat.x, quat.y, MUL) << endl;
								mout << "\tsw $t1 " << ans.addrOffset << "($t0)" << endl;
							}
						}
						else
						{
							if (getValue(quat.x) == 1)
							{
								Symbol sym = getSymbolInfo(quat.y);
								if (ans.addrOffset[0] == '$' && sym.addrOffset[0] == '$')
								{
									mout << "\tmove " << ans.addrOffset << " " << sym.addrOffset << endl;
								}
								else if (ans.addrOffset[0] == '$')
								{
									mout << "\tlw " << ans.addrOffset << " " << sym.addrOffset << "($t0)" << endl;
								}
								else if (sym.addrOffset[0] == '$')
								{
									mout << "\tsw " << sym.addrOffset << " " << ans.addrOffset << "($t0)" << endl;
								}
								else
								{
									mout << "\tlw $t1 " << sym.addrOffset << "($t0)" << endl;
									mout << "\tsw $t1 " << ans.addrOffset << "($t0)" << endl;
								}
							}
							else if (getValue(quat.x) == -1)
							{
								string tt1 = "$t1";
								Symbol sym = getSymbolInfo(quat.y);
								if (sym.addrOffset[0] == '$') tt1 = sym.addrOffset;
								else mout << "\tlw $t1 " << sym.addrOffset << "($t0)" << endl;

								if (ans.addrOffset[0] == '$')
								{
									mout << "\tsub " << ans.addrOffset << " $0 " << tt1 << endl;
								}
								else
								{
									mout << "\tsub $t1 $0 " << tt1 << endl;
									mout << "\tsw $t1 " << ans.addrOffset << "($t0)" << endl;
								}
							}
							else if (getValue(quat.x) == 2)
							{
								string tt1 = "$t1";
								Symbol sym = getSymbolInfo(quat.y);
								if (sym.addrOffset[0] == '$') tt1 = sym.addrOffset;
								else mout << "\tlw $t1 " << sym.addrOffset << "($t0)" << endl;

								if (ans.addrOffset[0] == '$')
								{
									mout << "\taddu " << ans.addrOffset << " " << tt1 << " " << tt1 << endl;
								}
								else
								{
									mout << "\taddu $t1 " << tt1 << " " << tt1 << endl;
									mout << "\tsw $t1 " << ans.addrOffset << "($t0)" << endl;
								}
							}
							else if (getValue(quat.x) == -2)
							{
								string tt1 = "$t1";
								Symbol sym = getSymbolInfo(quat.y);
								if (sym.addrOffset[0] == '$') tt1 = sym.addrOffset;
								else mout << "\tlw $t1 " << sym.addrOffset << "($t0)" << endl;

								if (ans.addrOffset[0] == '$')
								{
									mout << "\taddu " << ans.addrOffset << " " << tt1 << " " << tt1 << endl;
									mout << "\tsub " << ans.addrOffset << " $0 " << ans.addrOffset << endl;
								}
								else
								{
									mout << "\taddu $t1 " << tt1 << " " << tt1 << endl;
									mout << "\tsub $t1 $0 $t1" << endl;
									mout << "\tsw $t1 " << ans.addrOffset << "($t0)" << endl;
								}
							}
							else if (getValue(quat.x) == 0)
							{
								if (ans.addrOffset[0] == '$')
								{
									mout << "\tli " << ans.addrOffset << " 0" << endl;
								}
								else
								{
									mout << "\tsw $0 " << ans.addrOffset << "($t0)" << endl;
								}
							}
							else
							{
								string tt1 = "$t1";
								int pow = getPow(getValue(quat.x));
								Symbol sym = getSymbolInfo(quat.y);
								if (sym.addrOffset[0] == '$') tt1 = sym.addrOffset;
								else mout << "\tlw $t1 " << sym.addrOffset << "($t0)" << endl;

								if (pow > 0)
								{
									if (ans.addrOffset[0] == '$')
									{
										mout << "\tsll " << ans.addrOffset << " " << tt1 << " " << pow << endl;
										if (getValue(quat.x) < 0) mout << "\tsub " << ans.addrOffset << " $0 " << ans.addrOffset << endl;
									}
									else
									{
										mout << "\tsll $t1 " << tt1 << " " << pow << endl;
										if (getValue(quat.x) < 0) mout << "\tsub $t1 $0 $t1" << endl;
										mout << "\tsw $t1 " << ans.addrOffset << "($t0)" << endl;
									}
								}
								else
								{
									if (ans.addrOffset[0] == '$') mout << "\tmul " << ans.addrOffset << " " << tt1 << " " << quat.x << endl;
									else
									{
										mout << "\tmul $t1 " << tt1 << " " << quat.x << endl;
										mout << "\tsw $t1 " << ans.addrOffset << "($t0)" << endl;
									}
								}
							}
						}
					}
					else {
						if (isDigit(quat.y) || isChar(quat.y))
						{
							if (getValue(quat.y) == 1)
							{
								Symbol sym = getSymbolInfo(quat.x);
								if (ans.addrOffset[0] == '$' && sym.addrOffset[0] == '$')
								{
									mout << "\tmove " << ans.addrOffset << " " << sym.addrOffset << endl;
								}
								else if (ans.addrOffset[0] == '$')
								{
									mout << "\tlw " << ans.addrOffset << " " << sym.addrOffset << "($t0)" << endl;
								}
								else if (sym.addrOffset[0] == '$')
								{
									mout << "\tsw " << sym.addrOffset << " " << ans.addrOffset << "($t0)" << endl;
								}
								else
								{
									mout << "\tlw $t1 " << sym.addrOffset << "($t0)" << endl;
									mout << "\tsw $t1 " << ans.addrOffset << "($t0)" << endl;
								}
							}
							else if (getValue(quat.y) == -1)
							{
								string tt1 = "$t1";
								Symbol sym = getSymbolInfo(quat.x);
								if (sym.addrOffset[0] == '$') tt1 = sym.addrOffset;
								else mout << "\tlw $t1 " << sym.addrOffset << "($t0)" << endl;

								if (ans.addrOffset[0] == '$')
								{
									mout << "\tsub " << ans.addrOffset << " $0 " << tt1 << endl;
								}
								else
								{
									mout << "\tsub $t1 $0 " << tt1 << endl;
									mout << "\tsw $t1 " << ans.addrOffset << "($t0)" << endl;
								}
							}
							else if (getValue(quat.y) == 2)
							{
								string tt1 = "$t1";
								Symbol sym = getSymbolInfo(quat.x);
								if (sym.addrOffset[0] == '$') tt1 = sym.addrOffset;
								else mout << "\tlw $t1 " << sym.addrOffset << "($t0)" << endl;

								if (ans.addrOffset[0] == '$')
								{
									mout << "\taddu " << ans.addrOffset << " " << tt1 << " " << tt1 << endl;
								}
								else
								{
									mout << "\taddu $t1 " << tt1 << " " << tt1 << endl;
									mout << "\tsw $t1 " << ans.addrOffset << "($t0)" << endl;
								}
							}
							else if (getValue(quat.y) == -2)
							{
								string tt1 = "$t1";
								Symbol sym = getSymbolInfo(quat.x);
								if (sym.addrOffset[0] == '$') tt1 = sym.addrOffset;
								else mout << "\tlw $t1 " << sym.addrOffset << "($t0)" << endl;

								if (ans.addrOffset[0] == '$')
								{
									mout << "\taddu " << ans.addrOffset << " " << tt1 << " " << tt1 << endl;
									mout << "\tsub " << ans.addrOffset << " $0 " << ans.addrOffset << endl;
								}
								else
								{
									mout << "\taddu $t1 " << tt1 << " " << tt1 << endl;
									mout << "\tsub $t1 $0 $t1" << endl;
									mout << "\tsw $t1 " << ans.addrOffset << "($t0)" << endl;
								}
							}
							else if (getValue(quat.y) == 0)
							{
								if (ans.addrOffset[0] == '$')
								{
									mout << "\tli " << ans.addrOffset << " 0" << endl;
								}
								else
								{
									mout << "\tsw $0 " << ans.addrOffset << "($t0)" << endl;
								}
							}
							else
							{
								string tt1 = "$t1";
								int pow = getPow(getValue(quat.y));
								Symbol sym = getSymbolInfo(quat.x);
								if (sym.addrOffset[0] == '$') tt1 = sym.addrOffset;
								else mout << "\tlw $t1 " << sym.addrOffset << "($t0)" << endl;

								if (pow > 0)
								{
									if (ans.addrOffset[0] == '$')
									{
										mout << "\tsll " << ans.addrOffset << " " << tt1 << " " << pow << endl;
										if (getValue(quat.y) < 0) mout << "\tsub " << ans.addrOffset << " $0 " << ans.addrOffset << endl;
									}
									else
									{
										mout << "\tsll $t1 " << tt1 << " " << pow << endl;
										if (getValue(quat.y) < 0) mout << "\tsub $t1 $0 $t1" << endl;
										mout << "\tsw $t1 " << ans.addrOffset << "($t0)" << endl;
									}
								}
								else
								{
									if (ans.addrOffset[0] == '$') mout << "\tmul " << ans.addrOffset << " " << tt1 << " " << quat.y << endl;
									else
									{
										mout << "\tmul $t1 " << tt1 << " " << quat.y << endl;
										mout << "\tsw $t1 " << ans.addrOffset << "($t0)" << endl;
									}
								}
							}
						}
						else
						{
							string tt1 = "$t1", tt2 = "$t2";
							Symbol sym1 = getSymbolInfo(quat.x);
							Symbol sym2 = getSymbolInfo(quat.y);
							if (sym1.addrOffset[0] == '$') tt1 = sym1.addrOffset;
							else mout << "\tlw $t1 " << sym1.addrOffset << "($t0)" << endl;
							if (sym2.addrOffset[0] == '$') tt2 = sym2.addrOffset;
							else mout << "\tlw $t2 " << sym2.addrOffset << "($t0)" << endl;

							if (ans.addrOffset[0] == '$')
							{
								mout << "\tmul " << ans.addrOffset << " " << tt1 << " " << tt2 << endl;
							}
							else
							{
								mout << "\tmul $t1 " << tt1 << " " << tt2 << endl;
								mout << "\tsw $t1 " << ans.addrOffset << "($t0)" << endl;
							}
						}
					}
				}
				else if (quat.op == "/")
				{
					if (isDigit(quat.x) || isChar(quat.x))
					{
						if (isDigit(quat.y) || isChar(quat.y))
						{
							if (ans.addrOffset[0] == '$')
							{
								mout << "\tli " << ans.addrOffset << " " << calc(quat.x, quat.y, DIV) << endl;
							}
							else
							{
								mout << "\tli $t1 " << calc(quat.x, quat.y, DIV) << endl;
								mout << "\tsw $t1 " << ans.addrOffset << "($t0)" << endl;
							}
						}
						else
						{
							if (getValue(quat.x) == 0)
							{
								if (ans.addrOffset[0] == '$') mout << "\tli " << ans.addrOffset << " 0" << endl;
								else mout << "\tsw $0 " << ans.addrOffset << "($t0)" << endl;
							}
							else
							{
								string tt2 = "$t2";
								Symbol sym = getSymbolInfo(quat.y);
								mout << "\tli $t1 " << quat.x << endl;
								if (sym.addrOffset[0] == '$') tt2 = sym.addrOffset;
								else mout << "\tlw $t2 " << sym.addrOffset << "($t0)" << endl;
								mout << "\tdiv $t1 " << tt2 << endl;

								if (ans.addrOffset[0] == '$')
								{
									mout << "\tmflo " << ans.addrOffset << endl;
								}
								else
								{
									mout << "\tmflo $t1" << endl;
									mout << "\tsw $t1 " << ans.addrOffset << "($t0)" << endl;
								}
							}
						}
					}
					else {
						if (isDigit(quat.y) || isChar(quat.y))
						{
							if (getValue(quat.y) == 1)
							{
								Symbol sym = getSymbolInfo(quat.x);

								if (sym.addrOffset[0] == '$' && ans.addrOffset[0] == '$')
								{
									mout << "\tmove " << ans.addrOffset << " " << sym.addrOffset << endl;
								}
								else if (sym.addrOffset[0] == '$')
								{
									mout << "\tsw " << sym.addrOffset << " " << ans.addrOffset << "($t0)" << endl;
								}
								else if (ans.addrOffset[0] == '$')
								{
									mout << "\tlw " << ans.addrOffset << " " << sym.addrOffset << "($t0)" << endl;
								}
								else
								{
									mout << "\tlw $t1 " << sym.addrOffset << "($t0)" << endl;
									mout << "\tsw $t1 " << ans.addrOffset << "($t0)" << endl;
								}
							}
							else if (getValue(quat.y) == -1)
							{
								string tt1 = "$t1";
								Symbol sym = getSymbolInfo(quat.x);
								if (sym.addrOffset[0] == '$') tt1 = sym.addrOffset;
								else mout << "\tlw $t1 " << sym.addrOffset << "($t0)" << endl;

								if (ans.addrOffset[0] == '$')
								{
									mout << "\tsub " << ans.addrOffset << " $0 " << tt1 << endl;
								}
								else
								{
									mout << "\tsub $t1 $0 $t1" << endl;
									mout << "\tsw $t1 " << ans.addrOffset << "($t0)" << endl;
								}
							}
							else
							{
								/*int pow = getPow(getValue(quat.y));
								Symbol sym = getSymbolInfo(quat.x);
								mout << "\tlw $t1 " << sym.addrOffset << "($t0)" << endl;
								if (pow > 0)
								{
									mout << "\tsra $t1 $t1 " << pow << endl;
									if (getValue(quat.y) < 0) mout << "\tsub $t1 $0 $t1" << endl;
								}
								else
								{
									mout << "\tli $t2 " << quat.y << endl;
									mout << "\tdiv $t1 $t2" << endl;
									mout << "\tmflo $t1" << endl;
								}*/

								string tt1 = "$t1";
								Symbol sym = getSymbolInfo(quat.x);
								if (sym.addrOffset[0] == '$') tt1 = sym.addrOffset;
								else mout << "\tlw $t1 " << sym.addrOffset << "($t0)" << endl;
								mout << "\tli $t2 " << quat.y << endl;
								mout << "\tdiv " << tt1 << " $t2" << endl;

								if (ans.addrOffset[0] == '$')
								{
									mout << "\tmflo " << ans.addrOffset << endl;
								}
								else
								{
									mout << "\tmflo $t1" << endl;
									mout << "\tsw $t1 " << ans.addrOffset << "($t0)" << endl;
								}
							}
						}
						else
						{
							string tt1 = "$t1", tt2 = "$t2";
							Symbol sym1 = getSymbolInfo(quat.x);
							Symbol sym2 = getSymbolInfo(quat.y);
							if (sym1.addrOffset[0] == '$') tt1 = sym1.addrOffset;
							else mout << "\tlw $t1 " << sym1.addrOffset << "($t0)" << endl;
							if (sym2.addrOffset[0] == '$') tt2 = sym2.addrOffset;
							else mout << "\tlw $t2 " << sym2.addrOffset << "($t0)" << endl;
							mout << "\tdiv " << tt1 << " " << tt2 << endl;
							
							if (ans.addrOffset[0] == '$')
							{
								mout << "\tmflo " << ans.addrOffset << endl;
							}
							else
							{
								mout << "\tmflo $t1" << endl;
								mout << "\tsw $t1 " << ans.addrOffset << "($t0)" << endl;
							}
						}
					}
				}
			}
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
		FuncMap[curFunc.getFuncName()] = curFunc;
		interCode.insertQuat("", "4", "spAlloc", "ConstDef");
		interCode.insertQuat(value, to_string(curFunc.getOffset(ans)), "spSET", "");
	}
	else
	{
		offsetTop = roundUp(offsetTop);
		CodeGenSymTab.push_back(Symbol{ ans, CONST, type, levelNow, 0, to_string(offsetTop) });
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
			interCode.insertQuat("", to_string(size), "spAlloc", "VarDef");
			curFunc.insertDef(idenfr, type, size);
			FuncMap[curFunc.getFuncName()] = curFunc;
		}
		else
		{
			// 将数组标识符存代码生成符号表CodeGenSymTab
			offsetTop = roundUp(offsetTop);
			CodeGenSymTab.push_back(Symbol{ idenfr, VAR, type, levelNow, dim, to_string(offsetTop) });
			offsetTop += size;
		}


		// 二维
		if (D2 >= 0)
		{
			if (isInFunc) 
			{
				curFunc.insert2DCols(idenfr, D2);
				FuncMap[curFunc.getFuncName()] = curFunc;
			}
			else
			{
				ArrCols[idenfr] = D2;
			}
			
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
						int offset = curFunc.getOffset(idenfr) + deltaOff;
						interCode.insertQuat(tav.value, to_string(offset), "spARRSET", "");
						deltaOff += 4;
					}
					else
					{
						interCode.insertQuat(tav.value, idenfr, "ARRSETbyCONST", to_string(deltaOff));
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
					int offset = curFunc.getOffset(idenfr) + deltaOff;
					interCode.insertQuat(tav.value, to_string(offset), "spARRSET", "");
					deltaOff += 4;
				}
				else
				{
					interCode.insertQuat(tav.value, idenfr, "ARRSETbyCONST", to_string(deltaOff));
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
					interCode.insertQuat(tav.value, to_string(curFunc.getOffset(idenfr)), "spSET", "");
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
				interCode.insertQuat("", to_string(size), "spAlloc", "VarDef");
				curFunc.insertDef(idenfr, type, size);
				FuncMap[curFunc.getFuncName()] = curFunc;
				if (D2 > 0)
				{
					curFunc.insert2DCols(idenfr, D2);
					FuncMap[curFunc.getFuncName()] = curFunc;
				}
			}
			else
			{
				// 将数组标识符存代码生成符号表CodeGenSymTab
				offsetTop = roundUp(offsetTop);
				CodeGenSymTab.push_back(Symbol{ idenfr, VAR, type, levelNow, dim, to_string(offsetTop) });
				offsetTop += size;
				if (D2 > 0) ArrCols[idenfr] = D2;
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

			if (isInFunc)
			{
				curFunc.insert2DCols(idenfr, D2);
				FuncMap[curFunc.getFuncName()] = curFunc;
			}
			else
			{
				ArrCols[idenfr] = D2;
			}
			
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
		interCode.insertQuat("", to_string(size), "spAlloc", "VarDef");
		curFunc.insertDef(idenfr, type, size);
		FuncMap[curFunc.getFuncName()] = curFunc;
	}
	else
	{
		// 将数组标识符存代码生成符号表CodeGenSymTab
		offsetTop = roundUp(offsetTop);
		CodeGenSymTab.push_back(Symbol{ idenfr, VAR, type, levelNow, dim, to_string(offsetTop) });
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
		return ret;
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
	FuncMap[curFunc.getFuncName()] = curFunc;
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
		FuncMap[curFunc.getFuncName()] = curFunc;
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
		if (isInFunc && curFunc.hasCheck(i))
		{
			interCode.insertQuat(i0.value, to_string(curFunc.getOffset(i)), "spSET", "");
		}
		else
		{
			interCode.insertQuat(i, i0.value, "", ""); // 初始化循环变量
		}

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
		if (op == "-") y = op + y;
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

		if (isInFunc && curFunc.hasCheck(x))
		{
			string tmp = getNewAns(INT, 4);
			interCode.insertQuat(tmp, to_string(curFunc.getOffset(x)), "spGET", "");
			x = tmp;
		}
		if (isInFunc && curFunc.hasCheck(i))
		{
			string tmp = getNewAns(INT, 4);
			interCode.insertQuat(tmp, x, "addi", y);
			interCode.insertQuat(tmp, to_string(curFunc.getOffset(i)), "spSET", "");
		}
		else
		{
			interCode.insertQuat(i, x, "addi", y); // 循环变量递增
		}
		interCode.insertQuat(startLoopLabel, "", "j", ""); // 跳回循环开头
		interCode.insertQuat(endLoopLabel, "", ":", ""); // 循环结束标签
	}
	else if (wordNow.label == "DOTK")
	{
		string startLoopLabel = getNewLabel();
		interCode.insertQuat(startLoopLabel, "", ":", "");

		getSym(true, true);
		parseStmt(); // while
		getSym(true, true); // (
		getSym(true, true); // first(<条件>)
		parseNewCondition(startLoopLabel);
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
		vector<string> values;
		vector<string> offsets;
		int cnt = 0;
		int offset = 0;
		Function func = FuncMap[funcInfo.funcName];
		typeAndValue paraValue = parseExpr();
		values.push_back(paraValue.value);
		offsets.push_back(to_string(offset));
		//interCode.insertQuat(paraValue.value, to_string(offset), "spPush", "");
		offset += 4;

		cnt++;
		if (paraValue.type != symTab[cnt + funcInfo.pos].type) error(wordNow.lineID, "e");
		while (wordNow.label == "COMMA")
		{
			getSym(true, true);
			paraValue = parseExpr();
			values.push_back(paraValue.value);
			offsets.push_back(to_string(offset));
			//interCode.insertQuat(paraValue.value, to_string(offset), "spPush", "");
			offset += 4;

			cnt++;
			if (paraValue.type != symTab[cnt + funcInfo.pos].type) error(wordNow.lineID, "e");
		}
		if (cnt != funcInfo.paramNum) error(wordNow.lineID, "d");

		interCode.insertQuat("", to_string(values.size()*4), "spAlloc", "(ParaSize)");
		for (int i = 0; i < values.size(); i++)
		{
			interCode.insertQuat(values[i], offsets[i], "spPush", "(ParaPush)");
		}

		FuncMap[funcInfo.funcName] = func;
	}

	cout << "<值参数表>" << endl;
}

string parseFuncCallWithReturn()
{
	if (checkUndefined(wordNow.idenfr, wordNow.lineID))
	{
		while (wordNow.label != "SEMICN") getSym(true, true);
		return "parseFuncCall_Error_no_SEMICN";
	}

	FuncInfo funcInfo = getFuncInfo(wordNow.idenfr);
	getSym(true, true); // (
	getSym(true, true); // first(值参数表)
	parseValueParamTable(funcInfo); // )
	interCode.insertQuat(funcInfo.funcName, "", "jal", "");

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
	interCode.insertQuat(funcInfo.funcName, "", "jal", "");

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
		if (isInFunc)
		{
			interCode.insertQuat("", "", "raPush", "");
		}

		int interQuatSize = curFuncInterQuat.size() * 4;
		if (isInFunc)
		{
			interCode.insertQuat("", to_string(interQuatSize), "spAlloc", "(InterQuat)");
			curFunc.offsetUp(interQuatSize);
			FuncMap[curFunc.getFuncName()] = curFunc;
			for (int i = 0; i < interQuatSize / 4; i++)
			{
				interCode.insertQuat(curFuncInterQuat[i], to_string(i * 4), "spPush", "(InterQuat)");
			}
		}

		ret.type = getFuncInfo(wordNow.idenfr).type;

		Function func = FuncMap[wordNow.idenfr];
		int paraSize = func.getParaSize();

		ret.value = parseFuncCallWithReturn();

		if (paraSize > 0) interCode.insertQuat("", to_string(paraSize), "spFree", "");

		if (isInFunc)
		{
			for (int i = 0; i < interQuatSize / 4; i++)
			{
				interCode.insertQuat(curFuncInterQuat[i], to_string(i * 4), "spGET", "(InterQuat)");
			}
			interCode.insertQuat("", to_string(interQuatSize), "spFree", "(InterQuat)");
			curFunc.offsetDown(interQuatSize);
			FuncMap[curFunc.getFuncName()] = curFunc;
		}

		if (isInFunc)
		{
			interCode.insertQuat("", "", "raPop", "");
		}

	}
	// 表达式
	else if (wordNow.label == "LPARENT")
	{
		ret.type = INT;
		getSym(true, true);
		ret.value = parseExpr().value;
		if (isChar(ret.value)) ret.value = to_string((int)ret.value[1]);
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
		if (!isUndefined)
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
				if (isInFunc && curFunc.hasCheck(idenfr))
				{
					interCode.insertQuat(index1, index1, "*", to_string(curFunc.getArrCols(idenfr)));
				}
				else
				{
					interCode.insertQuat(index1, index1, "*", to_string(ArrCols[idenfr]));
				}
				string index2 = getNewAns(INT, 4);
				string x = expr2.value;
				interCode.insertQuat(index2, x, "*", "4");
				interCode.insertQuat(index1, index1, "+", index2);
			}

			string ans = getNewAns(ret.type, 4);
			if (isInFunc && curFunc.hasDef(idenfr))
			{
				interCode.insertQuat(index1, index1, "+", to_string(curFunc.getOffset(idenfr)));
				interCode.insertQuat(ans, index1, "spARRGET", "");
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
	string op = wordNow.idenfr;
	string label = getNewLabel();

	getSym(true, true);
	rightTav = parseExpr();

	interCode.insertQuat(label, leftTav.value, reverseOp(op), rightTav.value);

	if (rightTav.type != INT) error(wordNow.lineID, "f");

	cout << "<条件>" << endl;

	return label;
}

void parseNewCondition(string label)
{
	int type;
	typeAndValue leftTav, rightTav;
	leftTav = parseExpr();
	if (leftTav.type != INT) error(wordNow.lineID, "f");

	// <比较运算符>
	string op = wordNow.idenfr;

	getSym(true, true);
	rightTav = parseExpr();

	interCode.insertQuat(label, leftTav.value, op, rightTav.value);

	if (rightTav.type != INT) error(wordNow.lineID, "f");

	cout << "<条件>" << endl;
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
		interCode.insertQuat(endLabel, "", ":", "");
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
	string idenfr = wordNow.idenfr;
	int isUndefined = checkUndefined(idenfr, wordNow.lineID);
	if (!isUndefined && getQuantityInfo(idenfr).kind == CONST) error(wordNow.lineID, "j");
	else if (!isUndefined)
	{
		if (isInFunc && curFunc.hasCheck(idenfr))
		{
			int type = getQuantityInfo(idenfr).type;
			if (type == INT) interCode.insertQuat("", "INT", "ReadByType", "");
			else if (type == CHAR) interCode.insertQuat("", "CHAR", "ReadByType", "");
			interCode.insertQuat("$v0", to_string(curFunc.getOffset(idenfr)), "spSET$v0", idenfr);
		}
		else
		{
			interCode.insertQuat(idenfr, "", "read", "");
		}
	}
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
			if (i + 1 < CodeGenSymTab.size()) delta = stoi(CodeGenSymTab[i + 1].addrOffset) - stoi(CodeGenSymTab[i].addrOffset);
			else delta = usedOffTop - stoi(CodeGenSymTab[i].addrOffset);

			CodeGenSymTab[i].addrOffset = to_string(offsetTop);
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
	interCode.insertQuat(nextLabel, tavIn.value, reverseOp("=="), tav.value);

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
			interCode.insertQuat(retValue.value, "", "return", "");
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
	}
	
	if (isInMain) interCode.insertQuat("EndMain", "", "j", "");
	else interCode.insertQuat("END" + curFunc.getFuncName(), "", "j", "");

	cout << "<返回语句>" << endl;
}

void parseStmt()
{
	// 循环语句
	if (wordNow.label == "WHILETK" || wordNow.label == "FORTK" || wordNow.label == "DOTK")
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
			else
			{
				if (isInFunc)
				{
					interCode.insertQuat("", "", "raPush", "");
				}

				int size = curFuncInterQuat.size() * 4;
				if (isInFunc)
				{
					interCode.insertQuat("", to_string(size), "spAlloc", "(InterQuat)");
					curFunc.offsetUp(size);
					FuncMap[curFunc.getFuncName()] = curFunc;
					for (int i = 0; i < size / 4; i++)
					{
						interCode.insertQuat(curFuncInterQuat[i], to_string(i * 4), "spPush", "(InterQuat)");
					}
				}
				parseFuncCall();// 函数调用
				if (isInFunc)
				{
					for (int i = 0; i < size / 4; i++)
					{
						interCode.insertQuat(curFuncInterQuat[i], to_string(i * 4), "spGET", "(InterQuat)");
					}
					interCode.insertQuat("", to_string(size), "spFree", "(InterQuat)");
					curFunc.offsetDown(size);
					FuncMap[curFunc.getFuncName()] = curFunc;
				}

				if (isInFunc)
				{
					interCode.insertQuat("", "", "raPop", "");
				}
			}

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
	int size = func.getParaSize();

	//// 函数的参数压栈
	//for (int i = 0; i < curFunc.getParaNum(); i += 1)
	//{
	//	interCode.insertQuat("", curFunc.getParaName(i), "spPush", to_string(i));
	//}

	if (keptLabelMap.find(wordNow.idenfr) != keptLabelMap.end() && keptLabelMap.find(wordNow.idenfr)->second == "RETURNABLE")
	{
		parseFuncCallWithReturn();
	}
	else if (keptLabelMap.find(wordNow.idenfr) != keptLabelMap.end() && keptLabelMap.find(wordNow.idenfr)->second == "UNRETURNABLE")
	{
		parseFuncCallWithoutReturn();
	}

	if (size > 0) interCode.insertQuat("", to_string(size), "spFree", "");
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
		if (isInFunc && curFunc.hasCheck(idenfr))
		{
			interCode.insertQuat(x, to_string(curFunc.getOffset(idenfr)), "spSET", "");
		}
		else
		{
			interCode.insertQuat(idenfr, x, "", "");
		}
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
			if (isInFunc && curFunc.hasCheck(idenfr))
			{
				interCode.insertQuat(offset, offset, "+", to_string(curFunc.getOffset(idenfr)));
				interCode.insertQuat(value.value, offset, "spARRSET", "");
			}
			else
			{
				interCode.insertQuat(value.value, idenfr, "ARRSETbyVAR", offset);
			}
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
			if (isInFunc && curFunc.hasCheck(idenfr))
			{
				interCode.insertQuat(offset, offset, "*", to_string(curFunc.getArrCols(idenfr)));
			}
			else
			{
				interCode.insertQuat(offset, offset, "*", to_string(ArrCols[idenfr]));
			}
			interCode.insertQuat(offset, offset, "+", tmp);
			if (isInFunc && curFunc.hasCheck(idenfr))
			{
				interCode.insertQuat(offset, offset, "+", to_string(curFunc.getOffset(idenfr)));
				interCode.insertQuat(value.value, offset, "spARRSET", "");
			}
			else
			{
				interCode.insertQuat(value.value, idenfr, "ARRSETbyVAR", offset);
			}
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

	if (isInFunc)
	{
		interCode.insertQuat("", "4", "spAlloc", "(Ra)");
		interCode.insertQuat("", "", "raPush", "");
		curFunc.insertRa();
		FuncMap[curFunc.getFuncName()] = curFunc;
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
		//raChanged = false;
		string funcName = wordNow.idenfr;
		FuncVec.push_back(funcName);
		getSym(false, false);// 返回值类型 void/int/char  
		if (wordNow.label == "VOIDTK")
		{
			curFuncInterQuat.clear();

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
			curFuncInterQuat.clear();

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

		interCode.insertQuat("END"+funcName, "", ":", "");
		/*if (raChanged) */interCode.insertQuat("", "", "raPop", "");
		interCode.insertQuat("", to_string(curFunc.getDefAndRaSize()), "spFree", "");
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
	CodeGenSymTab.push_back(Symbol{ ans, VAR, type, levelNow, 0, to_string(offsetTop) });
	offsetTop += size;

	if (isInFunc)
	{
		curFuncInterQuat.push_back(ans);
	}

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
	printf("ERROR in getSymbolInfo() !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
	return Symbol{};
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
	keptLabelMap.insert(map<string, string>::value_type("do", "DOTK"));
}

string toLowerCase(string in)
{
	string out = "";
	for (int i = 0; i < in.length(); i++)
	{
		out += in[i] >= 'A' && in[i] <= 'Z' ? in[i] - ('A' - 'a') : in[i];
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