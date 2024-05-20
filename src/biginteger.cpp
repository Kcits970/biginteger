#include <iostream>
#include <string>
#include <stack>
#include "biginteger.h"
using namespace std;

void __raiseNegativeArgumentException()
{
	throw invalid_argument("received negative argument");
}

void __raiseZeroDivisionException()
{
	throw invalid_argument("division by zero");
}

void __raiseInvalidStringLiteralException()
{
	throw invalid_argument("string contains invalid characters");
}

BigInteger::BigInteger(int value)
{
	bytes = new unsigned char[sizeof(int)];
	size = sizeof(int);

	unsigned char *bytePtr = reinterpret_cast<unsigned char*>(&value);

	for (int i = 0; i < size; i++)
		bytes[i] = bytePtr[i];

	strip();
}

BigInteger::BigInteger(const BigInteger& source)
{
	bytes = new unsigned char[source.size];
	size = source.size;

	for (int i = 0; i < source.size; i++)
		bytes[i] = source.bytes[i];
}

BigInteger::BigInteger(const string& value)
{
	if (value.length() == 0)
		__raiseInvalidStringLiteralException();
	
	bool sign = (value[0] == '-');
	int start = (value[0] == '-' or value[0] == '+') ? 1 : 0;
	
	bytes = new unsigned char[(value.length() - start) / 2 + 1]();
	size = (value.length() - start) / 2 + 1;
	
	for (int i = start; i < value.length(); i++)
	{
		if (value[i] < '0' || value[i] > '9')
			__raiseInvalidStringLiteralException();
		
		fixedAdd(BigInteger(value[i] - '0'));
		
		if (i != value.length() - 1)
			fixedAdd((*this) * BigInteger(9));
	}
	
	if (sign)
		(*this) = -(*this);
}

BigInteger::~BigInteger()
{
	delete[] bytes;
}

void BigInteger::operator=(const BigInteger& source)
{
	if (this == &source)
		return;

	delete[] bytes;
	bytes = new unsigned char[source.size];
	size = source.size;

	for (int i = 0; i < source.size; i++)
		bytes[i] = source.bytes[i];
}

BigInteger& BigInteger::bitwiseInvert()
{
	for (int i = 0; i < size; i++)
		bytes[i] = ~bytes[i];

	return *this;
}

BigInteger& BigInteger::bitwiseIncrement()
{
	bool carry;
	for (int i = 0; i < size; i++)
	{
		carry = (bytes[i] == 0xFF) ? true : false;
		bytes[i]++;

		if (!carry)
			break;
	}

	return *this;
}

BigInteger& BigInteger::bitwiseNegate()
{
	return bitwiseInvert().bitwiseIncrement();
}

BigInteger& BigInteger::zeroOut()
{
	for (int i = 0; i < size; i++)
		bytes[i] = 0x00;

	return *this;
}

BigInteger& BigInteger::resize(int newSize)
{
	if (newSize < 0)
		__raiseNegativeArgumentException();

	if (newSize == size)
		return *this;

	if (newSize < size)
		return truncate(newSize);

	unsigned char *resizedBytes = new unsigned char[newSize];
	
	for (int i = 0; i < size; i++)
		resizedBytes[i] = bytes[i];

	unsigned char filler = isNegative() ? 0xFF : 0x00;

	for (int i = size; i < newSize; i++)
		resizedBytes[i] = filler;

	delete[] bytes;
	bytes = resizedBytes;
	size = newSize;
	return *this;
}

BigInteger& BigInteger::truncate(int newSize)
{
	if (newSize < 0)
		__raiseNegativeArgumentException();

	if (newSize >= size)
		return *this;

	unsigned char *resizedBytes = new unsigned char[newSize];

	for (int i = 0; i < newSize; i++)
		resizedBytes[i] = bytes[i];

	delete[] bytes;
	bytes = resizedBytes;
	size = newSize;
	return *this;
}

