#include<string>
#include<vector>
#include<map>
#include"SymbolTable.h"

using namespace std;

class Function
{
private:
	string funcName;
	int type;
	vector<string> paras;
	vector<string> defs;
	map<string, int> paraMap; // 参数的栈偏移
	map<string, int> defMap; // 定义的量的栈偏移
	map<string, int> paraType; // 参数的类型
	map<string, int> defType; // 定义的量的类型
	int raOff;
	int paraNum = 0;
	int defNum = 0;
	int totalNum = 0;

public:
	Function();
	Function(string funcName, int type);
	~Function();

	void insertPara(string paraName, int type);
	void insertDef(string defName, int size, int type);
	void insertDef(string defName, int type);
	void insertRa();

	inline bool hasPara(string paraName) { return paraMap.find(paraName) != paraMap.end(); }
	inline bool hasDef(string defName) { return defMap.find(defName) != defMap.end(); }
	inline bool hasCheck(string name) { return hasPara(name) || hasDef(name); }

	inline int getRaOff() { return raOff; }
	int getOffset(string name);

	inline int getParaSize() { return paraNum * 4; }
	inline int getDefSize() { return defNum * 4; }
	inline int getTotalSize() { return totalNum * 4; }

	inline string getParaName(int paraId) { return paras[paraId]; }

	inline int getParaNum() { return paraNum; }
	inline int getDefNum() { return defNum; }
	inline int getTotalNum() { return totalNum; }

	int getType(string name);

	inline string getFuncName() { return funcName; }
};