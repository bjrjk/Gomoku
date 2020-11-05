#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <algorithm>
#include <queue>
#include <cstdint>
#include "jsoncpp/json.h"
using namespace std;

//二维数组中的数值代表棋盘中摆放的棋子，每种数值代表一种棋子
const int EMPTY = 0; //该位置为空
const int BOT = 1; //该位置为机器人的棋子
const int PLAYER = 2; //该位置为人类的棋子
const int NOT_EXIST = -1; //该位置不存在（数组越界）

const int SIZE = 15; //棋盘边长
int DEPTH; //极大极小搜索深度
const int SCORE_LENGTH = 6; //Score*数组的长度
const int SHIFT_LENGTH = 8; //*Shift数组的长度

struct Position { //用来返回落子位置的数据结构
	int x, y;
};

int PositionNodeSortMethod = 0; //启发式评估用到的排序方式指示变量
struct PositionNode { //使用启发式评估对搜索落子顺序进行调整，从而便于α-β剪枝的数据结构
	int x, y; //落子坐标位置
	long long priority; //启发式评估的评估值
	PositionNode(int x, int y, long long priority) :x(x), y(y), priority(priority) {}
	//评估值小的优先搜索的排序函数
	bool cmpLess(const PositionNode& o1, const PositionNode& o2) const {
		return o2.priority < o1.priority;
	}
	//评估值大的优先搜索的排序函数
	bool cmpGreater(const PositionNode& o1, const PositionNode& o2) const {
		return o1.priority < o2.priority;
	}
	/*
	极大极小搜索函数根据当前搜索的层数选择使用排序方式：
	当PositionNodeSortMethod为0时，是模拟机器人落子，此时应当优先搜索评估值大的落子方案，使得对于机器人利益最大化。
	当PositionNodeSortMethod为1时，是模拟人类落子，此时应当优先搜索评估值小的落子方案，使得对于机器人利益最小化。
	*/
	bool operator <(const PositionNode& o2) const {
		if (PositionNodeSortMethod == 0)return cmpGreater(*this, o2);
		else return cmpLess(*this, o2);
	}
};

struct Grid {
	int grid[SIZE][SIZE]; //二维数组模拟棋盘
	//XShift和YShift数组是程序搜索过程中遍历指定方格的邻接方格的偏移量
	int XShift[SHIFT_LENGTH] = { -1, -1, -1, 0, 0, 1, 1, 1 };
	int YShift[SHIFT_LENGTH] = { -1, 0, 1, -1, 1, -1, 0, 1 };
	//死棋的评估分数（有一头被堵住，另一头没有被堵住，只有一头可以继续下棋）
	long long Score_E1[SCORE_LENGTH] = { 0, 1, 5, 25, 1250, 1000000 }; 
	//活棋的评估分数（两头没有被堵住，都可以下棋）
	long long Score_E2[SCORE_LENGTH] = { 0, 5, 20, 200, 1500, 1000000 };
	//XList和YList数组是程序中使用的临时数组
	int XList[50];
	int YList[50];

