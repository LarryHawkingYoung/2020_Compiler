#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stack>
#include<queue>
#include<map>
#define CONSTIDENTIFER 1
#define VARIABLENTIFER 2
#define RETURNFUNCIFER 3
#define UNRETUFUNCIFER 4
#define CHARVALUE 1
#define INTEVALUE 4
#define STATERETURN 3

/*
		function
		0:还没有被判断，或者其他
		1：常量
		2：变量
		3：有返回值函数
		4：无返回值函数
*/

using namespace std;
class Element {
public:
	Element(string word, vector<char> obj, int lx, int cx) {
		type = word;
		value = "";
		ln = lx;
		co = cx;
		for (int i = 0; i < obj.size(); i++) {
			value = value + obj[i];
		}
		function = 0;
		returnType = 0;
		paraNum = -1;
		arrayClom = "";
		offset = 0;
		constValue = "";
	}
	string getValue() {
		return value;
	}
	string toString() {
		string out;
		out = type + " ";
		for (int i = 0; i < value.size(); i++) {
			out = out + value[i];
		}
		return out;
	}
	bool isType(string in) {
		return type == in;
	}
	string getType() {
		return type;
	}
	int getLine() {
		return ln;
	}
	void setFunction(int x) {
		function = x;
	}
	int getParaNum() {
		return parameters.size();
	}
	int getFunction() {
		return function;
	}
	int getColm() {
		return co;
	}
	void setReturnType(int x) {
		returnType = x;
	}
	void setArrayColm(string x) {
		arrayClom = x;
	}
	string getArrayClom() {
		return arrayClom;
	}
	int getReturnType() {
		return returnType;
	}
	int getParameters(int num) {
		if (num > parameters.size()) {
			return -1;
		}
		return parameters[num - 1];
	}
	void addParameters(int type) {
		parameters.push_back(type);
	}
	void setConstValue(string a) {
		constValue = a;
	}
	string getConstValue() {
		return constValue;
	}
	void setOffset(int a) {
		offset = a;
	}
	int getOffset() {
		return offset;
	}
private:
	string type;
	string value;
	int ln;
	int co;
	int function;
	int paraNum;
	string arrayClom;
	int returnType;
	int offset;    //相对偏移量
	string constValue;
	vector<int> parameters;
	/*
		function
		0:还没有被判断，或者其他
		1：常量
		2：变量
		3：有返回值函数
		4：无返回值函数
	*/
};
typedef struct midcode {
	string func;
	string op1;
	string op2;
	string res;
}midCode;
char sym;
char nextLetter;
ifstream fin;
ofstream fout;
ofstream ferr;
ofstream mid;
ofstream mid2;
ofstream data;
ofstream mips0;
vector<char> buf;
vector<Element> storage;
queue<Element> errorWord;
vector<Element>  symbols;
vector<midCode> MidCodes;
vector<string>  strSymbols;
map<string, int> temp_alloc; //符号到分配空间偏移量//没有加字符串偏移的部分
string reg_sym[10];  //寄存器到符号 //一开始分配寄存器
int avalibleReg;
stack<int> symbolstable;
int start;
int index;
int line;
int colm;
bool unMatch = false;
bool isReturn;
int  reType;
bool hasReturn;
//生成四元式代码
int    midDstNum;
int    midLableNum;
int    codeptr; //静态分配地址，暂时
int    strSpace = 0;
stack<string> expre;

bool equalSymbolStr(string x, string y) {
	if (x.size() != y.size()) { return false; }
	int length = x.size();
	int j;
	for (j = 0; j < length; j++) {
		if (x[j] != y[j] && (x[j] + 32 != y[j] && x[j] - 32 != y[j])) {
			return false;
		}
	}
	return true;
}
int hasSymbolsStr(string y) {
	if (!((y[0] >= 'a' && y[0] <= 'z') || (y[0] >= 'A' && y[0] <= 'Z'))) {
		return -1;
	}
	for (int i = symbols.size() - 1; i >= 0; i--) {
		if (equalSymbolStr(symbols[i].getValue(), y)) {
			return i;
		}
	}
	return -1;
}
int codeptr_round(int size) {
	codeptr = ((codeptr % size) == 0) ? codeptr : ((codeptr / size) * size + size);
	return codeptr;
}
void codeptr_add(int size, int num) {
	codeptr = codeptr + num * size;
}
void addMipLw(string dst, int offset, string begin, int size) {
	if (size == 4) {
		mips0 << "\tlw\t" << dst
			<< "\t" << offset << "(" << begin << ")" << endl;
	}
	else {
		mips0 << "\tlb\t" << dst
			<< "\t" << offset << "(" << begin << ")" << endl;
	}
}
void addMipSw(string op, int offset, string begin, int size) {
	if (size == 4) {
		mips0 << "\tsw\t" << op
			<< "\t" << offset << "(" << begin << ")" << endl;
	}
	else if (size == 1) {
		mips0 << "\tsb\t" << op
			<< "\t" << offset << "(" << begin << ")" << endl;
	}
}
void addMipLi(string op, string value) {
	mips0 << "\tli\t" << op
		<< "\t" << value << endl;
}
void addMipLa(string op, string value) {
	mips0 << "\tla\t" << op
		<< "\t" << value << endl;
}
void addMipAdd(string res, string op1, string op2) {
	mips0 << "\tadd\t" << res << "\t" << op1
		<< "\t" << op2 << endl;
}
void addMipAddi(string res, string op1, string op2) {
	mips0 << "\taddi\t" << res << "\t" << op1
		<< "\t" << op2 << endl;
}
void addMipSub(string res, string op1, string op2) {
	mips0 << "\tsub\t" << res << "\t" << op1
		<< "\t" << op2 << endl;
}
void addMipSubi(string res, string op1, string op2) {
	mips0 << "\tsubi\t" << res << "\t" << op1
		<< "\t" << op2 << endl;
}
void addMipMult(string op1, string op2) {
	mips0 << "\tmult\t" << op1 << "\t" << op2 << endl;
}
void addMipDiv(string op1, string op2) {
	mips0 << "\tdiv\t" << op1 << "\t" << op2 << endl;
}
void addMipMflo(string op1) {
	mips0 << "\tmflo\t" << op1 << endl;
}
void addMipSys(int type) {
	addMipLi("$v0", to_string(type));
	mips0 << "\tsyscall\t" << endl;
}
void addMipSll(string res, string op1, string op2) {
	mips0 << "\tsll\t" << res << "\t" << op1
		<< "\t" << op2 << endl;
}
void addMipBjump(string type, string op1, string op2, string label) {
	mips0 << "\t" << type << "\t" << op1 << "\t" << op2
		<< "\t" << label << endl;
}
void addMipJump(string label) {
	mips0 << "\tj\t" << label << endl;
}

