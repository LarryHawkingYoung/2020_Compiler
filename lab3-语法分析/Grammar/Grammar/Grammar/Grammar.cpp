#include<iostream>
#include<string>
#include<fstream>
#include<map>

using namespace std;

int getNewIndex(int start, string str);
string toLowerCase(string in);
void TokenInit();
int getSym(bool forward, bool withOutput);

void error();
void parseProgram();
void parseConstDcrpt();
void parseConstDef();
void parseInteger();
void parseUnsignedInt();
void parseConst();
void parseVarDcrpt();
void parseVarDef();
void parseFuncDef();
void parseParamTable();
void parseDeclareHead();
void parseCompStmt();
void parseStmtList();
void parseStmt();
void parseLoopStmt();
void parseCondition();
void parseExpr();
void parseTerm();
void parseFactor();
void parseFuncCallWithReturn();
void parseFuncCallWithoutReturn();
void parseValueParamTable();
void parseStep();
void parseFuncCall();
void parseCondStmt();
void parseReadStmt();
void parseWriteStmt();
void parseSwitchStmt();
void parseCondTable();
void parseCondChildStmt();
void parseDefault();
void parseReturnStmt();
void parseAssignStmt();
void parseMainFunc();
void parseString();

ifstream fin("testfile.txt");
ofstream fout("output.txt");
string line;

string strcon;
string idenfr;
string intcon;
char ch;
map<string, string> keptLabelMap;

string mapList[10000][2];
int top = 0;// mapList的长度

int wordCnt = 0;
string couple[3] = {"", ""};// 1: label 2: value 3: return or not

int main()
{
	TokenInit();

	//_ASSERT(fin.is_open());
	while (getline(fin, line))
	{
		int ind = 0;
		//fout << ind << endl;
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
					strcon = strcon + ch;
					ind++;
				}
				//fout << "STRCON " << strcon << endl;
				mapList[top][0] = "STRCON";
				mapList[top][1] = strcon;
				top++;
				break;
			case '\'':
				ind++;
				//fout << "CHARCON " << line[ind] << endl;
				mapList[top][0] = "CHARCON";
				mapList[top][1] = line[ind];
				top++;
				ind++;
				break;
			case '+':
				//fout << "PLUS +" << endl;
				mapList[top][0] = "PLUS";
				mapList[top][1] = "+";
				top++;
				break;
			case '-':
				//fout << "MINU -" << endl;
				mapList[top][0] = "MINU";
				mapList[top][1] = "-";
				top++;
				break;
			case '*':
				//fout << "MULT *" << endl;
				mapList[top][0] = "MULT";
				mapList[top][1] = "*";
				top++;
				break;
			case '/':
				//fout << "DIV /" << endl;
				mapList[top][0] = "DIV";
				mapList[top][1] = "/";
				top++;
				break;
			case '<':
				switch (line[ind + 1])
				{
				case '=':
					//fout << "LEQ <=" << endl;
					mapList[top][0] = "LEQ";
					mapList[top][1] = "<=";
					top++;
					ind++;
					break;
				default:
					//fout << "LSS <" << endl;
					mapList[top][0] = "LSS";
					mapList[top][1] = "<";
					top++;
					break;
				}
				break;
			case '>':
				switch (line[ind + 1])
				{
				case '=':
					//fout << "GEQ >=" << endl;
					mapList[top][0] = "GEQ";
					mapList[top][1] = ">=";
					top++;
					ind++;
					break;
				default:
					//fout << "GRE >" << endl;
					mapList[top][0] = "GRE";
					mapList[top][1] = ">";
					top++;
					break;
				}
				break;
			case '=':
				switch (line[ind + 1])
				{
				case '=':
					//fout << "EQL ==" << endl;
					mapList[top][0] = "EQL";
					mapList[top][1] = "==";
					top++;
					ind++;
					break;
				default:
					//fout << "ASSIGN =" << endl;
					mapList[top][0] = "ASSIGN";
					mapList[top][1] = "=";
					top++;
					break;
				}
				break;
			case '!':
				//fout << "NEQ !=" << endl;
				mapList[top][0] = "NEQ";
				mapList[top][1] = "!=";
				top++;
				ind++;
				break;
			case ':':
				//fout << "COLON :" << endl;
				mapList[top][0] = "COLON";
				mapList[top][1] = ":";
				top++;
				break;
			case ';':
				//fout << "SEMICN ;" << endl;
				mapList[top][0] = "SEMICN";
				mapList[top][1] = ";";
				top++;
				break;
			case ',':
				//fout << "COMMA ," << endl;
				mapList[top][0] = "COMMA";
				mapList[top][1] = ",";
				top++;
				break;
			case '(':
				//fout << "LPARENT (" << endl;
				mapList[top][0] = "LPARENT";
				mapList[top][1] = "(";
				top++;
				break;
			case ')':
				//fout << "RPARENT )" << endl;
				mapList[top][0] = "RPARENT";
				mapList[top][1] = ")";
				top++;
				break;
			case '[':
				//fout << "LBRACK [" << endl;
				mapList[top][0] = "LBRACK";
				mapList[top][1] = "[";
				top++;
				break;
			case ']':
				//fout << "RBRACK ]" << endl;
				mapList[top][0] = "RBRACK";
				mapList[top][1] = "]";
				top++;
				break;
			case '{':
				//fout << "LBRACE {" << endl;
				mapList[top][0] = "LBRACE";
				mapList[top][1] = "{";
				top++;
				break;
			case '}':
				//fout << "RBRACE }" << endl;
				mapList[top][0] = "RBRACE";
				mapList[top][1] = "}";
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
						//fout << "IDENFR " << idenfr << endl;
						mapList[top][0] = "IDENFR";
						mapList[top][1] = idenfr;
						top++;
					}
					else
					{
						//fout << iter->second << " " << toLowerCase(idenfr) << endl;
						mapList[top][0] = iter->second;
						mapList[top][1] = idenfr;
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
					//fout << "INTCON " << intcon << endl;
					mapList[top][0] = "INTCON";
					mapList[top][1] = intcon;
					top++;
				}
				break;
			}

			ind++;
		}
	}


	if (getSym(true, true))
	{
		parseProgram();
	}

	fin.close();
	fout.close();

	/*fout << "\nfinished" << endl;
	getchar();*/

	return 0;
}

