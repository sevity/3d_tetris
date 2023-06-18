// YOUTUBE Version
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <fstream>
#include <sstream>
#include <ctime>
#include <cassert>

const int SCREEN_WIDTH = 600;
const int SCREEN_HEIGHT = 600;
const int BLOCK_CNT = 7;

using namespace std;
using point_2d = sf::Vector2f;
using point_3d = sf::Vector3f;

int blocks_cpy[BLOCK_CNT][3][3][3] = {};
int blocks[BLOCK_CNT][3][3][3] = {
	// 0.corner
	0,0,0, // 천장층
	0,0,0,
	1,0,0,

	0,0,0, // 중간층
	1,0,0,
	1,1,0,

	0,0,0, // 바닥층
	0,0,0,
	0,0,0,

	// 1.double1
	0,0,0, // 천장층
	0,0,0,
	1,0,0,

	0,0,0, // 중간층
	0,1,0,
	1,1,0,

	0,0,0, // 바닥층
	0,0,0,
	0,0,0,

	// 2.double2
	0,0,0, // 천장층
	0,0,0,
	1,0,0,

	0,0,0, // 중간층
	1,1,0,
	1,0,0,

	0,0,0, // 바닥층
	0,0,0,
	0,0,0,

	// 3.N
	0,0,0, // 천장층
	0,1,1,
	1,1,0,

	0,0,0, // 중간층
	0,0,0,
	0,0,0,

	0,0,0, // 바닥층
	0,0,0,
	0,0,0,

	// 4.n
	0,0,0, // 천장층
	1,0,0,
	1,1,0,

	0,0,0, // 중간층
	0,0,0,
	0,0,0,

	0,0,0, // 바닥층
	0,0,0,
	0,0,0,

	// 5.L
	0,0,0, // 천장층
	1,0,0,
	1,1,1,

	0,0,0, // 중간층
	0,0,0,
	0,0,0,

	0,0,0, // 바닥층
	0,0,0,
	0,0,0,

	// 6.T
	0,0,0, // 천장층
	0,1,0,
	1,1,1,

	0,0,0, // 중간층
	0,0,0,
	0,0,0,

	0,0,0, // 바닥층
	0,0,0,
	0,0,0,
};

const int WORLD_H = 10;
int world[WORLD_H][3][3] = {// 한층에 3x3면적을 가지는 10층짜리 월드생성
	//꼭대기 층
	0,0,0,
	0,0,0,
	0,0,0,

	0,0,0,
	0,0,0,
	0,0,0,

	0,0,0,
	0,0,0,
	0,0,0,

	0,0,0,
	0,0,0,
	0,0,0,

	0,0,0,
	0,0,0,
	0,0,0,

	0,0,0,
	0,0,0,
	0,0,0,

	0,0,0,
	0,0,0,
	0,0,0,

	0,0,0,
	0,0,0,
	0,0,0,

	0,0,0,
	0,0,0,
	0,0,0,

	//바닥층
	0,0,0,
	0,0,0,
	0,0,0,
};

void write_cube(vector<point_3d>& vertices, float x, float y, float z) {
	//정육면체의 정점 8개
	vertices.push_back(point_3d(x, y, z));
	vertices.push_back(point_3d(x + 1, y, z));
	vertices.push_back(point_3d(x + 1, y, z + 1));
	vertices.push_back(point_3d(x, y, z + 1));
	vertices.push_back(point_3d(x, y + 1, z));
	vertices.push_back(point_3d(x + 1, y + 1, z));
	vertices.push_back(point_3d(x + 1, y + 1, z + 1));
	vertices.push_back(point_3d(x, y + 1, z + 1));

	//원점 대칭으로 오프셋 변환
	for (int i = 0; i < 8; i++) {  // vertices는 inout value이기 때문에, 최근 추가한 8개에 대해서만 작업한다.
		point_3d& v = vertices[vertices.size() - 1 - i];
		v.x -= 1.5, v.y -= 1.5, v.z += 1.5;
	}
}



