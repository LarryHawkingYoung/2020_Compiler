#include "Function.h"
#include<string>
#include<iostream>

using namespace std;

Function::Function()
{
}

Function::Function(string funcName, int type)
{
	Function::funcName = funcName;
	Function::type = type;
}

Function::~Function()
{
}

void Function::insertPara(string paraName, int type)
{
	paraType[paraName] = type;
	paraMap[paraName] = totalNum * 4;
	paras.push_back(paraName);
	totalNum++;
	paraNum++;
}

void Function::insertDef(string defName, int size, int type)
{
	defType[defName] = type;
	defMap[defName] = 0;
	for (int i = 0; i < paras.size(); i++)
	{
		paraMap[paras[i]] += size;
	}
	for (int i = 0; i < defs.size(); i++)
	{
		defMap[defs[i]] += size;
	}
	defs.push_back(defName);
	totalNum++;
	defNum++;
}

void Function::insertDef(string defName, int type)
{
	defType[defName] = type;
	defMap[defName] = 0;
	for (int i = 0; i < paras.size(); i++)
	{
		paraMap[paras[i]] += 4;
	}
	for (int i = 0; i < defs.size(); i++)
	{
		defMap[defs[i]] += 4;
	}
	defs.push_back(defName);
	totalNum++;
	defNum++;
}

void Function::insertRa()
{
	for (int i = 0; i < paras.size(); i++)
	{
		paraMap[paras[i]] += 4;
	}
	for (int i = 0; i < defs.size(); i++)
	{
		defMap[defs[i]] += 4;
	}
	raOff = 0;
}

int Function::getOffset(string name)
{
	if (hasPara(name))
	{
		return paraMap[name];
	}
	else
	{
		return defMap[name];
	}
}

int Function::getType(string name)
{
	if (hasPara(name)) return paraType[name];
	else if (hasDef(name)) return defType[name];
	else
	{
		cout << "[getType] name not exist" << endl;
		getchar();
	}
}
