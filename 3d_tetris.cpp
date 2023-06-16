#include <SFML/Graphics.hpp>
#include <cmath>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <numeric>
#include <map>
#include <algorithm>
#include <set>
#include <assert.h>
using namespace std;

typedef sf::Vector3f point_3d;
typedef sf::Vector2f point_2d;
const int Z_OFFSET = 5;//원근감 조절을 위함

int blocks_cpy[7][3][3][3];
int blocks[7][3][3][3] = {
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
};
const int WORLD_H = 10;
int world[WORLD_H][3][3] = {
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

    0,0,0,
    0,0,0,
    0,0,0,
};


sf::Color lv_color[] = {sf::Color::Red, sf::Color::Cyan, sf::Color::Green, sf::Color::Blue, sf::Color(128, 128, 128), sf::Color::Yellow, sf::Color::Magenta, sf::Color::Red,
                     sf::Color::Cyan, sf::Color::Green, sf::Color::Blue};

sf::Vector3f rotate(const sf::Vector3f& point, float angleX, float angleY, float angleZ) {
    float radX = angleX * M_PI / 180.0f;
    float radY = angleY * M_PI / 180.0f;
    float radZ = angleZ * M_PI / 180.0f;
    float sinX = std::sin(radX), cosX = std::cos(radX);
    float sinY = std::sin(radY), cosY = std::cos(radY);
    float sinZ = std::sin(radZ), cosZ = std::cos(radZ);

    return {
        point.x * (cosY * cosZ) + point.y * (-cosX * sinZ + sinX * sinY * cosZ) + point.z * (sinX * sinZ + cosX * sinY * cosZ),
        point.x * (cosY * sinZ) + point.y * (cosX * cosZ + sinX * sinY * sinZ) + point.z * (-sinX * cosZ + cosX * sinY * sinZ),
        point.x * (-sinY) + point.y * (sinX * cosY) + point.z * (cosX * cosY)
    };
}
/*
sf::Vector2f project(const sf::Vector3f& point, float fov = M_PI/2) {
    float factor = tan(fov / 2.0f) / point.z;
    return sf::Vector2f(point.x * factor, point.y * factor);
}
*/
sf::Vector3f camera_position(0, 0, 0);  // 카메라 위치 설정. y값을 높게 설정하여 카메라를 높이 위치시킴
sf::Vector3f look_at(0, 0, 1);  // 카메라가 바라보는 방향 설정. y값을 낮게 설정하여 카메라가 아래를 향하게 함

sf::Vector2f project(const sf::Vector3f& point_) {
    point_3d point = point_;
    point.z += 1.50;
    float z_plane = 2.0f;  // Projection plane on z-axis
    sf::Vector3f direction = point - camera_position;
    float factor = (z_plane - camera_position.z) / direction.z;
    sf::Vector2f projected_point = sf::Vector2f(camera_position.x + direction.x * factor, camera_position.y + direction.y * factor);


    return projected_point;
}


// 뷰포트 변환
point_2d viewport(point_2d p, float screenWidth, float screenHeight) {
    // 좌표계를 (-1, 1)에서 (0, 1)로 변환
    float normalizedX = (p.x + 1.0f) / 2.0f;
    float normalizedY = (p.y + 1.0f) / 2.0f;

    // (0, 1)에서 실제 스크린 사이즈 만큼 확대
    float screenX = normalizedX * screenWidth;
    float screenY = normalizedY * screenHeight;

    return point_2d(screenX, screenY);
}

bool loadObjFile(const std::string& filename, std::vector<sf::Vector3f>& vertices, std::vector<std::vector<int>>& faces, std::vector<std::vector<int>>& lines) {
    std::ifstream file(filename);
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string type;
        iss >> type;

        if (type == "v") { // vertex
            float x, y, z; iss >> x >> y >> z;
            vertices.push_back(sf::Vector3f(x, y, z));
        } else if (type == "f") { // face
            std::vector<int> face; std::string vertex;
            while (iss >> vertex) {
                std::istringstream viss(vertex);
                int index; char separator;
                viss >> index >> separator;
                face.push_back(index - 1); // OBJ indices start from 1
            }
            faces.push_back(face);
        } else if (type == "l") { // line
            std::vector<int> line; std::string vertex;
            while (iss >> vertex) {
                std::istringstream viss(vertex);
                int index; char separator;
                viss >> index >> separator;
                line.push_back(index - 1); // OBJ indices start from 1
            }
            lines.push_back(line);
        }
    }
    file.close();
    return true;
}

