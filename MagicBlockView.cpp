
// MagicBlockView.cpp: CMagicBlockView 类的实现
//

#include "pch.h"
#include "framework.h"
// SHARED_HANDLERS 可以在实现预览、缩略图和搜索筛选器句柄的
// ATL 项目中进行定义，并允许与该项目共享文档代码。
#ifndef SHARED_HANDLERS
#include "MagicBlock.h"
#endif

#include "MagicBlockDoc.h"
#include "MagicBlockView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMagicBlockView

IMPLEMENT_DYNCREATE(CMagicBlockView, CView)

BEGIN_MESSAGE_MAP(CMagicBlockView, CView)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_ERASEBKGND()
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_MOUSEWHEEL()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSELEAVE()
	ON_WM_RBUTTONDBLCLK()
	ON_WM_TIMER()
	ON_WM_KEYUP()
END_MESSAGE_MAP()

// CMagicBlockView 构造/析构

CMagicBlockView::CMagicBlockView() noexcept :aveFps(FPS * 2)
{
	// TODO: 在此处添加构造代码

}

CMagicBlockView::~CMagicBlockView()
{
}

BOOL CMagicBlockView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: 在此处通过修改
	//  CREATESTRUCT cs 来修改窗口类或样式

	return CView::PreCreateWindow(cs);
}

// CMagicBlockView 绘图

void CMagicBlockView::OnDraw(CDC* pDC)
{
	CMagicBlockDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;
	// TODO: 在此处为本机数据添加绘制代码
	showGL(NULL);
	pDC->TextOutW(
		0, 0, (L"FPS:" + std::to_wstring(1 / double(aveFps))).c_str()
	);
	// 计时
	if (lastPaint.QuadPart > 0) {
		LARGE_INTEGER nowPaint, tc;
		QueryPerformanceFrequency(&tc);
		QueryPerformanceCounter(&nowPaint);
		aveFps += (double)(nowPaint.QuadPart - lastPaint.QuadPart) / double(tc.QuadPart);
	}
	QueryPerformanceCounter(&lastPaint);
	return;
}

void CMagicBlockView::showGL(CDC* pDC)
{
	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	magic.show();

	glPopMatrix();
	glDisable(GL_DEPTH_TEST);
	// 交换缓冲区  
	SwapBuffers(wglGetCurrentDC());
}

void CMagicBlockView::switchPDC(CDC* pDC)
{
	// 切换像素格式
	glPDC = pDC;
	setPixelFormat();
	// 创建渲染环境, 并使它成为当前渲染环境  
	if (cur) wglDeleteContext(cur);
	cur = wglCreateContext(pDC->GetSafeHdc());
	wglMakeCurrent(pDC->GetSafeHdc(), cur);
}

// 控制logic
void CMagicBlockView::setMode(bool run) {
	if (!run) {
		KillTimer(LOGIC_TIMER);
		start = false;
	}
	else {
		SetTimer(LOGIC_TIMER, 1000 / FPS, NULL);
		start = true;
	}
}

// 消息处理

void CMagicBlockView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	CRect rect;
	GetClientRect(&rect);
}

void CMagicBlockView::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (!KillTimer(AUTO_TIMER))
		SetTimer(AUTO_TIMER, 200 * 1, NULL);
	// setMode(!start);
	CView::OnLButtonDblClk(nFlags, point);
}

BOOL CMagicBlockView::OnEraseBkgnd(CDC* pDC)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	// 返回TRUE , 屏蔽擦除背景的行为
	return TRUE || CView::OnEraseBkgnd(pDC);
}

int CMagicBlockView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  在此添加您专用的创建代码
	setMode(true); // 设置运动模式

	switchPDC(new CClientDC(this));
	return 0;
}

void CMagicBlockView::OnDestroy()
{
	CView::OnDestroy();

	// TODO:  在此处添加消息处理程序代码
	wglMakeCurrent(NULL, NULL);
	if (cur) wglDeleteContext(cur);
}

void CMagicBlockView::OnInitialUpdate()
{
	CView::OnInitialUpdate();

	// TODO: 在此添加专用代码和/或调用基类
	glClearColor(1, 1, 1, 1);
}

void CMagicBlockView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);

	// TODO: 在此处添加消息处理程序代码
	glViewport(0, 0, cx, cy);
	// 设置投影矩阵(透视投影)  
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0, (GLfloat)cx / (GLfloat)cy, 1.0, 1000.0);

	resetEye();
}

BOOL CMagicBlockView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	dis -= 0.5f * zDelta / 120.0f;
	dis = std::min(std::max(dis, 15.0f), 100.0f);
	resetEye();
	return CView::OnMouseWheel(nFlags, zDelta, pt);
}