static bool __checkFF(unsigned char byteFF, unsigned char nextByte)
{
	return byteFF == 0xFF && nextByte & 0x80;
}

static bool __check00(unsigned char byte00, unsigned char nextByte)
{
	return byte00 == 0x00 && !(nextByte & 0x80);
}

BigInteger& BigInteger::strip()
{
	if (size == 1)
		return *this;

	int stripSize = 0;
	bool (*strippable)(unsigned char, unsigned char) = isNegative() ? __checkFF : __check00;

	for (int i = size - 1; i >= 1; i--)
	{
		if (strippable(bytes[i], bytes[i-1]))
			stripSize++;
		else
			break;
	}

	return truncate(size - stripSize);
}

BigInteger& BigInteger::bytewiseLShift(int amount)
{
	if (amount < 0)
		__raiseNegativeArgumentException();

	if (amount == 0)
		return *this;

	if (amount >= size)
		return zeroOut();

	for (int i = 0; i < size - amount; i++)
		bytes[size - 1 - i] = bytes[size - 1 - i - amount];

	for (int i = 0; i < amount; i++)
		bytes[i] = 0x00;

	return *this;
}

BigInteger& BigInteger::bytewiseRShift(int amount)
{
	if (amount < 0)
		__raiseNegativeArgumentException();

	if (amount == 0)
		return *this;

	if (amount >= size)
		return zeroOut();

	for (int i = 0; i < size - amount; i++)
		bytes[i] = bytes[i + amount];
	
	for (int i = 0; i < amount; i++)
		bytes[size - 1 - i] = 0x00;

	return *this;
}

BigInteger& BigInteger::bitwiseLShift(int amount)
{
	if (amount < 0)
		__raiseNegativeArgumentException();

	if (amount == 0)
		return *this;

	bytewiseLShift(amount / 8);
	amount %= 8;

	unsigned char bitsToCarry = 0x00;
	for (int i = 0; i < size; i++)
	{
		unsigned char lostBits = bytes[i] & (0xFF << 8 - amount);
		bytes[i] <<= amount;
		bytes[i] |= bitsToCarry;

		bitsToCarry = lostBits >> 8 - amount;
	}

	return *this;
}

BigInteger& BigInteger::bitwiseRShift(int amount)
{
	if (amount < 0)
		__raiseNegativeArgumentException();

	if (amount == 0)
		return *this;

	bytewiseRShift(amount / 8);
	amount %= 8;

	unsigned char bitsToCarry = 0x00;
	for (int i = size - 1; i >= 0; i--)
	{
		unsigned char lostBits = bytes[i] & (0xFF >> 8 - amount);
		bytes[i] >>= amount;
		bytes[i] |= bitsToCarry;

		bitsToCarry = lostBits << 8 - amount;
	}

	return *this;
}

bool BigInteger::isNegative() const
{
	return bytes[size - 1] & 0x80;
}

const BigInteger BigInteger::operator-() const
{
	return BigInteger(*this).resize(size+1).bitwiseInvert().bitwiseIncrement().strip();
}

const BigInteger BigInteger::abs() const
{
	return isNegative() ? -(*this) : *this;
}

bool BigInteger::operator<(const BigInteger& another) const
{
	return (*this - another).isNegative();
}

bool BigInteger::operator>(const BigInteger& another) const
{
	return (another - *this).isNegative();
}

bool BigInteger::operator<=(const BigInteger& another) const
{
	return !(*this > another);
}

bool BigInteger::operator>=(const BigInteger& another) const
{
	return !(*this < another);
}

bool BigInteger::operator==(const BigInteger& another) const
{
	return (*this <= another) && (*this >= another);
}

static bool __willOverflow(unsigned char byte1, unsigned char byte2, bool carry)
{
	return static_cast<unsigned int>(byte1) + static_cast<unsigned int>(byte2) + (carry ? 1 : 0) > 0xFF;
}

