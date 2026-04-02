#pragma once

#include <glm/vec3.hpp>
#include <vector>


struct CameraPosRot {
    glm::vec3 camPos{};
    glm::vec3 camFront{};
};

class CatmullRom {
public:
    ~CatmullRom();

    void LoadPoints(const char* file);

    void LoadPoints(const std::vector<CameraPosRot>& points);

    CameraPosRot GetPoint(float t) const;

    float alpha{ 0.5f };
    float tension{ 0.f };
private:
    const char* file{};

    std::vector<CameraPosRot> points{};

    
    struct Segment
    {
        glm::vec3 a;
        glm::vec3 b;
        glm::vec3 c;
        glm::vec3 d;
    };

    std::vector<Segment> posSegments{};
    std::vector<Segment> rotSegments{};
};