void parseConstDef()
{
	getSym(true, true);
	getSym(true, true);
	getSym(true, true);
	parseInteger();
	while (couple[0] == "COMMA")
	{
		getSym(true, true);
		getSym(true, true);
		getSym(true, true);
		parseInteger();
	}

	fout << "<常量定义>" << endl;
}

void parseInteger()
{
	if (couple[0] == "PLUS" || couple[0] == "MINU")
	{
		getSym(true, true);
	}
	else if (couple[0] == "CHARCON")
	{
		getSym(true, true);
		return;
	}
	parseUnsignedInt();
	
	fout << "<整数>" << endl;
}

void parseUnsignedInt()
{
	getSym(true, true); 
	fout << "<无符号整数>" << endl;
}

void parseConstDcrpt()
{
	if (getSym(true, true))
	{
		parseConstDef();
		if (couple[0] == "SEMICN")
		{
			getSym(true, true);
			while (couple[0] == "CONSTTK")
			{
				getSym(true, true);
				parseConstDef();
				getSym(true, true);
			}
		}
	}

	fout << "<常量说明>" << endl;
}

void parseVarDef()
{
	if (couple[0] != "INTTK" && couple[0] != "CHARTK") return;
	getSym(true, false);
	getSym(true, false);

	if (couple[0] == "LPARENT")
	{
		getSym(false, false);
		getSym(false, false);
		return;
	}

	getSym(false, false);
	getSym(false, false);
	getSym(true, true);
	getSym(true, true);

	if (couple[0] == "LBRACK")
	{
		getSym(true, true);
		parseUnsignedInt();
		getSym(true, true);
		if (couple[0] == "LBRACK")
		{
			getSym(true, true);
			parseUnsignedInt();
			getSym(true, true);
		}

		if (couple[0] == "ASSIGN")
		{
			while (couple[0] != "SEMICN")
			{
				if (!getSym(true, true)) break;
				if (couple[0] == "PLUS" || couple[0] == "MINU" || couple[0] == "INTCON" || couple[0] == "CHARCON")
				{
					parseConst();
				}
			}
			fout << "<变量定义及初始化>" << endl;
		}
		else
		{
			while (couple[0] != "SEMICN")
			{
				if (!getSym(true, true)) break;
				if (couple[0] == "LBRACK")
				{
					getSym(true, true);
					parseUnsignedInt();
				}
			}
			fout << "<变量定义无初始化>" << endl;
		}
	}
	else
	{
		if (couple[0] == "ASSIGN")
		{
			while (couple[0] != "SEMICN")
			{
				if (!getSym(true, true)) break;
				if (couple[0] == "PLUS" || couple[0] == "MINU" || couple[0] == "INTCON" || couple[0] == "CHARCON")
				{
					parseConst();
				}
			}
			fout << "<变量定义及初始化>" << endl;
		}
		else
		{
			while (couple[0] != "SEMICN")
			{
				if (!getSym(true, true)) break;
				if (couple[0] == "LBRACK")
				{
					getSym(true, true);
					parseUnsignedInt();
				}
			}
			fout << "<变量定义无初始化>" << endl;
		}
	}
	

	fout << "<变量定义>" << endl;
}

