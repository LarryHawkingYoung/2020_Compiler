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

int funcRetFlag = -1;// -1:���ں����� 0�����޷���ֵ������ 1�����з���ֵ������
int hasRet = 0; // ��ǰ������ֵ�ĺ��� �Ƿ� ��return���
int retType = -1; // ��ǰ������ֵ���� �� ����ֵ����

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
int top = 0;// wordList�ĳ���
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

// ��������
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
			if (quat.x != "") // ���ַ���
			{
				mout << "\tli $v0, 4" << endl;
				mout << "\tla $a0, " << quat.x << endl;
				mout << "\tsyscall" << endl;
			}
			if (quat.y != "") // �б��ʽ
			{
				if (quat.y[0] == '\'') // �ַ�����
				{
					mout << "\tli $v0, 11" << endl;
					mout << "\tli $a0, " << quat.y << endl;
				}
				else if (quat.y[0] >= '0' && quat.y[0] <= '9' || quat.y[0] == '-') // ��������
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
				if (quat.x[0] == '\'') // �ַ�����
				{
					mout << "\tli $t1, " << quat.x << endl;
				}
				else if (quat.x[0] >= '0' && quat.x[0] <= '9' || quat.x[0] == '-') // ��������
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
					if (quat.x[0] == '\'') // �ַ�����
					{
						mout << "\tli $t2, " << quat.x << endl;
					}
					else if (quat.x[0] >= '0' && quat.x[0] <= '9' || quat.x[0] == '-') // ��������
					{
						mout << "\tli $t2, " << quat.x << endl;
					}
					else
					{
						Symbol sym1 = getSymbolInfo(quat.x);
						mout << "\tlw $t2, " << sym1.addrOffset << "($t0)" << endl;
					}
				}
				if (quat.y[0] == '\'') // �ַ�����
				{
					mout << "\tli $t3, " << quat.y << endl;
				}
				else if (quat.y[0] >= '0' && quat.y[0] <= '9' || quat.y[0] == '-') // ��������
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

	cout << "<��������>" << endl;
}

void parsePerConstDef(int type) // ����ʶ�������������� / ����ʶ���������ַ���
{ // û�г�ǰ��
	getSym(true, true);//��ʶ��

	if (checkDupDef(wordNow.idenfr, wordNow.lineID))
	{
		getSym(true, true); // =
		getSym(true, true); // First(���� / �ַ�)
		if (type == INT) parseInteger();
		else parseChar();
		return;
	}

	symTab[++symTabTop] = Symbol{ wordNow.idenfr, CONST, type, levelNow, 0 };

	string ans = wordNow.idenfr;

	getSym(true, true); // =
	getSym(true, true); // First(���� / �ַ�)

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
		getSym(true, true); // first(<�޷�������>)
	}
	ret = ret + parseUnsignedInt();

	cout << "<����>" << endl;

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
	cout << "<�޷�������>" << endl;

	return ret;
}

void parseConstDcrpt()
{
	// const
	getSym(true, true); // first(<��������>)
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
			getSym(true, true); // first(<��������>)
			parseConstDef();
			if (wordNow.label != "SEMICN") // first(<��������>)
			{
				if (wordNow.isAtHead)
				{
					getSym(false, false);
					error(wordNow.lineID, "k");
					getSym(true, false);
				}
				else error(wordNow.lineID, "k");
			}
			else getSym(true, true); // first(<��������>)
		}
	}

	cout << "<����˵��>" << endl;
}

