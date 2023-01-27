#include "FastStack.h"

const char* FastStackOverflow::what() const throw () {
	return "FastStack overflowed.";
}

const char* FastStackUnderflow::what() const throw () {
	return "FastStack underflowed.";
}

const char* FastStackOutOfRange::what() const throw () {
	return "Trying to access a part of FastStack disposable memory region.";
}