void parseConst()
{
	parseInteger();

	fout << "<常量>" << endl;
}

void parseVarDcrpt()
{
	parseVarDef();
	do
	{
		if (!getSym(true, true)) break;
		if (couple[0] == "SEMICN") break;
		parseVarDef();
	} while (couple[0] == "SEMICN");

	fout << "<变量说明>" << endl;
}

void parseParamTable()
{
	getSym(true, true);
	getSym(true, true);
	while (couple[0] == "COMMA")
	{
		getSym(true, true);
		getSym(true, true);
		getSym(true, true);
	}

	fout << "<参数表>" << endl;
}

void parseDeclareHead()
{
	getSym(true, true);
	keptLabelMap.insert(map<string, string>::value_type(couple[1], "RETURNABLE"));
	getSym(true, true);

	fout << "<声明头部>" << endl;
}

void parseLoopStmt()
{
	if (couple[0] == "WHILETK")
	{
		getSym(true, true);
		getSym(true, true);
		parseCondition();
		getSym(true, true);
		parseStmt();
	}
	else if (couple[0] == "FORTK")
	{
		getSym(true, true);
		getSym(true, true);
		getSym(true, true);
		getSym(true, true);
		parseExpr();
		getSym(true, true);
		parseCondition();
		getSym(true, true);
		getSym(true, true);
		getSym(true, true);
		getSym(true, true);
		getSym(true, true);
		parseStep();
		getSym(true, true);
		parseStmt();
	}

	fout << "<循环语句>" << endl;
}

void parseStep()
{
	parseUnsignedInt();

	fout << "<步长>" << endl;
}

void parseValueParamTable()
{
	if (couple[0] == "RPARENT")
	{
		fout << "<值参数表>" << endl;
		return;
	}
	else
	{
		parseExpr();
		while (couple[0] == "COMMA")
		{
			getSym(true, true);
			parseExpr();
		}
	}

	fout << "<值参数表>" << endl;
}

void parseFuncCallWithReturn()
{
	getSym(true, true);
	getSym(true, true);
	parseValueParamTable();
	getSym(true, true);

	fout << "<有返回值函数调用语句>" << endl;
}

void parseFuncCallWithoutReturn()
{
	getSym(true, true);
	getSym(true, true);
	parseValueParamTable();
	getSym(true, true);

	fout << "<无返回值函数调用语句>" << endl;
}

void parseFactor()
{
	if (couple[0] == "PLUS" || couple[0] == "MINU" || couple[0] == "INTCON")
	{
		parseInteger();
	}
	else if (couple[0] == "CHARCON")
	{
		getSym(true, true);
	}
	else if (keptLabelMap.find(couple[1]) != keptLabelMap.end() && keptLabelMap.find(couple[1])->second == "RETURNABLE")
	{
		parseFuncCallWithReturn();
	}
	else if (couple[0] == "LPARENT")
	{
		getSym(true, true);
		parseExpr();
		getSym(true, true);
	}
	else
	{
		getSym(true, true);
		if (couple[0] == "LBRACK")
		{
			getSym(true, true);
			parseExpr();
			getSym(true, true);
			if (couple[0] == "LBRACK")
			{
				getSym(true, true);
				parseExpr();
				getSym(true, true);
			}
		}
	}

	fout << "<因子>" << endl;
}

