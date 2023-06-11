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
using namespace std;

typedef sf::Vector3f point_3d;
typedef sf::Vector2f point_2d;

int blocks[7][3][3][3] = {
    0,0,0, // 천장층
    1,1,0,
    1,1,0,

    0,0,0, // 중간층
    0,0,0,
    0,0,0,

    0,0,0, // 바닥층
    0,0,0,
    0,0,0,
};
int world[10][3][3] = {
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

    0,1,0,
    0,0,0,
    0,0,1,

    0,1,1,
    0,0,0,
    0,0,1,

    1,1,1,
    0,1,1,
    1,0,1,
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

sf::Vector2f project(const sf::Vector3f& point) {
    return sf::Vector2f(point.x / point.z, point.y / point.z);
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
int g_cnt = 0;
void processKeyInput(sf::Event& event, float& angleX, float& angleY, float& angleZ) {
    if (event.type == sf::Event::KeyPressed) {
        switch (event.key.code) {
            case sf::Keyboard::Q: angleX = 90.0f; break;
            case sf::Keyboard::W: angleY = 90.0f; break;
            case sf::Keyboard::E: angleZ = 90.0f; break;
            case sf::Keyboard::A: angleX = -90.0f; break;
            case sf::Keyboard::S: angleY = -90.0f; break;
            case sf::Keyboard::D: angleZ = -90.0f; break;
            case sf::Keyboard::Z: ++g_cnt; printf("g_cnt:%d\n", g_cnt);break;
            default: break;
        }
    }
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


void draw_block(sf::RenderWindow& window, std::vector<sf::Vector3f>& vertices, std::vector<std::vector<int>> lines) {
    for (const auto& line : lines) {
        sf::Vertex lineVertices[] = {
            sf::Vertex(viewport(project(vertices[line[0]]), 600, 600)),
            sf::Vertex(viewport(project(vertices[line[1]]), 600, 600))
        };
        window.draw(lineVertices, 2, sf::Lines);
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
    int base_idx = 0;

    //for (int z = 0; z < 5; ++z) {
    for (int z = 10; z >= 0; --z) {  // 10이 바닥
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
}
/*
void generate_blocks(int blocks[7][3][3][3], std::vector<sf::Vector3f>& vertices, std::vector<std::vector<int>>& faces) {
    int base_idx = 0;

    for (int z = 2; z >= 0; --z) {
        for (int y = 0; y < 3; ++y) {
            for (int x = 0; x < 3; ++x) {
                if (blocks[0][z][y][x] == 1) {
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
}
*/
struct Vector3fCompare {
    bool operator() (const sf::Vector3f& lhs, const sf::Vector3f& rhs) const {
        return std::tie(lhs.x, lhs.y, lhs.z) < std::tie(rhs.x, rhs.y, rhs.z);
    }
};

struct Line {
    sf::Vector3f v1;
    sf::Vector3f v2;
    Line(sf::Vector3f v1, sf::Vector3f v2) : v1(v1), v2(v2) {
        if (Vector3fCompare()(v2, v1)) std::swap(this->v1, this->v2);
    }
};

struct LineCompare {
    bool operator() (const Line& lhs, const Line& rhs) const {
        Vector3fCompare comp;
        return comp(lhs.v1, rhs.v1) ? true : (!comp(rhs.v1, lhs.v1) && comp(lhs.v2, rhs.v2));
    }
};

const std::vector<std::vector<int>> cube_lines = {
    {0, 1}, {1, 2}, {2, 3}, {3, 0},
    {4, 5}, {5, 6}, {6, 7}, {7, 4},
    {0, 4}, {1, 5}, {2, 6}, {3, 7}
};

void generate_blocks(int blocks[7][3][3][3], std::vector<sf::Vector3f>& vertices, std::vector<std::vector<int>>& lines) {
    int base_idx = 0;

    for (int z = 2; z >= 0; --z) {
        for (int y = 0; y < 3; ++y) {
            for (int x = 0; x < 3; ++x) {
                if (blocks[0][z][y][x] == 1) {
                    write_cube(vertices, x, y, z);
                    int cnt = 0;
                     for (const auto& line : cube_lines) {
                        lines.push_back({base_idx + line[0], base_idx + line[1]});
                        //if(++cnt>g_cnt) return;
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


    return;




    std::map<Line, int, LineCompare> line_map;
    for (const auto& line : lines) {
        Line current_line(vertices[line[0]], vertices[line[1]]);
        if (line_map.find(current_line) != line_map.end()) {
            line_map.erase(current_line);
        } else {
            line_map[current_line] = 1;
        }
    }

    lines.clear();
    for (const auto& line_map_entry : line_map) {
        std::vector<int> line_indices(2);
        for (int i = 0; i < vertices.size(); ++i) {
            if (vertices[i] == line_map_entry.first.v1) {
                line_indices[0] = i;
            }
            if (vertices[i] == line_map_entry.first.v2) {
                line_indices[1] = i;
            }
        }
        lines.push_back(line_indices);
    }

}

int main() {
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
    generate_blocks(blocks, vb, lb);

    sf::Vector3f center(0, 0, 0);  // 객체의 중심점 정의
    /*
    center = std::accumulate(v2.begin(), v2.end(), center);
    center /= (float)v2.size();
    center.y-=0.7;
    printf("center: %f, %f, %f\n", center.x, center.y, center.z);
    */


    std::vector<sf::Vector3f> vertices;
    std::vector<std::vector<int>> faces;
    std::vector<std::vector<int>> lines;
    generate_world_blocks(world, vertices, faces);
    std::sort(faces.begin(), faces.end(), [&](const std::vector<int>& face1, const std::vector<int>& face2) {
        return compareFaces(face1, face2, vertices);
    });
    while (window.isOpen()) {
        sf::Event event;
        float angleX = 0, angleY = 0, angleZ = 0;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            processKeyInput(event, angleX, angleY, angleZ);
        }
        for (auto& vertex : vb) vertex = rotate(vertex - center, angleX, angleY, angleZ) + center;

        window.clear();
        //draw_stage(window, vs, fs);
        //draw_world(window, vertices, faces);
        draw_block(window, vb, lb);
        window.display();
    }

    return 0;
}