//정육면체가 가지는 12개 모서리의 {시작정점, 끝정점} 정보를 리턴
const vector<vector<int>> cube_lines = {
	{0, 1}, {1, 2}, {2, 3}, {3, 0},
	{4, 5}, {5, 6}, {6, 7}, {7, 4},
	{0, 4}, {1, 5}, {2, 6}, {3, 7}
};

void generate_blocks(int kind, vector<point_3d>& vertices, vector<vector<int>>& lines) {
	vertices.clear(); lines.clear();
	vector<vector<point_3d>> lines2;  // lines가 정점번호를 저장하는 반면, lines2는 x,y,z좌표를 저장
	int base_idx = 0;
	for (int z = 2; z >= 0; --z) for (int y = 0; y < 3; ++y) for (int x = 0; x < 3; ++x)
		if (blocks[kind][z][y][x] == 1) {
			write_cube(vertices, x, y, z);
			for (const auto& line : cube_lines)
				lines.push_back({ base_idx + line[0], base_idx + line[1] }),
				lines2.push_back({ vertices[lines[lines.size() - 1][0]], vertices[lines[lines.size() - 1][1]] });
			base_idx += 8;
		}

	// 1x1x1 큐브끼리 만나는 면의 모서리들을 중복되므로 제거하기 위해 overlapping_edges에 넣어준다.
	vector<pair<point_3d, point_3d>> overlapping_edges;
	map<int, int> overlap_idx;
	for (int i = 0; i < lines2.size(); ++i) {
		const auto& edge1 = lines2[i];
		for (int j = i + 1; j < lines2.size(); ++j) {
			const auto& edge2 = lines2[j];
			if ((edge1[0] == edge2[0] && edge1[1] == edge2[1]) ||
				(edge1[0] == edge2[1] && edge1[1] == edge2[0])) {
				overlapping_edges.push_back(make_pair(edge1[0], edge1[1]));
				overlap_idx[i]++, overlap_idx[j]++;
			}
		}
	}

	vector<vector<int>> outline_edges;
	for (int i = 0; i < lines.size(); ++i) {
		// 0: 겹치지 않는 외각선 모서리
		// 2: 2번 겹치는 경우, 오목하게 꺾이면서 3개의 큐브가 만나는 모서리인데, 원작게임 기준으로 외각선 표시해준다.
		// 나머지는 불필요하게 중복되는 모서리들이라 모조리 제거해준다.
		if (overlap_idx[i] == 2 || overlap_idx[i] == 0)
			outline_edges.push_back(lines[i]);
	}
	lines = outline_edges;
}

void generate_world_blocks(vector<point_3d>& vertices, vector<vector<int>>& faces) {
	vertices.clear(); faces.clear();
	int base_idx = 0;

	for (int z = WORLD_H - 1; z >= 0; --z) for (int y = 0; y < 3; ++y)for (int x = 0; x < 3; ++x)
		if (world[z][y][x]) {
			write_cube(vertices, x, y, z);
			faces.push_back({ base_idx + 1, base_idx + 2, base_idx + 6, base_idx + 5 });  // 동쪽 face
			faces.push_back({ base_idx, base_idx + 3, base_idx + 7, base_idx + 4 });      // 서쪽 face
			faces.push_back({ base_idx + 4, base_idx + 5, base_idx + 6, base_idx + 7 });  // 남쪽 face
			faces.push_back({ base_idx, base_idx + 1, base_idx + 2, base_idx + 3 });      // 북쪽 face
			faces.push_back({ base_idx, base_idx + 1, base_idx + 5, base_idx + 4 });      // 하늘쪽 face (바닥 face는 어차피 안보이므로 생략한다)
			base_idx += 8;
		}
}

sf::Color level_color[] = { sf::Color::Red, sf::Color::Cyan, sf::Color::Green, sf::Color::Blue, sf::Color(128, 128, 128), sf::Color::Yellow, sf::Color::Magenta,
							sf::Color::Red, sf::Color::Cyan, sf::Color::Green, sf::Color::Blue };

