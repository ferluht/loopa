#pragma once
#include <atomic>
#include <cstring>

/**
 * @private
 */
template <typename T, size_t S>
struct DoubleRingBuffer {
	size_t start{0};
	size_t end{0};
	T data[2 * S];

	void push(T t) {
		size_t i = end % S;
		data[i] = t;
        data[i + S] = t;
		end++;
	}
	T shift() {
		size_t i = start % S;
		T t = data[i];
		start++;
		return t;
	}
	void clear() {
		start = end;
	}
	bool empty() const {
		return start >= end;
	}
	bool full() const {
		return end - start >= S;
	}
	size_t size() const {
		return end - start;
	}
	size_t capacity() const {
		return S - size();
	}
};