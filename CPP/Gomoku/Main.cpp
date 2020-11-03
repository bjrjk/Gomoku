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

const int EMPTY = 0;
const int BOT = 1;
const int PLAYER = 2;
const int NOT_EXIST = -1;

const int SIZE = 15;
int DEPTH;
const int SCORE_LENGTH = 6;
const int SHIFT_LENGTH = 8;

struct Position {
	int x, y;
};

int PositionNodeSortMethod = 0;
struct PositionNode {
	int x, y;
	long long priority;
	PositionNode(int x, int y, long long priority) :x(x), y(y), priority(priority) {}
	bool cmpLess(const PositionNode& o1, const PositionNode& o2) const {
		return o2.priority < o1.priority;
	}
	bool cmpGreater(const PositionNode& o1, const PositionNode& o2) const {
		return o1.priority < o2.priority;
	}
	bool operator <(const PositionNode& o2) const {
		if (PositionNodeSortMethod == 0)return cmpGreater(*this, o2);
		else return cmpLess(*this, o2);
	}
};

struct Grid {
	int grid[SIZE][SIZE];
	int XShift[SHIFT_LENGTH] = { -1, -1, -1, 0, 0, 1, 1, 1 };
	int YShift[SHIFT_LENGTH] = { -1, 0, 1, -1, 1, -1, 0, 1 };
	long long Score_E1[SCORE_LENGTH] = { 0, 1, 5, 25, 1250, 100000 };
	long long Score_E2[SCORE_LENGTH] = { 0, 5, 20, 200, 1500, 100000 };
	int XList[50];
	int YList[50];

	inline bool placeAt(int x, int y, int value) {
		if (x >= 0 && y >= 0 && x < SIZE && y < SIZE) {
			grid[x][y] = value;
			return true;
		}
		return false;
	}

	inline int getValueAt(int x, int y) {
		if (x >= 0 && y >= 0 && x < SIZE && y < SIZE)
			return grid[x][y];
		else
			return NOT_EXIST;
	}

	inline long long getScore(int status, int cnt, int edgeSituation) {
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

	inline int calculateEdgeSituation(int leftEdgeStatus, int rightEdgeStatus) {
		int cnt = 0;
		if (EMPTY == leftEdgeStatus)
			cnt++;
		if (EMPTY == rightEdgeStatus)
			cnt++;
		return cnt;
	}

	inline int getElemInArraySafe(int list[], int index, int size) {
		if (0 <= index && index < size)
			return list[index];
		else
			return NOT_EXIST;
	}

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

	long long Evaluate() { // 棋局评估函数
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

	long long EvaluateUnitDiff(int depth, int x, int y) {
		placeAt(x, y, EMPTY);
		long long sum1 = EvaluateUnit(x, y);
		placeAt(x, y, depth % 2 == 0 ? BOT : PLAYER);
		long long sum2 = EvaluateUnit(x, y);
		placeAt(x, y, EMPTY);
		return sum2 - sum1;
	}

	long long DFS2(int depth, Position* movePos, long long alpha, long long beta, long long evaluationValue) {
		// 极大极小搜索, depth%2==0时为BOT,depth%2==1时为PLAYER
		// alpha-beta 剪枝
		if (depth == DEPTH)
			return evaluationValue;
		long long selectedScore = depth % 2 == 0 ? alpha : beta;
		PositionNodeSortMethod = depth % 2;
		priority_queue<PositionNode> pq;
		for (int i = 0; i < SIZE; i++) {
			for (int j = 0; j < SIZE; j++) {
				if (getValueAt(i, j) != EMPTY)
					continue;
				bool flag = false;
				for (int k = 0; k < SHIFT_LENGTH; k++) {
					int value = getValueAt(i + XShift[k], j + YShift[k]);
					if (value == PLAYER || value == BOT) {
						flag = true;
						break;
					}
				}
				if (flag)
					pq.emplace(PositionNode(i, j, EvaluateUnitDiff(depth, i, j)));
			}
		}
		if (depth == 0) {
			PositionNode curPositionNode = pq.top();
			int i = curPositionNode.x, j = curPositionNode.y;
			movePos->x = i;
			movePos->y = j;
		}
		while (!pq.empty()) {
			PositionNode curPositionNode = pq.top();
			pq.pop();
			int i = curPositionNode.x, j = curPositionNode.y;
			placeAt(i, j, depth % 2 == 0 ? BOT : PLAYER);
			long long curScore;
			if (depth % 2 == 0) { // 极大层节点
				curScore = DFS2(depth + 1, NULL, selectedScore, beta, evaluationValue + curPositionNode.priority);
			}
			else { // 极小层节点
				curScore = DFS2(depth + 1, NULL, alpha, selectedScore, evaluationValue + curPositionNode.priority);
			}
			if (depth % 2 == 0) { // 极大层节点，取最大
				if (selectedScore < curScore) {
					selectedScore = curScore;
					if (depth == 0) {
						movePos->x = i;
						movePos->y = j;
					}
				}
			}
			else { // 极小层节点，取最小
				if (selectedScore > curScore) {
					selectedScore = curScore;
				}
			}
			placeAt(i, j, EMPTY);
			// alpha-beta剪枝
			if (depth % 2 == 0) {
				if (selectedScore >= beta) // beta剪枝
					return beta;
			}
			else {
				if (selectedScore <= alpha) // alpha剪枝
					return alpha;
			}
		}
		return selectedScore;
	}
	inline Json::Value ChoosePosition(int cnter)
	{
		Position move;
		Json::Value action;
		if (cnter != 0) {
			long long evaluationValue = INT64_MIN;
			for (DEPTH = 2; DEPTH <= 4; DEPTH += 2) {
				long long tmpEvaluationValue = DFS2(0, &move, INT64_MIN, INT64_MAX, 0);
				if (tmpEvaluationValue > evaluationValue) {
					action["x"] = move.x;
					action["y"] = move.y;
					evaluationValue = tmpEvaluationValue;
				}
			}
		}
		else {
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