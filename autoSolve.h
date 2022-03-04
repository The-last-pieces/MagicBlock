#pragma once
#include "utils.h"

// 可搜索最优解的魔方
class CanSolve {
public:
#pragma pack(push) 
#pragma pack(1)
	using MagicMeta = union TwoDim {
		struct mt
		{
			struct blocks
			{
				unsigned char
					fx : 1, fy : 1, fz : 1,// 真实坐标
					x : 1, y : 1, z : 1; // 原始坐标
				char sideMap[6]; // 6个方向的真实方向
			} bs[8];
			unsigned char ops[256] = {};
			unsigned char index = 0;
		}m;
		long long comp[7];
	};
	const int N = sizeof(MagicMeta);
#pragma pack(pop)
protected:
	// 执行一个操作
	virtual void instant(const MagicRotate&) = 0;
	// 获取第一阶段的操作空间
	virtual const vector<MagicRotate> getFirstActions() const = 0;
	// 获取第二阶段的操作空间
	virtual const vector<MagicRotate> getSecondActions() const = 0;
	// 判断是否处于第一阶段目标状态
	virtual bool judgeFirstStatus() const = 0;
	// 判断是否处于第二阶段目标状态
	virtual bool judgeSecondStatus() const = 0;
	// 压缩状态
	virtual const MagicMeta zipStatus() = 0;
	// 加载状态
	virtual void loadStatus(const MagicMeta&) = 0;
public:
	// 求最优解
	void solve() {
		resultHis.clear();
		solveWithState(&CanSolve::getFirstActions, &CanSolve::judgeFirstStatus);
		solveWithState(&CanSolve::getSecondActions, &CanSolve::judgeSecondStatus);
	}
protected:
	deque<MagicRotate> resultHis;
private:
	const array<long long, 8> getHash(const MagicMeta& meta) {
		array<long long, 8> ret;
		for (int i = 0; i < 8; ++i) ret[i] = meta.comp[i];
		return ret;
	}
	// 阶段性求解
	void solveWithState(
		const vector<MagicRotate>(CanSolve::* actionsCallback)() const,
		bool (CanSolve::* howJudge)() const
	) {
		map<array<long long, 8>, bool> hmp;
		queue<MagicMeta> q;
		auto status = zipStatus();
		q.push(status); hmp[getHash(status)] = true;
		auto actions = (this->*actionsCallback)();
		while (!q.empty()) {
			auto& node = q.front(); q.pop();
			loadStatus(node);
			if ((this->*howJudge)()) {
				for (int i = 0; i < node.m.index; ++i) {
					resultHis.push_back(actions[node.m.ops[i]]);
				}
				return;
			}
			for (auto& ac : actions) {
				instant(ac);
				auto status = zipStatus();
				status.m.ops[status.m.index++] = &ac - &actions.front();
				auto hash = getHash(status);
				if (!hmp.count(hash)) {
					q.push(status);
					hmp[hash] = true;
				}
				instant(-ac);
			}
		}
	}
};

// 二维魔方
class TwoDimBlock : public CanSolve {
public:
	static const int N = 2;
	static const int fps = 20;
private:
	// left->right x
	//  bottom->top y
	//   back->front z
	SixSideBlock blocks[N][N][N];
	Vector3D trimPosition[N][N][N];

	deque<MagicRotate> actions;
	deque<MagicRotate> his;

	const GColor front = GColor::make(255, 0, 0);		//前红
	const GColor back = GColor::make(255, 97, 0);		//后橙
	const GColor up = GColor::make(255, 255, 0);		//上黄
	const GColor down = GColor::make(200, 200, 200);	//下白
	const GColor left = GColor::make(0, 0, 255);		//左蓝
	const GColor right = GColor::make(0, 255, 0);		//右绿

	// 前红、后橙、上黄、下白、左蓝、右绿。
	const map<Side, GColor> colorMap = {
		{Side::front, front},
		{Side::back, back},
		{Side::up, up},
		{Side::down, down},
		{Side::left, left},
		{Side::right, right},
	};

private:
	bool atLay(int x, int y, int z, Side lay, int offset) const {
		assert(0 <= offset && offset <= N - 1);
		switch (lay)
		{
		case Side::up:
			return y == N - 1 - offset;
		case Side::down:
			return y == offset;
		case Side::front:
			return z == N - 1 - offset;
		case Side::back:
			return z == offset;
		case Side::left:
			return x == offset;
		case Side::right:
			return x == N - 1 - offset;
		default:
			return false;
		}
	}