void parseTerm()
{
	parseFactor();
	while (couple[0] == "MULT" || couple[0] == "DIV")
	{
		getSym(true, true);
		parseFactor();
	}

	fout << "<项>" << endl;
}

void parseExpr()
{
	if (couple[0] == "PLUS" || couple[0] == "MINU")
	{
		getSym(true, true);
	}
	parseTerm();
	while (couple[0] == "PLUS" || couple[0] == "MINU")
	{
		getSym(true, true);
		parseTerm();
	}

	fout << "<表达式>" << endl;
}

void parseCondition()
{
	parseExpr();
	getSym(true, true);
	parseExpr();

	fout << "<条件>" << endl;
}

void parseCondStmt()
{
	getSym(true, true);
	getSym(true, true);
	parseCondition();
	getSym(true, true);
	parseStmt();
	if (couple[0] == "ELSETK")
	{
		getSym(true, true);
		parseStmt();
	}

	fout << "<条件语句>" << endl;
}

void parseReadStmt()
{
	getSym(true, true);
	getSym(true, true);
	getSym(true, true);
	getSym(true, true);

	fout << "<读语句>" << endl;
}

void parseString()
{
	getSym(true, true);

	fout << "<字符串>" << endl;
}

void parseWriteStmt()
{
	getSym(true, true);
	getSym(true, true);
	if (couple[0] == "STRCON")
	{
		parseString();
		if (couple[0] == "COMMA")
		{
			getSym(true, true);
			parseExpr();
			getSym(true, true);
		}
		else
		{
			getSym(true, true);
		}
	}
	else
	{
		parseExpr();
		getSym(true, true);
	}

	fout << "<写语句>" << endl;
}

void parseCondChildStmt()
{
	getSym(true, true);
	parseConst();
	getSym(true, true);
	parseStmt();

	fout << "<情况子语句>" << endl;
}

void parseCondTable()
{
	while (couple[0] == "CASETK")
	{
		parseCondChildStmt();
	}

	fout << "<情况表>" << endl;
}

void parseDefault()
{
	getSym(true, true);
	getSym(true, true);
	parseStmt();

	fout << "<缺省>" << endl;
}

void parseSwitchStmt()
{
	getSym(true, true);
	getSym(true, true);
	parseExpr();
	getSym(true, true);
	getSym(true, true);
	parseCondTable();
	parseDefault();
	getSym(true, true);

	fout << "<情况语句>" << endl;
}

void parseReturnStmt()
{
	getSym(true, true);
	if (couple[0] == "LPARENT")
	{
		getSym(true, true);
		parseExpr();
		getSym(true, true);
	}

	fout << "<返回语句>" << endl;
}

void parseStmt()
{
	if (couple[0] == "WHILETK" || couple[0] == "FORTK")
	{
		parseLoopStmt();
	}
	else if (couple[0] == "IFTK")
	{
		parseCondStmt();
	}
	else if (couple[0] == "SCANFTK")
	{
		parseReadStmt();
		getSym(true, true);
	}
	else if (couple[0] == "PRINTFTK")
	{
		parseWriteStmt();
		getSym(true, true);
	}
	else if (couple[0] == "SWITCHTK")
	{
		parseSwitchStmt();
	}
	else if (couple[0] == "SEMICN")
	{
		getSym(true, true);
	}
	else if (couple[0] == "RETURNTK")
	{
		parseReturnStmt();
		getSym(true, true);
	}
	else if (couple[0] == "LBRACE")
	{
		getSym(true, true);
		parseStmtList();
		getSym(true, true);
	}
	else
	{
		getSym(true, false);
		if (couple[0] == "LPARENT")
		{
			getSym(false, false);
			parseFuncCall();
			getSym(true, true);
		}
		else
		{
			getSym(false, false);
			getSym(true, true);
			parseAssignStmt();
			getSym(true, true);
		}
	}

	fout << "<语句>" << endl;
}

