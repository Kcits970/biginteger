#include <iostream>
#include <string>
#include <stack>
#include "biginteger.h"
using namespace std;

static void __raiseNegativeArgumentException()
{
	throw invalid_argument("received non-positive argument");
}

static void __raiseZeroDivisionException()
{
	throw invalid_argument("division by zero");
}

static void __raiseInvalidStringLiteralException()
{
	throw invalid_argument("string contains invalid characters");
}

#define DEF_BIGINT(NAME, VALUE) static BigInteger BIGINTEGER_##NAME(VALUE);

DEF_BIGINT(ZERO, 0)
DEF_BIGINT(ONE, 1)
DEF_BIGINT(TWO, 2)
DEF_BIGINT(THREE, 3)
DEF_BIGINT(FOUR, 4)
DEF_BIGINT(FIVE, 5)
DEF_BIGINT(SIX, 6)
DEF_BIGINT(SEVEN, 7)
DEF_BIGINT(EIGHT, 8)
DEF_BIGINT(NINE, 9)
DEF_BIGINT(TEN, 10)

#undef DEF_BIGINT

static inline bool __testBit(unsigned char byte, int bitIndex)
{
	return byte & 0x01 << bitIndex;
}

static inline void __setBit(unsigned char& byte, bool set, int bitIndex)
{
	if (set)
		byte |= 0x01 << bitIndex;
	else
		byte &= ~(0x01 << bitIndex);
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

BigInteger::BigInteger(int allocationSize, bool zeroInitialize)
{
	if (allocationSize <= 0)
		__raiseNegativeArgumentException();

	if (zeroInitialize)
		bytes = new unsigned char[allocationSize]();
	else
		bytes = new unsigned char[allocationSize];

	size = allocationSize;
}

BigInteger::BigInteger(const BigInteger& source, int allocationSize, bool signExtend)
{
	if (allocationSize <= 0)
		__raiseNegativeArgumentException();
	
	bytes = new unsigned char[allocationSize];
	size = allocationSize;
	unsigned char filler = (source.isNegative() && signExtend) ? 0xFF : 0x00;
	
	for (int i = 0; i < allocationSize; i++)
		bytes[i] = i < source.size ? source.bytes[i] : filler;
}

BigInteger::BigInteger(const BigInteger& source)
{
	bytes = new unsigned char[source.size];
	size = source.size;

	for (int i = 0; i < source.size; i++)
		bytes[i] = source.bytes[i];
}

static BigInteger *__singleDigitInstances[] =
{
	&BIGINTEGER_ZERO,
	&BIGINTEGER_ONE,
	&BIGINTEGER_TWO,
	&BIGINTEGER_THREE,
	&BIGINTEGER_FOUR,
	&BIGINTEGER_FIVE,
	&BIGINTEGER_SIX,
	&BIGINTEGER_SEVEN,
	&BIGINTEGER_EIGHT,
	&BIGINTEGER_NINE,	
};

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
		
		fixedAdd(*__singleDigitInstances[value[i] - '0']);
		
		if (i != value.length() - 1)
			fixedUnsignedMultiply(BIGINTEGER_TEN);
	}
	
	if (sign)
		bitwiseNegate();

	strip();
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
	return byteFF == 0xFF && __testBit(nextByte, 7);
}