// 오일러 회전(실수 연산)
point_3d rotate(const point_3d& point, float angleX, float angleY, float angleZ) {
	float radX = angleX * 3.1415926 / 180.0f;  // degree를 radian으로 변환
	float radY = angleY * 3.1415926 / 180.0f;
	float radZ = angleZ * 3.1415926 / 180.0f;
	float sinX = sin(radX), cosX = cos(radX);
	float sinY = sin(radY), cosY = cos(radY);
	float sinZ = sin(radZ), cosZ = cos(radZ);
	return {
		point.x * (cosY * cosZ) + point.y * (-cosX * sinZ + sinX * sinY * cosZ) + point.z * (sinX * sinZ + cosX * sinY * cosZ),
		point.x * (cosY * sinZ) + point.y * (cosX * cosZ + sinX * sinY * sinZ) + point.z * (-sinX * cosZ + cosX * sinY * sinZ),
		point.x * (-sinY) + point.y * (sinX * cosY) + point.z * (cosX * cosY)
	};
}

point_3d camera_position(0, 0, 0.01);  // 카메라 위치 설정. 
point_3d look_at(0, 0, 1);  // 카메라가 바라보는 방향 설정

// 바닥이 확대되어 보이도록 Z=1이 아닌 Z=2평면에 투영
point_2d project(const point_3d& point_) {
	point_3d point = point_;
	point.z += 1.5f;
	float z_plane = 2.0f;  // Z=2평면에 투영
	point_3d direction = point - camera_position;
	float factor = (z_plane - camera_position.z) / direction.z;
	point_2d projected_point = point_2d(camera_position.x + direction.x * factor, camera_position.y + direction.y * factor);
	return projected_point;
}

// viewport변환 (화면크기 600, 600에 맞에 확대해준다
point_2d viewport(point_2d p) {
	//좌표계를 (-1, 1)에서 (0, 1)로 변환
	float normalizedX = (p.x + 1.0f) / 2.0f;
	float normalizedY = (p.y + 1.0f) / 2.0f;

	//(0,1)에서 실제 스크린 사이즈만큼 확대
	return { normalizedX * SCREEN_WIDTH, normalizedY * SCREEN_HEIGHT };
}