void draw_stage(sf::RenderWindow& window, std::vector<sf::Vector3f>& vertices, std::vector<std::vector<int>>& faces) {
    for (const auto& face : faces) {
        sf::Vertex line[] = {
            sf::Vertex(viewport(project(vertices[face[0]]), 600, 600), sf::Color::Green),
            sf::Vertex(viewport(project(vertices[face[1]]), 600, 600), sf::Color::Green),
            sf::Vertex(viewport(project(vertices[face[2]]), 600, 600), sf::Color::Green),
            sf::Vertex(viewport(project(vertices[face[3]]), 600, 600), sf::Color::Green),
            sf::Vertex(viewport(project(vertices[face[0]]), 600, 600), sf::Color::Green)
        };
        window.draw(line, 5, sf::LineStrip);
    }
}
bool compareFaces(const std::vector<int>& face1, const std::vector<int>& face2, const std::vector<sf::Vector3f>& vertices) {
    sf::Vector3f centroid1 = (vertices[face1[0]] + vertices[face1[1]] + vertices[face1[2]] + vertices[face1[3]]) / 4.0f;
    sf::Vector3f centroid2 = (vertices[face2[0]] + vertices[face2[1]] + vertices[face2[2]] + vertices[face2[3]]) / 4.0f;

    // Compute the distance from each centroid to the z-axis (in 3D space)
    if(centroid1.z == centroid2.z) {
        float distance1 = sqrt(centroid1.x * centroid1.x + centroid1.y * centroid1.y);
        float distance2 = sqrt(centroid2.x * centroid2.x + centroid2.y * centroid2.y);
        return distance1 > distance2;
    }

    return centroid1.z > centroid2.z;
}

void draw_world(sf::RenderWindow& window, std::vector<sf::Vector3f>& vertices, std::vector<std::vector<int>>& faces) {
    // sort the faces by depth
    std::stable_sort(faces.begin(), faces.end(), [&](const std::vector<int>& face1, const std::vector<int>& face2) {
        return compareFaces(face1, face2, vertices);
    });
    
    for (const auto& face : faces) {
        sf::ConvexShape polygon;
        polygon.setPointCount(4);
        polygon.setPoint(0, viewport(project(vertices[face[0]]), 600, 600));
        polygon.setPoint(1, viewport(project(vertices[face[1]]), 600, 600));
        polygon.setPoint(2, viewport(project(vertices[face[2]]), 600, 600));
        polygon.setPoint(3, viewport(project(vertices[face[3]]), 600, 600));
        polygon.setFillColor(lv_color[(int)vertices[face[0]].z]);
        window.draw(polygon);
        

        sf::Vertex line[] = {
            sf::Vertex(viewport(project(vertices[face[0]]), 600, 600), sf::Color::Black),
            sf::Vertex(viewport(project(vertices[face[1]]), 600, 600), sf::Color::Black),
            sf::Vertex(viewport(project(vertices[face[2]]), 600, 600), sf::Color::Black),
            sf::Vertex(viewport(project(vertices[face[3]]), 600, 600), sf::Color::Black),
            sf::Vertex(viewport(project(vertices[face[0]]), 600, 600), sf::Color::Black)
        };
        window.draw(line, 5, sf::LineStrip);
    }
}


void write_cube(std::vector<sf::Vector3f>& vertices, float x, float y, float z) {
    vertices.push_back(sf::Vector3f(x, y, z));
    vertices.push_back(sf::Vector3f(x+1, y, z));
    vertices.push_back(sf::Vector3f(x+1, y, z+1));
    vertices.push_back(sf::Vector3f(x, y, z+1));
    vertices.push_back(sf::Vector3f(x, y+1, z));
    vertices.push_back(sf::Vector3f(x+1, y+1, z));
    vertices.push_back(sf::Vector3f(x+1, y+1, z+1));
    vertices.push_back(sf::Vector3f(x, y+1, z+1));

    for(int i=0;i<8;i++){
        sf::Vector3f& vertex = vertices[vertices.size()-1-i];
        vertex.x -= 1.5;
        vertex.y -= 1.5;
        vertex.z += 1.5;
    }
}

