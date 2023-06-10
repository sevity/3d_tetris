#include <SFML/Graphics.hpp>
#include <cmath>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <numeric>

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

int main() {
    sf::RenderWindow window(sf::VideoMode(600, 600), "3D Object");

    std::vector<sf::Vector3f> vertices;
    std::vector<std::vector<int>> faces;
    std::vector<std::vector<int>> lines;
    loadObjFile("obj/stage.obj", vertices, faces, lines);

    std::vector<sf::Vector3f> v2;
    std::vector<std::vector<int>> f2;
    std::vector<std::vector<int>> l2;
    loadObjFile("obj/t_block_0.obj", v2, f2, l2);

    for (auto& vertex : vertices) {
        vertex.x -= 1.5;
        vertex.y -= 1.5;
        vertex.z += 1.5;
    }
    for (auto& vertex : v2) {
        vertex.x -= 1.5;
        vertex.y -= 1.5;
        vertex.z -= 0.5;
    }

    sf::Vector3f center(0, 0, 0);  // 객체의 중심점 정의
    center = std::accumulate(v2.begin(), v2.end(), center);
    center /= (float)v2.size();

    while (window.isOpen()) {
        sf::Event event;
        float angleX = 0, angleY = 0, angleZ = 0;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
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

        for (auto& vertex : v2) {
            vertex = rotate(vertex - center, angleX, angleY, angleZ) + center;
        }

        window.clear();
        for (const auto& face : faces) {
            sf::Vertex line[] = {
                sf::Vertex(project(vertices[face[0]]) * 300.f + sf::Vector2f(300, 300), sf::Color::Green),
                sf::Vertex(project(vertices[face[1]]) * 300.f + sf::Vector2f(300, 300), sf::Color::Green),
                sf::Vertex(project(vertices[face[2]]) * 300.f + sf::Vector2f(300, 300), sf::Color::Green),
                sf::Vertex(project(vertices[face[3]]) * 300.f + sf::Vector2f(300, 300), sf::Color::Green),
                sf::Vertex(project(vertices[face[0]]) * 300.f + sf::Vector2f(300, 300), sf::Color::Green)
            };
            window.draw(line, 5, sf::LineStrip);
        }
        for (const auto& line : l2) {
            sf::Vertex lineVertices[] = {
                sf::Vertex(project(v2[line[0]]) * 300.f + sf::Vector2f(300, 300)),
                sf::Vertex(project(v2[line[1]]) * 300.f + sf::Vector2f(300, 300))
            };
            window.draw(lineVertices, 2, sf::Lines);
        }

        window.display();
    }

    return 0;
}