void CMagicBlockView::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (!onTrack)
	{
		TRACKMOUSEEVENT tme;
		tme.cbSize = sizeof(TRACKMOUSEEVENT);
		tme.dwFlags = TME_LEAVE;//要触发的消息
		tme.hwndTrack = this->m_hWnd;
		tme.dwHoverTime = 10;// 若不设此参数，则无法触发mouseHover

		if (::_TrackMouseEvent(&tme)) //MOUSELEAVE|MOUSEHOVER消息由此函数触发
		{
			onTrack = true;
		}
	}
	if (nFlags & MK_LBUTTON) {
		if (!lastPoint) {
			lastPoint = new CPoint(point);
		}
		else {
			eyeXZ += (point.x - lastPoint->x) / 500.0f;
			eyeY += (point.y - lastPoint->y) / 500.0f;
			eyeXZ = trimAngle(eyeXZ);
			eyeY = trimAngle(eyeY);
			*lastPoint = point;
			resetEye();
		}
	}
	CView::OnMouseMove(nFlags, point);
}

void CMagicBlockView::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	delete lastPoint;
	lastPoint = nullptr;
	CView::OnLButtonUp(nFlags, point);
}

void CMagicBlockView::OnMouseLeave()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	delete lastPoint;
	lastPoint = nullptr;
	onTrack = false;
	CView::OnMouseLeave();
}

void CMagicBlockView::OnRButtonDblClk(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	magic.autoSolve();
	CView::OnRButtonDblClk(nFlags, point);
}

void CMagicBlockView::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	switch (nIDEvent)
	{
	case LOGIC_TIMER:
		//simulation();
		magic.logic();
		Invalidate();
		break;
	case AUTO_TIMER:
		//magic.test();
		//magic.rotate(Side::up, { 0 }, rand() % 2 + 1);
		magic.rotate(Side(rand() % 6), { rand() % magic.N }, rand() % 2 + 1, 5);
		break;
	default:
		CView::OnTimer(nIDEvent);
		break;
	}
}

void CMagicBlockView::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	static const map<UINT, vector<MagicRotate>> rmp = {
		{'U', {magic.U, magic.Up, magic.Ut}},
		{'D', {magic.D, magic.Dp, magic.Dt}},
		{'F', {magic.F, magic.Fp, magic.Ft}},
		{'B', {magic.B, magic.Bp, magic.Bt}},
		{'L', {magic.L, magic.Lp, magic.Lt}},
		{'R', {magic.R, magic.Rp, magic.Rt}},
	};
	if (rmp.count(nChar)) {
		auto vc = rmp.find(nChar)->second;

	}
	CView::OnKeyUp(nChar, nRepCnt, nFlags);
}


void CMagicBlockView::resetEye() {
	eyePoint.x = dis * cos(eyeY) * cos(eyeXZ);
	eyePoint.y = dis * sin(eyeY);
	eyePoint.z = dis * cos(eyeY) * sin(eyeXZ);

	headDirection.x = -sin(eyeY) * cos(eyeXZ);
	headDirection.y = cos(eyeY);
	headDirection.z = -sin(eyeY) * sin(eyeXZ);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(
		eyePoint.x, eyePoint.y, eyePoint.z, // 眼睛本体坐标
		0.0, 0.0, 0.0, // 视角中心位置
		headDirection.x, headDirection.y, headDirection.z  // 头顶朝向
	);

	Invalidate();
}

bool CMagicBlockView::setPixelFormat()
{
	static PIXELFORMATDESCRIPTOR pfd =
	{
		sizeof(PIXELFORMATDESCRIPTOR), // 结构的大小  
		1, // 结构的版本  
		PFD_DRAW_TO_WINDOW | // 在窗口(而不是位图)中绘图  
		PFD_SUPPORT_OPENGL | // 支持在窗口中进行OpenGL调用  
		// PFD_STEREO_DONTCARE |
		PFD_DOUBLEBUFFER, // 双缓冲模式  
		PFD_TYPE_RGBA, // RGBA颜色模式  
		32, // 需要32位颜色  
		0, 0, 0, 0, 0, 0, // 不用于选择模式  
		0, 0, // 不用于选择模式  
		0, 0, 0, 0, 0, // 不用于选择模式  
		16, // 深度缓冲区的大小  
		0, // 在此不使用  
		0, // 在此不使用  
		0, // 在此不使用  
		0, // 在此不使用  
		0, 0, 0 // 在此不使用  
	};
	// 选择一种与pfd所描述的最匹配的像素格式  
	// 为设备环境设置像素格式  
	int pixelformat;
	pixelformat = ChoosePixelFormat(glPDC->GetSafeHdc(), &pfd);
	if (0 == pixelformat) return false;
	// 为设备环境设置像素格式  
	return SetPixelFormat(glPDC->GetSafeHdc(), pixelformat, &pfd);
}

// CMagicBlockView 诊断

#ifdef _DEBUG
void CMagicBlockView::AssertValid() const
{
	CView::AssertValid();
}

void CMagicBlockView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CMagicBlockDoc* CMagicBlockView::GetDocument() const // 非调试版本是内联的
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CMagicBlockDoc)));
	return (CMagicBlockDoc*)m_pDocument;
}
#endif //_DEBUG


// CMagicBlockView 消息处理程序
