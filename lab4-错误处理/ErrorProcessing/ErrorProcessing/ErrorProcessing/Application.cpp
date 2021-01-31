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

int funcRetFlag = -1;// -1:���ں����� 0�����޷���ֵ������ 1�����з���ֵ������
int hasRet = 0; // ��ǰ������ֵ�ĺ��� �Ƿ� ��return���
int retType = -1; // ��ǰ������ֵ���� �� ����ֵ����

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

	cout << "<��������>" << endl;
}

void parsePerConstDef(int type) // ����ʶ�������������� / ����ʶ���������ַ���
{ // û�г�ǰ��
	getSym(true, true);//��ʶ��

	if (!checkDupDef(wordNow.idenfr, wordNow.lineID)) // û�б�ʶ���ض���
	{
		symTab[++symTabTop] = Symbol{ wordNow.idenfr, CONST, type, levelNow, 0 };
	}

	getSym(true, true); // =
	getSym(true, true); // First(���� / �ַ�)
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
		getSym(true, true); // first(<�޷�������>)
	}
	parseUnsignedInt();

	cout << "<����>" << endl;
}

void parseUnsignedInt()
{
	if (wordNow.label != "INTCON")
	{
		printf("Not INTCON ! In parseUnsignedInt");
		getchar();
	}
	getSym(true, true);
	cout << "<�޷�������>" << endl;
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
		if (wordNow.label == "LBRACK")// [ ��ά����
		{
			dim++;
			getSym(true, true);// first��<�޷�������>��
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

		// ��ά
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
					if (n != 0 && wordNow.label != "COMMA") { // ��������
						error(wordNow.lineID, "n");
						while (wordNow.label != "SEMICN") getSym(true, true);
						cout << "<�������弰��ʼ��>" << endl;
						return;
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
					cout << "<�������弰��ʼ��>" << endl;
					return;
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
			if (type != parseConst()) error(wordNow.lineID, "o");
		}

		cout << "<�������弰��ʼ��>" << endl;
	}
	// ���������޳�ʼ��
	else
	{// ��ʱ�Ѿ������� , �� ;
		if (!checkDupDef(idenfr, lineID))
			symTab[++symTabTop] = Symbol{ idenfr, VAR, type, levelNow, dim };

		while (wordNow.label == "COMMA")
		{
			getSym(true, true);
			parsePerVarDefNotInit(type);
		}//���˳���ѭ��������������� ��, ����Ϊ; �������; �����˳���ᱨ��

		cout << "<���������޳�ʼ��>" << endl;
	}

	cout << "<��������>" << endl;
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
		cout << "<����>" << endl;
		return INT;
	}
	else if (wordNow.label == "CHARCON")
	{
		getSym(true, true);
		cout << "<����>" << endl;
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
	checkDupDef(wordNow.idenfr, wordNow.lineID);
	symTab[++symTabTop] = Symbol{ wordNow.idenfr, PARAM, type, levelNow, 0 };
	getSym(true, true);
	while (wordNow.label == "COMMA")
	{
		getSym(true, true);// int or char
		if (wordNow.label == "INTTK") type = INT;
		else if (wordNow.label == "CHARTK") type = CHAR;
		getSym(true, true);// ��ʶ��
		checkDupDef(wordNow.idenfr, wordNow.lineID);
		symTab[++symTabTop] = Symbol{ wordNow.idenfr, PARAM, type, levelNow, 0 };
		getSym(true, true);
	}

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
		getSym(true, true); // ��ʶ��
		checkUndefined(wordNow.idenfr, wordNow.lineID);
		getSym(true, true); // =
		getSym(true, true); // first(���ʽ)
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
		else getSym(true, true); // first(����)
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
		else getSym(true, true); // ��ʶ��
		checkUndefined(wordNow.idenfr, wordNow.lineID);
		getSym(true, true); // =
		getSym(true, true); // ��ʶ��
		checkUndefined(wordNow.idenfr, wordNow.lineID);
		getSym(true, true); // + / -
		getSym(true, true); // first(����)
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

	cout << "<ѭ�����>" << endl;
}

void parseStep()
{
	parseUnsignedInt();

	cout << "<����>" << endl;
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

	cout << "<ֵ������>" << endl;
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

	cout << "<�з���ֵ�����������>" << endl;
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

int parseFactor()
{
	int type = -1;
	// ����
	if (wordNow.label == "PLUS" || wordNow.label == "MINU" || wordNow.label == "INTCON")
	{
		type = INT;
		parseInteger();
	}
	// �ַ�
	else if (wordNow.label == "CHARCON")
	{
		type = CHAR;
		getSym(true, true);
	}
	// �з���ֵ��������
	else if (keptLabelMap.find(wordNow.idenfr) != keptLabelMap.end() && keptLabelMap.find(wordNow.idenfr)->second == "RETURNABLE")
	{
		type = getFuncInfo(wordNow.idenfr).type;
		parseFuncCallWithReturn();
	}
	// ���ʽ
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
		// ��ʶ��
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

	cout << "<����>" << endl;
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

	cout << "<��>" << endl;
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

	cout << "<���ʽ>" << endl;

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

	cout << "<����>" << endl;
}

void parseCondStmt()
{
	// if
	getSym(true, true); // (
	getSym(true, true); // first(����)
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

	cout << "<�������>" << endl;
}

void parseReadStmt()
{
	// scanf
	getSym(true, true);// (
	getSym(true, true);// ��ʶ��
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

	cout << "<�����>" << endl;
}

void parseString()
{
	getSym(true, true);

	cout << "<�ַ���>" << endl;
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

	cout << "<д���>" << endl;
}

void parseCondChildStmt(int type)
{
	getSym(true, true);
	if (type != parseConst()) error(wordNow.lineID, "o");
	getSym(true, true);
	parseStmt();

	cout << "<��������>" << endl;
}

void parseCondTable(int type)
{
	while (wordNow.label == "CASETK")
	{
		parseCondChildStmt(type);
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
	getSym(true, true); // first(�����)
	parseCondTable(type); // first(ȱʡ)
	if (wordNow.label != "DEFAULTTK") error(wordNow.lineID, "p");
	else parseDefault(); // }
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
		int type = parseExpr();
		if (funcRetFlag == 1 && type != retType) error(wordNow.lineID, "h");// ���ʽ�����뷵��ֵ���Ͳ�һ��
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

	cout << "<��ֵ���>" << endl;
}

void parseStmtList()
{
	while (wordNow.label != "RBRACE")
	{
		parseStmt();
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
	string str = wordNow.label;// ����ֵ���� void/int/char
	getSym(true, false);// idenfr ������
	while (str != "VOIDTK" || wordNow.label != "MAINTK")
	{
		getSym(false, false);// ����ֵ���� void/int/char
		if (wordNow.label == "VOIDTK")
		{
			funcRetFlag = 0; // ����void����
			getSym(true, true);// void�ĺ�����
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
	}
	getSym(false, false);
}

void parseMainFunc()
{
	// void
	funcRetFlag = 0; // ����void����
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
	levelDown();
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