BigInteger& BigInteger::fixedAdd(const BigInteger& another)
{
	unsigned char filler = another.isNegative() ? 0xFF : 0x00;

	bool carry = false;
	for (int i = 0; i < size; i++)
	{
		unsigned char byteToAdd = i < another.size ? another.bytes[i] : filler;
		bool overflowed = __willOverflow(bytes[i], byteToAdd, carry);
		
		bytes[i] += byteToAdd;
		if (carry)
			bytes[i]++;

		carry = overflowed;
	}

	return *this;
}

const BigInteger BigInteger::operator+(const BigInteger& another) const
{
	return BigInteger(*this).resize(max(size, another.size) + 1).fixedAdd(another).strip();
}

const BigInteger BigInteger::operator-(const BigInteger& another) const
{
	return *this + -another;
}

const BigInteger BigInteger::operator*(const BigInteger& another) const
{
	bool resultSign = isNegative() != another.isNegative();

	BigInteger operand1(abs());
	BigInteger operand2(another.abs());
	BigInteger result(0);

	result.resize(operand1.size + operand2.size + 1);
	operand1.resize(operand1.size + operand2.size + 1);

	for (int i = 0; i < operand2.size; i++)
	{
		for (int bitIndex = 0; bitIndex < 8; bitIndex++)
		{
			if (operand2.bytes[i] & (0x01 << bitIndex))
				result.fixedAdd(operand1);
			
			operand1.bitwiseLShift(1);
		}
	}

	result.strip();
	return resultSign ? -result : result;
}


const BigInteger BigInteger::operator/(const BigInteger& another) const
{
	if (another == BigInteger(0))
		__raiseZeroDivisionException();

	bool resultSign = isNegative() != another.isNegative();

	BigInteger operand1(abs());
	BigInteger operand2(another.abs());

	BigInteger quotient = BigInteger(0).resize(operand1.size);
	BigInteger remainder = BigInteger(0).resize(operand1.size);

	for (int i = operand1.size - 1; i >= 0; i--)
	{
		for (int bitIndex = 7; bitIndex >= 0; bitIndex--)
		{
			remainder.bitwiseLShift(1);
			remainder.bytes[0] |= (operand1.bytes[i] & (0x01 << bitIndex)) ? 0x01 : 0x00;
			
			if (remainder >= operand2)
			{
				remainder.fixedAdd(-operand2);
				quotient.bytes[i] |= (0x01 << bitIndex);
			}
		}
	}
	
	quotient.strip();
	return resultSign ? -quotient : quotient;
}


const BigInteger BigInteger::operator%(const BigInteger& another) const
{
	return (*this) - ((*this) / another * another);
}

int BigInteger::toInt() const
{
	int value = 0;
	unsigned char *bytePtr = reinterpret_cast<unsigned char*>(&value);
	
	
	for (int i = 0; i < (sizeof(int) < size ? sizeof(int) : size); i++)
		bytePtr[i] = bytes[i];
	
	return value;
}

// this function becomes really time-consuming when the numbers get large.
// performance optimization is needed.

string BigInteger::toString() const
{
	if (*this == BigInteger(0))
		return "0";
	
	BigInteger absolute = abs();
	stack<char> characters;

	while (!(absolute == BigInteger(0)))
	{
		characters.push('0' + (absolute % BigInteger(10)).toInt());
		absolute = absolute / BigInteger(10);
	}

	if (isNegative())
		characters.push('-');

	char result[characters.size() + 1];
	int counter = 0;

	while (!characters.empty())
	{
		result[counter++] = characters.top();
		characters.pop();
	}

	result[counter] = 0;
	
	return result;
}

string BigInteger::binaryRepresentation() const
{
	string result;
	
	for (int i = size - 1; i >= 0; i--)
		for (unsigned char mask = 0x80; mask != 0; mask >>= 1)
			result.push_back(bytes[i] & mask ? '1' : '0');
	
	return result;
}

ostream& operator<<(ostream& outputStream, const BigInteger& object)
{
	return outputStream << object.toString();
}