	const vector<SixSideBlock*> getSide(Side lay, int offset = 0) {
		vector<SixSideBlock*> ret;
		for (int x = 0; x < N; ++x) {
			for (int y = 0; y < N; ++y) {
				for (int z = 0; z < N; ++z) {
					if (atLay(blocks[x][y][z].x, blocks[x][y][z].y, blocks[x][y][z].z, lay, offset)) {
						ret.push_back(&blocks[x][y][z]);
					}
				}
			}
		}
		assert(ret.size() == N * N);
		return ret;
	}
public:
	TwoDimBlock() {
		const float radius = 1.0f;
		const float offset = -N * radius / 2 + radius / 2;
		// 初始化block位置
		for (int x = 0; x < N; ++x) {
			for (int y = 0; y < N; ++y) {
				for (int z = 0; z < N; ++z) {
					Vector3D pos = { x * radius + offset, y * radius + offset, z * radius + offset };
					blocks[x][y][z].initWith(pos, radius);
					blocks[x][y][z].x = x;
					blocks[x][y][z].y = y;
					blocks[x][y][z].z = z;
					trimPosition[x][y][z] = pos;
				}
			}
		}
		// 初始化颜色
		for (auto& kv : colorMap) {
			for (auto ptr : getSide(kv.first)) {
				ptr->initSideColor(kv.first, kv.second);
			}
		}
	}

	void show() const {
		// 调用所有子块的show函数
		for (int x = 0; x < N; ++x) {
			for (int y = 0; y < N; ++y) {
				for (int z = 0; z < N; ++z) {
					blocks[x][y][z].show();
				}
			}
		}
	}

	void logic() {
		// 调用子块的logic函数
		/*for (int x = 0; x < N; ++x) {
			for (int y = 0; y < N; ++y) {
				for (int z = 0; z < N; ++z) {
					blocks[x][y][z].isBad = !isRightBlock(&blocks[x][y][z]);
				}
			}
		}*/
		// 执行旋转行为
		if (!actions.empty()) {
			auto& top = actions.front();
			if (top.times) {
				for (auto offset : top.offset) {
					for (auto ptr : getSide(top.lay, offset)) {
						ptr->editAngle(top.eulerOnce.x, top.eulerOnce.y, top.eulerOnce.z);
					}
				}
				--top.times;
			}
			if (top.times <= 0) {
				for (auto offset : top.offset) {
					for (auto ptr : getSide(top.lay, offset)) {
						ptr->rotateXYZ(top, N); // 更新xyz坐标
					}
				}
				actions.pop_front();
			}
		}
	}

	void autoSolve() {
		if (actions.size()) return;
		auto copy = *this;
		copy.solve();
		actions = copy.resultHis;
	}

public:
	// 旋转某个面times次,正为顺时针
	void rotate(Side lay, const vector<int>& offset = { 0 }, int times = 1, int frame = fps) {
		MagicRotate action(lay, times, frame, offset);
		actions.push_back(action);
		his.push_front(-action);
	}

	// 复位
	void refresh() {
		if (actions.empty()) {
			actions = his;
			his.clear();
		}
	}

	static const MagicRotate make(Side lay, const vector<int>& offset = { 0 }, bool isNeg = false, bool isTwo = false) {
		return MagicRotate(lay, (isTwo ? 2 : 1) * (isNeg ? -1 : 1), fps, offset);
	}

	static const vector<int> getN(int n) {
		vector<int> arr(n);
		for (int i = 0; i < n; ++i) arr[i] = i;;
		return arr;
	}