void parseVarDef()
{
	// ȷ������������
	int type = -1;
	if (wordNow.label == "INTTK") type = INT;
	else if (wordNow.label == "CHARTK") type = CHAR;
	if (type == -1)
	{
		printf("What the heck ! In parseVarDef()");
		getchar();
	}

	getSym(true, true);//��ʶ��
	string idenfr = wordNow.idenfr;
	int lineID = wordNow.lineID;
	getSym(true, true);//��һ������
	int dim = 0, D1 = -1, D2 = -1;
	// ȷ��������ά��
	if (wordNow.label == "LBRACK")// [ һά����
	{
		dim++;
		getSym(true, true);// first��<�޷�������>��
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
		if (wordNow.label == "LBRACK")// [ ��ά����
		{
			dim++;
			getSym(true, true);// first��<�޷�������>��
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

	// = �������弰��ʼ��
	if (wordNow.label == "ASSIGN")
	{
		if (checkDupDef(idenfr, lineID)) // ����ظ�����
		{
			while (wordNow.label != "SEMICN")
			{
				getSym(true, true);
			}
			return; // �˳� <��������>
		}

		// û���ظ�����
		symTab[++symTabTop] = Symbol{ idenfr, VAR, type, levelNow, dim };

		int size = (D2 > 0) ? (4 * D1 * D2) :
			((D1 > 0) ? (4 * D1) : 4);
		if (isInFunc)
		{
			curFunc.insertDef(idenfr, size, type);
		}
		else
		{
			// �������ʶ����������ɷ��ű�CodeGenSymTab
			offsetTop = roundUp(offsetTop);
			CodeGenSymTab.push_back(Symbol{ idenfr, VAR, type, levelNow, dim, offsetTop });
			offsetTop += size;
		}


		// ��ά
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
					if (n != 0 && wordNow.label != "COMMA") { // ��������
						error(wordNow.lineID, "n");
						while (wordNow.label != "SEMICN") getSym(true, true);
						cout << "<�������弰��ʼ��>" << endl;
						return;
					}

					// ��û���ִ����� ��
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
				if (wordNow.label != "RBRACE") { // ��������
					error(wordNow.lineID, "n");
					while (wordNow.label != "SEMICN") getSym(true, true);
					cout << "<�������弰��ʼ��>" << endl;
					return;
				}
				else getSym(true, true);
				if (m != 0 && wordNow.label != "COMMA") { // ��������
					error(wordNow.lineID, "n");
					while (wordNow.label != "SEMICN") getSym(true, true);
					cout << "<�������弰��ʼ��>" << endl;
					return;
				}
			}
			if (wordNow.label != "RBRACE") { // ��������
				error(wordNow.lineID, "n");
				while (wordNow.label != "SEMICN") getSym(true, true);
				cout << "<�������弰��ʼ��>" << endl;
				return;
			}
			else getSym(true, true);
		}
		// һά
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
					cout << "<�������弰��ʼ��>" << endl;
					return;
				}
				// ��û���ִ����� ��
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
				cout << "<�������弰��ʼ��>" << endl;
				return;
			}
			else getSym(true, true);
		}
		// ��ά
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

		cout << "<�������弰��ʼ��>" << endl;
	}
	// ���������޳�ʼ��
	else
	{// ��ʱ�Ѿ������� , �� ;
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
				// �������ʶ����������ɷ��ű�CodeGenSymTab
				offsetTop = roundUp(offsetTop);
				CodeGenSymTab.push_back(Symbol{ idenfr, VAR, type, levelNow, dim, offsetTop });
				offsetTop += size;
			}
		}

		while (wordNow.label == "COMMA")
		{
			getSym(true, true);
			parsePerVarDefNotInit(type);
		}//���˳���ѭ��������������� ��',' ����Ϊ';' �������';' �����˳���ᱨ��

		cout << "<���������޳�ʼ��>" << endl;
	}

	cout << "<��������>" << endl;
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
		// �������ʶ����������ɷ��ű�CodeGenSymTab
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
		cout << "<����>" << endl;
		ret.type = INT;
		return ret;
	}
	else if (wordNow.label == "CHARCON")
	{
		ret.value = parseChar();
		cout << "<����>" << endl;
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
	// first(<��������>)
	parseVarDef(); // ;
	getSym(true, true); // first(<��������>) / first({���з���ֵ�������壾|���޷���ֵ�������壾}����������) / first(������У�)
	while (true)
	{
		if (wordNow.label != "INTTK" && wordNow.label != "CHARTK") break; // �ų� <�����> �� �޷���ֵ���� �Ŀ�����
		getSym(true, false); // ͵�� �� 1  ��������
		getSym(true, false); // ͵�� �� 2  �����ţ�
		if (wordNow.label == "LPARENT") // ������ 
		{
			getSym(false, false); // ����
			getSym(false, false); // ����
			break; // �ų��з���ֵ�����Ŀ�����
		}
		getSym(false, false); // ����
		getSym(false, false); // ����
		parseVarDef(); // �������� // ;
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
		else getSym(true, true); // ���һ��������һ��ѭ��
	}

	cout << "<����˵��>" << endl;
}

