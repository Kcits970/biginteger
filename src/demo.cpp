#include <iostream>
#include "biginteger.h"
using namespace std;

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		cout << "usage: ./demo <integer>" << endl;
		return 1;
	}
	
	BigInteger bigInt(argv[1]);
	cout << bigInt << '=' << bigInt.binaryRepresentation() << endl;
	
	return 0;
}

