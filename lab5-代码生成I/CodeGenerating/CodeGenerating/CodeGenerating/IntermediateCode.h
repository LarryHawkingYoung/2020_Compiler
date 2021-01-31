#include<string>
#include<vector>

using namespace std;

struct Quaternion
{
	string ans;
	string x;
	string op;
	string y;
};

class IntermediateCode
{
public:
	IntermediateCode();
	~IntermediateCode();

	void insertQuat(string ans, string x, string op, string y);
	int getQuatNum() const { return quatVec.size(); }
	void outputQuats();
	Quaternion getQuaternion(int index) const { return quatVec[index]; }

private:
	vector<Quaternion> quatVec;

};