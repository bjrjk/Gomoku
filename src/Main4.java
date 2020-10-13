import java.util.*;
import org.json.simple.JSONValue;

class Main4 {
	public static final int EMPTY = 0;
	public static final int BOT = 1;
	public static final int PLAYER = 2;
	public static final int NOT_EXIST = -1;

	static class Position {
		public int x, y;
	}

	static class PositionNode {
		public int x, y;
		public long priority;

		public PositionNode(int x, int y, long priority) {
			this.x = x;
			this.y = y;
			this.priority = priority;
		}

		public static Comparator<PositionNode> cmpGreater = new Comparator<PositionNode>() {
			@Override
			public int compare(PositionNode o1, PositionNode o2) {
				return (int) (o2.priority - o1.priority);
			}
		};
		public static Comparator<PositionNode> cmpLess = new Comparator<PositionNode>() {
			@Override
			public int compare(PositionNode o1, PositionNode o2) {
				return (int) (o1.priority - o2.priority);
			}
		};
	}

	static class Grid {
		public static final int SIZE = 15;
		public static final int DEPTH = 5;
		private int[][] grid = new int[SIZE][SIZE];
		private int[] XShift = { -1, -1, -1, 0, 0, 1, 1, 1 };
		private int[] YShift = { -1, 0, 1, -1, 1, -1, 0, 1 };
		private long[] Score_E1 = { 0, 2, 40, 500, 5000, 70000, 750000, 8000000, 80000000 };
		private long[] Score_E2 = { 0, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000 };
		private int[] XList = new int[50];
		private int[] YList = new int[50];

		public boolean placeAt(int x, int y, int value) {
			if (x >= 0 && y >= 0 && x < SIZE && y < SIZE) {
				grid[x][y] = value;
				return true;
			}
			return false;
		}

		public int getValueAt(int x, int y) {
			if (x >= 0 && y >= 0 && x < SIZE && y < SIZE)
				return grid[x][y];
			else
				return NOT_EXIST;
		}

		private long getScore(int status, int cnt, int edgeSituation) {
			// assert (0 <= cnt && cnt <= 5);
			// assert (status == EMPTY || status == BOT || status == PLAYER);
			if (edgeSituation == 0)
				return 0;
			else if (edgeSituation == 1) {
				switch (status) {
				case BOT:
					return cnt >= Score_E1.length ? Score_E1[Score_E1.length - 1] : Score_E1[cnt];
				case PLAYER:
					return -(cnt >= Score_E1.length ? Score_E1[Score_E1.length - 1] : Score_E1[cnt]);
				default:
					return 0;
				}
			} else { // edgeSituation==2
				switch (status) {
				case BOT:
					return cnt >= Score_E2.length ? Score_E2[Score_E2.length - 1] : Score_E2[cnt];
				case PLAYER:
					return -(cnt >= Score_E2.length ? Score_E2[Score_E2.length - 1] : Score_E2[cnt]);
				default:
					return 0;
				}
			}
		}

		private int calculateEdgeSituation(int leftEdgeStatus, int rightEdgeStatus) {
			int cnt = 0;
			if (EMPTY == leftEdgeStatus)
				cnt++;
			if (EMPTY == rightEdgeStatus)
				cnt++;
			return cnt;
		}

		private int getElemInArraySafe(int[] list, int index, int size) {
			if (0 <= index && index < size)
				return list[index];
			else
				return NOT_EXIST;
		}

