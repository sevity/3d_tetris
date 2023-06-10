#include <SFML/Graphics.hpp>
#include <cmath>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <numeric>

typedef sf::Vector3f point_3d;
typedef sf::Vector2f point_2d;

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
void processKeyInput(sf::Event& event, float& angleX, float& angleY, float& angleZ) {
    if (event.type == sf::Event::KeyPressed) {
        switch (event.key.code) {
            case sf::Keyboard::Q: angleX = 90.0f; break;
            case sf::Keyboard::W: angleY = 90.0f; break;
            case sf::Keyboard::E: angleZ = 90.0f; break;
            case sf::Keyboard::A: angleX = -90.0f; break;
            case sf::Keyboard::S: angleY = -90.0f; break;
            case sf::Keyboard::D: angleZ = -90.0f; break;
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

void generate_world_blocks(std::vector<sf::Vector3f>& vertices, std::vector<std::vector<int>>& faces, int world[10][3][3]) {
    int base_idx = 0;

    //for (int z = 0; z < 5; ++z) {
    for (int z = 10; z >= 0; --z) {  // 10이 바닥
        for (int y = 0; y < 3; ++y) {
            for (int x = 0; x < 3; ++x) {
                if (world[z][y][x] == 1) {
                    write_cube(vertices, x, y, z);
                    // 바닥 back face
                    //faces.push_back({base_idx+2, base_idx+3, base_idx+7, base_idx+6});
                    // E right face
                    faces.push_back({base_idx+1, base_idx+2, base_idx+6, base_idx+5});
                    // W left face
                    faces.push_back({base_idx, base_idx+3, base_idx+7, base_idx+4});
                    // S top face
                    faces.push_back({base_idx+4, base_idx+5, base_idx+6, base_idx+7});
                    // N bottom face
                    faces.push_back({base_idx, base_idx+1, base_idx+2, base_idx+3});
                    // 뚜껑 front face
                    faces.push_back({base_idx, base_idx+1, base_idx+5, base_idx+4});
                    base_idx += 8;
                }
            }
        }
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

    std::vector<sf::Vector3f> v2;
    std::vector<std::vector<int>> f2;
    std::vector<std::vector<int>> l2;
    loadObjFile("obj/t_block_0.obj", v2, f2, l2);

    for (auto& vertex : v2) {
        vertex.x -= 1.5;
        vertex.y -= 1.5;
        vertex.z -= 0.5;
    }

    sf::Vector3f center(0, 0, 0);  // 객체의 중심점 정의
    center = std::accumulate(v2.begin(), v2.end(), center);
    center /= (float)v2.size();
    center.y-=0.7;
    printf("center: %f, %f, %f\n", center.x, center.y, center.z);


    int world[10][3][3] = {
        0,0,0,
        0,0,0,
        0,0,1,

        0,0,0,
        0,0,0,
        0,0,1,

        0,0,0,
        0,0,0,
        0,0,1,

        0,0,0,
        0,0,0,
        0,0,1,

        0,0,0,
        0,0,0,
        0,0,1,

        0,0,0,
        0,0,0,
        0,0,1,

        0,0,0,
        0,0,0,
        0,0,1,

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

    std::vector<sf::Vector3f> vertices;
    std::vector<std::vector<int>> faces;
    std::vector<std::vector<int>> lines;
    generate_world_blocks(vertices, faces, world);
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
        for (auto& vertex : v2) vertex = rotate(vertex - center, angleX, angleY, angleZ) + center;

        window.clear();
        draw_stage(window, vs, fs);
        draw_world(window, vertices, faces);
        draw_block(window, v2, l2);
        window.display();
    }

    return 0;
}