static bool __check00(unsigned char byte00, unsigned char nextByte)
{
	return byte00 == 0x00 && !__testBit(nextByte, 7);
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

BigInteger& BigInteger::bitwiseLShift()
{
	bool shiftOver = false;
	for (int i = 0; i < size; i++)
	{
		bool lost = __testBit(bytes[i], 7);
		bytes[i] <<= 1;
		
		if (shiftOver)
			__setBit(bytes[i], true, 0);

		shiftOver = lost;
	}

	return *this;
}

BigInteger& BigInteger::arithmeticRShift()
{
	bool sign = isNegative();

	bool shiftOver = false;
	for (int i = size - 1; i >= 0; i--)
	{
		bool lost = __testBit(bytes[i], 0);
		bytes[i] >>= 1;

		if (shiftOver)
			__setBit(bytes[i], true, 7);

		shiftOver = lost;
	}

	if (sign)
		__setBit(bytes[size - 1], true, 7);

	return *this;
}

bool BigInteger::isNegative() const
{
	return __testBit(bytes[size - 1], 7);
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
	if (isNegative() && !another.isNegative())
		return true;

	if (!isNegative() && another.isNegative())
		return false;

	int compareLength = max(size, another.size);
	unsigned char filler = isNegative() ? 0xFF : 0x00;

	for (int i = compareLength - 1; i >= 0; i--)
	{
		unsigned char byte1 = i < size ? bytes[i] : filler;
		unsigned char byte2 = i < another.size ? another.bytes[i] : filler;

		if (byte1 < byte2)
			return true;
		else if (byte1 > byte2)
			return false;
	}

	return false;
}

bool BigInteger::operator>(const BigInteger& another) const
{
	return another < (*this);
}

bool BigInteger::operator==(const BigInteger& another) const
{
	if (isNegative() != another.isNegative())
		return false;

	int compareLength = max(size, another.size);
	unsigned char filler = isNegative() ? 0xFF : 0x00;

	for (int i = 0; i < compareLength; i++)
	{
		unsigned char byte1 = i < size ? bytes[i] : filler;
		unsigned char byte2 = i < another.size ? another.bytes[i] : filler;

		if (byte1 != byte2)
			return false;
	}

	return true;
}

bool BigInteger::operator<=(const BigInteger& another) const
{
	return !(*this > another);
}

bool BigInteger::operator>=(const BigInteger& another) const
{
	return !(*this < another);
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

static void __bytecopy(const unsigned char *src, int length, unsigned char *dst)
{
	for (int i = 0; i < length; i++)
		dst[i] = src[i];
}

BigInteger& BigInteger::fixedUnsignedMultiply(const BigInteger& multiplier)
{
	BigInteger multiplicand(*this);
	zeroOut();

	for (int i = 0; i < multiplier.size; i++)
	{
		for (int bitIndex = 0; bitIndex < 8; bitIndex++)
		{
			if (__testBit(multiplier.bytes[i], bitIndex))
				fixedAdd(multiplicand);

			multiplicand.bitwiseLShift();
		}
	}

	return *this;
}

//Booth's multiplication algorithm.
const BigInteger BigInteger::operator*(const BigInteger& multiplier) const
{
	BigInteger __a(size + 1 + multiplier.size + 1, true);
	BigInteger __s(size + 1 + multiplier.size + 1, true);
	BigInteger __p(size + 1 + multiplier.size + 1, true);

	BigInteger multiplicand(size + 1, false);
	__bytecopy(bytes, size, multiplicand.bytes);
	multiplicand.bytes[size] = isNegative() ? 0xFF : 0x00;

	__bytecopy(multiplicand.bytes, multiplicand.size, __a.bytes + multiplier.size + 1);
	multiplicand.bitwiseNegate();
	__bytecopy(multiplicand.bytes, multiplicand.size, __s.bytes + multiplier.size + 1);
	__bytecopy(multiplier.bytes, multiplier.size, __p.bytes + 1);
	
	for (int i = 0; i < multiplier.size; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			if (!__testBit(__p.bytes[1], 0) && __testBit(__p.bytes[0], 7))
				__p.fixedAdd(__a);
			else if (__testBit(__p.bytes[1], 0) && !__testBit(__p.bytes[0], 7))
				__p.fixedAdd(__s);
	
			__p.arithmeticRShift();
		}
	}

	BigInteger product(size + 1 + multiplier.size, false);
	__bytecopy(__p.bytes + 1, __p.size - 1, product.bytes);
	return product.strip();
}

//Generic binary long division algorithm.
pair<BigInteger, BigInteger> BigInteger::absoluteDivisionPair(const BigInteger& dividend, const BigInteger& divisor)
{
	if (divisor == BIGINTEGER_ZERO)
		__raiseZeroDivisionException();

	BigInteger absDividend(dividend, dividend.size + 1, true);
	BigInteger absDivisor(divisor, divisor.size + 1, true);
	BigInteger negativeDivisor(absDivisor);

	if (absDividend.isNegative())
		absDividend.bitwiseNegate();
	
	if (absDivisor.isNegative())
		absDivisor.bitwiseNegate();
	else
		negativeDivisor.bitwiseNegate();

	BigInteger quotient(absDividend.size, true);
	BigInteger remainder(absDividend.size, true);

	for (int i = absDividend.size - 1; i >= 0; i--)
	{
		for (int bitIndex = 7; bitIndex >= 0; bitIndex--)
		{
			remainder.bitwiseLShift();
			__setBit(remainder.bytes[0], __testBit(absDividend.bytes[i], bitIndex), 0);

			if (remainder >= absDivisor)
			{
				remainder.fixedAdd(negativeDivisor);
				quotient.bytes[i] |= (0x01 << bitIndex);
			}
		}
	}
	
	return pair(quotient.strip(), remainder.strip());
}

const BigInteger BigInteger::operator/(const BigInteger& another) const
{	
	pair<BigInteger, BigInteger> divisionPair = absoluteDivisionPair(*this, another);

	return isNegative() != another.isNegative() ? -divisionPair.first : divisionPair.first;
}


const BigInteger BigInteger::operator%(const BigInteger& another) const
{
	pair<BigInteger, BigInteger> divisionPair = absoluteDivisionPair(*this, another);

	return isNegative() ? -divisionPair.second : divisionPair.second;
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
	if (*this == BIGINTEGER_ZERO)
		return "0";
	
	BigInteger absolute = abs();
	stack<char> characters;

	while (!(absolute == BIGINTEGER_ZERO))
	{
		pair<BigInteger, BigInteger> divisionPair = absoluteDivisionPair(absolute, BIGINTEGER_TEN);
		characters.push('0' + divisionPair.second.toInt());
		absolute = divisionPair.first;
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
