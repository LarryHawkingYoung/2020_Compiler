#include<string>

using namespace std;

enum KindEnum {
	CONST,
	VAR,
	FUNC,
	PARAM
};

enum TypeEnum {
	CHAR,
	INT,
	VOID
};

struct Symbol
{
	string idenfr;
	int kind;
	int type;
	int level;
	int dim;
	unsigned int addrOffset;
};