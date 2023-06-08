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

bool loadObjFile(const std::string& filename, std::vector<sf::Vector3f>& vertices, std::vector<std::vector<int>>& faces) {
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
        }
    }
    file.close();
    return true;
}

int main() {
    sf::RenderWindow window(sf::VideoMode(600, 600), "3D Object");

    std::vector<sf::Vector3f> vertices;
    std::vector<std::vector<int>> faces;
    loadObjFile("obj/stage.obj", vertices, faces);

    for (auto& vertex : vertices) {
        vertex.x -= 1.5;
        vertex.y -= 1.5;
        vertex.z += 1.5;
        //vertex.y *= -1;
    }

    sf::Vector3f center(0, 0, 0);  // 객체의 중심점 정의
    center = std::accumulate(vertices.begin(), vertices.end(), center);
    center /= (float)vertices.size();

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        float angleX = sf::Keyboard::isKeyPressed(sf::Keyboard::Q) ? 3.0f : 0.0f;
        float angleY = sf::Keyboard::isKeyPressed(sf::Keyboard::W) ? 3.0f : 0.0f;
        float angleZ = sf::Keyboard::isKeyPressed(sf::Keyboard::E) ? 3.0f : 0.0f;
        angleX = sf::Keyboard::isKeyPressed(sf::Keyboard::A) ? -3.0f : angleX;
        angleY = sf::Keyboard::isKeyPressed(sf::Keyboard::S) ? -3.0f : angleY;
        angleZ = sf::Keyboard::isKeyPressed(sf::Keyboard::D) ? -3.0f : angleZ;

        for (auto& vertex : vertices) {
            vertex = rotate(vertex - center, angleX, angleY, angleZ) + center;
        }

        window.clear();

        for (const auto& face : faces) {
            sf::Vertex line[] = {
                sf::Vertex(project(vertices[face[0]]) * 300.f + sf::Vector2f(300, 300)),
                sf::Vertex(project(vertices[face[1]]) * 300.f + sf::Vector2f(300, 300)),
                sf::Vertex(project(vertices[face[2]]) * 300.f + sf::Vector2f(300, 300)),
                sf::Vertex(project(vertices[face[3]]) * 300.f + sf::Vector2f(300, 300)),
                sf::Vertex(project(vertices[face[0]]) * 300.f + sf::Vector2f(300, 300))
            };
            window.draw(line, 5, sf::LineStrip);
        }

        window.display();
    }

    return 0;
}

