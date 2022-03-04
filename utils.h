#pragma once
#include "pch.h"

#include <vector>
#include <string>
#include <deque>
#include <map>
#include <functional>
#include <algorithm>
#include <queue>
#include <unordered_map>
#include <array>
#include <iterator>
using std::array;
using std::unordered_map;
using std::queue;
using std::vector;
using std::deque;
using std::pair;
using std::map;

class AveNum {
private:
	double ave;
	size_t maxSize;
public:
	explicit AveNum(size_t maxSize) {
		setSize(maxSize);
	}
	void setSize(size_t size) {
		maxSize = size;
		ave = 0;
	}
	operator double() {
		return ave;
	}
	AveNum& operator+=(double val) {
		expand(val);
		return *this;
	}
private:
	void expand(double cur) {
		if (maxSize) ave = (cur + ave * (maxSize - 1)) / maxSize;
	}
};

struct GColor
{
	float r = 0.f, g = 0.f, b = 0.f;

	bool operator==(const GColor& other) const {
		return eq(r, other.r) && eq(g, other.g) && eq(b, other.b);
	}
	bool operator!=(const GColor& other) const {
		return !(*this == other);
	}

	static bool eq(float a, float b) {
		int range = 255 * 10;
		return int(range * a) == int(range * b);
	}

	static const GColor make(int r, int g, int b) {
		return { r * 1.0f ,g * 1.0f ,b * 1.0f };
	}

	static const GColor black() {
		static const GColor c = { 0.0f,0.0f,0.0f };
		return c;
	}
};

struct Vector3D
{
	float x, y, z;

	const Vector3D operator*(const glm::mat4& matrix) const {
		glm::vec4 result = matrix * glm::vec4(x, y, z, 1.0);
		return { result.x,result.y,result.z };
	}

	float range(const Vector3D& other) const {
		return std::max({ std::abs(other.x - x), std::abs(other.y - y), std::abs(other.z - z) });
	}
};

enum class Side
{
	up = 5,
	down = 2,
	front = 3,
	back = 0,
	left = 1,
	right = 4
};

class MagicRotate {
	const float PI = 180;
public:
	Side lay; // 要旋转的面从哪边开始计数
	vector<int> offset; // 计数值

	int times; // 一共旋转几次(一帧一次)
	int scale; // 几个90度

	Vector3D eulerOnce; // 单次旋转的角度
	Vector3D eulerTotal; // 旋转若干个90度 
public:
	MagicRotate(Side lay, int scale, int frame, const vector<int>& offset = { 0 }) :lay(lay), scale(scale), offset(offset) {
		if (scale < 0) scale *= -3;
		scale %= 4;
		if (scale == 3) scale = -1;
		float total = PI / 2 * scale;
		float once = total / frame;
		times = frame;

		static const map<Side, vector<int>> rmp = {
			{Side::front, {0,0,-1}},
			{Side::back, {0,0,1}},
			{Side::up, {0,-1,0}},
			{Side::down, {0,1,0}},
			{Side::left, {1,0,0}},
			{Side::right, {-1,0,0}},
		};
		auto& axis = rmp.find(lay)->second;
		eulerOnce.x = axis[0] * once;
		eulerOnce.y = axis[1] * once;
		eulerOnce.z = axis[2] * once;

		eulerTotal.x = axis[0] * total;
		eulerTotal.y = axis[1] * total;
		eulerTotal.z = axis[2] * total;
	}

	MagicRotate& operator=(const MagicRotate& obj) {
		lay = obj.lay;
		offset = obj.offset;
		times = obj.times;
		eulerOnce = obj.eulerOnce;
		eulerTotal = obj.eulerTotal;
		scale = obj.scale;
		return *this;
	}

	const MagicRotate operator-() const {
		return MagicRotate(lay, -scale, times, offset);
	}

	const MagicRotate operator*(int repeat) const {
		return MagicRotate(lay, repeat * scale, times, offset);
	}

	const MagicRotate operator+(const MagicRotate& other) const {
		return MagicRotate(lay, scale + other.scale, times, offset);
	}

	// 只执行一次
	const MagicRotate one() const {
		return MagicRotate(lay, scale, 1, offset);
	}
	// 重置frame
	const MagicRotate frame(int newFrame) const {
		return MagicRotate(lay, scale, newFrame, offset);
	}

	// 重置offset
	const MagicRotate off(int newoff) const {
		return MagicRotate(lay, scale, times, { newoff });
	}
	const MagicRotate off(const vector<int>& newoff) const {
		return MagicRotate(lay, scale, times, newoff);
	}
};

struct BlockPos {
	int x = 0, y = 0, z = 0;
};

// 封装OpenGL的绘图函数
class GLPaintObject {
protected:
	static void setColor(float r, float g, float b) {
		glColor3f(r / 255, g / 255, b / 255);
	}
	static void setWidth(float width) {
		glLineWidth(width);
	}
	static void line2D(float x1, float y1, float x2, float y2) {
		glBegin(GL_LINES);
		glVertex2f(x1, y1);
		glVertex2f(x2, y2);
		glEnd();
	}
	static void line3D(float x1, float y1, float z1, float x2, float y2, float z2) {
		glBegin(GL_LINES);
		glVertex3f(x1, y1, z1);
		glVertex3f(x2, y2, z2);
		glEnd();
	}
	static void line3D(const Vector3D& p1, const Vector3D& p2) {
		line3D(p1.x, p1.y, p1.z, p2.x, p2.y, p2.z);
	}
	static void rectangle(const Vector3D& p1, const Vector3D& p2, const Vector3D& p3, const Vector3D& p4) {
		glBegin(GL_QUADS);
		glVertex3f(p1.x, p1.y, p1.z);
		glVertex3f(p2.x, p2.y, p2.z);
		glVertex3f(p3.x, p3.y, p3.z);
		glVertex3f(p4.x, p4.y, p4.z);
		glEnd();
	}
	static void loopLine(const vector<Vector3D>& points) {
		glBegin(GL_LINE_LOOP);
		for (auto& pos : points) {
			glVertex3f(pos.x, pos.y, pos.z);
		}
		glEnd();
	}
};