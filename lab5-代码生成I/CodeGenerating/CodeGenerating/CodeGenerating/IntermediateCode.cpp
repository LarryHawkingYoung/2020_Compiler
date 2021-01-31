#include"IntermediateCode.h"
#include<string>
#include<fstream>

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
	iout << "ans\t" << "x\t" << "op\t" << "y" << endl;
	for (int i = 0; i < quatVec.size(); i++)
	{
		iout << quatVec[i].ans << "\t" << quatVec[i].x << "\t" << quatVec[i].op << "\t" << quatVec[i].y << endl;
	}
}

