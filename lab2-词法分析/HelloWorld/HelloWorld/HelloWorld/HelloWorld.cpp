#include<iostream>
#include<string>
#include<fstream>
#include<map>

int getNewIndex(int start, std::string str);
std::string toLowerCase(std::string in);

std::ifstream fin("testfile.txt");
std::ofstream fout("output.txt");
std::string line;

std::string strcon;
std::string idenfr;
std::string intcon;
char ch;
std::map<std::string, std::string> keptLabelMap;

int main()
{
	keptLabelMap.insert(std::map<std::string, std::string>::value_type("const", "CONSTTK"));
	keptLabelMap.insert(std::map<std::string, std::string>::value_type("int", "INTTK"));
	keptLabelMap.insert(std::map<std::string, std::string>::value_type("char", "CHARTK"));
	keptLabelMap.insert(std::map<std::string, std::string>::value_type("void", "VOIDTK"));
	keptLabelMap.insert(std::map<std::string, std::string>::value_type("main", "MAINTK"));
	keptLabelMap.insert(std::map<std::string, std::string>::value_type("if", "IFTK"));
	keptLabelMap.insert(std::map<std::string, std::string>::value_type("else", "ELSETK"));
	keptLabelMap.insert(std::map<std::string, std::string>::value_type("switch", "SWITCHTK"));
	keptLabelMap.insert(std::map<std::string, std::string>::value_type("case", "CASETK"));
	keptLabelMap.insert(std::map<std::string, std::string>::value_type("default", "DEFAULTTK"));
	keptLabelMap.insert(std::map<std::string, std::string>::value_type("while", "WHILETK"));
	keptLabelMap.insert(std::map<std::string, std::string>::value_type("for", "FORTK"));
	keptLabelMap.insert(std::map<std::string, std::string>::value_type("scanf", "SCANFTK"));
	keptLabelMap.insert(std::map<std::string, std::string>::value_type("printf", "PRINTFTK"));
	keptLabelMap.insert(std::map<std::string, std::string>::value_type("return", "RETURNTK"));

	//std::cout << fin.is_open() << std::endl;
	while (std::getline(fin, line))
	{
		int ind = 0;
		//std::cout << ind << std::endl;
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
					fout << "STRCON " << strcon << std::endl;
					break;
				case '\'':
					ind++;
					fout << "CHARCON " << line[ind] << std::endl;
					ind++;
					break;
				case '+':
					fout << "PLUS +" << std::endl;
					break;
				case '-':
					fout << "MINU -" << std::endl;
					break;
				case '*':
					fout << "MULT *" << std::endl;
					break;
				case '/':
					fout << "DIV /" << std::endl;
					break;
				case '<':
					switch (line[ind+1])
					{
						case '=':
							fout << "LEQ <=" << std::endl;
							ind++;
							break;
						default:
							fout << "LSS <" << std::endl;
							break;
					}
					break;
				case '>':
					switch (line[ind + 1])
					{
						case '=':
							fout << "GEQ >=" << std::endl;
							ind++;
							break;
						default:
							fout << "GRE >" << std::endl;
							break;
					}
					break;
				case '=':
					switch (line[ind + 1])
					{
						case '=':
							fout << "EQL ==" << std::endl;
							ind++;
							break;
						default:
							fout << "ASSIGN =" << std::endl;
							break;
					}
					break;
				case '!':
					fout << "NEQ !=" << std::endl;
					ind++;
					break;
				case ':':
					fout << "COLON :" << std::endl;
					break;
				case ';':
					fout << "SEMICN ;" << std::endl;
					break;
				case ',':
					fout << "COMMA ," << std::endl;
					break;
				case '(':
					fout << "LPARENT (" << std::endl;
					break;
				case ')':
					fout << "RPARENT )" << std::endl;
					break;
				case '[':
					fout << "LBRACK [" << std::endl;
					break;
				case ']':
					fout << "RBRACK ]" << std::endl;
					break;
				case '{':
					fout << "LBRACE {" << std::endl;
					break;
				case '}':
					fout << "RBRACE }" << std::endl;
					break;
				default:
					if (ch == '_' || ch >= 'a' && ch <= 'z' || ch >= 'A' && ch <= 'Z')
					{
						idenfr.clear();
						idenfr += ch;
						ch = line[ind + 1];
						while (ch == '_' || ch >= 'a' && ch <= 'z' || ch >= 'A' && ch <= 'Z' || ch>='0'&&ch<='9')
						{
							idenfr += ch;
							ind++;
							ch = line[ind+1];
						}
						std::map<std::string, std::string>::iterator iter = keptLabelMap.find(toLowerCase(idenfr));
						if (iter == keptLabelMap.end())
						{
							fout << "IDENFR " << idenfr << std::endl;
						}
						else
						{
							fout << iter->second << " " << idenfr << std::endl;
						}
					}
					else if (ch >= '0' && ch <= '9')
					{
						intcon.clear();
						intcon += ch;
						while (line[ind+1] >= '0' && line[ind+1] <= '9')
						{
							ind++;
							ch = line[ind];
							intcon += ch;
						}
						fout << "INTCON " << intcon << std::endl;
					}
					break;
			}

			ind++;
		}
	}
}

std::string toLowerCase(std::string in)
{
	std::string out = "";
	for (int i = 0; i < in.length(); i++)
	{
		out += in[i] >= 'a' && in[i] <= 'z' ? in[i] : in[i] - ('A' - 'a');
	}
	return out;
}

int getNewIndex(int start, std::string str)
{
	int i = start;
	while (str[i] == '\t' || str[i] == ' ') i++;
	return i;
}