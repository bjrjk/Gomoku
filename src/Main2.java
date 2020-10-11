import java.util.*;
import org.json.simple.JSONValue;

class Main2 {
	public static final int EMPTY = 0;
	public static final int BOT = 1;
	public static final int PLAYER = 2;
	public static final int NOT_EXIST = -1;

	static class Position {
		public int x, y;
	}

	static class PositionNode {
		public int x, y, priority;

		public PositionNode(int x, int y, int priority) {
			this.x = x;
			this.y = y;
			this.priority = priority;
		}

		public static Comparator<PositionNode> cmpGreater = new Comparator<PositionNode>() {
			@Override
			public int compare(PositionNode o1, PositionNode o2) {
				return o2.priority - o1.priority;
			}
		};
		public static Comparator<PositionNode> cmpLess = new Comparator<PositionNode>() {
			@Override
			public int compare(PositionNode o1, PositionNode o2) {
				return o1.priority - o2.priority;
			}
		};
	}

	static class Grid {
		public static final int SIZE = 15;
		public static final int DEPTH = 4;
		private int[][] grid = new int[SIZE][SIZE];
		private int[] XShift = { -1, -1, -1, 0, 0, 1, 1, 1 };
		private int[] YShift = { -1, 0, 1, -1, 1, -1, 0, 1 };
		private int[] Score = { 0, 10, 100, 1000, 10000, 100000 };

		public void placeAt(int x, int y, int value) {
			if (x >= 0 && y >= 0 && x < SIZE && y < SIZE)
				grid[x][y] = value;
		}

		public int getValueAt(int x, int y) {
			if (x >= 0 && y >= 0 && x < SIZE && y < SIZE)
				return grid[x][y];
			else
				return NOT_EXIST;
		}

		private int getScore(int status, int cnt) {
			// assert (0 <= cnt && cnt <= 5);
			assert (status == EMPTY || status == BOT || status == PLAYER);
			switch (status) {
			case BOT:
				return cnt > 5 ? Score[5] : Score[cnt];
			case PLAYER:
				return -(cnt > 5 ? Score[5] : Score[cnt]);
			default:
				return 0;
			}
		}

		private int SequenceEvaluate(ArrayList<Integer> XList, ArrayList<Integer> YList) {
			assert (XList.size() == YList.size());
			int sum = 0;
			int status = EMPTY, cnt = 0;
			for (int i = 0; i < XList.size(); i++) {
				int curGrid = getValueAt(XList.get(i), YList.get(i));
				if (status == curGrid)
					cnt++;
				else {
					sum += getScore(status, cnt);
					status = curGrid;
					cnt = 1;
				}
			}
			sum += getScore(status, cnt);
			return sum;
		}

		private int Evaluate() { // 棋局评估函数
			int sum = 0;
			ArrayList<Integer> XList = new ArrayList<Integer>();
			ArrayList<Integer> YList = new ArrayList<Integer>();
			// 评估每一行
			for (int i = 0; i < SIZE; i++) {
				XList.clear();
				YList.clear();
				for (int j = 0; j < SIZE; j++) {
					XList.add(i);
					YList.add(j);
				}
				sum += SequenceEvaluate(XList, YList);
			}
			// 评估每一列
			for (int i = 0; i < SIZE; i++) {
				XList.clear();
				YList.clear();
				for (int j = 0; j < SIZE; j++) {
					XList.add(j);
					YList.add(i);
				}
				sum += SequenceEvaluate(XList, YList);
			}
			// 评估每个左上-右下对角线

			// 左侧第0列开始的对角线
			for (int line = 0; line < SIZE; line++) {
				XList.clear();
				YList.clear();
				for (int x = line, y = 0; x < SIZE && y < SIZE; x++, y++) {
					XList.add(x);
					YList.add(y);
				}
				sum += SequenceEvaluate(XList, YList);
			}

			// 上侧第0行开始的对角线
			for (int row = 1; row < SIZE; row++) {
				XList.clear();
				YList.clear();
				for (int x = 0, y = row; x < SIZE && y < SIZE; x++, y++) {
					XList.add(x);
					YList.add(y);
				}
				sum += SequenceEvaluate(XList, YList);
			}
			// 评估每个右上-左下对角线

			// 右侧第14列开始的对角线
			for (int line = 0; line < SIZE; line++) {
				XList.clear();
				YList.clear();
				for (int x = line, y = SIZE - 1; x < SIZE && y >= 0; x++, y--) {
					XList.add(x);
					YList.add(y);
				}
				sum += SequenceEvaluate(XList, YList);
			}

			// 上侧第0行开始的对角线
			for (int row = 0; row < SIZE - 1; row++) {
				XList.clear();
				YList.clear();
				for (int x = 0, y = row; x < SIZE && y >= 0; x++, y--) {
					XList.add(x);
					YList.add(y);
				}
				sum += SequenceEvaluate(XList, YList);
			}
			return sum;
		}

