#pragma once
#include "pch.h"
#include "utils.h"

// 六面体
class SixSideBlock : public GLPaintObject {
	const float PI = 180;
public:
	int x, y, z; // 在魔方的离散坐标系中的坐标
	bool isBad = false; // 朝向是否正确
private:
	// 中心坐标 , 相对世界坐标
	Vector3D position = {};
	// 面由哪几个点组成 
	const int sideMap[6][4] = {
		 0, 2, 3, 1, // back
		 0, 4, 6, 2, // left
		 0, 1, 5, 4, // bottom
		 4, 5, 7, 6, // front
		 1, 3, 7, 5, // right
		 2, 6, 7, 3, // top
	};

	GColor colors[6]; // 6个面的颜色
	GColor factColors[6]; // 真实视角中6个面的颜色
	Side factSide[6] = { Side::back,Side::left,Side::down,Side::front,Side::right,Side::up };
private:
	float width; // 边长
	// 八个顶点的位置,相对自身中心
	Vector3D vertexs[8];
	Vector3D rotatedVertexs[8];
	Vector3D rotatedPosition;
private:
	void transfrom(const glm::mat4& matrix) {
		for (int i = 0; i < 8; ++i) {
			rotatedVertexs[i] = rotatedVertexs[i] * matrix;
		}
		rotatedPosition = rotatedPosition * matrix;
	}

	void rotateColor(const MagicRotate& rotate) {
		if (rotate.scale != 1) {
			MagicRotate copy = rotate;
			copy.scale = 1;
			int times = rotate.scale < 0 ? (-3 * rotate.scale) % 4 : rotate.scale % 4;
			for (int i = 0; i < times; ++i) {
				rotateColor(copy);
			}
			return;
		}
		// 旋转视角颜色
		roll(rotate, factColors);
		roll(rotate, factSide);
	}

	template<typename T>
	void roll(const MagicRotate& rotate, T arr[6]) {
		static const map<Side, vector<Side>> smp = {
			{Side::back, {Side::up,Side::left,Side::down,Side::right}},
			{Side::front, {Side::up,Side::right,Side::down,Side::left}},

			{Side::left, {Side::up,Side::front,Side::down,Side::back}},
			{Side::right, {Side::up,Side::back,Side::down,Side::front}},

			{Side::down, {Side::front,Side::right,Side::back,Side::left}},
			{Side::up, {Side::front,Side::left,Side::back,Side::right}},
		};
		auto& toSwap = smp.find(rotate.lay)->second;
		int index = 0, next = getNext(index, rotate.scale);
		T last = arr[int(toSwap[next])];
		for (int i = 3; i >= 0; --i) {
			index = i;
			next = getNext(index, rotate.scale);
			if (i <= 0) break;
			arr[int(toSwap[index])] = arr[int(toSwap[next])];
		}
		arr[int(toSwap[0])] = last;
	}

	int getNext(int now, int offset) {
		int next = now - offset;
		next = next < 0 ? (-next * 3) % 4 : next % 4;
		return next;
	}

	//float trimAngle(float angle) {
	//	// 0,1,2,3 倍的w/2 - w
	//	float w = PI;
	//	while (angle < -w) angle += 2 * w;
	//	int scale = int(std::roundf((angle + w) * 2 / w));
	//	return (scale % 4) * w / 2 - w;
	//}
public:
	SixSideBlock() {

	}

	void initWith(const Vector3D& pos = {}, float radius = 1.0f) {
		position = {};
		width = radius;
		// 默认全黑
		for (int i = 0; i < 6; ++i) factColors[i] = colors[i] = GColor::black();
		// 初始化顶角
		for (int i = 0; i < 8; ++i) rotatedVertexs[i] = vertexs[i] = {
			 (!!(i & (1 << 0)) - width / 2),
			 (!!(i & (1 << 1)) - width / 2),
			 (!!(i & (1 << 2)) - width / 2),
		};
		rotatedPosition = position;
		setPosition(pos);
	}

	void setPosition(const Vector3D& pos) {
		setPosition(pos.x, pos.y, pos.z);
	}

