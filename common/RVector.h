#pragma once

template <typename T>
class RVec2 {
public:
	RVec2() {
		RVec2((T)0);
	}
	RVec2(T a, T b) {
		tComponents[0] = a;
		tComponents[1] = b;
	}
	RVec2(T a) {
		tComponents[0] = tComponents[1] = a;
	}
	RVec2(const RVec2<T>& other) {
		tComponents[0] = other.tComponents[0];
		tComponents[1] = other.tComponents[1];
	}

	T Length() {
		return (T)sqrt((tComponents[0] * tComponents[0]) + (tComponents[1] * tComponents[1]));
	}

	void Normalize() {
		T length = Length();
		tComponents[0] /= length;
		tComponents[1] /= length;
	}

	void Add(const RVec2<T>& other) {
		tComponents[0] += other.tComponents[0];
		tComponents[1] += other.tComponents[1];
	}

	void Subtract(RVec2<T> other) {
		tComponents[0] -= other.tComponents[0];
		tComponents[1] -= other.tComponents[1];
	}

	void Copy(const RVec2<T>& other) {
		tComponents[0] = other.tComponents[0];
		tComponents[1] = other.tComponents[1];
	}

	void Multiply(const RVec2<T>& other) {
		tComponents[0] *= other.tComponents[0];
		tComponents[1] *= other.tComponents[1];
	}

	T GetX() {
		return tComponents[0];
	}

	T GetY() {
		return tComponents[1];
	}

	bool Within(const RVec2<T>& other, const T distance) {
		if((tComponents[0] < (other.tComponents[0] + distance) &&
			tComponents[0] > (other.tComponents[0] - distance)) &&
			(tComponents[1] < (other.tComponents[1] + distance) &&
			tComponents[1] > (other.tComponents[1] - distance))) {
				return true;
		}
		return false;
	}

	RVec2<T>& operator=(const RVec2<T>& right) {
		Copy(right);
		return *this;
	}

	RVec2<T>& operator+=(const RVec2<T>& right) {
		Add(right);
		return *this;
	}

	RVec2<T>& operator-=(const RVec2<T>& right) {
		Subtract(right);
		return *this;
	}

	RVec2<T>& operator*=(const RVec2<T>& right) {
		Multiply(right);
		return *this;
	}

	RVec2<T> operator-(const RVec2<T>& right) {
		RVec2<T> result(*this);
		result -= right;
		return result;
	}

	RVec2<T> operator+(const RVec2<T>& right) {
		RVec2<T> result(*this);
		result += right;
		return result;
	}

	RVec2<T> operator*(const RVec2<T>& right) {
		RVec2<T> result(*this);
		result *= right;
		return result;
	}
private:
	T tComponents[2];
};