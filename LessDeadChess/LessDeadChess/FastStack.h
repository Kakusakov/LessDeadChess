#pragma once
#include <stdexcept>

struct FastStackOverflow : public std::out_of_range {
	const char* what() const throw ();
};

struct FastStackUnderflow : public std::out_of_range {
	const char* what() const throw ();
};

struct FastStackOutOfRange : public std::out_of_range {
	const char* what() const throw ();
};

template<typename T, size_t size>
class FastStack {
private:
	T mStack[size];
	size_t mIdx = 0;
public:
	inline const size_t length() const { return mIdx; }
	inline const size_t capacity() const { return size; }
	inline void push(T value) {
		if (mIdx == size) throw FastStackOverflow();
		mStack[mIdx++] = value;
	}
	inline T pop() {
		if (mIdx == 0) throw FastStackUnderflow();
		return mStack[--mIdx];
	}
	inline void remove(size_t amount) {
		if (mIdx < amount) throw FastStackOverflow();
		mIdx -= amount;
	}
	inline void clear() { mIdx = 0; }
	inline T& operator[](const size_t index) const { 
		if (index >= mIdx) throw FastStackOutOfRange();
		return mStack[index - 1];
	}
};