	void setPosition(float x, float y, float z) {
		// 更新中心坐标
		glm::mat4 trans(1.0f);
		trans = glm::translate(trans, glm::vec3(x - position.x, y - position.y, z - position.z));
		transfrom(trans);

		position = { x,y,z };
		// updateFactVertexs();
	}

	void editAngle(float x, float y, float z, bool set = false) {

		// 旋转顺序为yxz
		glm::mat4 trans = glm::mat4(1.0f);
		trans = glm::rotate(trans, glm::radians(y), glm::vec3(0.0f, 1.0f, 0.0f));
		trans = glm::rotate(trans, glm::radians(x), glm::vec3(1.0f, 0.0f, 0.0f));
		trans = glm::rotate(trans, glm::radians(z), glm::vec3(0.0f, 0.0f, 1.0f));

		transfrom(trans);
	}

	void show() const {
		// 绘制6个面
		glFrontFace(GL_CCW);//逆时针
		glCullFace(GL_BACK);
		glEnable(GL_CULL_FACE);
		// glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		for (int i = 0; i < 6; ++i) {
			if (isBad) {
				setColor(colors[i].r / 2, colors[i].g / 2, colors[i].b / 2);
			}
			else {
				setColor(colors[i].r, colors[i].g, colors[i].b);
			}
			rectangle(
				rotatedVertexs[sideMap[i][0]],
				rotatedVertexs[sideMap[i][1]],
				rotatedVertexs[sideMap[i][2]],
				rotatedVertexs[sideMap[i][3]]
			);
		}
		// 连接所有棱
		glEnable(GL_LINE_SMOOTH);
		setColor(255.0f, 0.0f, 255.0f);
		setWidth(2.0f);
		for (int i = 0; i < 6; ++i) {
			loopLine({
				rotatedVertexs[sideMap[i][0]],
				rotatedVertexs[sideMap[i][1]],
				rotatedVertexs[sideMap[i][2]],
				rotatedVertexs[sideMap[i][3]]
				});
		}
	}

	void initSideColor(Side side, const GColor& color) {
		factColors[int(side)] = colors[int(side)] = color;
	}

	void initSide(Side side, Side fac) {
		factSide[int(side)] = fac;
	}

	void logic() {

	}

	float range(const Vector3D& pos) {
		return rotatedPosition.range(pos);
	}

	/*void trim() {
		eulerAngle.x = trimAngle(eulerAngle.x);
		eulerAngle.y = trimAngle(eulerAngle.y);
		eulerAngle.z = trimAngle(eulerAngle.z);

		updateFactVertexs();
	}*/

	const BlockPos predictXYZ(const MagicRotate& rotate, const int N) const {
		glm::mat4 trans = glm::mat4(1.0f);
		trans = glm::rotate(trans, glm::radians(rotate.eulerTotal.y), glm::vec3(0.0f, 1.0f, 0.0f));
		trans = glm::rotate(trans, glm::radians(rotate.eulerTotal.x), glm::vec3(1.0f, 0.0f, 0.0f));
		trans = glm::rotate(trans, glm::radians(rotate.eulerTotal.z), glm::vec3(0.0f, 0.0f, 1.0f));

		const float offset = (1 - N) / 2.0f;

		Vector3D xyz = { float(x) + offset,float(y) + offset,float(z) + offset };
		xyz = xyz * trans;

		return { std::lroundf(xyz.x - offset),std::lroundf(xyz.y - offset),std::lroundf(xyz.z - offset) };
	}

	const BlockPos offset(int dx, int dy, int dz) {
		return { x + dx,y + dy,z + dz };
	}

	const SixSideBlock virtualRotate(const MagicRotate& rotate, const int N) const {
		SixSideBlock copy = *this;
		copy.rotateXYZ(rotate, N);
		return copy;
	}

	void rotateXYZ(const MagicRotate& rotate, const int N) {
		// 旋转离散坐标
		auto pos = predictXYZ(rotate, N);
		x = pos.x; y = pos.y; z = pos.z;

		rotateColor(rotate);
	}

	Side getSideOf(const GColor& toFind) {
		for (int i = 0; i < 6; ++i) {
			if (factColors[i] == toFind) {
				return Side(i);
			}
		}
		assert(false);
		return Side::front;
	}
	Side getSideOf(Side side) {
		for (int i = 0; i < 6; ++i) {
			if (factSide[i] == side) {
				return Side(i);
			}
		}
		assert(false);
		return Side::front;
	}
	const GColor getColorOf(Side side) {
		return factColors[int(side)];
	}

	// 确保为中心
	const GColor getColor() {
		for (int i = 0; i < 6; ++i) {
			if (!(colors[i] == GColor::make(0, 0, 0))) {
				return colors[i];
			}
		}
		assert(false);
		return {};
	}
	// 确保为棱
	const GColor getAnother(const GColor & not) {
		for (int i = 0; i < 6; ++i) {
			if (!(colors[i] == not || colors[i] == GColor::make(0, 0, 0))) {
				return colors[i];
			}
		}
		assert(false);
		return {};
	}

	bool hasColor(const GColor& has) {
		for (int i = 0; i < 6; ++i) {
			if (colors[i] == has) {
				return true;
			}
		}
		return false;
	}
	bool isCenter() {
		int ct = 0;
		for (int i = 0; i < 6; ++i) {
			if (!(colors[i] == GColor::make(0, 0, 0))) {
				++ct;
			}
		}
		return ct == 1;
	}
	const vector<GColor> getAllColor() {
		vector<GColor> ret;
		for (int i = 0; i < 6; ++i) {
			if (!(colors[i] == GColor::make(0, 0, 0))) {
				ret.push_back(colors[i]);
			}
		}
		return ret;
	}
};