void generate_world_blocks(int world[10][3][3], std::vector<sf::Vector3f>& vertices, std::vector<std::vector<int>>& faces) {
    vertices.clear();
    faces.clear();
    int base_idx = 0;

    //for (int z = 0; z < 5; ++z) {
    for (int z = 9; z >= 0; --z) {  // 9가 바닥
        for (int y = 0; y < 3; ++y) {
            for (int x = 0; x < 3; ++x) {
                if (world[z][y][x] == 1) {
                    write_cube(vertices, x, y, z);
                    //faces.push_back({base_idx+2, base_idx+3, base_idx+7, base_idx+6}); // 바닥 back face
                    faces.push_back({base_idx+1, base_idx+2, base_idx+6, base_idx+5});   // E right face
                    faces.push_back({base_idx, base_idx+3, base_idx+7, base_idx+4});     // W left face
                    faces.push_back({base_idx+4, base_idx+5, base_idx+6, base_idx+7});   // S top face
                    faces.push_back({base_idx, base_idx+1, base_idx+2, base_idx+3});     // N bottom face
                    faces.push_back({base_idx, base_idx+1, base_idx+5, base_idx+4});     // 뚜껑 front face
                    base_idx += 8;
                }
            }
        }
    }
    /*
    std::sort(faces.begin(), faces.end(), [&](const std::vector<int>& face1, const std::vector<int>& face2) {
        return compareFaces(face1, face2, vertices);
    });
    */
}

const std::vector<std::vector<int>> cube_lines = {
    {0, 1}, {1, 2}, {2, 3}, {3, 0},
    {4, 5}, {5, 6}, {6, 7}, {7, 4},
    {0, 4}, {1, 5}, {2, 6}, {3, 7}
};

void generate_blocks(int blocks[7][3][3][3], int kind, std::vector<sf::Vector3f>& vertices, std::vector<std::vector<int>>& lines) {
    vertices.clear();
    lines.clear();
    int base_idx = 0;

    for (int z = 2; z >= 0; --z) {
        for (int y = 0; y < 3; ++y) {
            for (int x = 0; x < 3; ++x) {
                if (blocks[kind][z][y][x] == 1) {
                    write_cube(vertices, x, y, z);
                    int cnt = 0;
                     for (const auto& line : cube_lines) {
                        lines.push_back({base_idx + line[0], base_idx + line[1]});
                    }
                    base_idx += 8;
                }
            }
        }
    }

    vector<vector<point_3d>> lines2;
    for (const auto& line : lines) {
        vector<point_3d> vp;
        vp.push_back(vertices[line[0]]);
        vp.push_back(vertices[line[1]]);
        lines2.push_back(vp);
    }

    std::vector<std::pair<sf::Vector3f, sf::Vector3f>> overlapping_edges;
    map<int, int> overlap_idx;

    for (size_t i = 0; i < lines2.size(); ++i) {
        const auto& edge1 = lines2[i];
        
        for (size_t j = i + 1; j < lines2.size(); ++j) {
            const auto& edge2 = lines2[j];
            
            if ((edge1[0] == edge2[0] && edge1[1] == edge2[1]) ||
                (edge1[0] == edge2[1] && edge1[1] == edge2[0])) {
                overlapping_edges.push_back(std::make_pair(edge1[0], edge1[1]));
                overlap_idx[i] ++;
                overlap_idx[j] ++;
            }
        }
    }
    vector<vector<int>> lines3;
    for (size_t i = 0; i < lines.size(); ++i) {
        if(overlap_idx[i]==2 || overlap_idx[i]==0)
            lines3.push_back(lines[i]);
    }
    lines = lines3;
}