void parseParamTable()
{
	// int or char
	int type = -1;
	if (wordNow.label == "INTTK") type = INT;
	else if (wordNow.label == "CHARTK") type = CHAR;
	else // ˵��Ϊ�ղ�����
	{
		cout << "<������>" << endl;
		return;
	}
	getSym(true, true);// ��ʶ��
	curFunc.insertPara(wordNow.idenfr, type);
	checkDupDef(wordNow.idenfr, wordNow.lineID);
	symTab[++symTabTop] = Symbol{ wordNow.idenfr, PARAM, type, levelNow, 0 };
	getSym(true, true);
	while (wordNow.label == "COMMA")
	{
		getSym(true, true);// int or char
		if (wordNow.label == "INTTK") type = INT;
		else if (wordNow.label == "CHARTK") type = CHAR;
		getSym(true, true);// ��ʶ��
		curFunc.insertPara(wordNow.idenfr, type);
		checkDupDef(wordNow.idenfr, wordNow.lineID);
		symTab[++symTabTop] = Symbol{ wordNow.idenfr, PARAM, type, levelNow, 0 };
		getSym(true, true);
	}

	FuncMap[curFunc.getFuncName()] = curFunc;

	cout << "<������>" << endl;

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
	getSym(true, true);// ��ʶ��
	if (checkDupDef(wordNow.idenfr, wordNow.lineID))
	{
		symTab[++symTabTop] = Symbol{ wordNow.idenfr, FUNC, type, levelNow, 0 }; // �������ȥ
		getSym(true, true);
		cout << "<����ͷ��>" << endl;
		return 1;
	}

	curFunc = Function(wordNow.idenfr, type);
	interCode.insertQuat(wordNow.idenfr, "", ":", ""); // ������

	symTab[++symTabTop] = Symbol{ wordNow.idenfr, FUNC, type, levelNow, 0 };
	keptLabelMap.insert(map<string, string>::value_type(wordNow.idenfr, "RETURNABLE"));
	getSym(true, true);// (

	cout << "<����ͷ��>" << endl;

	return 0;
}

void parseLoopStmt()
{
	if (wordNow.label == "WHILETK")
	{
		getSym(true, true); // '('
		getSym(true, true); // first(<����>)

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
		getSym(true, true); // ��ʶ��
		string i = wordNow.idenfr;
		checkUndefined(i, wordNow.lineID);
		getSym(true, true); // =
		getSym(true, true); // first(���ʽ)
		typeAndValue i0 = parseExpr(); // ;
		interCode.insertQuat(i, i0.value, "", ""); // ��ʼ��ѭ������

		// ������
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
		else getSym(true, true); // first(����)

		string startLoopLabel = getNewLabel();
		interCode.insertQuat(startLoopLabel, "", ":", ""); // ѭ����ʼ��ǩ
		string endLoopLabel = parseCondition(); // ; �����ж����������Ƿ���ת��ѭ������

		// ������
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
		else getSym(true, true); // ��ʶ��
		checkUndefined(wordNow.idenfr, wordNow.lineID);

		getSym(true, true); // =
		getSym(true, true); // ��ʶ��
		string x = wordNow.idenfr;
		checkUndefined(x, wordNow.lineID);
		getSym(true, true); // + / -
		string op = wordNow.idenfr;
		getSym(true, true); // first(����)
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

		interCode.insertQuat(i, x, op, y); // ѭ����������
		interCode.insertQuat(startLoopLabel, "", "j", ""); // ����ѭ����ͷ
		interCode.insertQuat(endLoopLabel, "", ":", ""); // ѭ��������ǩ
	}

	cout << "<ѭ�����>" << endl;
}

