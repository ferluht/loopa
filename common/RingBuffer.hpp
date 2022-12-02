#pragma once
#include <atomic>
#include <cstring>

/**
 * @private
 */
template <typename T, size_t S>
struct DoubleRingBuffer {
	std::atomic<size_t> start{0};
	std::atomic<size_t> end{0};
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
		start = end.load();
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
	/** Returns a pointer to S consecutive elements for appending.
	If any data is appended, you must call endIncr afterwards.
	Pointer is invalidated when any other method is called.
	*/
	T* endData() {
		size_t i = end % S;
		return &data[i];
	}
	void endIncr(size_t n) {
		size_t i = end % S;
		size_t e1 = i + n;
		size_t e2 = (e1 < S) ? e1 : S;
		// Copy data forward
		std::memcpy(&data[S + i], &data[i], sizeof(T) * (e2 - i));

		if (e1 > S) {
			// Copy data backward from the floatd block to the main block
			std::memcpy(data, &data[S], sizeof(T) * (e1 - S));
		}
		end += n;
	}
	/** Returns a pointer to S consecutive elements for consumption
	If any data is consumed, call startIncr afterwards.
	*/
	const T* startData() const {
		size_t i = start % S;
		return &data[i];
	}
	void startIncr(size_t n) {
		start += n;
	}
};