void InitReg() {
	for (int i = 0; i < 10; i++) {
		reg_sym[i] = "";
	}
	avalibleReg = 0;
}
int useReg() {
	int use = avalibleReg;
	avalibleReg = (avalibleReg + 1) % 10;
	if (reg_sym[use] != "") {
		if (reg_sym[use][0] == '\'' || reg_sym[use][0] == '-'
			|| (reg_sym[use][0] >= '0' && reg_sym[use][0] <= '9')) {
			return use;
		}
		if (temp_alloc.find(reg_sym[use]) == temp_alloc.end()) {
			if (reg_sym[use][0] == '@') {
				temp_alloc.insert(pair<string, int>(reg_sym[use], codeptr_round(4)));
				addMipSw("$t" + to_string(use), codeptr_round(4) + strSpace, "$s0", 4);
				codeptr_add(4, 1);
			}
			else {
				int search = hasSymbolsStr(reg_sym[use]);
				int offset = symbols[search].getOffset();
				temp_alloc.insert(pair<string, int>(reg_sym[use], offset));
				addMipSw("$t" + to_string(use), offset + strSpace, "$s0",
					symbols[search].getReturnType());
			}
		}
		else {
			int search = hasSymbolsStr(reg_sym[use]);
			if (search != -1) {
				addMipSw("$t" + to_string(use), temp_alloc[reg_sym[use]] + strSpace, "$s0",
					symbols[search].getReturnType());
			}
			else {
				addMipSw("$t" + to_string(use), temp_alloc[reg_sym[use]] + strSpace, "$s0", 4);
			}
		}
	}
	return use;
}
string getSymReg(string now, bool getValue, int search) {
	//转换成小写
	//cout << "begin find reg:	" <<now<< endl;
	if (now[0] == '\'' || now[0] == '-'
		|| (now[0] >= '0' && now[0] <= '9')) {
		if (now == "0" || now == "-0") {
			return "$0";
		}
		for (int i = 0; i < 10; i++) {
			if (reg_sym[i] == now) {
				return "$t" + to_string(i);
			}
		}
		int use = useReg();
		reg_sym[use] = now;
		if (getValue) {
			addMipLi("$t" + to_string(use), now);
		}
		return "$t" + to_string(use);
	}
	else if (now[0] != '@') {
		string temp = "";
		//cout << "get reg iden is " <<now<< endl;
		for (int i = 0; i < now.size(); i++) {
			if (now[i] >= 'A' && now[i] <= 'Z') {
				char x = (now[i] + 32);
				temp = temp + x;
			}
			else {
				temp = temp + now[i];
			}
		}
		//cout << "temp is "<<temp << endl;
		for (int i = 0; i < 10; i++) {
			if (reg_sym[i] == temp) {
				cout << "find" << endl;
				return "$t" + to_string(i);
			}
		}
		int use = useReg();
		reg_sym[use] = temp;
		if (getValue) {
			if (temp_alloc.find(temp) == temp_alloc.end()) {
				temp_alloc.insert(pair<string, int>(temp, symbols[search].getOffset()));
			}
			//cout << "load the value" << endl;
			if (search != -1) {
				addMipLw("$t" + to_string(use), temp_alloc[temp] + strSpace, "$s0",
					symbols[search].getReturnType());
			}
		}
		return "$t" + to_string(use);
	}
	else {
		for (int i = 0; i < 10; i++) {
			if (reg_sym[i] == now) {
				return "$t" + to_string(i);
			}
		}
		int use = useReg();
		reg_sym[use] = now;
		if (getValue) {
			if (temp_alloc.find(now) != temp_alloc.end()) {
				addMipLw("$t" + to_string(use), temp_alloc[now] + strSpace, "$s0", 4);
			}
		}
		return "$t" + to_string(use);
	}
}
void MidCMIPS() {
	//常量的转换应该在四元式里面转换，在这边转换好像不是很优雅
	//cout << "zer is " << -7 * (7 * ((+1) * 5 * 'a'));
	if (strSpace != 0) {
		mips0 << ".data" << endl;
		for (int i = 0; i < strSymbols.size(); i++) {
			mips0 << "\tstr" << i << ": .asciiz \"" << strSymbols[i] << "\"" << endl;
		}
	}
	mips0 << ".text" << endl;
	strSpace = ((strSpace % 4) == 0) ? strSpace : ((strSpace / 4) * 4 + 4);
	addMipLi("$s0", "0x10010000");
	InitReg();
	for (int i = 0; i < MidCodes.size(); i++) {
		if (MidCodes[i].func == "assign") {
			string op1 = MidCodes[i].op1;
			string op2 = MidCodes[i].op2;
			int search1 = hasSymbolsStr(op1);
			int search2 = hasSymbolsStr(op2);
			string op1_reg = getSymReg(op1, false, search1);
			//cout << "assign\t" <<op1<<"\t"<< op1_reg << endl;
			if (op2[0] == '\'' || (op2[0] >= '0' && op2[0] <= '9') || op2[0] == '-') {
				addMipLi(op1_reg, op2);
			}
			else {
				addMipAdd(op1_reg, getSymReg(op2, true, search2), "$0");
			}
		}
		else if (MidCodes[i].func == "add") {
			string op1 = MidCodes[i].op1;
			string op2 = MidCodes[i].op2;
			string res = MidCodes[i].res;
			int search1 = hasSymbolsStr(op1);
			int search2 = hasSymbolsStr(op2);
			if (search1 != -1 && symbols[search1].getFunction() == CONSTIDENTIFER) {
				op1 = symbols[search1].getConstValue();
			}
			if (search2 != -1 && symbols[search2].getFunction() == CONSTIDENTIFER) {
				op2 = symbols[search2].getConstValue();
			}
			bool op1_num = ((op1[0] >= '0' && op1[0] <= '9') || op1[0] == '-' || op1[0] == '\'');
			bool op2_num = ((op2[0] >= '0' && op2[0] <= '9') || op2[0] == '-' || op2[0] == '\'');
			if (!op1_num && op2_num) {
				addMipAddi(getSymReg(res, false, -1), getSymReg(op1, true, search1), op2);
			}
			else if (op1_num && !op2_num) {
				addMipAddi(getSymReg(res, false, -1), getSymReg(op2, true, search2), op1);
			}
			else if (op1_num && op2_num) {
				int result = atoi(op1.c_str()) + atoi(op2.c_str());
				addMipLi(getSymReg(res, false, -1), to_string(result));
			}
			else {
				string op1_reg = getSymReg(op1, true, search1);
				//cout << "\t" << op1_reg << "\t" << op1<<endl;
				addMipAdd(getSymReg(res, false, -1), op1_reg,
					getSymReg(op2, true, search2));
			}
		}
		else if (MidCodes[i].func == "sub") {
			string op1 = MidCodes[i].op1;
			string op2 = MidCodes[i].op2;
			string res = MidCodes[i].res;
			int search1 = hasSymbolsStr(op1);
			int search2 = hasSymbolsStr(op2);
			if (search1 != -1 && symbols[search1].getFunction() == CONSTIDENTIFER) {
				op1 = symbols[search1].getConstValue();
			}
			if (search2 != -1 && symbols[search2].getFunction() == CONSTIDENTIFER) {
				op2 = symbols[search2].getConstValue();
			}
			bool op1_num = ((op1[0] >= '0' && op1[0] <= '9') || op1[0] == '-' || op1[0] == '\'');
			bool op2_num = ((op2[0] >= '0' && op2[0] <= '9') || op2[0] == '-' || op2[0] == '\'');
			if (!op1_num && op2_num) {
				addMipSubi(getSymReg(res, false, -1), getSymReg(op1, true, search1), op2);
			}
			else if (op1_num && !op2_num) {
				addMipSubi(getSymReg(res, false, -1), getSymReg(op2, true, search2), op1);
				addMipSub(getSymReg(res, false, -1), "$0", getSymReg(res, true, -1));
			}
			else if (op1_num && op2_num) {
				int result = atoi(op1.c_str()) - atoi(op2.c_str());
				addMipLi(getSymReg(res, false, -1), to_string(result));
			}
			else {
				addMipSub(getSymReg(res, false, -1), getSymReg(op1, true, search1),
					getSymReg(op2, true, search2));
			}
		}
		else if (MidCodes[i].func == "mult") {
			string op1 = MidCodes[i].op1;
			string op2 = MidCodes[i].op2;
			string res = MidCodes[i].res;
			int search1 = hasSymbolsStr(op1);
			int search2 = hasSymbolsStr(op2);
			if (search1 != -1 && symbols[search1].getFunction() == CONSTIDENTIFER) {
				op1 = symbols[search1].getConstValue();
			}
			if (search2 != -1 && symbols[search2].getFunction() == CONSTIDENTIFER) {
				op2 = symbols[search2].getConstValue();//qwq
			}
			bool op1_num = ((op1[0] >= '0' && op1[0] <= '9') || op1[0] == '-' || op1[0] == '\'');
			bool op2_num = ((op2[0] >= '0' && op2[0] <= '9') || op2[0] == '-' || op2[0] == '\'');
			if (!op1_num && op2_num) {
				addMipLi("$1", op2);
				addMipMult(getSymReg(op1, true, search1), "$1");
				addMipMflo(getSymReg(res, false, -1));
			}
			else if (op1_num && !op2_num) {
				addMipLi("$1", op1);
				addMipMult(getSymReg(op2, true, search2), "$1");
				addMipMflo(getSymReg(res, false, -1));
			}
			else if (!op1_num && !op2_num) {
				addMipMult(getSymReg(op1, true, search1), getSymReg(op2, true, search2));
				addMipMflo(getSymReg(res, false, -1));
			}
			else {
				int result = atoi(op1.c_str()) * atoi(op2.c_str());
				addMipLi(getSymReg(res, false, -1), to_string(result));
			}
		}
		else if (MidCodes[i].func == "div") {
			string op1 = MidCodes[i].op1;
			string op2 = MidCodes[i].op2;
			string res = MidCodes[i].res;
			int search1 = hasSymbolsStr(op1);
			int search2 = hasSymbolsStr(op2);
			if (search1 != -1 && symbols[search1].getFunction() == CONSTIDENTIFER) {
				op1 = symbols[search1].getConstValue();
			}
			if (search2 != -1 && symbols[search2].getFunction() == CONSTIDENTIFER) {
				op2 = symbols[search2].getConstValue();
			}
			bool op1_num = ((op1[0] >= '0' && op1[0] <= '9') || op1[0] == '-' || op1[0] == '\'');
			bool op2_num = ((op2[0] >= '0' && op2[0] <= '9') || op2[0] == '-' || op2[0] == '\'');
			if (!op1_num && op2_num) {
				addMipLi("$1", op2);
				addMipDiv(getSymReg(op1, true, search1), "$1");
				addMipMflo(getSymReg(res, false, -1));
			}
			else if (op1_num && !op2_num) {
				addMipLi("$1", op1);
				addMipDiv("$1", getSymReg(op2, true, search2));
				addMipMflo(getSymReg(res, false, -1));
			}
			else if (!op1_num && !op2_num) {
				addMipDiv(getSymReg(op1, true, search1), getSymReg(op2, true, search2));
				addMipMflo(getSymReg(res, false, -1));
			}
			else {
				int result = atoi(op1.c_str()) / atoi(op2.c_str());
				addMipLi(getSymReg(res, false, -1), to_string(result));
			}
		}
		else if (MidCodes[i].func == "scanc") {
			addMipSys(12);
			int search = hasSymbolsStr(MidCodes[i].op1);
			addMipAdd(getSymReg(MidCodes[i].op1, false, search), "$0", "$v0");
		}
		else if (MidCodes[i].func == "scanf") {
			addMipSys(5);
			string op1 = MidCodes[i].op1;
			int search = hasSymbolsStr(MidCodes[i].op1);
			addMipAdd(getSymReg(MidCodes[i].op1, false, search), "$0", "$v0");
		}
		else if (MidCodes[i].func == "printc") {
			string op1 = MidCodes[i].op1;
			int search1 = hasSymbolsStr(op1);
			if (search1 != -1 && symbols[search1].getFunction() == CONSTIDENTIFER) {
				op1 = symbols[search1].getConstValue();
			}
			bool isChar = op1[0] == '\'';
			if (!isChar) {
				addMipAdd("$a0", getSymReg(op1, true, search1), "$0");
			}
			else {
				addMipLi("$a0", op1);
			}
			addMipSys(11);
			addMipLi("$a0", "\'\\n\'");
			addMipSys(11);
		}
		else if (MidCodes[i].func == "prints") {
			string str = MidCodes[i].op1;
			addMipLa("$a0", str);
			int num = atoi(str.substr(3, str.size() - 1).c_str());
			int size = strSymbols[num].size();
			if (strSymbols[num].size() >= 2 && strSymbols[num][size - 2] == '\\') {
				size = size - 1;
			}
			addMipLi("$a1", to_string(size));
			addMipSys(4);
		}
		else if (MidCodes[i].func == "printf") {
			string op1 = MidCodes[i].op1;
			int search1 = hasSymbolsStr(op1);
			if (search1 != -1 && symbols[search1].getFunction() == CONSTIDENTIFER) {
				op1 = symbols[search1].getConstValue();
			}
			bool isNum = ((op1[0] >= '0' && op1[0] <= '9') || op1[0] == '-');
			if (!isNum) {
				addMipAdd("$a0", getSymReg(op1, true, search1), "$0");
			}
			else {
				addMipLi("$a0", op1);
			}
			addMipSys(1);
			addMipLi("$a0", "\'\\n\'");
			addMipSys(11);
		}
		else if (MidCodes[i].func == "sll") {
			//先默认第二个数是一个常数
			string op1 = MidCodes[i].op1;
			string op2 = MidCodes[i].op2;
			int search1 = hasSymbolsStr(MidCodes[i].op1);
			int search2 = hasSymbolsStr(MidCodes[i].op2);
			if (search1 != -1 && symbols[search1].getFunction() == CONSTIDENTIFER) {
				op1 = symbols[search1].getConstValue();
				addMipLi("$1", op1);
				addMipSll(getSymReg(MidCodes[i].res, false, -1),
					op1, MidCodes[i].op2);
			}
			else {
				addMipSll(getSymReg(MidCodes[i].res, false, -1),
					getSymReg(MidCodes[i].op1, true, search1), MidCodes[i].op2);
			}
		}
		else if (MidCodes[i].func == "set") {
			mips0 << MidCodes[i].op1 << ":" << endl;
		}
		else if (MidCodes[i].func[0] == 'b') {
			string op1 = MidCodes[i].op1;
			string op2 = MidCodes[i].op2;
			int search1 = hasSymbolsStr(op1);
			int search2 = hasSymbolsStr(op2);
			if (search1 != -1 && symbols[search1].getFunction() == CONSTIDENTIFER) {
				op1 = symbols[search1].getConstValue();
			}
			if (search2 != -1 && symbols[search2].getFunction() == CONSTIDENTIFER) {
				op2 = symbols[search2].getConstValue();
			}
			bool op1_num = ((op1[0] >= '0' && op1[0] <= '9') || op1[0] == '-' || op1[0] == '\'');
			bool op2_num = ((op2[0] >= '0' && op2[0] <= '9') || op2[0] == '-' || op2[0] == '\'');
			if (op1_num && op2_num) {
				int result = atoi(op1.c_str()) - atoi(op2.c_str());
				if (result != 0) {
					addMipLi("$1", to_string(result));
					addMipBjump(MidCodes[i].func, "$1", "$0", MidCodes[i].res);
				}
				else {
					addMipBjump(MidCodes[i].func, "$0", "$0", MidCodes[i].res);
				}
			}
			else if (op1_num) {
				addMipLi("$1", op1);
				addMipBjump(MidCodes[i].func, "$1", getSymReg(op2, search2, true), MidCodes[i].res);
			}
			else if (op2_num) {
				addMipLi("$1", op2);
				addMipBjump(MidCodes[i].func, getSymReg(op1, search1, true), "$1", MidCodes[i].res);
			}
			else {
				addMipBjump(MidCodes[i].func, getSymReg(op1, search1, true), getSymReg(op2, search2, true), MidCodes[i].res);
			}
		}
		else if (MidCodes[i].func == "stoa") {
			string op1 = MidCodes[i].op1;
			string op2 = MidCodes[i].op2;
			string op3 = MidCodes[i].res;
			int search1 = hasSymbolsStr(op1);
			int search2 = hasSymbolsStr(op2);
			int search3 = hasSymbolsStr(op3);
			int size = symbols[search1].getReturnType();
			if (search2 != -1 && symbols[search2].getFunction() == CONSTIDENTIFER) {
				op2 = symbols[search2].getConstValue();
			}
			if (search3 != -1 && symbols[search3].getFunction() == CONSTIDENTIFER) {
				op3 = symbols[search3].getConstValue();
			}
			bool op2_num = ((op2[0] >= '0' && op2[0] <= '9') || op2[0] == '-' || op2[0] == '\'');
			bool op3_num = ((op3[0] >= '0' && op3[0] <= '9') || op3[0] == '-' || op3[0] == '\'');
			//这边蛮试一下不知道会不会有错，加strSpace
			int offset = symbols[search1].getOffset() + strSpace;
			if (op2_num && !op3_num) {
				offset = offset + atoi(op2.c_str());
				addMipSw(getSymReg(op3, true, search3), offset, "$s0", size);
			}
			else if (!op2_num && op3_num) {
				addMipAdd("$1", getSymReg(op2, true, search2), "$s0");
				string value = getSymReg(op3, true, -1);
				if (value != "$0") {
					addMipLi(value, op3);
				}
				addMipSw(value, offset, "$1", size);
			}
			else if (op2_num && op3_num) {
				offset = offset + atoi(op2.c_str());
				string value = getSymReg(op3, true, -1);
				if (value != "$0") {
					addMipLi(value, op3);
				}
				addMipSw(value, offset, "$s0", size);
			}
			else if (!op2_num && !op3_num) {
				addMipAdd("$1", getSymReg(op2, true, search2), "$s0");
				addMipSw(getSymReg(op3, true, search3), offset, "$1", size);
			}
		}
		else if (MidCodes[i].func == "geta") {
			// 没有考虑讲得到的值直接赋值给另一个标识符的情况
			//优化可以往这个方面进行考虑
			string op1 = MidCodes[i].op1;
			string op2 = MidCodes[i].op2;
			string res = MidCodes[i].res;
			int search1 = hasSymbolsStr(op1);
			int search2 = hasSymbolsStr(op2);
			int size = symbols[search1].getReturnType();
			if (search2 != -1 && symbols[search2].getFunction() == CONSTIDENTIFER) {
				op2 = symbols[search2].getConstValue();
			}
			bool op2_num = ((op2[0] >= '0' && op2[0] <= '9') || op2[0] == '-' || op2[0] == '\'');
			int offset = symbols[search1].getOffset() + strSpace;
			if (op2_num) {
				offset = offset + atoi(op2.c_str());
				addMipLw(getSymReg(res, false, -1), offset, "$s0", size);
			}
			else {
				addMipAdd("$1", "$s0", getSymReg(op2, true, search2));
				addMipLw(getSymReg(res, false, -1), offset, "$1", size);
			}
		}
		else if (MidCodes[i].func == "goto") {
			addMipJump(MidCodes[i].op1);
		}
	}
}
void addMidCode(string func, string op1, string op2, string dst) {
	mid << func + "\t" + op1 + "\t" + op2 + "\t" + dst + "\n";
	midCode x = { func,op1,op2,dst };
	MidCodes.push_back(x);
}
void push_expre(string x) {
	string dst;
	string op1;
	string op2;
	if (x == "+") {
		dst = "@" + to_string(midDstNum);
		midDstNum++;
		op2 = expre.top();
		expre.pop();
		op1 = expre.top();
		expre.pop();
		addMidCode("add", op1, op2, dst);
		expre.push(dst);
	}
	else if (x == "-")
	{
		dst = "@" + to_string(midDstNum);
		midDstNum++;
		op2 = expre.top();
		expre.pop();
		op1 = expre.top();
		expre.pop();
		addMidCode("sub", op1, op2, dst);
		expre.push(dst);
	}
	else if (x == "*") {
		dst = "@" + to_string(midDstNum);
		midDstNum++;
		op2 = expre.top();
		expre.pop();
		op1 = expre.top();
		expre.pop();
		addMidCode("mult", op1, op2, dst);
		expre.push(dst);
	}
	else if (x == "/")
	{
		dst = "@" + to_string(midDstNum);
		midDstNum++;
		op2 = expre.top();
		expre.pop();
		op1 = expre.top();
		expre.pop();
		addMidCode("div", op1, op2, dst);
		expre.push(dst);
	}
	else {
		expre.push(x);
	}
}
string getArray(int search, string dimention1, string dimention2) {
	int type = symbols[search].getReturnType();
	string arrayColm = symbols[search].getArrayClom();
	if (dimention1 != "" && dimention2 == "") {
		if (type == INTEVALUE) {
			addMidCode("sll", dimention1, "2", "@" + to_string(midDstNum));
		}
		midDstNum++;
		addMidCode("geta", symbols[search].getValue(),
			"@" + to_string(midDstNum - 1), "@" + to_string(midDstNum));
		midDstNum++;
		return "@" + to_string(midDstNum - 1);
	}
	else if (dimention1 != "" && dimention2 != "") {
		push_expre(dimention1);
		push_expre(arrayColm);
		push_expre("*");
		string tmp = expre.top();
		expre.pop();
		push_expre(tmp);
		push_expre(dimention2);
		push_expre("+");
		tmp = expre.top();
		expre.pop();
		if (type == INTEVALUE) {
			addMidCode("sll", tmp, "2", tmp);
		}
		addMidCode("geta", symbols[search].getValue(),
			tmp, "@" + to_string(midDstNum));
		midDstNum++;
		return "@" + to_string(midDstNum - 1);
	}
	return "";
}
void stoArray(int search, string dimention1, string dimention2, string value) {
	int type = symbols[search].getReturnType();
	string arrayColm = symbols[search].getArrayClom();
	if (dimention1 != "" && dimention2 == "") {
		if (type == INTEVALUE) {
			addMidCode("sll", dimention1, "2", "@" + to_string(midDstNum));
		}
		midDstNum++;
		addMidCode("stoa", symbols[search].getValue(),
			"@" + to_string(midDstNum - 1), value);
	}
	else if (dimention1 != "" && dimention2 != "") {
		push_expre(dimention1);
		push_expre(arrayColm);
		push_expre("*");
		string tmp = expre.top();
		expre.pop();
		push_expre(tmp);
		push_expre(dimention2);
		push_expre("+");
		tmp = expre.top();
		expre.pop();
		if (type == INTEVALUE) {
			addMidCode("sll", tmp, "2", tmp);
		}
		addMidCode("stoa", symbols[search].getValue(),
			tmp, value);
	}
}