string  parseStep()
{
	string step = parseUnsignedInt();

	cout << "<����>" << endl;

	return step;
}

void parseValueParamTable(FuncInfo funcInfo)
{
	if (wordNow.label == "RPARENT")
	{
		if (funcInfo.paramNum != 0) {
			error(wordNow.lineID, "d");
		}
		cout << "<ֵ������>" << endl;
		return;
	}
	else if (wordNow.label == "SEMICN")
	{
		cout << "<ֵ������>" << endl;
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

	cout << "<ֵ������>" << endl;
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
	getSym(true, true); // first(ֵ������)
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

	cout << "<�з���ֵ�����������>" << endl;

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
	getSym(true, true); // first(ֵ������)
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

	cout << "<�޷���ֵ�����������>" << endl;
}

typeAndValue parseFactor()
{
	typeAndValue ret;
	ret.type = -1;
	// ����
	if (wordNow.label == "PLUS" || wordNow.label == "MINU" || wordNow.label == "INTCON")
	{
		ret.type = INT;
		ret.value = parseInteger();
	}
	// �ַ�
	else if (wordNow.label == "CHARCON")
	{
		ret.type = CHAR;
		ret.value = parseChar();
	}
	// �з���ֵ��������
	else if (keptLabelMap.find(wordNow.idenfr) != keptLabelMap.end() && keptLabelMap[wordNow.idenfr] == "RETURNABLE")
	{
		ret.type = getFuncInfo(wordNow.idenfr).type;
		ret.value = parseFuncCallWithReturn();
	}
	// ���ʽ
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
	// �������������飩������
	else
	{
		string idenfr = wordNow.idenfr;
		// ��ʶ��
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
			// �±����ַ� ����
			if (!isUndefined && expr1.type == CHAR) error(wordNow.lineID, "i");
			// ȱ ']' ����
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

			// �����һά����
			string index1 = getNewAns(INT, 4);
			string x = expr1.value;
			interCode.insertQuat(index1, x, "*", "4");

			if (wordNow.label == "LBRACK")
			{
				getSym(true, true);
				typeAndValue expr2 = parseExpr();
				// �±����ַ� ����
				if (!isUndefined && expr2.type == CHAR) error(wordNow.lineID, "i");
				// ȱ ']' ����
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

				// ����ڶ�ά����
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

	cout << "<����>" << endl;
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

	cout << "<��>" << endl;
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

	cout << "<���ʽ>" << endl;

	return ret;
}

string parseCondition()
{
	int type;
	typeAndValue leftTav, rightTav;
	leftTav = parseExpr();
	if (leftTav.type != INT) error(wordNow.lineID, "f");

	// <�Ƚ������>
	string label = getNewLabel();
	interCode.insertQuat(label, leftTav.value, wordNow.idenfr, rightTav.value);

	getSym(true, true);
	rightTav = parseExpr();
	if (rightTav.type != INT) error(wordNow.lineID, "f");

	cout << "<����>" << endl;

	return label;
}

void parseCondStmt()
{
	// if
	getSym(true, true); // (
	getSym(true, true); // first(����)
	string elseLabel = parseCondition();

	// ������
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
		//��ת��� ���� endLabel
		interCode.insertQuat(endLabel, "", "j", "");
		//�����ǩ�ڴ� elseLabel
		interCode.insertQuat(elseLabel, "", ":", "");
		getSym(true, true);
		parseStmt();
	}
	else {
		interCode.insertQuat(elseLabel, "", ":", "");
	}

	cout << "<�������>" << endl;
}

void parseReadStmt()
{
	// scanf
	getSym(true, true);// (
	getSym(true, true);// ��ʶ��
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

	cout << "<�����>" << endl;
}

void parseString()
{
	getSym(true, true);

	cout << "<�ַ���>" << endl;
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

		// ����֮ǰ����ı䡢������ƫ��
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

	cout << "<д���>" << endl;
}

string parseCondChildStmt(typeAndValue tavIn, string endLabel)
{
	getSym(true, true); // first(<����>)
	typeAndValue tav = parseConst();

	string nextLabel = getNewLabel();
	interCode.insertQuat(nextLabel, tavIn.value, "==", tav.value);

	// ������
	if (tavIn.type != tav.type) error(wordNow.lineID, "o");
	getSym(true, true);

	parseStmt();

	interCode.insertQuat(endLabel, "", "j", "");

	cout << "<��������>" << endl;

	return nextLabel;
}

void parseCondTable(typeAndValue tav, string endLabel)
{
	while (wordNow.label == "CASETK")
	{
		string nextLabel = parseCondChildStmt(tav, endLabel);
		interCode.insertQuat(nextLabel, "", ":", "");
	}

	cout << "<�����>" << endl;
}

void parseDefault()
{
	getSym(true, true);
	getSym(true, true);
	parseStmt();

	cout << "<ȱʡ>" << endl;
}

void parseSwitchStmt()
{
	getSym(true, true);// (
	getSym(true, true);// first(���ʽ)
	typeAndValue tav = parseExpr(); // )

	// ������
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
	getSym(true, true); // first(�����)
	parseCondTable(tav, endLabel); // first(<ȱʡ>)

	if (wordNow.label != "DEFAULTTK") error(wordNow.lineID, "p");
	else parseDefault(); // }

	interCode.insertQuat(endLabel, "", ":", "");

	getSym(true, true);

	cout << "<������>" << endl;
}

void parseReturnStmt()
{
	// return
	getSym(true, true);
	if (wordNow.label == "LPARENT")
	{
		if (funcRetFlag == 0) error(wordNow.lineID, "g"); // void������ ���� return(
		getSym(true, true);
		if (funcRetFlag == 1 && wordNow.label == "RPARENT") error(wordNow.lineID, "h");// return();
		typeAndValue retValue = parseExpr();
		if (isInFunc && funcRetFlag == 1)
		{
			interCode.insertQuat("", retValue.value, "return", "");
		}
		if (funcRetFlag == 1 && retValue.type != retType) error(wordNow.lineID, "h");// ���ʽ�����뷵��ֵ���Ͳ�һ��
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

	cout << "<�������>" << endl;
}

void parseStmt()
{
	// ѭ�����
	if (wordNow.label == "WHILETK" || wordNow.label == "FORTK")
	{
		parseLoopStmt();
	}
	// �������
	else if (wordNow.label == "IFTK")
	{
		parseCondStmt();
	}
	// �����
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
	// д���
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
	// ������
	else if (wordNow.label == "SWITCHTK")
	{
		parseSwitchStmt();
	}
	// �����
	else if (wordNow.label == "SEMICN")
	{
		getSym(true, true);
	}
	// �������
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
	// �����
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
		// �����������
		if (wordNow.label == "LPARENT")
		{
			getSym(false, false);
			if (checkUndefined(wordNow.idenfr, wordNow.lineID))
			{
				while (wordNow.label != "SEMICN") getSym(true, true);
			}
			else parseFuncCall();// ��������

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
		// ��ֵ���
		else
		{
			getSym(false, false);
			parseAssignStmt();// ��ֵ���
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

	cout << "<���>" << endl;
}

void parseFuncCall()
{
	Function func = FuncMap[wordNow.idenfr];
	int size = func.getTotalSize();
	interCode.insertQuat("", to_string(size), "spAlloc", "");

	// �����Ĳ���ѹջ
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

	cout << "<��ֵ���>" << endl;
}

void parseStmtList()
{
	while (wordNow.label != "RBRACE")
	{
		parseStmt();
		if (isFuncEnd) return;
		if (isMainEnd) return;
	}

	cout << "<�����>" << endl;
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

	cout << "<�������>" << endl;
}

void parseFuncDef()
{
	interCode.insertQuat("main", "", "j", "");
	isInFunc = true;

	string str = wordNow.label;// ����ֵ���� void/int/char
	getSym(true, false);// idenfr ������
	while (str != "VOIDTK" || wordNow.label != "MAINTK")
	{
		getSym(false, false);// ����ֵ���� void/int/char  
		if (wordNow.label == "VOIDTK")
		{
			funcRetFlag = 0; // ����void����
			getSym(true, true);// void�ĺ�����

			curFunc = Function(wordNow.idenfr, VOID);
			interCode.insertQuat(wordNow.idenfr, "", ":", ""); // ������

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
			else cout << "<������>" << endl;
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
			getSym(true, true);// first(<�������>)
			parseCompStmt();// }

			funcRetFlag = -1;// �˳�����
			/*if (isDupDef) levelDown(true);
			else levelDown();*/
			levelDown(isDupDef); // ����ظ�����ͰѺ�����Ҳɾ�ˣ��������º�������param
			getSym(true, true);
			cout << "<�޷���ֵ��������>" << endl;
		}
		else
		{
			funcRetFlag = 1; // �����з���ֵ�ĺ���
			int isDupDef = parseDeclareHead(); //��
			levelUp();
			getSym(true, true);
			if (wordNow.label != "RPARENT") {
				parseParamTable();
			}
			else cout << "<������>" << endl;
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

			getSym(true, true); // first(<�������>)
			parseCompStmt(); // }
			if (!hasRet) error(wordNow.lineID, "h");
			funcRetFlag = -1;
			levelDown(isDupDef); // ����ظ�����ͰѺ�����Ҳɾ�ˣ��������º�������param
			getSym(true, true);
			cout << "<�з���ֵ��������>" << endl;

			hasRet = 0;
		}
		str = wordNow.label;
		getSym(true, false);

		interCode.insertQuat("", "", "jr", ""); // ������ת����
	}
	getSym(false, false);

	isInFunc = false;
}

void parseMainFunc()
{
	// void
	interCode.insertQuat("main", "", ":", "");
	funcRetFlag = 0; // ����void����
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
	getSym(true, true);// first(��������䣾)
	parseCompStmt();
	// }
	funcRetFlag = -1;//�˳�����
	isInMain = false;
	//levelDown();
	getSym(true, true);

	cout << "<������>" << endl;
}

void parseProgram()
{
	if (wordNow.label == "CONSTTK")
	{
		parseConstDcrpt();
	}

	if (wordNow.label == "VOIDTK")
	{
		getSym(true, false);// ͵��һ��
		if (wordNow.label == "MAINTK")
		{
			getSym(false, false);// ���˵� void

			parseMainFunc();
		}
		else
		{
			getSym(false, false);// ���˵� void

			parseFuncDef();
			parseMainFunc();
		}
	}
	else if (wordNow.label == "INTTK" || wordNow.label == "CHARTK")
	{
		getSym(true, false); // ͵�� �� 1
		getSym(true, false); // ͵�� �� 2
		if (wordNow.label != "LPARENT") // != <����˵��>
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

	cout << "<����>" << endl;
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
		symTabTop--; // �Ѻ�����Ҳɾ��
		return;
	}
	//������������
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