bool loadObjFile(const string filename, vector<point_3d>& vertices, vector<vector<int>>& faces) {
	ifstream file(filename);
	string line;
	while (getline(file, line)) {
		istringstream iss(line);
		string type;
		iss >> type;
		if (type == "v") {  //vertex
			float x, y, z; iss >> x >> y >> z;
			vertices.push_back(point_3d(x, y, z));
		}
		else if (type == "f") {  //face
			vector<int> face; string vertex;
			while (iss >> vertex) {
				istringstream viss(vertex);
				int index; char separator;
				viss >> index >> separator;
				face.push_back(index - 1);  // OBJ파일은 인덱스가 1부터 시작하므로 0으로 보정
			}
			faces.push_back(face);
		}
	}
	file.close();
	return true;
}
void draw_stage(sf::RenderWindow& window, vector<point_3d>& vertices, vector<vector<int>>& faces) {
	for (auto& face : faces) {
		sf::Vertex line[] = {
			sf::Vertex(viewport(project(vertices[face[0]])), sf::Color::Green),
			sf::Vertex(viewport(project(vertices[face[1]])), sf::Color::Green),
			sf::Vertex(viewport(project(vertices[face[2]])), sf::Color::Green),
			sf::Vertex(viewport(project(vertices[face[3]])), sf::Color::Green),
			sf::Vertex(viewport(project(vertices[face[0]])), sf::Color::Green),
		};
		window.draw(line, 5, sf::LinesStrip);
	}
}
void draw_world(sf::RenderWindow& window, vector<point_3d>& vertices, vector<vector<int>>& faces) {
	auto compareFaces = [&](const vector<int>& face1, const vector<int>& face2, vector<point_3d>& vertices) {
		//정육면체의 면에 대해서 각각 중심점(centroid)을 구함
		point_3d centroid1 = (vertices[face1[0]] + vertices[face1[1]] + vertices[face1[2]] + vertices[face1[3]]) / 4.0f;
		point_3d centroid2 = (vertices[face2[0]] + vertices[face2[1]] + vertices[face2[2]] + vertices[face2[3]]) / 4.0f;

		//중심점에 대해서 z축 깊이를 구함
		if (centroid1.z == centroid2.z) {  // tie breaker
			float distance1 = centroid1.x * centroid1.x + centroid1.y * centroid1.y;
			float distance2 = centroid2.x * centroid2.x + centroid2.y * centroid2.y;
			return distance1 > distance2;
		}
		return centroid1.z > centroid2.z;
	};


	//깊이를 기준으로 정렬하여, 외곽선이 이상해 보이는 부분을 수정
	stable_sort(faces.begin(), faces.end(), [&](const vector<int>& face1, const vector<int>& face2) {
		return compareFaces(face1, face2, vertices);
		});

	for (const auto& face : faces) {
		//색칠
		sf::ConvexShape polygon;
		polygon.setPointCount(4);
		polygon.setPoint(0, viewport(project(vertices[face[0]])));
		polygon.setPoint(1, viewport(project(vertices[face[1]])));
		polygon.setPoint(2, viewport(project(vertices[face[2]])));
		polygon.setPoint(3, viewport(project(vertices[face[3]])));
		polygon.setFillColor(level_color[(int)vertices[face[0]].z]);
		window.draw(polygon);

		//외곽선 표시
		sf::Vertex line[] = {
			sf::Vertex(viewport(project(vertices[face[0]])), sf::Color::Black),
			sf::Vertex(viewport(project(vertices[face[1]])), sf::Color::Black),
			sf::Vertex(viewport(project(vertices[face[2]])), sf::Color::Black),
			sf::Vertex(viewport(project(vertices[face[3]])), sf::Color::Black),
			sf::Vertex(viewport(project(vertices[face[0]])), sf::Color::Black),
		};
		window.draw(line, 5, sf::LineStrip);
	}
}