		private int EvaluateUnit(int x, int y) {
			int sum = 0;
			ArrayList<Integer> XList = new ArrayList<Integer>();
			ArrayList<Integer> YList = new ArrayList<Integer>();
			// 评估行
			for (int j = y - 5; j <= y + 5; j++) {
				XList.add(x);
				YList.add(j);
			}
			sum += SequenceEvaluate(XList, YList);
			// 评估列
			for (int j = x - 5; j <= x + 5; j++) {
				XList.add(j);
				YList.add(y);
			}
			sum += SequenceEvaluate(XList, YList);
			// 评估左上-右下对角线
			for (int i = x - 5, j = y - 5; i <= x + 5 && j <= y + 5; i++, j++) {
				XList.add(i);
				YList.add(j);
			}
			sum += SequenceEvaluate(XList, YList);

			// 评估右上-左下对角线
			for (int i = x - 5, j = y + 5; i <= x + 5 && j >= y - 5; i++, j--) {
				XList.add(i);
				YList.add(j);
			}
			sum += SequenceEvaluate(XList, YList);
			return sum;
		}

		private int DFS2(int depth, Position movePos, int cutValue) {
			// 极大极小搜索, depth%2==0时为BOT,depth%2==1时为PLAYER
			// alpha-beta 剪枝, cutValue
			if (depth == DEPTH)
				return Evaluate();
			int selectedScore = depth % 2 == 0 ? Integer.MIN_VALUE : Integer.MAX_VALUE;
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
						pq.add(new PositionNode(i, j, EvaluateUnit(i, j)));
				}
			}
			int nodeCnt = 0;
			while (!pq.isEmpty()) {
				nodeCnt++;
				if (nodeCnt > 500)
					break;
				PositionNode curPositionNode = pq.poll();
				int i = curPositionNode.x, j = curPositionNode.y;
				placeAt(i, j, depth % 2 == 0 ? BOT : PLAYER);
				int curScore = DFS2(depth + 1, new Position(), selectedScore);
				if (depth % 2 == 0) {
					if (selectedScore < curScore) {
						selectedScore = curScore;
						movePos.x = i;
						movePos.y = j;
					}
				} else {
					if (selectedScore > curScore) {
						selectedScore = curScore;
						movePos.x = i;
						movePos.y = j;
					}
				}
				placeAt(i, j, EMPTY);
				// alpha-beta剪枝
				if (depth % 2 == 0) {
					if (selectedScore > cutValue) // beta剪枝
						break;
				} else {
					if (selectedScore < cutValue) // alpha剪枝
						break;
				}
			}
			return selectedScore;
		}

		public Map<String, Integer> ChoosePosition() { // 选择位置
			Position move = new Position();
			DFS2(0, move, Integer.MAX_VALUE);
			Map<String, Integer> map = new HashMap<String, Integer>();
			map.put("x", move.x);
			map.put("y", move.y);
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
		for (Map<String, Long> rec : requests) {
			grid.placeAt(rec.get("x").intValue(), rec.get("y").intValue(), PLAYER);
		}
		for (Map<String, Long> rec : responses) {
			grid.placeAt(rec.get("x").intValue(), rec.get("y").intValue(), BOT);
		}
		Map output = new HashMap();
		output.put("response", grid.ChoosePosition());
		System.out.print(JSONValue.toJSONString(output));
		scan.close();
	}
}