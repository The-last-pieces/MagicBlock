
// MagicBlockView.h: CMagicBlockView 类的接口
//

#pragma once

#include "magic.h"

class CMagicBlockView : public CView
{
	// OpenGL 相关
private:
	HGLRC cur = NULL;
	CDC* glPDC = NULL;
	void showGL(CDC* pDC);
	void switchPDC(CDC* pDC);

	Vector3D eyePoint{};
	Vector3D headDirection = { 0.0f,1.0f,0.0f };

	const float PI = 4.0f * atanf(1);
	float eyeXZ = PI / 4, eyeY = PI / 4, dis = 10.0f;
	CPoint* lastPoint = nullptr;

	bool onTrack = false;

private:
	static const int LOGIC_TIMER = 1;
	static const int AUTO_TIMER = 2;


	bool start = false;
	const int FPS = 60;

	MagicBlock magic{};

	AveNum aveFps;
	LARGE_INTEGER lastPaint{};

private:

	float trimAngle(float angle) {
		if (angle > PI)
			angle -= 2 * PI;
		else if (angle < -PI)
			angle += 2 * PI;
		return angle;
	}

	void resetEye();

	void setMode(bool run);

	//void simulation(const int maxCheck = 10); // 最大碰撞检测次数
	//bool checkCollid(); // 检测是否有碰撞发生 , 并计算碰撞结果
	//void paintPosition(CDC*);
	//void playCollideVoice();

protected: // 仅从序列化创建
	CMagicBlockView() noexcept;
	DECLARE_DYNCREATE(CMagicBlockView)

	// 特性
public:
	CMagicBlockDoc* GetDocument() const;

	// 操作
public:

	// 重写
public:
	virtual void OnDraw(CDC* pDC);  // 重写以绘制该视图
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	// 实现
public:
	virtual ~CMagicBlockView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

	// 生成的消息映射函数
protected:
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	bool setPixelFormat();
	afx_msg void OnDestroy();
	virtual void OnInitialUpdate();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();
	afx_msg void OnRButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
};

#ifndef _DEBUG  // moveBallsDemoView.cpp 中的调试版本
inline CMagicBlockDoc* CMagicBlockView::GetDocument() const
{
	return reinterpret_cast<CMagicBlockDoc*>(m_pDocument);
}
#endif