// 3阶魔方
class MagicBlock {
public:
	static const int N = 3;
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

private:
	bool atLay(int x, int y, int z, Side lay, int offset) {
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
	MagicBlock() {
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
		// 前红、后橙、上黄、下白、左蓝、右绿。
		const map<Side, GColor> colorMap = {
			{Side::front, front},
			{Side::back, back},
			{Side::up, up},
			{Side::down, down},
			{Side::left, left},
			{Side::right, right},
		};
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
		Dt = make(Side::down, TS), Dt2 = Dt * 2,
		M = make(Side::left, { 1 }),
		Mp = -M, M2 = M * 2;

	// 自动求解相关
	void autoSolve() {
		if (actions.size() || N != 3) return;
		MagicBlock& copy = *(new MagicBlock(*this));
		copy.actions = copy.his = {};
		// 第一步
		copy.resetYellow();
		// 第二步
		copy.resetWhiteEdges();
		// 第三步
		copy.resetWhiteCorner();
		// 第四步
		copy.resetCenterEdges();
		// 第五步
		copy.resetYellowEdges();
		// 第六步
		copy.resetYellowCornersUpSide();
		// 第七步
		copy.resetYellowCorner();
		// 第八步
		copy.resetRestEdges();
		// 获取解法
		copyResult(copy);
	}
private:
	// 将黄色中心块置于up面
	void resetYellow() {
		auto& yellow = blocks[1][2][1];
		Side side = yellow.getSideOf(up);
		switch (side)
		{
		case Side::up:
			return;
		case Side::down:
			instant(Rt2);

			break;
		case Side::front:
			instant(Rt);
			break;
		case Side::back:
			instant(-Rt);
			break;
		case Side::left:
			instant(Ft);
			break;
		case Side::right:
			instant(-Ft);
			break;
		default:
			break;
		}
	}

	// 将白色棱置于正确位置
	void resetWhiteEdges() {
		auto whites = at(getEdges(down));
		std::function<bool(SixSideBlock*)> judge = std::bind(&MagicBlock::isRightBlock, this, std::placeholders::_1);
		if (std::all_of(whites.begin(), whites.end(), judge)) {
			return;
		}
		// 形成顶层反十字
		for (auto ptr : whites) {
			dealWhiteEdges(ptr);
		}
		// 将白色棱置于down面,且第二面对齐
		dealBottomEdges();
	}

	void dealWhiteEdges(SixSideBlock* ptr) {
		// 处理中间层
		if (ptr->y == 1) {
			// 转到前面来
			exeUtilColorOnSide(Ut, ptr, down, Side::front);
			if (isAt(ptr, Side::left)) {
				while (find(ptr->predictXYZ(Lp, N))->getColorOf(Side::up) == down) {
					instant(U);
				}
				instant(Lp);
			}
			if (isAt(ptr, Side::right)) {
				while (find(ptr->predictXYZ(R, N))->getColorOf(Side::up) == down) {
					instant(U);
				}
				instant(R);
			}
		}
		// 处理顶层
		else if (ptr->y == 2) {
			if (ptr->getSideOf(down) != Side::up) {
				exeUtilOnSide(Ut, ptr, Side::front);
				instant(F); // 转移为中间层
				dealWhiteEdges(ptr);
			}
		}
		// 处理底层
		else if (ptr->y == 0) {
			if (ptr->getSideOf(down) != Side::down) {
				exeUtilOnSide(Ut, ptr, Side::front);
				while (find(ptr->predictXYZ(F2, N))->getColorOf(Side::up) == down) {
					instant(U);
				}
				instant(F); // 转移为中间层
				dealWhiteEdges(ptr);
			}
			else {
				exeUtilOnSide(Ut, ptr, Side::front);
				while (find(ptr->predictXYZ(F2, N))->getColorOf(Side::up) == down) {
					instant(U);
				}
				instant(F2);
			}
		}
	}

	void dealBottomEdges() {
		auto whites = at(getEdges(down));
		auto centers = at({ getCenter(front),getCenter(left),getCenter(back),getCenter(right) });
		for (int i = 0; i < 4; ++i) {
			for (auto ptr : whites) {
				auto other = ptr->getAnother(down);
				if (find(ptr->offset(0, -1, 0))->getColor() == other) {
					dealOneBottom(ptr, other);
				}
			}
			instant(U);
		}
	}

	void dealOneBottom(SixSideBlock* ptr, const GColor& other) {
		switch (find(ptr->offset(0, -1, 0))->getSideOf(other))
		{
		case Side::left:
			instant(L2);
			break;
		case Side::right:
			instant(R2);
			break;
		case Side::front:
			instant(F2);
			break;
		case Side::back:
			instant(B2);
			break;
		default:
			break;
		}
	}

	// 将白色角置于正确位置
	void resetWhiteCorner() {
		auto whites = at(getCorners(down));
		std::sort(whites.begin(), whites.end(), [](SixSideBlock* a, SixSideBlock* b)->bool {
			return a->y < b->y;
			});
		for (int i = 0; i < 4; ++i) {
			bool has = false;
			{
				for (int j = 0; j < 4; ++j) {
					if (!isRightBlock(whites[j]) && whites[j]->y != 0) {
						dealWhiteCorners(whites[j]);
						has = true;
						break;
					}
				}
			}
			if (!has) {
				has = false;
				for (int j = 0; j < 4; ++j) {
					if (!isRightBlock(whites[j])) {
						dealWhiteCorners(whites[j]);
						has = true;
						break;
					}
				}
			}
			if (!has) {
				return;
			}
		}
	}

	void dealWhiteCorners(SixSideBlock* ptr) {
		const auto Down = D.off({ 0,1 });
		// 处理底层
		if (ptr->y == 0) {
			if (!isRightBlock(ptr)) {
				exeUtilOnSide(Ut, ptr, Side::front);
				if (isAt(ptr, Side::left)) {
					instant({ F,U,Fp });
				}
				if (isAt(ptr, Side::right)) {
					instant({ Fp,Up,F });
				}
				dealWhiteCorners(ptr); // 转为顶层处理
			}
		}
		// 处理顶层角块
		else if (ptr->y == 2) {
			// 白色面为top
			if (ptr->getSideOf(down) == Side::up) {
				exeUtilOnSide(Ut, ptr, Side::front);
				if (!isAt(ptr, Side::right)) instant(Up);
				auto left = at(getCenter(ptr->getColorOf(Side::right)));
				exeUtilOnSide(Down, left, Side::front);
				instant({ R,U2,Rp,Up,R,U,Rp });
			}
			// 白色面在正面
			else {
				exeUtilColorOnSide(U, ptr, down, Side::front);
				if (isAt(ptr, Side::left)) {
					auto left = at(getCenter(ptr->getColorOf(Side::left)));
					exeUtilOnSide(Down, left, Side::left);
					instant({ F,U,Fp });
				}
				if (isAt(ptr, Side::right)) {
					auto right = at(getCenter(ptr->getColorOf(Side::right)));
					exeUtilOnSide(Down, right, Side::right);
					instant({ Fp,Up,F });
				}
			}
		}
	}

	// 将中间一层的棱置于正确位置
	void resetCenterEdges() {
		vector<SixSideBlock*> centers = {
			&blocks[0][1][0],
			&blocks[2][1][2],
			&blocks[0][1][2],
			&blocks[2][1][0],
		};
		std::sort(centers.begin(), centers.end(), [](SixSideBlock* a, SixSideBlock* b)->bool {
			return a->y < b->y;
			});
		for (auto ptr : centers) {
			dealCenterEdges(ptr, centers);
		}
	}

	void dealCenterEdges(SixSideBlock* ptr, const vector<SixSideBlock*>& all) {
		// 处理中层
		if (ptr->y == 1) {
			if (!isRightBlock(ptr)) {
				exeUtilOnSide(Ut, ptr, Side::front);
				if (isAt(ptr, Side::left)) instant(Dt);
				while (std::find(all.begin(), all.end(), find(ptr->predictXYZ(R, N))) != all.end())
					instant(U);
				instant({ U,R,Up,Rp,Up,Fp,U,F });
				dealCenterEdges(ptr, all); // 转顶层
			}
		}
		// 处理顶层
		else if (ptr->y == 2) {
			auto notTrim = ptr->getColorOf(Side::up);
			auto toTrim = ptr->getAnother(notTrim);
			auto nt = at(getCenter(notTrim)), tt = at(getCenter(toTrim));
			exeUtilOnSide(Ut, tt, Side::front);
			if (isAt(nt, Side::left)) {
				instant(Dt);
				exeUtilOnSide(U, ptr, Side::right);
				instant({ Up,Fp,U,F,U,R,Up,Rp });
			}
			else {
				exeUtilOnSide(U, ptr, Side::front);
				instant({ U,R,Up,Rp,Up,Fp,U,F });
			}
		}
	}

	// 将黄色棱置于正确位置
	void resetYellowEdges() {
		auto up = this->up;
		auto yellow = at(getEdges(up));
		vector<SixSideBlock*> upArr;
		std::copy_if(yellow.begin(), yellow.end(), std::back_inserter(upArr), [up](SixSideBlock* ptr)->bool {
			return ptr->getSideOf(up) == Side::up;
			});

		if (upArr.size() == 4) {
			return;
		}
		const vector<MagicRotate> method = { F,R,U,Rp,Up,Fp };
		if (upArr.size() == 0) {
			instant(method, 2);
			instant(U);
			instant(method);
		}
		else if (upArr.size() == 2) {
			if (find(upArr[0]->predictXYZ(U2, N)) == upArr[1]) {
				exeUtilOnSide(U, upArr[0], Side::left);
				instant(method);
			}
			else {
				exeUtilOnSide(U, upArr[0], Side::left);
				if (isAt(upArr[1], Side::front)) {
					instant(U);
				}
				instant(method, 2);
			}
		}
	}

	// 将黄色角的up面置于正确位置
	void resetYellowCornersUpSide() {
		auto up = this->up;
		auto yellow = at(getCorners(up));
		static const vector<MagicRotate> m1 = { Rp,U2,R,U,Rp,U,R };
		static const vector<MagicRotate> m2 = { Up,R,U2,Rp,Up,R,Up,Rp };
		vector<SixSideBlock*> upArr, notArr;
		std::copy_if(yellow.begin(), yellow.end(), std::back_inserter(upArr),
			[up](SixSideBlock* ptr)->bool {
				return ptr->getSideOf(up) == Side::up;
			}
		);
		std::copy_if(yellow.begin(), yellow.end(), std::back_inserter(notArr),
			[up](SixSideBlock* ptr)->bool {
				return ptr->getSideOf(up) != Side::up;
			}
		);
		if (upArr.size() == 4) return;
		if (upArr.size() == 1) {
			auto only = upArr[0];
			exeUtilOnSide(U, only, Side::front);
			if (isAt(only, Side::left)) instant(Up);
			if (find(only->predictXYZ(U, N))->getSideOf(up) == Side::left) {
				instant(m1);
			}
			else {
				instant(m2);
			}
		}
		else if (upArr.size() == 2) {
			auto group = groupByColorSide(notArr, up);
			if (group.size() == 1) {
				exeUtilColorOnSide(U, notArr[0], up, Side::front);
				instant(m2);
				instant(U2);
				instant(m1);
			}
			else if (group.size() == 2) {
				if (find(notArr[0]->predictXYZ(U2, N)) == notArr[1]) {
					exeUtilColorOnSide(U, notArr[0], up, Side::right);
					if (notArr[0]->getSideOf(up) != Side::back) instant(Up);
					instant(m1);
					instant(m2);
				}
				else {
					exeUtilOnSide(U, notArr[0], Side::right);
					if (!isAt(notArr[1], Side::right)) instant(Up);
					instant(m2);
					instant(m1);
				}
			}
		}
		else if (upArr.size() == 0) {
			auto group = groupByColorSide(notArr, up);
			if (group.size() == 2) {
				if (group.count(Side::left)) instant(U);
				instant(m1, 2);
			}
			else if (group.size() == 3) {
				auto& two = std::find_if(group.begin(), group.end(),
					[](const decltype(*group.begin())& val)->bool {
						return val.second.size() == 2;
					}
				)->second;
				exeUtilColorOnSide(U, two[0], up, Side::front);
				instant(m1);
				instant(Up);
				instant(m1);
			}
		}
	}

	const map<Side, vector<SixSideBlock*>> groupByColorSide(const vector<SixSideBlock*>& arr, const GColor& color) {
		map<Side, vector<SixSideBlock*>> ret;
		for (auto& val : arr) {
			ret[val->getSideOf(color)].push_back(val);
		}
		return ret;
	}

	// 将黄色角置于正确位置
	void resetYellowCorner() {
		static const vector<MagicRotate> method = {
			R,Bp,R,F2,Rp,B,R,F2,R2
		};
		auto yellow = at(getCorners(up));
		if (trimTop(yellow)) return;
		for (int i = 0; i < 4; ++i) {
			vector<SixSideBlock*> frontArr;
			std::copy_if(yellow.begin(), yellow.end(), std::back_inserter(frontArr),
				std::bind(&MagicBlock::isAt, this, std::placeholders::_1, Side::front, 0)
			);
			auto c = frontArr[0]->getColorOf(Side::front);
			if (c != GColor::black() && c == frontArr[1]->getColorOf(Side::front)) {
				instant(method);
				trimTop(yellow);
				return;
			}
			if (i == 3) break;
			instant(U);
		}
		instant(method);
		resetYellowCorner();
	}

	bool trimTop(const vector<SixSideBlock*>& arr) {
		for (int i = 0; i < 4; ++i) {
			if (std::all_of(arr.begin(), arr.end(),
				std::bind(&MagicBlock::isRightBlock, this, std::placeholders::_1)
			)) {
				return true;
			}
			instant(U);
		}
		return false;
	}

	// 将最后四个棱置于正确位置
	void resetRestEdges() {
		static const vector<MagicRotate> method = {
			R,Up,R,U,R,U,R,Up,Rp,Up,R2
		};
		// 恢复正常转体
		auto frontSide = at(getCenter(front));
		exeUtilOnSide(Ut, frontSide, Side::front);

		auto yellow = at(getEdges(up));

		vector<SixSideBlock*> yes;
		std::copy_if(yellow.begin(), yellow.end(), std::back_inserter(yes),
			std::bind(&MagicBlock::isRightBlock, this, std::placeholders::_1));
		if (yes.size() == 1) {
			// 三个异位
			exeUtilOnSide(Ut, yes[0], Side::back);
			auto frontBlock = find(yes[0]->predictXYZ(U2, N));
			if (frontBlock->getColorOf(Side::front) == at(getCenter(Side::right))->getColor()) {
				instant(method);
			}
			else {
				instant(method, 2);
				instant(Up);
			}
		}
		else if (yes.size() == 0) {
			// 四个异位
			auto frontBlock = *std::find_if(yellow.begin(), yellow.end(),
				std::bind(&MagicBlock::isAt, this, std::placeholders::_1, Side::front, 0));
			auto fc = frontBlock->getColorOf(Side::front);
			if (fc == back) {
				instant(method);
				instant(U);
				instant(method);
			}
			else if (fc == right) {
				instant(method);
				instant(Up);
				instant(method);
			}
			else if (fc == left) {
				instant(Up);
				instant(method);
				instant(Up);
				instant(method);
				instant(U);
			}
		}

		exeUtilOnSide(Ut, frontSide, Side::front);
		trimTop(yellow);
	}
public:
	void test() {
		if (actions.size()) return;
		srand((unsigned int)(time(NULL)));
		MagicBlock* copy = new MagicBlock();
		for (int i = 0; i < 20; ++i) {
			auto op = MagicRotate(Side(rand() % 6), { rand() % 2 + 1 }, fps, { rand() % N });
			copy->instant(op);
			actions.push_back(op.frame(15));
		}
		actions = zipRotate(actions);
		/*actions.push_back(MagicRotate(Side(rand() % 6), 0, 50, { rand() % N }));
		copy.autoSolve();
		for (auto& op : copy.actions) {
			actions.push_back(op.frame(10));
		}
		actions.push_back(MagicRotate(Side(rand() % 6), 0, 50, { rand() % N }));*/
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

	const BlockPos getCenter(const GColor& has) {
		auto center = at({ 1,1,1 });
		for (int i = 0; i < 6; ++i) {
			int index = i / 2, val = i % 2 ? 1 : -1;
			int x = 0 == index ? val : 0;
			int y = 1 == index ? val : 0;
			int z = 2 == index ? val : 0;
			if (at(center->offset(x, y, z))->hasColor(has)) {
				return center->offset(x, y, z);
			}
		}
		assert(false);
		return { 1,1,1 };
	}

	const BlockPos getCenter(Side side) {
		auto center = at({ 1,1,1 });
		for (int i = 0; i < 6; ++i) {
			int index = i / 2, val = i % 2 ? 1 : -1;
			int x = 0 == index ? val : 0;
			int y = 1 == index ? val : 0;
			int z = 2 == index ? val : 0;
			if (isAt(at(center->offset(x, y, z)), side)) {
				return center->offset(x, y, z);
			}
		}
		assert(false);
		return { 1,1,1 };
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

	bool isAt(SixSideBlock* ptr, Side side, int offset = 0) {
		return atLay(ptr->x, ptr->y, ptr->z, side, offset);
	}

	// 判断其颜色是否与其所在面的中心点同色
	bool isRightBlock(SixSideBlock* ptr) {
		const auto all = ptr->getAllColor();
		for (auto& c : all) {
			Side side = ptr->getSideOf(c);
			if (!(at(getCenter(side))->getColor() == c)) {
				return false;
			}
		}
		return true;
	}

private:
	// 执行相关

	void instant(MagicRotate action) {
		action = action.one();
		actions.push_back(action);
		his.push_back(action);
		logic();
	}

	void instant(const vector<MagicRotate>& actions, int repeat = 1) {
		for (int i = 0; i < repeat; ++i) {
			for (auto& a : actions) {
				instant(a);
			}
		}
	}

	// 执行某个旋转,直到此块位于某个面
	void exeUtilOnSide(const MagicRotate& action, SixSideBlock* ptr, Side side, int offset = 0) {
		while (!isAt(ptr, side, offset)) {
			instant(action);
		}
	}
	// 执行某个旋转,直到此块的某一颜色位于某个面
	void exeUtilColorOnSide(const MagicRotate& action, SixSideBlock* ptr, const GColor& color, Side side) {
		while (ptr->getSideOf(color) != side) {
			instant(action);
		}
	}
private:
	void copyResult(const MagicBlock& copy) {
		actions = zipRotate(copy.his);
		for (auto& a : actions) {
			a = a.frame(fps / 3);
		}
		his = {};
	}
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
		int len = int(arr.size());
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

