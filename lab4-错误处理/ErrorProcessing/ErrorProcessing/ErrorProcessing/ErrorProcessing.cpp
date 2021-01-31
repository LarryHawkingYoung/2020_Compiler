#include"ErrorProcessing.h"
#include<iostream>
#include<vector>
#include<algorithm>
#include<fstream>
#include<string>

using namespace std;

ofstream eout("error.txt");

void ErrorProcessing::sortErrOutVec()
{
	sort(errOutVec.begin(), errOutVec.end(), cmp);
}

ErrorProcessing::ErrorProcessing()
{
}

ErrorProcessing::~ErrorProcessing()
{
	eout.close();
}

void ErrorProcessing::insertError(int lineID, string errType)
{
	ErrorOutput err{ lineID, errType };

	if (errOutVec.empty()) errOutVec.push_back(err);
	else
	{
		if (checkLineEqual(err, errOutVec.back()) && err.errType == "d" && errOutVec.back().errType == "e")
		{
			errOutVec.pop_back();
			errOutVec.push_back(err);
		}
		else if (!checkErrEqual(err, errOutVec.back()))
		{
			errOutVec.push_back(err);
		}
	}
}

void ErrorProcessing::outputErrors()
{
	sortErrOutVec();
	for (int i = 0; i < getErrNum(); i++)
	{
		eout << errOutVec[i].lineID << " " << errOutVec[i].errType << endl;
	}
}