int main() {
	srand(time(0));
	vector<point_3d> vertex_stage;
	vector<vector<int>> face_stage;
	vector<point_3d> vertex_block;
	vector<vector<int>> line_block;
	vector<point_3d> vertex_world;
	vector<vector<int>> face_world;

	loadObjFile("obj/stage.obj", vertex_stage, face_stage);
	for (auto& v : vertex_stage) v.x -= 1.5, v.y -= 1.5, v.z += 1.5;
	for (int k = 0; k < BLOCK_CNT; ++k)for (int z = 0; z < 3; ++z) for (int y = 0; y < 3; ++y) for (int x = 0; x < 3; ++x)
		blocks_cpy[k][z][y][x] = blocks[k][z][y][x];

	int block_kind = 0, cz = 0;  // cz for current Z
	auto new_block = [&]() {
		block_kind = rand() % BLOCK_CNT, cz = 0;
		//block_kind = 0; //temp
		for (int z = 0; z < 3; ++z) for (int y = 0; y < 3; ++y) for (int x = 0; x < 3; ++x) blocks[block_kind][z][y][x] = blocks_cpy[block_kind][z][y][x];
	};
	new_block();

	int fake_block[3][3][3] = {};
	auto check_block = [&](bool fake = false) {
		int(*block)[3][3] = blocks[block_kind];
		if (fake) block = fake_block;
		for (int z = 0; z < 3; ++z) for (int y = 0; y < 3; ++y) for (int x = 0; x < 3; ++x) {
			if (block[z][y][x] == 0) continue;
			if (z + cz < 0 || z + cz >= 10) return false;  // 천장이나 바닥에 닿음
			if (world[cz + z][y][x]) return false;  // world블록들과 충돌
		}
		return true;
	};

	auto clear_lines = [&]() {
		int to = WORLD_H - 1;
		//바닥부터 천장 방향으로..
		bool audio_played = false;  // 여러층 클리어시 소리가 반복해서 나는걸 피하기 위함
		for (int from = WORLD_H - 1; from >= 0; from--) {
			int cube_cnt = 0;
			for (int y = 0; y < 3; ++y) for (int x = 0; x < 3; ++x)if (world[from][y][x]) cube_cnt++;
			//만약 3x3=9칸이 다 채워졌으면 클리어해준다.
			if (cube_cnt >= 9) {
				//한층 클리어..
				if (audio_played == false) {
					audio_played = true;
					static sf::SoundBuffer buffer;
					static sf::Sound sound;
					if (buffer.getSampleCount() == 0) {
						buffer.loadFromFile("snd/effect.wav");
						sound.setBuffer(buffer);
					}
					sound.play();
					while (sound.getStatus() == sf::Sound::Playing) sf::sleep(sf::microseconds(10));
				}
			}
			else {  // 블록은 있는데 9칸이 안채워졌으면 복사한다. (결과적으로 클리어된 층을 제외하고 복사해서 압축되는 모양이 됨)
				for (int y = 0; y < 3; ++y) for (int x = 0; x < 3; ++x) world[to][y][x] = world[from][y][x];
				to--;
			}
		}
		//to가 0이 될때까지 마저 복사해준다.
		for (int i = to; i >= 0; --i)
			for (int y = 0; y < 3; ++y) for (int x = 0; x < 3; ++x) world[to][y][x] = 0;
	};

	static bool game_over = false;
	auto go_down = [&]() {
		if (cz == 0 && check_block() == false) game_over = true;
		if (game_over) return false;
		cz++;
		if (check_block() == false) {  // 바닥이나 아래쪽 블럭에 닿음
			cz--;
			for (int z = 0; z < 3; ++z) for (int y = 0; y < 3; ++y) for (int x = 0; x < 3; ++x)
				if (blocks[block_kind][z][y][x]) world[cz + z][y][x] = 1;  // 닿은 부분의 블록을 월드블록으로 전환
			clear_lines();
			new_block();  // 바닥에 닿았으므로 새로운 블록 생성
			return false;
		}
		return true;
	};

	int maxx = 0, maxy = 0, maxz = 0;
	int minx = 2, miny = 2, minz = 2;
	auto get_minmax = [&](bool fake = false) {
		int(*block)[3][3] = blocks[block_kind];
		if (fake) block = fake_block;
		maxx = 0, maxy = 0, maxz = 0;
		minx = 2, miny = 2, minz = 2;
		for (int z = 0; z < 3; ++z) for (int y = 0; y < 3; ++y) for (int x = 0; x < 3; ++x)
			if (block[z][y][x])
				minx = min(minx, x), maxx = max(maxx, x),
				miny = min(miny, y), maxy = max(maxy, y),
				minz = min(minz, z), maxz = max(maxz, z);
	};

	auto move_block_inside = [&](int xx, int yy, bool fake = false) {
		assert(abs(xx) <= 1 && abs(yy) <= 1);
		get_minmax(fake);

		int(*block)[3][3] = blocks[block_kind];
		if (fake) block = fake_block;

		if (xx > 0 && maxx == 2) return false;
		if (xx < 0 && minx == 0) return false;
		if (yy > 0 && maxy == 2) return false;
		if (yy < 0 && miny == 0) return false;

		int ori[3][3][3] = {}; for (int z = 0; z < 3; ++z) for (int y = 0; y < 3; ++y) for (int x = 0; x < 3; ++x) ori[z][y][x] = block[z][y][x];
		if (xx < 0) {
			int temp[3][3][3] = {};
			for (int z = 0; z < 3; ++z) for (int y = 0; y < 3; ++y) for (int x = 0; x < 2; ++x) temp[z][y][x] = block[z][y][x + 1];
			for (int z = 0; z < 3; ++z) for (int y = 0; y < 3; ++y) for (int x = 0; x < 3; ++x) block[z][y][x] = temp[z][y][x];
		}
		if (xx > 0) {
			int temp[3][3][3] = {};
			for (int z = 0; z < 3; ++z) for (int y = 0; y < 3; ++y) for (int x = 0; x < 2; ++x) temp[z][y][x + 1] = block[z][y][x];
			for (int z = 0; z < 3; ++z) for (int y = 0; y < 3; ++y) for (int x = 0; x < 3; ++x) block[z][y][x] = temp[z][y][x];
		}
		if (yy < 0) {
			int temp[3][3][3] = {};
			for (int z = 0; z < 3; ++z) for (int y = 0; y < 2; ++y) for (int x = 0; x < 3; ++x) temp[z][y][x] = block[z][y + 1][x];
			for (int z = 0; z < 3; ++z) for (int y = 0; y < 3; ++y) for (int x = 0; x < 3; ++x) block[z][y][x] = temp[z][y][x];
		}
		if (yy > 0) {
			int temp[3][3][3] = {};
			for (int z = 0; z < 3; ++z) for (int y = 0; y < 2; ++y) for (int x = 0; x < 3; ++x) temp[z][y + 1][x] = block[z][y][x];
			for (int z = 0; z < 3; ++z) for (int y = 0; y < 3; ++y) for (int x = 0; x < 3; ++x) block[z][y][x] = temp[z][y][x];
		}
		if (check_block(fake) == false) {
			for (int z = 0; z < 3; ++z) for (int y = 0; y < 3; ++y) for (int x = 0; x < 3; ++x) block[z][y][x] = ori[z][y][x];
			return false;
		}
		return true;
	};


	sf::RenderWindow window(sf::VideoMode(SCREEN_WIDTH, SCREEN_HEIGHT), "3D Tetris");


	const int ANI_STEP = 50; // animation steps
	auto smooth_draw = [&](sf::Vector3f angle, point_3d center, point_3d center_to, float speedMultiplier = 1.0) {
		int TO_STEP = ANI_STEP / speedMultiplier;
		for (int step = 0; step < TO_STEP; step++) {
			window.clear();
			draw_stage(window, vertex_stage, face_stage);
			draw_world(window, vertex_world, face_world);
			auto draw_block = [&]() {
				for (const auto& line : line_block) {
					sf::Vertex lineVertices[] = {
						sf::Vertex(viewport(project(vertex_block[line[0]] + point_3d(
							(center_to.x - center.x) * step / TO_STEP,
							(center_to.y - center.y) * step / TO_STEP,
							(center_to.z - center.z) * step / TO_STEP + cz)))),
						sf::Vertex(viewport(project(vertex_block[line[1]] + point_3d(
							(center_to.x - center.x) * step / TO_STEP,
							(center_to.y - center.y) * step / TO_STEP,
							(center_to.z - center.z) * step / TO_STEP + cz))))
					};
					window.draw(lineVertices, 2, sf::Lines);
				}
			};
			draw_block();
			for (auto& vertex : vertex_block)
				vertex = rotate(vertex - center, angle.x / TO_STEP, angle.y / TO_STEP, angle.z / TO_STEP) + center;

			window.display();
		}
	};
	sf::Clock clock;
	while (window.isOpen()) {
		auto move_block = [&](point_2d delta) {
			for (int z = 0; z < 3; ++z)for (int y = 0; y < 3; ++y)for (int x = 0; x < 3; ++x) fake_block[z][y][x] = blocks[block_kind][z][y][x];
			get_minmax(); point_3d center((minx + maxx) / 2.0 - 1.0, (miny + maxy) / 2.0 - 1.0, 0);
			move_block_inside(delta.x, delta.y, true); get_minmax(true); point_3d center_to((minx + maxx) / 2.0 - 1.0, (miny + maxy) / 2.0 - 1.0, 0);
			smooth_draw({ 0,0,0 }, center, center_to);
			move_block_inside(delta.x, delta.y);
			generate_blocks(block_kind, vertex_block, line_block);
		};
		auto move_down = [&](float speedMultiplier = 1.0) {
			if (game_over) return false;
			for (int z = 0; z < 3; ++z)for (int y = 0; y < 3; ++y)for (int x = 0; x < 3; ++x) fake_block[z][y][x] = blocks[block_kind][z][y][x];
			get_minmax(); point_3d center(0, 0, 0);
			get_minmax(true); point_3d center_to(0, 0, 1);
			smooth_draw({ 0,0,0 }, center, center_to, speedMultiplier);
			return go_down();
		};
		static float prev = clock.getElapsedTime().asSeconds();
		if (clock.getElapsedTime().asSeconds() - prev >= 1.5) {
			prev = clock.getElapsedTime().asSeconds();
			move_down();
		}
		float angleX = 0, angleY = 0, angleZ = 0;
		sf::Event event;
		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed) window.close();
			if (event.type == sf::Event::KeyPressed) {
				switch (event.key.code) {
				case sf::Keyboard::Q: angleX = -90.0f; break;
				case sf::Keyboard::W: angleY = -90.0f; break;
				case sf::Keyboard::E: angleZ = -90.0f; break;
				case sf::Keyboard::A: angleX = 90.0f; break;
				case sf::Keyboard::S: angleY = 90.0f; break;
				case sf::Keyboard::D: angleZ = 90.0f; break;
				case sf::Keyboard::Up:    move_block({ 0, -1 }); break;
				case sf::Keyboard::Down:  move_block({ 0, 1 }); break;
				case sf::Keyboard::Left:  move_block({ -1, 0 }); break;
				case sf::Keyboard::Right: move_block({ 1, 0 }); break;
				case sf::Keyboard::Space:
					while (move_down(10.0)); break;
				default: break;
				}
			}
		}
		{//정수회전
			auto rotateX = [&](bool fake = false) {
				int temp[3][3][3] = {};
				int(*block)[3][3];
				block = blocks[block_kind];
				if (fake) block = fake_block;
				int cy = 0; while (move_block_inside(0, -1, fake)) cy++;
				for (int z = 0; z < maxy + 1; ++z)for (int y = 0; y < maxz + 1; ++y)for (int x = 0; x < 3; ++x)temp[z][y][x] = block[y][maxy - z][x];
				for (int z = 0; z < 3; ++z)for (int y = 0; y < 3; ++y)for (int x = 0; x < 3; ++x) block[z][y][x] = temp[z][y][x];
				for (int i = 0; i < cy; ++i) move_block_inside(0, 1, fake);
			};
			auto rotateY = [&](bool fake = false) {
				int temp[3][3][3] = {};
				int(*block)[3][3];
				block = blocks[block_kind];
				if (fake) block = fake_block;
				int cx = 0; while (move_block_inside(-1, 0, fake)) cx++;
				for (int z = 0; z < maxx + 1; ++z)for (int y = 0; y < 3; ++y)for (int x = 0; x < maxz + 1; ++x)temp[z][y][x] = block[maxz - x][y][z];
				for (int z = 0; z < 3; ++z)for (int y = 0; y < 3; ++y)for (int x = 0; x < 3; ++x) block[z][y][x] = temp[z][y][x];
				for (int i = 0; i < cx; ++i) move_block_inside(1, 0, fake);
			};
			auto rotateZ = [&](bool fake = false) {
				int temp[3][3][3] = {};
				int(*block)[3][3];
				block = blocks[block_kind];
				if (fake) block = fake_block;
				int cx = 0; while (move_block_inside(-1, 0, fake)) cx++;
				int cy = 0; while (move_block_inside(0, -1, fake)) cy++;
				for (int z = 0; z < 3; ++z)for (int y = 0; y < maxx + 1; ++y)for (int x = 0; x < maxy + 1; ++x)temp[z][y][x] = block[z][maxy - x][y];
				for (int z = 0; z < 3; ++z)for (int y = 0; y < 3; ++y)for (int x = 0; x < 3; ++x) block[z][y][x] = temp[z][y][x];
				for (int i = 0; i < cx; ++i) move_block_inside(1, 0, fake);
				for (int i = 0; i < cy; ++i) move_block_inside(0, 1, fake);
			};
			auto smooth_rotateX = [&](float angle) {
				get_minmax(true); point_3d center_to(0, (miny + maxy) / 2.0 - 1.0, (minz + maxz) / 2.0 + 2.0);
				get_minmax(); point_3d center(0, (miny + maxy) / 2.0 - 1.0, (minz + maxz) / 2.0 + 2.0);
				smooth_draw({ angle, 0, 0 }, center, center_to);
			};
			auto smooth_rotateY = [&](float angle) {
				get_minmax(true); point_3d center_to((minx + maxx) / 2.0 - 1.0, 0, (minz + maxz) / 2.0 + 2.0);
				get_minmax(); point_3d center((minx + maxx) / 2.0 - 1.0, 0, (minz + maxz) / 2.0 + 2.0);
				smooth_draw({ 0, angle, 0 }, center, center_to);
			};
			auto smooth_rotateZ = [&](float angle) {
				get_minmax(true); point_3d center_to((minx + maxx) / 2.0 - 1.0, (miny + maxy) / 2.0 - 1.0, 0);
				get_minmax(); point_3d center((minx + maxx) / 2.0 - 1.0, (miny + maxy) / 2.0 - 1.0, 0);
				smooth_draw({ 0,0,angle }, center, center_to);
			};
			for (int z = 0; z < 3; ++z) for (int y = 0; y < 3; ++y) for (int x = 0; x < 3; ++x) fake_block[z][y][x] = blocks[block_kind][z][y][x];

			if (angleX > 0) {
				rotateX(true); smooth_rotateX(-90);
				rotateX();
				if (check_block() == false) {
					rotateX(), rotateX(), rotateX();  // 한쪽 방향으로 세 번 돌리면, 반대 방향으로 한 번 돌린 것과 같다.
				}
			}
			else if (angleX < 0) {
				rotateX(true); smooth_rotateX(90);
				rotateX(), rotateX(), rotateX();
				if (check_block() == false) rotateX();
			}
			if (angleY > 0) {
				rotateY(true); smooth_rotateY(90);
				rotateY(), rotateY(), rotateY();
				if (check_block() == false) rotateY();
			}
			else if (angleY < 0) {
				rotateY(true); smooth_rotateY(-90);
				rotateY();
				if (check_block() == false) {
					rotateY(), rotateY(), rotateY();
				}
			}
			if (angleZ > 0) {
				rotateZ(true); smooth_rotateZ(90);
				rotateZ();
				if (check_block() == false) {
					rotateZ(), rotateZ(), rotateZ();
				}
			}
			else if (angleZ < 0) {
				rotateZ(true); smooth_rotateZ(-90);
				rotateZ(), rotateZ(), rotateZ();
				if (check_block() == false) rotateZ();
			}
		}
		window.clear();
		draw_stage(window, vertex_stage, face_stage);
		generate_world_blocks(vertex_world, face_world);
		draw_world(window, vertex_world, face_world);
		if (game_over) {
			sf::Font font;
			font.loadFromFile("font/arial.ttf");
			sf::Text text; text.setFont(font); text.setFillColor(sf::Color::Red);
			text.setString("Game Over!");
			text.setCharacterSize(54);  // in pixels
			sf::FloatRect textRect = text.getLocalBounds();
			text.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
			text.setPosition(point_2d(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2));
			window.draw(text);
		}
		else {
			generate_blocks(block_kind, vertex_block, line_block);
			auto draw_block = [&]() {
				for (const auto& line : line_block) {
					sf::Vertex lineVertices[] = {
						sf::Vertex(viewport(project(vertex_block[line[0]] + point_3d(0, 0, cz)))),
						sf::Vertex(viewport(project(vertex_block[line[1]] + point_3d(0, 0, cz))))
					};
					window.draw(lineVertices, 2, sf::Lines);
				};
			};
			draw_block();
		}
		window.display();
	}
	return 0;
}