void addStorage(string type) {
	Element x = { type,buf ,line ,colm };
	colm++;
	buf.clear();
	storage.push_back(x);
}
void addErrorWord() {
	colm++;
	Element x = { "Error",buf,line,colm };
	colm++;
	buf.clear();
	errorWord.push(x);
}
bool equalSymbol(Element ex, Element ey) {
	string x = ex.getValue();
	string y = ey.getValue();
	if (x.size() != y.size()) { return false; }
	int length = x.size();
	int j;
	for (j = 0; j < length; j++) {
		if (x[j] != y[j] && (x[j] + 32 != y[j] && x[j] - 32 != y[j])) {
			cout << "x is " << x << " " << "x[j] is "
				<< x[j] << " y is " << y << " y[j] " << y[j] << endl;;
			return false;
		}
	}
	return true;
}
int hasSymbols(Element y) {
	for (int i = symbols.size() - 1; i >= 0; i--) {
		if (equalSymbol(symbols[i], y)) {
			return i;
		}
	}
	return -1;
}

void errorOut(string x, int li, int co) {
	while (!errorWord.empty() && errorWord.front().getLine() <= li) {
		if (errorWord.front().getLine() == li
			&& co > errorWord.front().getLine()) {
			break;
		}
		ferr << errorWord.front().getLine() << " " << "a" << "\n";
	}
	ferr << li << " " << x << "\n";
	//fout << x<<" " << li << "\n";
}
void getsym() {
	sym = nextLetter;
	//fin.get(nextLetter);

	if (!fin.eof()) {
		fin.get(nextLetter);
		//存在下一个字符
	}
	else {
		nextLetter = '\0';		//不存在下一个字符
	}
}
int is_letter(char x) {
	if (x >= 'a' && x <= 'z') {
		return 1;
	}
	if (x >= 'A' && x <= 'Z') {
		return 1;
	}
	if (x == '_') {
		return 1;
	}
	return 0;
}
int is_num(char x) {
	if (x >= '0' && x <= '9') {
		return 1;
	}
	return 0;
}
int is_blank(char x) {
	if (x == '\n') { line++; colm = 1; }
	if (x == ' ' || x == '\r' || x == '\t' || x == '\n') {
		return 1;
	}
	return 0;
}
void identifier() {
	while (is_letter(sym) || is_num(sym)) {
		buf.push_back(sym);
		getsym();
	}
	addStorage("IDENFR");
}
void word_identify(const char* word, int length, string out) {
	int i = 0;
	while (i < length) {
		if (word[i] != sym && sym + 32 != word[i]) {
			identifier(); return;
		}
		else {
			buf.push_back(sym);
			getsym();
			i++;
		}
	}
	if (is_letter(sym) || is_num(sym)) {
		//std::cout << "nishirong nice!\n";
		identifier(); return;
	}
	addStorage(out);
}
void alphabet() {

	switch (sym)
	{
	case 'C':
	case 'c':
		if (nextLetter == 'o' || nextLetter == 'O') {
			//std::cout << "nishirong!";
			word_identify("const", 5, "CONSTTK");
		}
		else if (nextLetter == 'h' || nextLetter == 'H') { word_identify("char", 4, "CHARTK"); }
		else if (nextLetter == 'a' || nextLetter == 'A') { word_identify("case", 4, "CASETK"); }
		else { identifier(); }
		break;
	case 'D':
	case 'd':
		word_identify("default", 7, "DEFAULTTK");
		break;
	case 'E':
	case 'e':
		word_identify("else", 4, "ELSETK");
		break;
	case 'F':
	case 'f':
		word_identify("for", 3, "FORTK");
		break;
	case 'I':
	case 'i':
		if (nextLetter == 'f' || nextLetter == 'F') {
			word_identify("if", 2, "IFTK");
		}
		else {
			word_identify("int", 3, "INTTK");
		}
		break;
	case 'M':
	case 'm':
		word_identify("main", 4, "MAINTK");
		break;
	case 'P':
	case 'p':
		word_identify("printf", 6, "PRINTFTK");
		break;
	case 'R':
	case 'r':
		word_identify("return", 6, "RETURNTK");
		break;
	case 'S':
	case 's':
		if (nextLetter == 'w' || nextLetter == 'W') {
			word_identify("switch", 6, "SWITCHTK");
		}
		else {
			word_identify("scanf", 5, "SCANFTK");
		}
		break;
	case 'V':
	case 'v':
		word_identify("void", 4, "VOIDTK");
		break;
	case 'W':
	case 'w':
		word_identify("while", 5, "WHILETK");
		break;
	default:
		if (is_letter(sym)) {
			identifier();
		}
		break;
	}
}
void alnum_identify() {
	while (isalnum(sym)) {
		buf.push_back(sym);
		getsym();
	}
	addStorage("INTCON");
}
void letter_identify() {
	getsym();
	if (sym == '+' || sym == '-' || sym == '*' || sym == '/'
		|| is_letter(sym) || isalnum(sym)) {
		buf.push_back(sym);
		getsym();
	}
	int flag = 0;
	while (sym != '\'' && sym != '\0') {
		buf.push_back(sym);
		getsym();
		flag = 1;
	}
	if (flag == 1) {
		addStorage("VCHARCON");
		getsym();
		return;
	}
	getsym();
	addStorage("CHARCON");
}
void string_identify() {
	getsym();
	int flag = 0;
	while (sym != '\"') {
		if (sym == 32 || sym == 33 || (sym >= 35 && sym <= 126)) {
			buf.push_back(sym);
			getsym();
			flag = 1;
		}
		else { break; }
	}
	if (flag == 0) {
		while (sym != '\"' && sym != '\0') {
			buf.push_back(sym);
			getsym();
		}
		addStorage("VSTRCON");
		getsym();
		return;
	}
	addStorage("STRCON");
	getsym();
}