	const vector<int> TS = getN(N); // 所有面
	// p为逆，2为旋转180度，t为整个面旋转
	const MagicRotate
		F = make(Side::front),
		Fp = -F, F2 = F * 2,
		Ft = make(Side::front, TS), Ft2 = Ft * 2,
		B = make(Side::back),
		Bp = -B, B2 = B * 2,
		Bt = make(Side::back, TS), Bt2 = Bt * 2,
		R = make(Side::right),
		Rp = -R, R2 = R * 2,
		Rt = make(Side::right, TS), Rt2 = Rt * 2,
		L = make(Side::left),
		Lp = -L, L2 = L * 2,
		Lt = make(Side::left, TS), Lt2 = Lt * 2,
		U = make(Side::up),
		Up = -U, U2 = U * 2,
		Ut = make(Side::up, TS), Ut2 = Ut * 2,
		D = make(Side::down),
		Dp = -D, D2 = D * 2,
		Dt = make(Side::down, TS), Dt2 = Dt * 2;
private:
	const vector<SixSideBlock*> getAllBlocks() const {
		SixSideBlock* arr = (SixSideBlock*)blocks;
		vector<SixSideBlock*> v;
		for (int i = 0; i < N * N * N; ++i)v.push_back(&arr[i]);
		return v;
	}
private:
	// 执行一个操作
	void instant(const MagicRotate& action) final {
		auto fac = action.one();
		actions.push_back(fac);
		logic();
	}
	// 获取第一阶段的操作空间
	const vector<MagicRotate> getFirstActions() const final {
		return {
			F ,Fp ,F2,
			B ,Bp ,B2,
			R ,Rp ,R2,
			L ,Lp ,L2,
			U ,Up ,U2,
			D ,Dp ,D2,
		};
	}
	// 获取第二阶段的操作空间
	const vector<MagicRotate> getSecondActions() const final {
		return {};
	}
	// 判断是否处于第一阶段目标状态
	bool judgeFirstStatus() const final {
		vector<SixSideBlock*> v = getAllBlocks();
		return std::all_of(v.begin(), v.end(), std::bind(&TwoDimBlock::isRightBlock, this, std::placeholders::_1));
	}
	// 判断是否处于第二阶段目标状态
	bool judgeSecondStatus() const final {
		return true;
	}
	// 压缩状态
	const MagicMeta zipStatus() final {
		MagicMeta meta = { {} };
		for (int x = 0; x < N; ++x) {
			for (int y = 0; y < N; ++y) {
				for (int z = 0; z < N; ++z) {
					auto ptr = &blocks[x][y][z];
					auto& ref = meta.m.bs[x * 4 + y * 2 + z];
					ref.x = x; ref.y = y; ref.z = z;
					ref.fx = ptr->x; ref.fy = ptr->y; ref.fz = ptr->z;
					for (int i = 0; i < 6; ++i) {
						ref.sideMap[i] =
							(ptr->getColorOf(Side(i)) == GColor::black() ? -1 : 1) *
							(1 + char(ptr->getSideOf(Side(i))));
					}
				}
			}
		}
		return meta;
	}
	// 加载状态
	void loadStatus(const MagicMeta& meta) final {
		for (int x = 0; x < N; ++x) {
			for (int y = 0; y < N; ++y) {
				for (int z = 0; z < N; ++z) {
					auto ptr = &blocks[x][y][z];
					auto& ref = meta.m.bs[x * 4 + y * 2 + z];
					ptr->x = ref.fx; ptr->y = ref.fy; ptr->y = ref.fy;
					for (int i = 0; i < 6; ++i) {
						if (ref.sideMap[i] < 0) {
							ptr->initSideColor(Side(i), GColor::black());
						}
						else {
							ptr->initSideColor(Side(i), colorMap.find(Side(ref.sideMap[i] - 1))->second);
						}
						ptr->initSide(Side(i), Side(std::abs(ref.sideMap[i]) - 1));
					}
				}
			}
		}
	}
	void instant(const vector<MagicRotate>& actions, int repeat = 1) {
		for (int i = 0; i < repeat; ++i) {
			for (auto& a : actions) {
				instant(a);
			}
		}
	}
private:
	SixSideBlock* find(int x, int y, int z) {
		SixSideBlock* arr = (SixSideBlock*)blocks;
		return std::find_if(arr, arr + N * N * N, [=](const SixSideBlock& obj)->bool {
			return obj.x == x && obj.y == y && obj.z == z;
			});
	}

	SixSideBlock* find(const BlockPos& pos) {
		return find(pos.x, pos.y, pos.z);
	}