int main() {
    srand(time(0));
    sf::RenderWindow window(sf::VideoMode(600, 600), "3D Object");

    std::vector<sf::Vector3f> vs;
    std::vector<std::vector<int>> fs;
    std::vector<std::vector<int>> ls;
    loadObjFile("obj/stage.obj", vs, fs, ls);
    for (auto& vertex : vs) {
        vertex.x -= 1.5;
        vertex.y -= 1.5;
        vertex.z += 1.5;
    }

    std::vector<sf::Vector3f> vb;
    std::vector<std::vector<int>> fb;
    std::vector<std::vector<int>> lb;
    //loadObjFile("obj/t_block_0.obj", v2, f2, l2);
    for(int k=0;k<7;k++)for(int z=0;z<3;z++)for(int y=0;y<3;y++)for(int x=0;x<3;x++) blocks_cpy[k][z][y][x] = blocks[k][z][y][x];
    generate_blocks(blocks, 0, vb, lb);

    sf::Vector3f center(0, 0, 0);  // 객체의 중심점 정의

    std::vector<sf::Vector3f> vertices;
    std::vector<std::vector<int>> faces;
    std::vector<std::vector<int>> lines;
    generate_world_blocks(world, vertices, faces);
    int kind, cz;
	auto new_block = [&]()
	{
		kind = rand() % 6, cz = 0;
        //kind = 4;//temp
        for(int z=0;z<3;z++)for(int y=0;y<3;y++)for(int x=0;x<3;x++) blocks[kind][z][y][x] = blocks_cpy[kind][z][y][x];
	};
	new_block();

    int fake_block[3][3][3] = {};
	auto check_block = [&](bool fake=false)
	{
        int (*block)[3][3] = blocks[kind];
        if(fake){
            //for (int z = 0; z < 3; ++z) for (int y = 0; y < 3; ++y) for (int x = 0; x < 3; ++x) fake_block[z][y][x] = block[z][y][x];
            block=fake_block;
        }
        for (int z = 0; z < 3; z++) {
            for (int y = 0; y < 3; y++) {
                for (int x = 0; x < 3; x++) {
                    if (block[z][y][x] == 0) {
                        continue;
                    }
                    if (z + cz < 0 || z + cz >= 10) {
                        return false; // hit ceil or bottom
                    }
                    if (world[cz + z][y][x]) {
                        return false; // collision with world blocks
                    }
                }
            }
        }
		return true;
	};

    int maxx = 0, maxy = 0, maxz = 0;
    int minx = 2, miny = 2, minz = 2;
    auto get_minmax = [&](bool fake=false) {
        int (*block)[3][3] = blocks[kind];
        if(fake){
            //for (int z = 0; z < 3; ++z) for (int y = 0; y < 3; ++y) for (int x = 0; x < 3; ++x) fake_block[z][y][x] = block[z][y][x];
            block=fake_block;
        }
        maxx = 0, maxy = 0, maxz = 0;
        minx = 2, miny = 2, minz = 2;
        for (int z = 0; z < 3; ++z) for (int y = 0; y < 3; ++y) for (int x = 0; x < 3; ++x) {
            if(block[z][y][x])
                minx = min(minx, x), maxx = max(maxx, x),
                miny = min(miny, y), maxy = max(maxy, y),
                minz = min(minz, z), maxz = max(maxz, z);
        } 

    };
	auto move_block_inside = [&](int xx, int yy, bool fake=false)
	{
        assert(abs(xx)<=1 && abs(yy)<=1);
        get_minmax(fake);

        int (*block)[3][3] = blocks[kind];
        if(fake){
            //for (int z = 0; z < 3; ++z) for (int y = 0; y < 3; ++y) for (int x = 0; x < 3; ++x) fake_block[z][y][x] = block[z][y][x];
            block=fake_block;
        }

        if(xx>0 && maxx==2) return false;
        if(xx<0 && minx==0) return false;
        if(yy>0 && maxy==2) return false;
        if(yy<0 && miny==0) return false;

        int ori[3][3][3] = {};
        for (int z = 0; z < 3; ++z) for (int y = 0; y < 3; ++y) for (int x = 0; x < 3; ++x) ori[z][y][x] = block[z][y][x];

        if(xx<0){
            int temp[3][3][3] = {};
            for (int z = 0; z < 3; ++z) for (int y = 0; y < 3; ++y) for (int x = 0; x < 2; ++x) temp[z][y][x] = block[z][y][x+1];
            for (int z = 0; z < 3; ++z) for (int y = 0; y < 3; ++y) for (int x = 0; x < 3; ++x) block[z][y][x] = temp[z][y][x];
        }
        if(xx>0){
            int temp[3][3][3] = {};
            for (int z = 0; z < 3; ++z) for (int y = 0; y < 3; ++y) for (int x = 0; x < 2; ++x) temp[z][y][x+1] = block[z][y][x];
            for (int z = 0; z < 3; ++z) for (int y = 0; y < 3; ++y) for (int x = 0; x < 3; ++x) block[z][y][x] = temp[z][y][x];
        }
        if(yy<0){
            int temp[3][3][3] = {};
            for (int z = 0; z < 3; ++z) for (int y = 0; y < 2; ++y) for (int x = 0; x < 3; ++x) temp[z][y][x] = block[z][y+1][x];
            for (int z = 0; z < 3; ++z) for (int y = 0; y < 3; ++y) for (int x = 0; x < 3; ++x) block[z][y][x] = temp[z][y][x];
        }
        if(yy>0){
            int temp[3][3][3] = {};
            for (int z = 0; z < 3; ++z) for (int y = 0; y < 2; ++y) for (int x = 0; x < 3; ++x) temp[z][y+1][x] = block[z][y][x];
            for (int z = 0; z < 3; ++z) for (int y = 0; y < 3; ++y) for (int x = 0; x < 3; ++x) block[z][y][x] = temp[z][y][x];
        }
        if(check_block(fake)==false){
            for (int z = 0; z < 3; ++z) for (int y = 0; y < 3; ++y) for (int x = 0; x < 3; ++x) block[z][y][x] = ori[z][y][x];
            return false;
        }
        return true;
	};

	auto clear_lines = [&]()
	{
		int to = WORLD_H - 1;
		//from bottom line to top line...
		for (int from = WORLD_H - 1; from >= 0; from--)
		{
			int cnt = 0;
			for (int y = 0; y < 3; y++)
			for (int x = 0; x < 3; x++)
                if (world[from][y][x])cnt++;
			//if current line is not full, copy it(survived line)
			if (cnt < 3 * 3)
			{
                for (int y = 0; y < 3; y++)
                for (int x = 0; x < 3; x++)
				    world[to][y][x] = world[from][y][x];
				to--;
			}
			//otherwise it will be deleted(clear the line)
		}
        for (int i = to; i>=0;i--){
            for (int y = 0; y < 3; y++)
            for (int x = 0; x < 3; x++)
                world[i][y][x] = 0;
        }
	};
    bool game_over = false;
	auto go_down = [&]()
	{
		if (cz == 0 && check_block() == false) {// game over
            game_over = true;
        }
        if (game_over) return false;
		cz++;
		if (check_block() == false) // hit bottom
		{
			cz--;
			for(int z=0;z<3;z++)for (int y = 0; y < 3; y++)for (int x = 0; x < 3; x++)
				if (blocks[kind][z][y][x])
				{
					world[cz+z][y][x] = 1; //kind + 1;//+1 for avoiding 0
				}
			clear_lines();
            //start next block
            new_block();
			return false;
		}
		return true;
	};

    sf::Clock clock;
    while (window.isOpen()) {
		static float prev = clock.getElapsedTime().asSeconds();
		if (clock.getElapsedTime().asSeconds() - prev >= 555.5)
		{
			prev = clock.getElapsedTime().asSeconds();
			go_down();
		}
        sf::Event event;
        float angleX = 0, angleY = 0, angleZ = 0;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::KeyPressed) {
                switch (event.key.code) {
                    case sf::Keyboard::Q: angleX = -90.0f; break;
                    case sf::Keyboard::W: angleY = -90.0f; break;
                    case sf::Keyboard::E: angleZ = -90.0f; break;
                    case sf::Keyboard::A: angleX = 90.0f; break;
                    case sf::Keyboard::S: angleY = 90.0f; break;
                    case sf::Keyboard::D: angleZ = 90.0f; break;
                    case sf::Keyboard::Up:
                        move_block_inside(0,-1); break;
                    case sf::Keyboard::Down:
                        move_block_inside(0,1); break;
                    case sf::Keyboard::Left:
                        move_block_inside(-1,0); break;
                    case sf::Keyboard::Right:
                        move_block_inside(1,0); break;
                    case sf::Keyboard::Space:
                        while(go_down() == true);
                        break;
                    default: break;
                }
            }
        }

        //for (auto& vertex : vb) vertex = rotate(vertex - center, angleX, angleY, angleZ) + center;
        //인티저회전
        {
            auto rotateZ = [&](bool fake = false) {
                int temp[3][3][3] = {};
                int (*block)[3][3];
                block = blocks[kind];
                if(fake){
                    block = fake_block;
                }

                int cx = 0; while(move_block_inside(-1,0, fake)) cx++;
                int cy = 0; while(move_block_inside(0,-1, fake)) cy++;
                for (int z = 0; z < 3; ++z) for (int y = 0; y < maxx+1; ++y) for (int x = 0; x < maxy+1; ++x) temp[z][y][x] = block[z][maxy-x][y];
                for (int z = 0; z < 3; ++z) for (int y = 0; y < 3; ++y) for (int x = 0; x < 3; ++x) block[z][y][x] = temp[z][y][x];
                for(int i=0;i<cx;i++) move_block_inside(1,0, fake);
                for(int i=0;i<cy;i++) move_block_inside(0,1, fake);
            };
            auto rotateY = [&](bool fake = false) {
                int temp[3][3][3] = {};
                int (*block)[3][3];
                block = blocks[kind];
                if(fake){
                    block = fake_block;
                }
                int cx = 0; while(move_block_inside(-1,0, fake)) cx++;
                for (int z = 0; z < maxx+1; ++z) for (int y = 0; y < 3; ++y) for (int x = 0; x < maxz+1; ++x) temp[z][y][x] = block[maxz-x][y][z];
                for (int z = 0; z < 3; ++z) for (int y = 0; y < 3; ++y) for (int x = 0; x < 3; ++x) block[z][y][x] = temp[z][y][x];
                for(int i=0;i<cx;i++) move_block_inside(1,0, fake);
            };
            auto rotateX = [&](bool fake = false) {
                int temp[3][3][3] = {};
                int cy = 0; while(move_block_inside(0,-1, fake)) cy++;
                for (int z = 0; z < maxy+1; ++z) for (int y = 0; y < maxz+1; ++y) for (int x = 0; x < 3; ++x) temp[z][y][x] = blocks[kind][y][maxy-z][x];
                for (int z = 0; z < 3; ++z) for (int y = 0; y < 3; ++y) for (int x = 0; x < 3; ++x) blocks[kind][z][y][x] = temp[z][y][x];
                for(int i=0;i<cy;i++) move_block_inside(0,1, fake);
            };
            auto smooth_rotateZ = [&](float angle)
            {// animation here
                get_minmax(true);
                sf::Vector3f center_to((minx+maxx)/2.0-1.0, (miny+maxy)/2.0-1, 0);  // 객체의 중심점 정의

                get_minmax();
                sf::Vector3f center((minx+maxx)/2.0-1.0, (miny+maxy)/2.0-1, 0);  // 객체의 중심점 정의
                printf("center: %f, %f, %f, center_to: %f, %f, %f\n", center.x, center.y, center.z, center_to.x, center_to.y, center_to.z);
                //printf("minx:%d, maxx:%d, miny:%d, maxy:%d\n", minx, maxx, miny, maxy);
                const int ANI_STEP = 8;
                for(int step = 0; step < ANI_STEP; step++)
                {
                    window.clear();
                    draw_stage(window, vs, fs);
                    draw_world(window, vertices, faces);
                    auto draw_block = [&]() {
                        for (const auto& line : lb) {
                            sf::Vertex lineVertices[] = {
                                sf::Vertex(viewport(project(vb[line[0]]+point_3d((center_to.x-center.x)*step/ANI_STEP,(center_to.y-center.y)*step/ANI_STEP,cz)), 600, 600)),
                                sf::Vertex(viewport(project(vb[line[1]]+point_3d((center_to.x-center.x)*step/ANI_STEP,(center_to.y-center.y)*step/ANI_STEP,cz)), 600, 600)),
                            };
                            window.draw(lineVertices, 2, sf::Lines);
                        }
                    };
                    draw_block();
                    for (auto& vertex : vb) vertex = rotate(vertex - center, 0, 0, angle/ANI_STEP) + center;
                    window.display();
                }
            };

            auto smooth_rotateY = [&](float angle)
            {// animation here
                get_minmax(true);
                sf::Vector3f center_to((minx+maxx)/2.0-0.5, 0, (minz+maxz)/2.0+1.5);  // 객체의 중심점 정의

                get_minmax();
                sf::Vector3f center((minx+maxx)/2.0-1, 0, (minz+maxz)/2.0+1.5);  // 객체의 중심점 정의
                printf("center: %f, %f, %f, center_to: %f, %f, %f\n", center.x, center.y, center.z, center_to.x, center_to.y, center_to.z);
                //printf("minx:%d, maxx:%d, miny:%d, maxy:%d\n", minx, maxx, miny, maxy);
                const int ANI_STEP = 8;
                for(int step = 0; step < ANI_STEP; step++)
                {
                    window.clear();
                    draw_stage(window, vs, fs);
                    draw_world(window, vertices, faces);
                    auto draw_block = [&]() {
                        for (const auto& line : lb) {
                            sf::Vertex lineVertices[] = {
                                sf::Vertex(viewport(project(vb[line[0]]+point_3d((center_to.x-center.x)*step/ANI_STEP,(center_to.y-center.y)*step/ANI_STEP,(center_to.z-center.z)*step/ANI_STEP+cz)), 600, 600)),
                                sf::Vertex(viewport(project(vb[line[1]]+point_3d((center_to.x-center.x)*step/ANI_STEP,(center_to.y-center.y)*step/ANI_STEP,(center_to.z-center.z)*step/ANI_STEP+cz)), 600, 600)),
                            };
                            window.draw(lineVertices, 2, sf::Lines);
                        }
                    };
                    draw_block();
                    for (auto& vertex : vb) vertex = rotate(vertex - center, 0, angle/ANI_STEP, 0) + center;
                    window.display();
                }
            };



            for (int z = 0; z < 3; ++z) for (int y = 0; y < 3; ++y) for (int x = 0; x < 3; ++x) fake_block[z][y][x] = blocks[kind][z][y][x];
            if(angleX > 0){
                rotateX();
                if(check_block() == false) {
                    if(check_block() == false) rotateX(), rotateX(),rotateX();
                }
            } else if(angleX < 0){
                rotateX(), rotateX(),rotateX();
                if(check_block() == false) {
                    if(check_block() == false) rotateX();
                }
            }
            if(angleY > 0){
                rotateY(true);
                smooth_rotateY(90);
                rotateY(), rotateY(),rotateY();
                if(check_block() == false) {
                    if(check_block() == false) rotateY();
                }
            } else if(angleY < 0){
                rotateY(true);
                smooth_rotateY(-90);
                rotateY();
                if(check_block() == false) {
                    rotateY(), rotateY(),rotateY();
                }
            }
            if(angleZ > 0){
                rotateZ(true);
                smooth_rotateZ(90);
                rotateZ();
                if(check_block() == false) {
                    rotateZ(), rotateZ(),rotateZ();
                }
            } else if(angleZ < 0){
                rotateZ(true);
                smooth_rotateZ(-90);
                rotateZ(), rotateZ(),rotateZ();
                if(check_block() == false) {
                    rotateZ();
                }
            }
        }


        window.clear();
        draw_stage(window, vs, fs);
        generate_world_blocks(world, vertices, faces);
        draw_world(window, vertices, faces);
        auto draw_block = [&]() {
            for (const auto& line : lb) {
                sf::Vertex lineVertices[] = {
                    sf::Vertex(viewport(project(vb[line[0]]+point_3d(0,0,cz)), 600, 600)),
                    sf::Vertex(viewport(project(vb[line[1]]+point_3d(0,0,cz)), 600, 600)),
                };
                window.draw(lineVertices, 2, sf::Lines);
            }
        };
        if(game_over){
            sf::Font font;
            font.loadFromFile("fonts/arial.ttf");
            sf::Text text;
            text.setFont(font);
            text.setFillColor(sf::Color::Red);
            text.setString("Game Over");
            text.setCharacterSize(54); // in pixels
            sf::FloatRect textRect = text.getLocalBounds();
            text.setOrigin(textRect.left + textRect.width/2.0f, textRect.top  + textRect.height/2.0f);
            text.setPosition(sf::Vector2f(window.getSize().x/2.0f,window.getSize().y/2.0f));
            window.draw(text);
        } else {
            generate_blocks(blocks, kind, vb, lb);
            draw_block();
        }

        
        window.display();
    }

    return 0;
}