	//将类型为value的棋子落子在棋盘(x,y)坐标，成功返回true，坐标不存在返回false
	inline bool placeAt(int x, int y, int value) {
		if (x >= 0 && y >= 0 && x < SIZE && y < SIZE) {
			grid[x][y] = value;
			return true;
		}
		return false;
	}
	//获得棋盘上(x,y)坐标位置的棋子类型，若坐标不存在返回NOT_EXIST
	inline int getValueAt(int x, int y) {
		if (x >= 0 && y >= 0 && x < SIZE && y < SIZE)
			return grid[x][y];
		else
			return NOT_EXIST;
	}
	//传入棋子类型status，连续棋子数cnt，两边有几边是空的edgeSituation，获取当前连续棋子的评估分数
	inline long long getScore(int status, int cnt, int edgeSituation) {
		//当两边都被堵住时，如果在中间下棋有可能成五，需要返回一个评估值，解决了不堵中间的Bug
		if (edgeSituation == 0) { 
			switch (status) {
			case BOT:
				return cnt == 5 ? Score_E2[5] : 0;
			case PLAYER:
				return -(cnt == 5 ? Score_E2[5] : 0);
			default:
				return 0;
			}
		}
		//两边有一边为空或两边都为空时，到Score_E1和Score_E2数组中去查找对应的棋子评估分数并返回
		else if (edgeSituation == 1) {
			switch (status) {
			case BOT:
				return cnt >= SCORE_LENGTH ? Score_E1[SCORE_LENGTH - 1] : Score_E1[cnt];
			case PLAYER:
				return -(cnt >= SCORE_LENGTH ? Score_E1[SCORE_LENGTH - 1] : Score_E1[cnt]);
			default:
				return 0;
			}
		}
		else { // edgeSituation==2
			switch (status) {
			case BOT:
				return cnt >= SCORE_LENGTH ? Score_E2[SCORE_LENGTH - 1] : Score_E2[cnt];
			case PLAYER:
				return -(cnt >= SCORE_LENGTH ? Score_E2[SCORE_LENGTH - 1] : Score_E2[cnt]);
			default:
				return 0;
			}
		}
	}
	//传入两边的棋子状态，计算两边为空的位置格数
	inline int calculateEdgeSituation(int leftEdgeStatus, int rightEdgeStatus) {
		int cnt = 0;
		if (EMPTY == leftEdgeStatus)
			cnt++;
		if (EMPTY == rightEdgeStatus)
			cnt++;
		return cnt;
	}
	//下标不越界的访问数组，当越界时返回NOT_EXIST
	inline int getElemInArraySafe(int list[], int index, int size) {
		if (0 <= index && index < size)
			return list[index];
		else
			return NOT_EXIST;
	}
	//计算XList（存储x坐标的数组）和YList（存储y坐标的数组）所指定的连成一条线上的棋子的评估分数
	long long SequenceEvaluate(int XList[], int YList[], int size) {
		int XSize = size, YSize = size;
		long long sum = 0;
		int status = EMPTY, cnt = 0;
		int leftEdge = -1, rightEdge;
		for (int i = 0; i < XSize; i++) {
			int curGrid = getValueAt(XList[i], YList[i]);
			if (status == curGrid)
				cnt++;
			else {
				rightEdge = i;
				sum += getScore(status, cnt,
					calculateEdgeSituation(
						getValueAt(getElemInArraySafe(XList, leftEdge, XSize),
							getElemInArraySafe(YList, leftEdge, YSize)),
						getValueAt(getElemInArraySafe(XList, rightEdge, XSize),
							getElemInArraySafe(YList, rightEdge, YSize))));
				status = curGrid;
				cnt = 1;
				leftEdge = i - 1;
			}
		}
		rightEdge = XSize;
		sum += getScore(status, cnt,
			calculateEdgeSituation(
				getValueAt(getElemInArraySafe(XList, leftEdge, XSize),
					getElemInArraySafe(YList, leftEdge, YSize)),
				getValueAt(getElemInArraySafe(XList, rightEdge, XSize),
					getElemInArraySafe(YList, rightEdge, YSize))));
		return sum;
	}
	//棋局评估函数
	long long Evaluate() { 
		long long sum = 0;
		int ptr = 0;

		// 评估每一行
		for (int i = 0; i < SIZE; i++) {
			ptr = 0;
			for (int j = 0; j < SIZE; j++) {
				XList[ptr] = i;
				YList[ptr] = j;
				ptr++;
			}
			sum += SequenceEvaluate(XList, YList, ptr);
		}
		// 评估每一列
		for (int i = 0; i < SIZE; i++) {
			ptr = 0;
			for (int j = 0; j < SIZE; j++) {
				XList[ptr] = j;
				YList[ptr] = i;
				ptr++;
			}
			sum += SequenceEvaluate(XList, YList, ptr);
		}
		// 评估每个左上-右下对角线

		// 左侧第0列开始的对角线
		for (int line = 0; line < SIZE; line++) {
			ptr = 0;
			for (int x = line, y = 0; x < SIZE && y < SIZE; x++, y++) {
				XList[ptr] = x;
				YList[ptr] = y;
				ptr++;
			}
			sum += SequenceEvaluate(XList, YList, ptr);
		}

		// 上侧第0行开始的对角线
		for (int row = 1; row < SIZE; row++) {
			ptr = 0;
			for (int x = 0, y = row; x < SIZE && y < SIZE; x++, y++) {
				XList[ptr] = x;
				YList[ptr] = y;
				ptr++;
			}
			sum += SequenceEvaluate(XList, YList, ptr);
		}
		// 评估每个右上-左下对角线

		// 右侧第14列开始的对角线
		for (int line = 0; line < SIZE; line++) {
			ptr = 0;
			for (int x = line, y = SIZE - 1; x < SIZE && y >= 0; x++, y--) {
				XList[ptr] = x;
				YList[ptr] = y;
				ptr++;
			}
			sum += SequenceEvaluate(XList, YList, ptr);
		}

		// 上侧第0行开始的对角线
		for (int row = 0; row < SIZE - 1; row++) {
			ptr = 0;
			for (int x = 0, y = row; x < SIZE && y >= 0; x++, y--) {
				XList[ptr] = x;
				YList[ptr] = y;
				ptr++;
			}
			sum += SequenceEvaluate(XList, YList, ptr);
		}
		return sum;
	}
	//评估坐标(x,y)处所对应的分数
	long long EvaluateUnit(int x, int y) {
		long long sum = 0;
		int ptr = 0;

		// 评估行
		ptr = 0;
		for (int j = 0; j < SIZE; j++) {
			XList[ptr] = x;
			YList[ptr] = j;
			ptr++;
		}
		sum += SequenceEvaluate(XList, YList, ptr);
		// 评估列
		ptr = 0;
		for (int j = 0; j < SIZE; j++) {
			XList[ptr] = j;
			YList[ptr] = y;
			ptr++;
		}
		sum += SequenceEvaluate(XList, YList, ptr);
		// 评估左上-右下对角线
		ptr = 0;
		for (int i = x - min(x, y), j = y - min(x, y); i < SIZE && j < SIZE; i++, j++) {
			XList[ptr] = i;
			YList[ptr] = j;
			ptr++;
		}
		sum += SequenceEvaluate(XList, YList, ptr);

		// 评估右上-左下对角线
		ptr = 0;
		for (int i = x + min(SIZE - x, y), j = y - min(SIZE - x, y); i >= 0 && j < SIZE; i--, j++) {
			XList[ptr] = i;
			YList[ptr] = j;
			ptr++;
		}
		sum += SequenceEvaluate(XList, YList, ptr);
		return sum;
	}
	//评估在坐标(x,y)处落子时，对总评估分数会产生的差值，这样可以加快搜索速度
	//落子种类由搜索深度决定
	long long EvaluateUnitDiff(int depth, int x, int y) {
		placeAt(x, y, EMPTY);
		long long sum1 = EvaluateUnit(x, y);
		placeAt(x, y, depth % 2 == 0 ? BOT : PLAYER);
		long long sum2 = EvaluateUnit(x, y);
		placeAt(x, y, EMPTY);
		return sum2 - sum1;
	}
	//极大极小搜索与α-β剪枝搜索函数
	//参数为当前搜索深度depth，返回的落子位置数据结构movePos，α值alpha，β值beta，棋局评估分数evaluationValue
	long long DFS2(int depth, Position* movePos, long long alpha, long long beta, long long evaluationValue) {
		//depth%2==0时为BOT，depth%2==1时为PLAYER
		if (depth == DEPTH) //到达边界深度时，结束搜索，直接返回棋局评估分数
			return evaluationValue;
		long long selectedScore = depth % 2 == 0 ? alpha : beta; //根据是极大层还是极小层决定剪枝的边界分数是α还是β
		PositionNodeSortMethod = depth % 2; //根据搜索层数选择启发式评估的排序方式
		priority_queue<PositionNode> pq; //使用优先队列对启发式评估的落子位置排序
		//循环遍历整个棋盘，寻找可以落子的位置
		for (int i = 0; i < SIZE; i++) {
			for (int j = 0; j < SIZE; j++) {
				if (getValueAt(i, j) != EMPTY) //当前格子不为空时，不能落子，跳过
					continue;
				bool flag = false;
				for (int k = 0; k < SHIFT_LENGTH; k++) { 
					//当前格子周围邻接的格子如果有子，就把它当成一个可能的落子位置并加以评估
					int value = getValueAt(i + XShift[k], j + YShift[k]);
					if (value == PLAYER || value == BOT) {
						flag = true;
						break;
					}
				}
				if (flag) //启发式评估成功之后加入优先队列进行排序
					pq.emplace(PositionNode(i, j, EvaluateUnitDiff(depth, i, j)));
			}
		}
		if (depth == 0) { //若深度为0的话，首先选择启发式评估值最大的落子情况初始化返回的落子位置
			PositionNode curPositionNode = pq.top();
			int i = curPositionNode.x, j = curPositionNode.y;
			movePos->x = i;
			movePos->y = j;
		}
		while (!pq.empty()) { //当优先队列非空时，取出评估落子情况用的数据结构并准备向下搜索
			PositionNode curPositionNode = pq.top();
			pq.pop();
			int i = curPositionNode.x, j = curPositionNode.y;
			placeAt(i, j, depth % 2 == 0 ? BOT : PLAYER); //根据搜索层数选择落子类型是机器人还是人类
			long long curScore;
			if (depth % 2 == 0) { // 极大层节点时，继续搜索极小层节点
				curScore = DFS2(depth + 1, NULL, selectedScore, beta, evaluationValue + curPositionNode.priority);
			}
			else { // 极小层节点时，继续搜索极大层节点
				curScore = DFS2(depth + 1, NULL, alpha, selectedScore, evaluationValue + curPositionNode.priority);
			}
			if (depth % 2 == 0) { //极大层节点，取最大的棋局评估值更新α值
				if (selectedScore < curScore) {
					selectedScore = curScore;
					if (depth == 0) { //当搜索深度为0时，更新返回的落子位置
						movePos->x = i;
						movePos->y = j;
					}
				}
			}
			else { //极小层节点，取最小的棋局评估值更新β值
				if (selectedScore > curScore) {
					selectedScore = curScore;
				}
			}
			placeAt(i, j, EMPTY); //回溯
			//α-β剪枝
			if (depth % 2 == 0) { //极大层进行β剪枝
				if (selectedScore >= beta)
					return beta;
			}
			else { //极小层进行α剪枝
				if (selectedScore <= alpha) 
					return alpha;
			}
		}
		return selectedScore; //如果没有剪枝，返回最终的棋局评估结果
	}
	//选择落子位置的函数
	inline Json::Value ChoosePosition(int cnter)
	{
		Position move;
		Json::Value action;
		if (cnter != 0) { //机器人后手的情况
			long long evaluationValue = INT64_MIN;
			for (DEPTH = 2; DEPTH <= 4; DEPTH += 2) { //分别搜索两层和四层的情况，取最优解
				long long tmpEvaluationValue = DFS2(0, &move, INT64_MIN, INT64_MAX, 0);
				if (tmpEvaluationValue > evaluationValue) {
					action["x"] = move.x;
					action["y"] = move.y;
					evaluationValue = tmpEvaluationValue;
				}
			}
		}
		else { //机器人先手落子在棋盘中心
			action["x"] = 7;
			action["y"] = 7;
		}
		return action;
	}
	Grid() {
		memset(grid, EMPTY, sizeof(grid));
	}
};

Grid grid;
int main() {
	string str;
	getline(cin, str);
	Json::Reader reader;
	Json::Value input;
	reader.parse(str, input);
	int turnID = input["responses"].size();
	int cnter = 0;
	//读取数据，模拟落子过程
	for (int i = 0; i < turnID; i++) {
		if (grid.placeAt(input["requests"][i]["x"].asInt(), input["requests"][i]["y"].asInt(), PLAYER))
			cnter++;
		grid.placeAt(input["responses"][i]["x"].asInt(), input["responses"][i]["y"].asInt(), BOT);
	}
	if (grid.placeAt(input["requests"][turnID]["x"].asInt(), input["requests"][turnID]["y"].asInt(), PLAYER))
		cnter++;
	Json::Value ret;
	ret["response"] = grid.ChoosePosition(cnter);
	Json::FastWriter writer;
	cout << writer.write(ret) << endl;
	return 0;
}