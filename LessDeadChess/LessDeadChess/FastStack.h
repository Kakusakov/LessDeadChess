#pragma once
#include <stdexcept>
#include <array>

struct FastStackOverflow : public std::exception {
	const char* what() const throw ();
};

struct FastStackUnderflow : public std::exception {
	const char* what() const throw ();
};

struct FastStackOutOfRange : public std::exception {
	const char* what() const throw ();
};

//template<typename T, size_t size>
//class FastStack {
//private:
//	T mStack[size] = {};
//	size_t mIdx = 0;
//public:
//	inline const size_t length() const { return mIdx; }
//	inline const size_t capacity() const { return size; }
//	inline void push(T value) {
//		if (mIdx == size) throw FastStackOverflow();
//		mStack[mIdx++] = value;
//	}
//	inline T pop() {
//		if (mIdx == 0) throw FastStackUnderflow();
//		return mStack[--mIdx];
//	}
//	inline void remove(size_t amount) {
//		if (mIdx < amount) throw FastStackOverflow();
//		mIdx -= amount;
//	}
//	inline void clear() { mIdx = 0; }
//	inline T& operator[](const size_t index) { 
//		if (index >= mIdx) throw FastStackOutOfRange();
//		return mStack[index];
//	}
//};

template<typename T, size_t capacity>
class FastStack {
private:
	std::array<T, capacity> mStack = {};
	size_t mSize = 0;
public:
	inline const size_t size() const { return mSize; }
	inline const T peek(size_t idx) const {
		if (idx >= mSize) throw FastStackOutOfRange();
		return mStack[idx];
	}
	inline T& top() {
		if (mSize == 0) throw FastStackOutOfRange();
		return mStack[mSize - 1];
	}
	inline void push(const T& value) {
		if (mSize == capacity) {
			//throw FastStackOverflow();
			return;
		}
		mStack[mSize++] = value;
	}
	inline void pop(size_t count = 1) {
		if (mSize < count) throw FastStackUnderflow();
		mSize -= count;
	}
};