	SixSideBlock* at(const BlockPos& pos) {
		return &blocks[pos.x][pos.y][pos.z];
	}

	const vector<SixSideBlock*> at(const vector<BlockPos>& pos) {
		int len = int(pos.size());
		vector<SixSideBlock*> ret(len);
		for (int i = 0; i < len; ++i) {
			ret[i] = at(pos[i]);
		}
		return ret;
	}

	const vector<BlockPos> getEdges(const GColor& has) {
		vector<BlockPos> ret;
		for (int x = 0; x < N; ++x) {
			for (int y = 0; y < N; ++y) {
				for (int z = 0; z < N; ++z) {
					auto ptr = at({ x, y, z });
					if ((isAt(ptr, Side::left, 1) || isAt(ptr, Side::up, 1) || isAt(ptr, Side::front, 1))
						&& blocks[x][y][z].hasColor(has) && blocks[x][y][z].getAllColor().size() == 2) {
						ret.push_back({ x,y,z });
					}
				}
			}
		}
		return ret;
	}

	const vector<BlockPos> getCorners(const GColor& has) {
		vector<BlockPos> ret;
		for (int x = 0; x < N; ++x) {
			for (int y = 0; y < N; ++y) {
				for (int z = 0; z < N; ++z) {
					if ((x == 0 || x == N - 1) && (y == 0 || y == N - 1) && (z == 0 || z == N - 1)
						&& blocks[x][y][z].hasColor(has)) {
						ret.push_back({ x,y,z });
					}
				}
			}
		}
		return ret;
	}

	bool isAt(SixSideBlock* ptr, Side side, int offset = 0) const {
		return atLay(ptr->x, ptr->y, ptr->z, side, offset);
	}

	// 判断其颜色是否与其所在面的中心点同色
	bool isRightBlock(SixSideBlock* ptr) const {
		const auto all = ptr->getAllColor();
		vector<SixSideBlock*> v = getAllBlocks();
		for (auto& c : all) {
			Side side = ptr->getSideOf(c);
			if (!(colorMap.find(side)->second == c)) {
				return false;
			}
		}
		return true;
	}
private:
	const vector<int> getLayVector(Side lay, int offset) {
		int x = 0, y = 0, z = 0, w = 0;
		switch (lay)
		{
		case Side::up:
			y = 1; w = N - 1 - offset;
			break;
		case Side::down:
			y = 1; w = offset;
			break;
		case Side::front:
			z = 1; w = N - 1 - offset;
			break;
		case Side::back:
			z = 1; w = offset;
			break;
		case Side::left:
			x = 1; w = offset;
			break;
		case Side::right:
			x = 1; w = N - 1 - offset;
			break;
		default:
			return {};
		}
		return { x,y,z,w };
	}
	bool isSameLay(const MagicRotate& a, const MagicRotate& b) {
		if (a.offset == b.offset && a.lay == b.lay) return true;
		if (a.offset == TS && b.offset == TS)
			return getLayVector(a.lay, 0) == getLayVector(b.lay, N - 1);
		if (a.offset.size() != 1 || b.offset.size() != 1)return false;
		else return getLayVector(a.lay, a.offset[0]) == getLayVector(b.lay, b.offset[0]);
	}

	const MagicRotate merge(const MagicRotate& a, const MagicRotate& b) {
		if (a.lay == b.lay) {
			return a + b;
		}
		else {
			return a + (-b);
		}
	}

	const deque<MagicRotate> zipRotate(const deque<MagicRotate>& arr) {
		deque<MagicRotate> ret;
		int len = arr.size();
		MagicRotate* ptr = nullptr;
		for (int i = 0; i < len; ++i) {
			if (!ptr) {
				ptr = new MagicRotate(arr[i]);
			}
			else {
				if (isSameLay(*ptr, arr[i])) {
					*ptr = merge(*ptr, arr[i]);
				}
				else {
					if (ptr->scale) ret.push_back(*ptr);
					delete ptr;
					ptr = nullptr;
					--i;
				}
			}
		}
		if (ptr) {
			if (ptr->scale) ret.push_back(*ptr);
			delete ptr;
			ptr = nullptr;
		}
		return ret;
	}
};