void parseFuncCall()
{
	if (keptLabelMap.find(couple[1]) != keptLabelMap.end() && keptLabelMap.find(couple[1])->second == "RETURNABLE")
	{
		parseFuncCallWithReturn();
	}
	else if (keptLabelMap.find(couple[1]) != keptLabelMap.end() && keptLabelMap.find(couple[1])->second == "UNRETURNABLE")
	{
		parseFuncCallWithoutReturn();
	}
}

void parseAssignStmt()
{
	if (couple[0] == "ASSIGN")
	{
		getSym(true, true);
		parseExpr();
	}
	else if (couple[0] == "LBRACK")
	{
		getSym(true, true);
		parseExpr();
		getSym(true, true);
		if (couple[0] == "ASSIGN")
		{
			getSym(true, true);
			parseExpr();
		}
		else if (couple[0] == "LBRACK")
		{
			getSym(true, true);
			parseExpr();
			getSym(true, true);
			getSym(true, true);
			parseExpr();
		}
	}

	fout << "<赋值语句>" << endl;
}

void parseStmtList()
{
	while (couple[0] != "RBRACE")
	{
		parseStmt();
	}

	fout << "<语句列>" << endl;
}

void parseCompStmt()
{
	if (couple[0] == "CONSTTK")
	{
		parseConstDcrpt();
	}

	if (couple[0] == "INTTK" || couple[0] == "CHARTK")
	{
		parseVarDcrpt();
	}

	parseStmtList();

	fout << "<复合语句>" << endl;
}

void parseFuncDef()
{
	string str = couple[0];
	getSym(true, false);
	while (str != "VOIDTK" || couple[0] != "MAINTK")
	{
		getSym(false, false);
		if (couple[0] == "VOIDTK")
		{
			getSym(true, true);
			keptLabelMap.insert(map<string, string>::value_type(couple[1], "UNRETURNABLE"));
			getSym(true, true);
			getSym(true, true);
			if (couple[0] != "RPARENT") parseParamTable();
			else fout << "<参数表>" << endl;
			getSym(true, true);
			getSym(true, true);
			parseCompStmt();
			getSym(true, true);
			fout << "<无返回值函数定义>" << endl;
		}
		else
		{
			parseDeclareHead();
			getSym(true, true);
			if (couple[0] != "RPARENT") parseParamTable();
			else fout << "<参数表>" << endl;
			getSym(true, true);
			getSym(true, true);
			parseCompStmt();
			getSym(true, true);
			fout << "<有返回值函数定义>" << endl;
		}
		str = couple[0];
		getSym(true, false);
	}
	getSym(false, false);
}

void parseMainFunc()
{
	getSym(true, true);
	getSym(true, true);
	getSym(true, true);
	getSym(true, true);
	getSym(true, true);
	parseCompStmt();
	getSym(true, true);

	fout << "<主函数>" << endl;
}

void parseProgram()
{
	if (couple[0] == "CONSTTK")
	{
		parseConstDcrpt();
	}

	if (couple[0] == "VOIDTK")
	{
		getSym(true, false);
		if (couple[0] == "MAINTK")
		{
			getSym(false, false);

			parseMainFunc();
		}
		else
		{
			getSym(false, false);

			parseFuncDef();
			parseMainFunc();
		}
	}
	else if (couple[0] == "INTTK" || couple[0] == "CHARTK")
	{
		getSym(true, false);
		getSym(true, false);
		if (couple[0] != "LPARENT")
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

	fout << "<程序>" << endl;
}

void error()
{

}

int getSym(bool forward, bool withOutput = false)
{
	if (forward)
	{
		if (withOutput && couple[0] != "" && couple[1] != "")
			fout << couple[0] << " " << couple[1] << endl;

		if (wordCnt >= top)
		{
			couple[0] = couple[1] = "";
			return 0;
		}
		else
		{
			couple[0] = mapList[wordCnt][0];
			couple[1] = mapList[wordCnt][1];
			wordCnt++;
			return 1;
		}
	}
	else
	{
		couple[0] = mapList[wordCnt - 2][0];
		couple[1] = mapList[wordCnt - 2][1];
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