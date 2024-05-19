#ifndef __BIGINTEGER_H__
#define __BIGINTEGER_H__

#include <ostream>
#include <string.h>

class BigInteger
{
	unsigned char *bytes;
	int size;

	BigInteger& bitwiseInvert();
	BigInteger& bitwiseIncrement();
	BigInteger& bitwiseNegate();
	BigInteger& zeroOut();
	BigInteger& truncate(int);
	BigInteger& resize(int);
	BigInteger& strip();

	BigInteger& bytewiseLShift(int);
	BigInteger& bytewiseRShift(int);
	BigInteger& bitwiseLShift(int);
	BigInteger& bitwiseRShift(int);

public:
	BigInteger(int);
	BigInteger(const BigInteger&);
	BigInteger(const std::string&);
	~BigInteger();
	void operator=(const BigInteger&);

	bool isNegative() const;
	const BigInteger operator-() const;
	const BigInteger abs() const;
	bool operator==(const BigInteger&) const;
	bool operator<(const BigInteger&) const;
	bool operator>(const BigInteger&) const;
	bool operator<=(const BigInteger&) const;
	bool operator>=(const BigInteger&) const;

	const BigInteger operator+(const BigInteger&) const;
	const BigInteger operator-(const BigInteger&) const;
	const BigInteger operator*(const BigInteger&) const;
	const BigInteger operator/(const BigInteger&) const;
	const BigInteger operator%(const BigInteger&) const;
	
	int toInt() const;
	std::string toString() const;
	std::string binaryRepresentation() const;
	friend std::ostream& operator<<(std::ostream&, const BigInteger&);
};

#endif
