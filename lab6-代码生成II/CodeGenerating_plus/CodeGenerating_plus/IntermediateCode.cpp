#include"IntermediateCode.h"
#include<string>
#include<fstream>
#include<vector>
#include<algorithm>

using namespace std;

ofstream iout("intermediateCode.txt");

IntermediateCode::IntermediateCode()
{
}

IntermediateCode::~IntermediateCode()
{
	iout.close();
}

void IntermediateCode::insertQuat(string ans, string x, string op, string y)
{
	quatVec.push_back(Quaternion{ ans, x, op, y });
}

void IntermediateCode::outputQuats()
{
	iout << "[ans]\t" << "[x]\t" << "[op]\t" << "[y]" << endl;
	for (int i = 0; i < quatVec.size(); i++)
	{
		iout << quatVec[i].ans << "\t" << quatVec[i].x << "\t" << quatVec[i].op << "\t" << quatVec[i].y << endl;
	}
}

void IntermediateCode::OptimizeCode()
{
	vector<Quaternion> betterQuatVec;
	for (int i = 0; i < quatVec.size(); i++)
	{
		if (i == 0) betterQuatVec.push_back(quatVec[i]);
		else
		{
			if (betterQuatVec.back().op == "j" && quatVec[i].op == ":" && betterQuatVec.back().ans == quatVec[i].ans)
			{
				betterQuatVec.pop_back();
			}
			betterQuatVec.push_back(quatVec[i]);
		}
	}
	quatVec.clear();
	quatVec = betterQuatVec;
}

void IntermediateCode::processSym(string idenfr)
{
	int cnt = 0;
	for (int i = 0; i < quatVec.size(); i++)
	{
		if (quatVec[i].ans == idenfr) cnt++;
		if (quatVec[i].x == idenfr) cnt++;
		if (quatVec[i].y == idenfr) cnt++;
	}
	SymCntSort.push_back(SymAndCnt{ idenfr, cnt });
}

bool cmp(const SymAndCnt& a, const SymAndCnt& b)
{
	return a.cnt > b.cnt;
}

vector<SymAndCnt> IntermediateCode::getSortedSymCnt()
{
	sort(SymCntSort.begin(), SymCntSort.end(), cmp);
	return SymCntSort;
}

