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

class Ball {
	const double PI = atan(1) * 4;
	const double transform = PI / 180;
private:
	double x, y;// ��λΪ����

	int radius = 10; // ��λΪ����
	double speed = 100; // ��λΪ����
	double angle; // ���满���Ƶ�ֵ

	bool banCollid = false;

	deque<pair<int, int>> hisPos;
	const size_t maxHis = 50 * 1;
public:
	Ball(int sx, int sy) {
		x = sx; y = sy;
		setAngle(rand() % 360);
		radius = rand() % 10 + 20;
	}

	double area() const {
		return radius * radius * 4;
	}

	// ���ýǶ��Ƶ�ֵ
	void setAngle(double angle) {
		this->angle = angle * transform;
	}

	void move(int fps) {
		// �ı�����
		x += speed * cos(angle) / fps;
		y += speed * sin(angle) / fps;

		// �ı����ٶ�
		double a = 100000;
		speed += (5.0 / fps) * a / (radius * radius);
		// ������ת�ٶȷ���
		angle += 1.0 / fps;

		saveHis();
	}

	void showOn(CDC* pDC)const {
		COLORREF color = banCollid ? RGB(255, 0, 0) : RGB(255, 0, 255);
		CBrush brush(color), * old = pDC->SelectObject(&brush);

		// ����text����
		auto oldMode = pDC->SetTextAlign(TA_CENTER | VTA_CENTER);
		auto oldBack = pDC->SetBkMode(TRANSPARENT);
		auto oldText = pDC->SetTextColor(RGB(0, 255, 255));

		// ��������λ��
		int px = int(x), py = int(y);

		//// ����
		// ���岿��
		pDC->Ellipse(px - radius, py - radius, px + radius, py + radius);
		// ��ǰ�ٶ���ֵ
		pDC->TextOutW(px, py, std::to_wstring(int(speed)).c_str());
		// β��
		CPoint tail(int(x - radius * cos(angle)), int(y - radius * sin(angle)));
		pDC->MoveTo(tail);
		for (size_t i = 0; i < hisPos.size(); ++i) {
			int blue = int(1.0 * i / hisPos.size() * 100) + 100;
			CPen pen(PS_SOLID, int(radius * (hisPos.size() - i) / hisPos.size()) + 1, RGB(0, 0, blue)),
				* oldPen = pDC->SelectObject(&pen);
			pDC->LineTo(hisPos[i].first, hisPos[i].second);
			pDC->SelectObject(oldPen);
			pen.DeleteObject();
		}

		// �ָ�����
		pDC->SelectObject(old);
		pDC->SetTextAlign(oldMode);
		pDC->SetBkMode(oldBack);
		pDC->SetTextColor(oldText);

		// �ͷ���Դ
		brush.DeleteObject();
	}

	// ����Ƿ���ײ���˱߽�
	bool checkBond(const CRect& viewRect) {
		const CRect rect(
			viewRect.left + radius, viewRect.top + radius,
			viewRect.right - radius, viewRect.bottom - radius
		);
		bool ret = false;
		if (x < rect.left) {
			x = rect.left;
			mirrorWith(90);
			ret = true;
		}
		if (x > rect.right) {
			x = rect.right;
			mirrorWith(90);
			ret = true;
		}
		if (y < rect.top) {
			y = rect.top;
			mirrorWith(0);
			ret = true;
		}
		if (y > rect.bottom) {
			y = rect.bottom;
			mirrorWith(0);
			ret = true;
		}
		if (ret) {
			speed *= 0.95;
		}
		return ret;
	}

	void finishCollid() {
		banCollid = false;
	}

	static bool collidWith(Ball& b1, Ball& b2) {
		if (b1.banCollid || b2.banCollid)return false;
		double m1 = b1.radius * b1.radius, m2 = b2.radius * b2.radius;
		double vx1 = b1.speed * cos(b1.angle), vy1 = b1.speed * sin(b1.angle);
		double vx2 = b2.speed * cos(b2.angle), vy2 = b2.speed * sin(b2.angle);
		// ��ײ���ٶ�
		double vx1_ = solveV1(m1, m2, vx1, vx2), vy1_ = solveV1(m1, m2, vy1, vy2);
		double vx2_ = solveV2(m1, m2, vx1, vx2), vy2_ = solveV2(m1, m2, vy1, vy2);
		b1.apply(vx1_, vy1_);
		b2.apply(vx2_, vy2_);
		// b1.banCollid = b2.banCollid = true;

		// �غϵ�����������Ҫ��������
		double angle = atan2(b2.y - b1.y, b2.x - b1.x);
		double radius = b1.radius + b2.radius + 1;
		double distance = sqrt(pow(b1.x - b2.x, 2) + pow(b1.y - b2.y, 2));
		// off1 * m1 = off2 * m2
		// off1+off2+dis=radius
		double off1 = (radius - distance) / (1 + m1 / m2);
		double off2 = m1 / m2 * off1;
		b1.x -= off1 * cos(angle);
		b1.y -= off1 * sin(angle);

		b2.x += off2 * cos(angle);
		b2.y += off2 * sin(angle);
		return true;
	}

	static bool isCollid(const Ball& b1, const Ball& b2) {
		return pow(b1.x - b2.x, 2) + pow(b1.y - b2.y, 2) <= pow(b1.radius + b2.radius, 2);
	}

private:
	static double solveV1(double m1, double m2, double v1, double v2) {
		return ((m1 - m2) * v1 + 2 * m2 * v2) / (m1 + m2);
	}
	static double solveV2(double m1, double m2, double v1, double v2) {
		return ((m2 - m1) * v2 + 2 * m1 * v1) / (m1 + m2);
	}

	void apply(double vx, double vy) {
		angle = atan2(vy, vx);
		speed = sqrt(vx * vx + vy * vy);
	}

	// angleΪ���� , axisΪ�Ƕ�
	double mirrorAngle(double angle, double axis) {
		return 2 * (axis * transform) - angle;
	}
	void mirrorWith(double axis) {
		angle = mirrorAngle(angle, axis);
	}

	void saveHis() {
		if (rand() % 1) return;
		pair<int, int> p = { int(x - radius * cos(angle)), int(y - radius * sin(angle)) };
		if (hisPos.empty()) {
			hisPos.push_front(p);
		}
		else {
			const auto& front = hisPos.front();
			int minRange = 10;
			if (pow(front.first - p.first, 2) + pow(front.second - p.second, 2) > minRange * minRange) {
				hisPos.push_front(p);
			}
		}
		while (hisPos.size() > maxHis) hisPos.pop_back();
	}
};