		private long SequenceEvaluate(int[] XList, int[] YList, int size) {
			int XSize = size, YSize = size;
			long sum = 0;
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

		private long Evaluate() { // 棋局评估函数
			long sum = 0;
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

		private long EvaluateUnit(int x, int y) {
			long sum = 0;
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
			for (int i = x - Math.min(x, y), j = y - Math.min(x, y); i < SIZE && j < SIZE; i++, j++) {
				XList[ptr] = i;
				YList[ptr] = j;
				ptr++;
			}
			sum += SequenceEvaluate(XList, YList, ptr);

			// 评估右上-左下对角线
			ptr = 0;
			for (int i = x - Math.min(x, SIZE - y), j = y + Math.min(x, SIZE - y); i < SIZE && j >= 0; i++, j--) {
				XList[ptr] = i;
				YList[ptr] = j;
				ptr++;
			}
			sum += SequenceEvaluate(XList, YList, ptr);
			return sum;
		}

		private long EvaluateUnitDiff(int depth, int x, int y) {
			placeAt(x, y, EMPTY);
			long sum1 = EvaluateUnit(x, y);
			placeAt(x, y, depth % 2 == 0 ? BOT : PLAYER);
			long sum2 = EvaluateUnit(x, y);
			placeAt(x, y, EMPTY);
			return sum2 - sum1;
		}

		private long DFS2(int depth, Position movePos, long cutValue, long evaluationValue) {
			// 极大极小搜索, depth%2==0时为BOT,depth%2==1时为PLAYER
			// alpha-beta 剪枝, cutValue
			if (depth == DEPTH)
				return evaluationValue;
			long selectedScore = depth % 2 == 0 ? Long.MIN_VALUE : Long.MAX_VALUE;
			Queue<PositionNode> pq = new PriorityQueue<PositionNode>(
					depth % 2 == 0 ? PositionNode.cmpGreater : PositionNode.cmpLess);
			for (int i = 0; i < SIZE; i++) {
				for (int j = 0; j < SIZE; j++) {
					if (getValueAt(i, j) != EMPTY)
						continue;
					boolean flag = false;
					for (int k = 0; k < XShift.length; k++) {
						int value = getValueAt(i + XShift[k], j + YShift[k]);
						if (value == PLAYER || value == BOT) {
							flag = true;
							break;
						}
					}
					if (flag)
						pq.add(new PositionNode(i, j, EvaluateUnitDiff(depth, i, j)));
				}
			}
			while (!pq.isEmpty()) {
				PositionNode curPositionNode = pq.poll();
				int i = curPositionNode.x, j = curPositionNode.y;
				placeAt(i, j, depth % 2 == 0 ? BOT : PLAYER);
				long curScore = DFS2(depth + 1, null, selectedScore, evaluationValue + curPositionNode.priority);
				if (depth % 2 == 0) {
					if (selectedScore < curScore) {
						selectedScore = curScore;
						if (depth == 0) {
							movePos.x = i;
							movePos.y = j;
						}
					}
				} else {
					if (selectedScore > curScore) {
						selectedScore = curScore;
					}
				}
				placeAt(i, j, EMPTY);
				// alpha-beta剪枝
				if (depth % 2 == 0) {
					if (selectedScore >= cutValue) // beta剪枝
						break;
				} else {
					if (selectedScore <= cutValue) // alpha剪枝
						break;
				}
			}
			return selectedScore;
		}

		public Map<String, Integer> ChoosePosition(int cnter) { // 选择位置
			Position move = new Position();
			Map<String, Integer> map = new HashMap<String, Integer>();
			if(cnter!=0) {
				DFS2(0, move, Long.MAX_VALUE, Evaluate());
				map.put("x", move.x);
				map.put("y", move.y);
			}else {
				map.put("x", 7);
				map.put("y", 7);
			}
			return map;
		}
	}

	public static void main(String[] args) {
		Scanner scan = new Scanner(System.in);
		String input = scan.nextLine();
		Map<String, List> obj = (Map) JSONValue.parse(input);
		Grid grid = new Grid();
		List<Map> requests = obj.get("requests");
		List<Map> responses = obj.get("responses");
		int cnter = 0;
		for (Map<String, Long> rec : requests) {
			if (grid.placeAt(rec.get("x").intValue(), rec.get("y").intValue(), PLAYER))
				cnter++;
		}
		for (Map<String, Long> rec : responses) {
			grid.placeAt(rec.get("x").intValue(), rec.get("y").intValue(), BOT);
		}
		Map output = new HashMap();
		output.put("response", grid.ChoosePosition(cnter));
		System.out.print(JSONValue.toJSONString(output));
		scan.close();
	}
}