int isSEMICN() {
	if (storage[index].isType("SEMICN")) { index++; }
	else {
		errorOut("k", storage[index - 1].getLine(),
			storage[index - 1].getColm());
	}

	return 0;
}
int isRPARENT() {
	if (storage[index].isType("RPARENT")) { index++; }
	else {
		errorOut("l", storage[index].getLine(),
			storage[index].getColm());
	}
	return 0;
}
int isRBRACK() {
	if (storage[index].isType("RBRACK")) { index++; }
	else {
		errorOut("m", storage[index].getLine(),
			storage[index].getColm());
	}
	return 0;
}
int UnSignIntergers() {
	//cout << "unsigned intergers!" << endl;
	//cout << storage[index].toString() << endl;
	if (storage[index].isType("INTCON")) {
		//cout << "unsigned intergers! in in in" << endl;
		expre.push(storage[index].getValue());
		index++;
		while (start < index) {
			fout << storage[start].toString() << endl;
			start++;
		}
		fout << "<无符号整数>" << endl;

		return INTEVALUE;
	}
	else { return 0; }
}
int Intergers() {
	bool isMINU = false;
	if (storage[index].isType("PLUS") ||
		storage[index].isType("MINU")) {
		if (storage[index].isType("MINU")) {
			isMINU = true;
		}
		index++;
	}
	int result = UnSignIntergers();
	if (result != 0) {
		string inte = expre.top();
		expre.pop();
		if (isMINU) {
			expre.push("-" + inte);
		}
		else { expre.push(inte); }
	}

	while (start < index) {
		fout << storage[start].toString() << endl;
		start++;
	}
	fout << "<整数>" << endl;
	return result;
	//return 0;
}
int ConstValue(int expect) {
	if (storage[index].isType("CHARCON")) {
		expre.push("\'" + storage[index].getValue() + "\'");
		//VCHARCON
		if (expect != CHARVALUE) {
			errorOut("o", storage[index].getLine(), storage[index].getColm());
		}
		index++;
	}
	else if (storage[index].isType("VCHARCON")) {
		errorOut("a", storage[index].getLine(), storage[index].getColm());
		if (expect != CHARVALUE) {
			errorOut("o", storage[index].getLine(), storage[index].getColm());
		}
		index++;
	}
	else {
		Intergers();
		if (expect == CHARVALUE) {
			errorOut("o", storage[index].getLine(), storage[index].getColm());
		}
	}

	while (start < index) {
		fout << storage[start].toString() << endl;
		start++;
	}
	fout << "<常量>" << endl;

	return 0;
}
int ConstantDefine() {
	//int now = index && 保存回溯的位置
	//int num = 0;
	if (storage[index].isType("INTTK")) {
		index++;
		while (storage[index].isType("IDENFR")) {
			if (hasSymbols(storage[index]) >= symbolstable.top()) {
				errorOut("b", storage[index].getLine(),
					storage[index].getColm());
				//fout << "b " << storage[index].getLine() << "\n";
			}
			else {
				storage[index].setReturnType(INTEVALUE);
				storage[index].setFunction(CONSTIDENTIFER);
				symbols.push_back(storage[index]);
			}
			index++;

			if (storage[index].isType("ASSIGN")) {
				index++;
			}
			else {
				return -1;
			}
			Intergers();
			symbols.back().setConstValue(expre.top());
			expre.pop();
			if (!storage[index].isType("COMMA")) {

				while (start < index) {
					fout << storage[start].toString() << endl;
					start++;
				}
				fout << "<常量定义>" << endl;

				return 0;
			}
			else {
				index++;
				//cout << "const define check!! , " << storage[index].toString() << endl;
			}
		}
	}
	else if (storage[index].isType("CHARTK")) {
		index++;
		while (storage[index].isType("IDENFR")) {
			if (hasSymbols(storage[index]) != -1) {
				errorOut("b", storage[index].getLine(),
					storage[index].getColm());
				//fout << "b " << storage[index].getLine() << "\n";
			}
			else {
				storage[index].setReturnType(CHARVALUE);
				storage[index].setFunction(CONSTIDENTIFER);
				symbols.push_back(storage[index]);
			}
			index++;
			if (storage[index].isType("ASSIGN")) {
				index++;
			}
			else {
				return -1;
			}
			if (storage[index].isType("CHARCON")) {
				symbols.back().setConstValue("\'" + storage[index].getValue() + "\'");
				index++;
			}
			else if (storage[index].isType("VCHARCON")) {
				errorOut("a", storage[index].getLine(), storage[index].getColm());
				index++;
			}
			else { return -1; }
			if (!storage[index].isType("COMMA")) {

				while (start < index) {
					fout << storage[start].toString() << endl;
					start++;
				}
				fout << "<常量定义>" << endl;

				return 0;
			}
			else {
				index++;
			}
		}
	}
	return -1;
}
int ConstantDescript() {
	//int now = index && 保存回溯的位置
	//
	while (storage[index].isType("CONSTTK")) {
		index++;
		ConstantDefine();
		isSEMICN();
	}

	while (start < index) {
		fout << storage[start].toString() << endl;
		start++;
	}
	fout << "<常量说明>" << endl;

	return 0;
}
int VariableInit() {

	int mmm = 0;
	if (storage[index].isType("CHARTK")) {
		mmm = CHARVALUE;
		index++;
	}
	else if (storage[index].isType("INTTK")) {
		mmm = INTEVALUE;
		index++;
	}
	else { return -1; }
	//cout << "VariableInit in 1" << endl;
	int type = 0;
	int d1 = 0, d2 = 0;
	int flag = 0; //是否已经有过一个变量定义无初始化程序
	while (true) {
		type = 0;
		if (storage[index].isType("IDENFR")) {
			if (hasSymbols(storage[index]) >= symbolstable.top()) {
				errorOut("b", storage[index].getLine(),
					storage[index].getColm());
			}
			else {
				storage[index].setReturnType(mmm);
				storage[index].setFunction(VARIABLENTIFER);
				storage[index].setOffset(codeptr_round(mmm));
				codeptr_add(mmm, 1);
				symbols.push_back(storage[index]);
			}
			index++;
		}
		else { return -1; }
		if (storage[index].isType("LBRACK")) {
			index++;
			UnSignIntergers();
			type = 1;
			d1 = atoi(expre.top().c_str());
			expre.pop();
			isRBRACK();
			if (storage[index].isType("LBRACK")) {

				index++;
				type = 2;
				UnSignIntergers();
				symbols.back().setArrayColm(storage[index - 1].getValue());
				d2 = atoi(expre.top().c_str());
				expre.pop();
				isRBRACK();
			}
			if (d2 != 0) {
				codeptr_add(mmm, d1 * d2 - 1);
			}
			else {
				codeptr_add(mmm, d1 - 1);
			}
		}
		if (storage[index].isType("ASSIGN")) {
			//cout << "return " << endl;
			if (flag == 0) { break; }
			else { return -1; }
		}
		if (storage[index].isType("COMMA")) {
			index++;
			flag = 1; continue;
		}

		while (start < index) {
			fout << storage[start].toString() << endl;
			start++;
		}
		fout << "<变量定义无初始化>" << endl;

		return 0;
	}
	if (type == 0) {
		index++;
		//cout << storage[index].getValue() << "  cjdsicewed!!!! scdwecdeq" << endl;
		ConstValue(mmm);
		addMidCode("assign", symbols.back().getValue(), expre.top(), "");
		expre.pop();
	}
	else if (type == 1) {
		index++;
		if (storage[index].isType("LBRACE")) {
			index++;
		}
		else { return -1; }
		ConstValue(mmm);
		d1--;
		int i = 0;
		addMidCode("stoa", symbols.back().getValue(), to_string((i++) * mmm), expre.top());
		expre.pop();
		while (true) {
			if (storage[index].isType("COMMA")) {
				index++;
			}
			else {
				break;
			}
			ConstValue(mmm);
			addMidCode("stoa", symbols.back().getValue(), to_string((i++) * mmm), expre.top());
			expre.pop();
			d1--;
		}
		if (d1 != 0) {
			errorOut("n", storage[index].getLine(), storage[index].getColm());
		}
		if (storage[index].isType("RBRACE")) {
			index++;
		}
		else { return -1; }
	}
	else if (type == 2) {
		int i = 0; int j = 0;
		int k = 0;
		bool error = false;
		index++;
		if (storage[index].isType("LBRACE")) {
			index++;
		}
		else {
			return -1;
		}
		if (storage[index].isType("LBRACE")) {
			index++;
		}
		else { return -1; }
		ConstValue(mmm);
		addMidCode("stoa", symbols.back().getValue(), to_string((k++) * mmm), expre.top());
		expre.pop();
		j++;
		while (true) {
			if (storage[index].isType("COMMA")) {
				index++;
			}
			else { break; }
			ConstValue(mmm);
			addMidCode("stoa", symbols.back().getValue(), to_string((k++) * mmm), expre.top());
			expre.pop();
			j++;
		}
		if (j != d2) {
			error = true;
		}
		j = 0;
		if (storage[index].isType("RBRACE")) {
			index++;
		}
		else { return -1; }
		i++;
		while (true) {
			if (storage[index].isType("COMMA")) {
				index++;
			}
			else {
				break;
			}
			if (storage[index].isType("LBRACE")) {
				index++;
			}
			else { return -1; }
			ConstValue(mmm);
			addMidCode("stoa", symbols.back().getValue(), to_string((k++) * mmm), expre.top());
			expre.pop();
			j++;
			while (true) {
				if (storage[index].isType("COMMA")) {
					index++;
				}
				else { break; }
				ConstValue(mmm);
				addMidCode("stoa", symbols.back().getValue(), to_string((k++) * mmm), expre.top());
				expre.pop();
				j++;
			}
			if (j != d2) {
				error = true;
			}
			j = 0;
			if (storage[index].isType("RBRACE")) {
				index++;
			}
			else { return -1; }
			i++;
		}
		if (i != d1 || error) {
			errorOut("n", storage[index].getLine(), storage[index].getColm());
		}
		if (storage[index].isType("RBRACE")) {
			index++;
		}
		else { return -1; }
	}
	else {
		return -1;
	}

	while (start < index) {
		fout << storage[start].toString() << endl;
		start++;
	}
	fout << "<变量定义及初始化>" << endl;

	return 0;
}  //一会需要仔细看
int VariableDefine() {
	VariableInit();

	while (start < index) {
		fout << storage[start].toString() << endl;
		start++;
	}
	fout << "<变量定义>" << endl;

	return 0;
}
int VariableDescript() {
	//保证了进来的不会是函数说明，但是我不知道还会不会有别的
	//句柄是 char int
	//程序保证了至少进来一个
	int num = 0;
	while (true) {
		if ((storage[index].isType("INTTK") || storage[index].isType("CHARTK")) &&
			!storage[index + 2].isType("LPARENT")) {
			VariableDefine();
		}
		else { break; }
		num++;
		//cout << storage[index].getValue() << endl;
		isSEMICN();
	}
	if (num > 0) {

		while (start < index) {
			fout << storage[start].toString() << endl;
			start++;
		}
		fout << "<变量说明>" << endl;

		return 0;
	}
	else { return -1; }
}
int HeadDeclaration() {
	isReturn = true;
	if (storage[index].isType("INTTK")) {
		reType = INTEVALUE;
		index++;
	}
	else if (storage[index].isType("CHARTK")) {
		reType = CHARVALUE;
		index++;
	}
	else { return -1; }
	if (storage[index].isType("IDENFR")) {
		//cout << "nishirong" << endl;
		if (hasSymbols(storage[index]) != -1) {
			errorOut("b", storage[index].getLine(),
				storage[index].getColm());
			index++;
			return -2;
		}
		else {
			storage[index].setReturnType(reType);
			storage[index].setFunction(RETURNFUNCIFER);
			symbols.push_back(storage[index]);
			symbolstable.push(symbols.size());
		}
		index++;

		while (start < index) {
			fout << storage[start].toString() << endl;
			start++;
		}
		fout << "<声明头部>" << endl;

		return 0;
	}
	else { return -1; }
}
int Parameters(bool flag) {
	//cout << "Parameters excute here " << endl;
	int num = 0;
	int name = symbols.size() - 1;
	while (storage[index].isType("INTTK") || storage[index].isType("CHARTK")) {

		if (flag) {  //标记函数名称没有重名
			if (storage[index].isType("INTTK")) {
				symbols[name].addParameters(INTEVALUE);
			}
			else {
				symbols[name].addParameters(CHARVALUE);
			}
		}
		index++;
		if (storage[index].isType("IDENFR")) {
			int search = hasSymbols(storage[index]);
			if (search >= symbolstable.top()) {
				errorOut("b", storage[index].getLine(),
					storage[index].getColm());
			}
			else {
				//cout << storage[index].getValue() << endl;
				symbols.push_back(storage[index]);
			}
			index++;
			num++;
		}
		else { return -1; }
		if (storage[index].isType("COMMA")) { index++; }
		else { break; }
	}

	while (start < index) {
		fout << storage[start].toString() << endl;
		start++;
	}
	fout << "<参数表>" << endl;

	return num;
}
int StatementColumn();
int Statement();
int Expression();
int ReturnCallStatement();
int Condition() {
	bool inValid = true;
	if (Expression() == CHARVALUE) {
		inValid = false;
		errorOut("f", storage[index].getLine(), storage[index].getColm());
	}
	string cont = "";
	if (storage[index].isType("LSS") ||
		storage[index].isType("LEQ") ||
		storage[index].isType("EQL") ||
		storage[index].isType("NEQ") ||
		storage[index].isType("GRE") ||
		storage[index].isType("GEQ")) {
		cont = storage[index].isType("LSS") ? "bge" :
			storage[index].isType("LEQ") ? "bgt" :
			storage[index].isType("EQL") ? "bne" :
			storage[index].isType("NEQ") ? "beq" :
			storage[index].isType("GRE") ? "ble" : "blt";
		index++;
	}
	else { return -1; }
	if (Expression() == CHARVALUE && inValid == true) {
		//inValid = false;
		errorOut("f", storage[index].getLine(), storage[index].getColm());
	}
	expre.push(cont);
	while (start < index) {
		fout << storage[start].toString() << endl;
		start++;
	}
	fout << "<条件>" << endl;

	return 0;
}
int factor() {

	int returnValue;
	if (storage[index].isType("IDENFR")) {

		int search = hasSymbols(storage[index]);
		if (search == -1) {
			errorOut("c", storage[index].getLine(),
				storage[index].getColm());
			index++;
			return -1;
		}
		else {
			returnValue = symbols[search].getReturnType();
			if (symbols[search].getFunction() == RETURNFUNCIFER) {
				//cout << "nishirong wudi" << endl;
				ReturnCallStatement();
			}
			else {
				index++;
				string dimention1 = "";
				string dimention2 = "";
				string dst = "";
				if (storage[index].isType("LBRACK")) {
					index++;
					if (Expression() != INTEVALUE) {
						errorOut("i", storage[index].getLine(),
							storage[index].getColm());
					}
					dimention1 = expre.top();
					expre.pop();
					isRBRACK();
					if (storage[index].isType("LBRACK")) {
						index++;
						if (Expression() != INTEVALUE) {
							errorOut("i", storage[index].getLine(),
								storage[index].getColm());
						}
						dimention2 = expre.top();
						expre.pop();
						isRBRACK();
					}
				}
				if (dimention1 != "") {
					dst = getArray(search, dimention1, dimention2);
					push_expre(dst);
				}
				else {
					push_expre(symbols[search].getValue());
				}
			}
			//return returnValue;
		}
	}
	else if (storage[index].isType("CHARCON")) {
		string dst = storage[index].getValue();
		index++;
		returnValue = CHARVALUE;
		push_expre("\'" + dst + "\'");
		//return CHARVALUE;
	}
	else if (storage[index].isType("VCHARCON")) {
		errorOut("a", storage[index].getLine(), storage[index].getColm());
		index++;
		returnValue = CHARVALUE;
	}
	else if (storage[index].isType("LPARENT")) {
		index++;
		Expression();
		isRPARENT();
		returnValue = INTEVALUE;
	}
	else {
		returnValue = Intergers();
	}

	while (start < index) {
		fout << storage[start].toString() << endl;
		start++;
	}
	fout << "<因子>" << endl;

	return returnValue;
}
int Term() {
	bool multi = false;
	bool hasError = false;
	int z = factor();
	bool isMULT = true;
	if (z == -1) {
		hasError = true;
	}
	//忘了为什么要做零的判断了
	while (storage[index].isType("MULT") ||
		storage[index].isType("DIV")) {
		if (storage[index].isType("MULT")) {
			isMULT = true;
		}
		else {
			isMULT = false;
		}
		multi = true;
		index++;
		if (factor() == -1) {
			hasError = true;
		}//不知道要不要考虑回溯，不管了，俺先给写了吧；
		if (isMULT) {
			push_expre("*");
		}
		else {
			push_expre("/");
		}
	}

	while (start < index) {
		fout << storage[start].toString() << endl;
		start++;
	}
	fout << "<项>" << endl;

	if (hasError) {
		return -1;
	}
	if (!multi && z == CHARVALUE) {
		return CHARVALUE;
	}
	else {
		return INTEVALUE;
	}
}
int Expression() {
	bool isChar = true;
	bool hasError = false;
	bool isPlus = true;
	if (storage[index].isType("PLUS") ||
		storage[index].isType("MINU")) {
		if (storage[index].isType("MINU")) {
			push_expre("0");
			isPlus = false;
		}
		else {
			isPlus = true;
		}
		isChar = false;
		index++;
	}

	int z = Term();

	if (z == -1) {
		hasError = true;
	}
	if (!isPlus) {
		push_expre("-");
	}
	while (storage[index].isType("PLUS") ||
		storage[index].isType("MINU")) {
		isChar = false;
		if (storage[index].isType("PLUS")) {
			isPlus = true;
		}
		else { isPlus = false; }
		index++;
		if (Term() == -1) {
			hasError = true;
		};
		if (isPlus) { push_expre("+"); }
		else { push_expre("-"); }
	}
	if (hasError) {
		return -1;
	}
	if (isChar && z == CHARVALUE) {
		return CHARVALUE;
	}

	while (start < index) {
		fout << storage[start].toString() << endl;
		start++;
	}
	fout << "<表达式>" << endl;

	return INTEVALUE;
}
int Step() {
	UnSignIntergers();

	while (start < index) {
		fout << storage[start].toString() << endl;
		start++;
	}
	fout << "<步长>" << endl;

	return  0;
}
int LoopStatement() {
	string lab1 = "lable_" + to_string(midLableNum++),
		lab2 = "lable_" + to_string(midLableNum++);
	int result = 0;
	if (storage[index].isType("WHILETK")) {
		index++;
		if (storage[index].isType("LPARENT")) {
			index++;
		}
		else { return -1; }
		Condition();
		string func = expre.top();
		expre.pop();
		string op2 = expre.top();
		expre.pop();
		string op1 = expre.top();
		expre.pop();
		addMidCode("set", lab1, "", "");
		addMidCode(func, op1, op2, lab2);
		isRPARENT();
		Statement();
		addMidCode("goto", lab1, "", "");
		addMidCode("set", lab2, "", "");
	}
	else if (storage[index].isType("FORTK")) {
		index++;
		if (storage[index].isType("LPARENT")) {
			index++;
		}
		else { return -1; }
		int search = -1;
		if (storage[index].isType("IDENFR")) {
			search = hasSymbols(storage[index]);
			if (search == -1) {
				errorOut("c", storage[index].getLine(),
					storage[index].getColm());
			}
			else if (symbols[search].getFunction() == CONSTIDENTIFER) {
				errorOut("j", storage[index].getLine(),
					storage[index].getColm());
			}
			index++;
		}
		else { return -1; }
		if (storage[index].isType("ASSIGN")) { index++; }
		else { return -1; }
		Expression();
		string op2 = expre.top();
		string loop = "lable_" + to_string(midLableNum++);
		string endLoop = "lable_" + to_string(midLableNum++);
		expre.pop();
		addMidCode("assign", symbols[search].getValue(), op2, "");
		isSEMICN();
		addMidCode("set", loop, "", "");
		Condition();
		string func = expre.top();
		expre.pop();
		op2 = expre.top();
		expre.pop();
		string op1 = expre.top();
		expre.pop();
		addMidCode(func, op1, op2, endLoop);
		isSEMICN();
		if (storage[index].isType("IDENFR")) {
			search = hasSymbols(storage[index]);
			if (search == -1) {
				errorOut("c", storage[index].getLine(),
					storage[index].getColm());
			}
			else if (symbols[search].getFunction() == CONSTIDENTIFER) {
				errorOut("j", storage[index].getLine(),
					storage[index].getColm());
			}
			index++;
		}
		else { return -1; }
		if (storage[index].isType("ASSIGN")) { index++; }
		else { return -1; }
		if (storage[index].isType("IDENFR")) {
			if (hasSymbols(storage[index]) == -1) {
				errorOut("c", storage[index].getLine(),
					storage[index].getColm());
			}
			push_expre(storage[index].getValue());
			index++;
		}
		else { return -1; }
		bool isPlus = true;
		if (storage[index].isType("PLUS") ||
			storage[index].isType("MINU")) {
			isPlus = (storage[index].isType("PLUS"));
			index++;
		}
		else { return -1; }
		Step();
		isRPARENT();
		result = Statement();
		if (isPlus) {
			push_expre("+");
		}
		else { push_expre("-"); }
		addMidCode("assign", symbols[search].getValue(), expre.top(), "");
		expre.pop();
		addMidCode("goto", loop, "", "");
		addMidCode("set", endLoop, "", "");
	}
	else { return -1; }

	while (start < index) {
		fout << storage[start].toString() << endl;
		start++;
	}
	fout << "<循环语句>" << endl;

	return result;
}
int ConditionStatement() {
	string lableA = "lable_" + to_string(midLableNum++),
		lableB = "lable_";
	if (storage[index].isType("IFTK")) {
		index++;
	}
	else { return -1; }
	if (storage[index].isType("LPARENT")) {
		index++;
	}
	else { return -1; }
	Condition();
	string func = expre.top();
	expre.pop();
	string op2 = expre.top();
	expre.pop();
	string op1 = expre.top();
	expre.pop();
	addMidCode(func, op1, op2, lableA);
	isRPARENT();
	bool hasElse = false;
	Statement();
	if (storage[index].isType("ELSETK")) {
		lableB = lableB + to_string(midLableNum++);
		addMidCode("goto", lableB, "", "");
		addMidCode("set", lableA, "", "");
		index++;
		hasElse = true;
		Statement();
		addMidCode("set", lableB, "", "");
	}
	else {
		addMidCode("set", lableA, "", "");
	}
	while (start < index) {
		fout << storage[start].toString() << endl;
		start++;
	}
	fout << "<条件语句>" << endl;

	return 0;
}
int ValueTable(int search) {
	int num = 0;

	if (storage[index].isType("RPARENT") || storage[index].isType("SEMICN")) {
		;
	}
	else {
		int re = Expression();
		bool hasError = false;
		if (re == -1) hasError = true;
		else if (re == 0) {
			return 0;
		}
		num++;
		if (!hasError && re != symbols[search].getParameters(num)) {
			//symbols[search]
			unMatch = true;
		}
		while (storage[index].isType("COMMA")) {
			index++;
			re = Expression();
			if (re == -1) hasError = true;
			num++;
			if (!hasError && re != symbols[search].getParameters(num)) {
				unMatch = true;
			}
		}
	}

	while (start < index) {
		fout << storage[start].toString() << endl;
		start++;
	}
	fout << "<值参数表>" << endl;


	return num;
}
int ReturnCallStatement() {
	//保证进来都是可以返回的
	int numS = 0;
	bool nofindname = false;
	int search = 0;
	cout << "you fan hui zhi han shu lie biao1   " << numS << endl;
	if (storage[index].isType("IDENFR")) {
		search = hasSymbols(storage[index]);
		if (search == -1) {
			nofindname = true;
			errorOut("c", storage[index].getLine(),
				storage[index].getColm());
			return -1;
		}
		else {
			numS = symbols[search].getParaNum();
		}
		index++;
	}
	else { return -1; }
	if (storage[index].isType("LPARENT")) {
		index++;
	}
	else { return -1; }
	//cout << "nishirong dd" << endl;
	int numV = ValueTable(search);
	/// <summary>
	///
	/// </summary>
	/// <returns></returns>

	if (numS != numV && !nofindname) {
		errorOut("d", storage[index].getLine(), storage[index].getColm());
	}
	else if (unMatch) {
		errorOut("e", storage[index].getLine(), storage[index].getColm());
	}
	unMatch = false;
	isRPARENT();

	while (start < index) {
		fout << storage[start].toString() << endl;
		start++;
	}
	fout << "<有返回值函数调用语句>" << endl;

	return 0;
}
int UnReturnCallStatement() {
	//保证进来都是不可以返回的
	//cout << "nishinishi" << endl;
	int numS = 0, numV = 0;
	int search;
	bool nofindname = false;
	if (storage[index].isType("IDENFR")) {
		search = hasSymbols(storage[index]);
		if (search == -1) {
			nofindname = true;
			errorOut("c", storage[index].getLine(),
				storage[index].getColm());
		}
		else {
			numS = symbols[search].getParaNum();
		}
		index++;
	}
	else { return -1; }
	if (storage[index].isType("LPARENT")) {
		index++;
	}
	else { return -1; }
	numV = ValueTable(search);
	if (numS != numV && !nofindname) {
		errorOut("d", storage[index].getLine(), storage[index].getColm());
	}
	else if (unMatch) {
		errorOut("e", storage[index].getLine(), storage[index].getColm());
	}
	unMatch = false;
	isRPARENT();

	while (start < index) {
		fout << storage[start].toString() << endl;
		start++;
	}
	fout << "<无返回值函数调用语句>" << endl;

	return 0;
}
int AssignStatement() {
	int search = -1;
	string dimention1 = "", dimention2 = "";
	if (storage[index].isType("IDENFR")) {
		search = hasSymbols(storage[index]);
		if (search == -1) {
			errorOut("c", storage[index].getLine(),
				storage[index].getColm());
		}
		else if (symbols[search].getFunction() == CONSTIDENTIFER) {
			errorOut("j", storage[index].getLine(),
				storage[index].getColm());
		}
		index++;
	}
	else { return -1; }
	if (storage[index].isType("LBRACK")) {
		index++;
		if (Expression() != INTEVALUE) {
			errorOut("i", storage[index].getLine(),
				storage[index].getColm());
		}
		dimention1 = expre.top();
		expre.pop();
		isRBRACK();
		if (storage[index].isType("LBRACK")) {
			index++;
			if (Expression() != INTEVALUE) {
				errorOut("i", storage[index].getLine(),
					storage[index].getColm());
			}
			dimention2 = expre.top();
			expre.pop();
			isRBRACK();
		}
	}
	if (storage[index].isType("ASSIGN")) {
		index++;
	}
	else { return -1; }
	Expression();
	string assignValue = expre.top();
	expre.pop();
	if (dimention1 == "" && dimention2 == "") {
		addMidCode("assign", symbols[search].getValue(), assignValue, "");
	}
	else {
		stoArray(search, dimention1, dimention2, assignValue);
	}
	while (start < index) {
		fout << storage[start].toString() << endl;
		start++;
	}
	fout << "<赋值语句>" << endl;

	return 0;
}
int ScanfStatement() {
	if (storage[index].isType("SCANFTK")) {
		index++;
	}
	else { return -1; }
	if (storage[index].isType("LPARENT")) {
		index++;
	}
	else { return -1; }
	if (storage[index].isType("IDENFR")) {
		int search = hasSymbols(storage[index]);
		if (search == -1) {
			errorOut("c", storage[index].getLine(),
				storage[index].getColm());
		}
		else if (symbols[search].getFunction() == CONSTIDENTIFER) {
			errorOut("j", storage[index].getLine(),
				storage[index].getColm());
		}
		if (symbols[search].getReturnType() == INTEVALUE) {
			addMidCode("scanf", symbols[search].getValue(), "", "");
		}
		else {
			addMidCode("scanc", symbols[search].getValue(), "", "");
		}
		index++;
	}
	else { return -1; }
	isRPARENT();

	while (start < index) {
		fout << storage[start].toString() << endl;
		start++;
	}
	fout << "<读语句>" << endl;

	return 0;
}
int PrintfStatement() {
	if (storage[index].isType("PRINTFTK")) {
		index++;
	}
	else { return -1; }
	if (storage[index].isType("LPARENT")) {
		index++;
	}
	else { return -1; }
	//字符串也要输出那就输出吧
	if (storage[index].isType("STRCON")) {
		strSymbols.push_back(storage[index].getValue());
		strSpace = strSpace + storage[index].getValue().size() + 1;
		index++;
		addMidCode("prints", "str" + to_string(strSymbols.size() - 1), "", "");
		while (start < index) {
			fout << storage[start].toString() << endl;
			start++;
		}
		fout << "<字符串>" << endl;

		if (storage[index].isType("COMMA")) {
			index++;
			int returnValue = Expression();
			string op = expre.top();
			expre.pop();
			cout << "第二个输出语句" << endl;
			if (returnValue == INTEVALUE) {
				addMidCode("printf", op, "", "");
			}
			else {
				addMidCode("printc", op, "", "");
			}
		}
		else {
			strSymbols[strSymbols.size() - 1] = strSymbols[strSymbols.size() - 1] + "\\n";
			strSpace++;
		}
	}
	else if (storage[index].isType("VSTRCON")) {
		errorOut("a", storage[index].getLine(), storage[index].getColm());
		index++;

		while (start < index) {
			fout << storage[start].toString() << endl;
			start++;
		}
		fout << "<字符串>" << endl;

		if (storage[index].isType("COMMA")) {
			index++;
			Expression();
		}
	}
	else {
		int returnValue = Expression();
		string op = expre.top();
		expre.pop();
		if (returnValue == INTEVALUE) {
			addMidCode("printf", op, "", "");
		}
		else {
			addMidCode("printc", op, "", "");
		}
	}
	isRPARENT();

	while (start < index) {
		fout << storage[start].toString() << endl;
		start++;
	}
	fout << "<写语句>" << endl;

	return 0;
}
int SituationSentence(int expect, string lab0) {
	if (storage[index].isType("CASETK")) {
		index++;
	}
	else { return -1; }
	ConstValue(expect);
	string op2 = expre.top();
	expre.pop();
	string op1 = expre.top();
	addMidCode("bne", op1, op2, "lable_" + to_string(midLableNum));
	if (storage[index].isType("COLON")) {
		index++;
	}
	else { return -1; }
	int result = Statement();
	addMidCode("goto", lab0, "", "");
	while (start < index) {
		fout << storage[start].toString() << endl;
		start++;
	}
	fout << "<情况子语句>" << endl;
	return result;
	//return 0;
}
int SituationTable(int expect, string lab0) {

	bool isreturn = true;
	if (SituationSentence(expect, lab0) != STATERETURN)
		isreturn = false;
	addMidCode("set", "lable_" + to_string(midLableNum++), "", "");
	while (storage[index].isType("CASETK")) {
		if (SituationSentence(expect, lab0) != STATERETURN)
			isreturn = false;
		addMidCode("set", "lable_" + to_string(midLableNum++), "", "");
	}

	while (start < index) {
		fout << storage[start].toString() << endl;
		start++;
	}
	fout << "<情况表>" << endl;

	if (isreturn) {
		return  STATERETURN;
	}
	return 0;
}
int DefaultDlare() {
	if (storage[index].isType("DEFAULTTK")) {
		index++;
	}
	else {
		errorOut("p", storage[index].getLine(), storage[index].getColm());
	}
	if (storage[index].isType("COLON")) {
		index++;
	}
	else { return -1; }
	return Statement();

	while (start < index) {
		fout << storage[start].toString() << endl;
		start++;
	}
	fout << "<缺省>" << endl;

}
int SwitchStatement() {
	if (storage[index].isType("SWITCHTK")) {
		index++;
	}
	else { return -1; }
	if (storage[index].isType("LPARENT")) {
		index++;
	}
	else { return -1; }
	int z = Expression();
	isRPARENT();
	if (storage[index].isType("LBRACE")) {
		index++;
	}
	else { return -1; }
	int isreturn1 = 0;
	int isreturn2 = 0;
	string lab0 = "lable_" + to_string(midLableNum++);
	if (z == CHARVALUE) {
		isreturn1 = SituationTable(CHARVALUE, lab0);
	}
	else {
		isreturn1 = SituationTable(INTEVALUE, lab0);
	}
	isreturn2 = DefaultDlare();
	expre.pop();//pop case of (expre)
	addMidCode("set", lab0, "", "");
	if (storage[index].isType("RBRACE")) {
		index++;
	}
	else { return -1; }

	while (start < index) {
		fout << storage[start].toString() << endl;
		start++;
	}
	fout << "<情况语句>" << endl;

	return 0;
}
int ReturnStatement() {
	int re = 0;
	if (storage[index].isType("RETURNTK")) {
		index++;
	}
	else { return -1; }
	if (storage[index].isType("LPARENT")) {
		index++;
		re = Expression();
		if (!isReturn) {
			errorOut("g", storage[index].getLine(), storage[index].getColm());
		}
		else if (re != reType) {
			errorOut("h", storage[index].getLine(), storage[index].getColm());
			hasReturn = true;
		}
		else {
			hasReturn = true;
		}
		isRPARENT();
	}
	else {
		if (isReturn) {
			errorOut("h", storage[index].getLine(), storage[index].getColm());
			hasReturn = true;
		}
		return 0;
	}

	while (start < index) {
		fout << storage[start].toString() << endl;
		start++;
	}
	fout << "<返回语句>" << endl;

	return re;
}
int Statement() {
	//1--表示该语句的句柄不是语句
	//int re = 0;
	if (storage[index].isType("WHILETK") ||
		storage[index].isType("FORTK")) {
		return LoopStatement();
	}
	else if (storage[index].isType("IFTK")) {
		return ConditionStatement();
	}
	else if (storage[index].isType("IDENFR")) {
		int search = hasSymbols(storage[index]);
		if (search == -1) {
			errorOut("c", storage[index].getLine(),
				storage[index].getColm());
			//先这么处理吧！
			while (!storage[index].isType("SEMICN")) {
				index++;
			}
		}
		else {
			//cout << "ni shi rong " << storage[index].toString()<< endl;
			if (symbols[search].getFunction() == RETURNFUNCIFER) {
				ReturnCallStatement();
			}
			else if (symbols[search].getFunction() == UNRETUFUNCIFER) {
				UnReturnCallStatement();
			}
			else {
				AssignStatement();
			}
		}
		isSEMICN();
	}
	else if (storage[index].isType("SCANFTK")) {
		ScanfStatement();
		isSEMICN();
	}
	else if (storage[index].isType("PRINTFTK")) {
		PrintfStatement();
		isSEMICN();
	}
	else if (storage[index].isType("SWITCHTK")) {
		return SwitchStatement();
	}
	else if (storage[index].isType("RETURNTK")) {
		ReturnStatement();
		isSEMICN();
		//cout << "nnnsss" << endl;
	}
	else if (storage[index].isType("SEMICN")) {
		index++;
	}
	else if (storage[index].isType("LBRACE")) {
		index++;
		int x = StatementColumn();
		if (storage[index].isType("RBRACE")) { index++; }
		else { return -1; }
		return x;
	}
	else { return 1; }

	while (start < index) {
		fout << storage[start].toString() << endl;
		start++;
	}
	fout << "<语句>" << endl;

	return 0;
}
int StatementColumn() {
	//cout << "StatementColumn here" << endl;
	int re = 0;
	while (true) {
		re = Statement();
		if (re == 1) {
			break;
		}
	}

	while (start < index) {
		fout << storage[start].toString() << endl;
		start++;
	}
	fout << "<语句列>" << endl;

	return 0;
}
int CompoundStatement() {
	//cout<<"CompoundStatement "<<endl;

	if (storage[index].isType("CONSTTK")) {
		//cout << "CompoundStatement1 " << endl;
		ConstantDescript();
	}
	if ((storage[index].isType("INTTK") || storage[index].isType("CHARTK")) &&
		!storage[index + 2].isType("LPARENT")) {
		//cout << "CompoundStatement2 " << endl;
		VariableDescript();
	}
	int result = StatementColumn();
	while (start < index) {
		fout << storage[start].toString() << endl;
		start++;
	}
	fout << "<复合语句>" << endl;
	return result;
}
int ReturnFucDescript() {
	int head = HeadDeclaration();
	//int name = index - 1;
	//cout << "dead" << endl;
	int name = symbols.size() - 1;
	int x = symbols[name].getReturnType();
	/*
	cout << "有返回函数的名字  " << symbols[name].getValue() << endl;
	cout << storage[index-3].getValue() <<" ";
	cout << storage[index-2].getValue() <<" ";
	cout << storage[index - 1].getValue() <<" ";
	cout << storage[index].getValue()<<" ";
	cout << storage[index+1].getValue() << endl;
	*/
	if (storage[index].isType("LPARENT")) { index++; }
	else { return -1; }
	if (head == -2) {
		Parameters(false);
	}
	else { Parameters(true); }
	isRPARENT();
	if (storage[index].isType("LBRACE")) { index++; }
	else { return -1; }
	int re = CompoundStatement();
	if (!hasReturn) {
		errorOut("h", storage[index].getLine(), storage[index].getColm());
	}
	if (storage[index].isType("RBRACE")) { index++; }
	else { return -1; }
	isReturn = false;
	hasReturn = false;
	int xx = symbolstable.top();
	symbolstable.pop();
	auto it = symbols.begin();
	symbols.erase(it + xx, it + symbols.size());

	while (start < index) {
		fout << storage[start].toString() << endl;
		start++;
	}
	fout << "<有返回值函数定义>" << endl;

	return 0;
}
int UnReturnFucDescript() {
	//int name;
	//int num;
	if (storage[index].isType("VOIDTK")) {
		index++;
	}
	else { return -1; }
	int name = -1;
	bool hasSameName = false;
	if (storage[index].isType("IDENFR")) {
		//name = index;
		if (hasSymbols(storage[index]) != -1) {
			hasSameName = true;
			errorOut("b", storage[index].getLine(), storage[index].getColm());
			//fout << "b " << storage[index].getLine() << "\n";
		}
		else {
			storage[index].setFunction(UNRETUFUNCIFER);
			symbols.push_back(storage[index]);
			name = symbols.size() - 1;
			symbolstable.push(symbols.size());
		}
		index++;
	}
	else { return -1; }
	if (storage[index].isType("LPARENT")) { index++; }
	else { return -1; }
	Parameters(!hasSameName);
	isRPARENT();
	if (storage[index].isType("LBRACE")) { index++; }
	else { return -1; }
	CompoundStatement();
	if (storage[index].isType("RBRACE")) { index++; }
	else { return -1; }
	int x = symbolstable.top();
	symbolstable.pop();
	auto it = symbols.begin();
	symbols.erase(it + x, it + symbols.size());

	while (start < index) {
		fout << storage[start].toString() << endl;
		start++;
	}
	fout << "<无返回值函数定义>" << endl;

	return 0;
}
int MainFuncDescript() {
	//cout << "MainFunction define " << endl;
	//cout << storage[index].getLine() << endl;
	//cout << storage[index].toString() << " ";

	if (storage[index].isType("VOIDTK")) {
		index++;
	}
	else { return -1; }
	symbolstable.push(symbols.size());
	if (storage[index].isType("MAINTK")) {
		index++;
	}
	else { return -1; }
	if (storage[index].isType("LPARENT")) { index++; }
	else { return -1; }
	isRPARENT();
	if (storage[index].isType("LBRACE")) { index++; }
	else { return -1; }
	CompoundStatement();
	if (storage[index].isType("RBRACE")) { index++; }
	else { return -1; }
	while (start < index) {
		fout << storage[start].toString() << endl;
		start++;
	}
	fout << "<主函数>" << endl;
	symbolstable.pop();
	return 0;
}
int Program() {
	//int now = start;
	symbolstable.push(0);
	if (storage[index].isType("CONSTTK")) {
		ConstantDescript();
	}
	if ((storage[index].isType("INTTK") || storage[index].isType("CHARTK")) &&
		!storage[index + 2].isType("LPARENT")) {
		VariableDescript();
	}

	while (storage[index].isType("INTTK") || storage[index].isType("INTTK") ||
		!(storage[index].isType("VOIDTK") && storage[index + 1].isType("MAINTK"))) {
		if (storage[index].isType("VOIDTK")) {
			UnReturnFucDescript();
		}
		else {
			ReturnFucDescript();
		}
	}
	MainFuncDescript();
	while (start < index) {
		fout << storage[start].toString() << endl;
		start++;
	}
	fout << "<程序>" << endl;
	return 0;
}
int main() {
	fin.open("testfile.txt");
	fout.open("output.txt");
	ferr.open("error.txt");
	mid.open("18231078_倪施榕_优化前中间代码.txt");
	mid2.open("18231078_倪施榕_优化后中间代码.txt");
	mips0.open("mips.txt");
	midDstNum = 0;
	midLableNum = 0;
	fin.get(nextLetter);
	//cout << nextLetter << "test 换行1" << endl;
	getsym();
	//cout << sym << "test 换行2" << endl;
	line = 1;
	colm = 1;
	while (nextLetter != '\0') {
		if (is_letter(sym)) {
			alphabet();
		}
		else if (sym == '+') {
			buf.push_back('+');
			addStorage("PLUS");
			getsym();
		}
		else if (sym == '-') {
			buf.push_back('-');
			addStorage("MINU");
			getsym();
		}
		else if (sym == '\'') {
			letter_identify();
		}
		else if (sym == '\"') {
			string_identify();
		}
		else if (sym == '<') {
			if (nextLetter != '\0' && nextLetter == '=') {
				getsym();
				buf.push_back('<');
				buf.push_back('=');
				addStorage("LEQ");
				getsym();
			}
			else {
				buf.push_back('<');
				addStorage("LSS");
				getsym();
			}
		}
		else if (sym == '>') {
			if (nextLetter != '\0' && nextLetter == '=') {
				getsym();
				buf.push_back('>');
				buf.push_back('=');
				addStorage("GEQ");
				getsym();
			}
			else {
				buf.push_back('>');
				addStorage("GRE");
				getsym();
			}
		}
		else if (sym == '=') {
			if (nextLetter != '\0' && nextLetter == '=') {
				getsym();
				buf.push_back('=');
				buf.push_back('=');
				addStorage("EQL");
				getsym();
			}
			else {
				buf.push_back('=');
				addStorage("ASSIGN");
				getsym();
			}
		}
		else if (sym == '!') {
			if (nextLetter != '\0' && nextLetter == '=') {
				getsym();
				buf.push_back('!');
				buf.push_back('=');
				addStorage("NEQ");
				getsym();
			}
		}
		else if (is_num(sym)) {

			alnum_identify();
		}
		else if (sym == '\n') {
			line++;
			colm = 1;
			getsym();
		}
		else {
			switch (sym) {
			case '*':
				buf.push_back('*');
				addStorage("MULT");
				break;
			case '/':
				buf.push_back('/');
				addStorage("DIV");
				break;
			case ':':
				buf.push_back(':');
				addStorage("COLON");
				break;
			case ';':
				buf.push_back(';');
				addStorage("SEMICN");
				break;
			case ',':
				buf.push_back(',');
				addStorage("COMMA");
				break;
			case '(':
				buf.push_back('(');
				addStorage("LPARENT");
				break;
			case ')':
				buf.push_back(')');
				addStorage("RPARENT");
				break;
			case '[':
				buf.push_back('[');
				addStorage("LBRACK");
				break;
			case ']':
				buf.push_back(']');
				addStorage("RBRACK");
				break;
			case '{':
				buf.push_back('{');
				addStorage("LBRACE");
				break;
			case '}':
				buf.push_back('}');
				addStorage("RBRACE");
				break;
			default:
				//先不管了单放一个进去，裂开
				if (!is_blank(sym)) {
					buf.push_back(sym);
					addErrorWord();
				}
				break;
			}
			getsym();
		}
	}
	index = 0; start = 0;
	Program();
	MidCMIPS();
}