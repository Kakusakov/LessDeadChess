#pragma once
#include <stdexcept>
#include <array>
#include <algorithm>

#include <iterator> // For std::forward_iterator_tag
#include <cstddef>  // For std::ptrdiff_t

struct FastStackOverflow : public std::exception {
	const char* what() const throw () {
		return "FastStack overflowed.";
	}
};

struct FastStackUnderflow : public std::exception {
	const char* what() const throw () {
		return "FastStack underflowed.";
	}
};

struct FastStackOutOfRange : public std::exception {
	const char* what() const throw () {
		return "Trying to access a part of "
			   "FastStack's disposable memory region.";
	}
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
		if (mSize == capacity) throw FastStackOverflow();
		mStack[mSize++] = value;
	}
	inline void insert(const T& value, size_t idx) {
		if (mSize == capacity) throw FastStackOverflow();
		if (idx > mSize) throw FastStackOutOfRange();
		const auto last = mStack.begin() + (++mSize);
		std::rotate(mStack.begin() + idx, last - 1, last);
		mStack[idx] = value;
	}
	inline void pop(size_t count = 1) {
		if (mSize < count) throw FastStackUnderflow();
		mSize -= count;
	}
	inline std::_Array_iterator<T, capacity> begin() {
		return mStack.begin(); 
	}
	inline std::_Array_iterator<T, capacity> end() {
		return mStack.begin() + mSize; 
	}
	inline std::_Array_const_iterator<T, capacity> cbegin() {
		return mStack.cbegin(); 
	}
	inline std::_Array_const_iterator<T, capacity> cend() {
		return mStack.cbegin() + mSize; 
	}
};
