#include "CatmullRom.h"

#include <glm/common.hpp>
#include <glm/geometric.hpp>
#include <iostream>
#include <fstream>



float RandomFloat() {
    return static_cast<float>(rand()) / RAND_MAX;
}



// https://qroph.github.io/2018/07/30/smooth-paths-using-catmull-rom-splines.html


CatmullRom::~CatmullRom()
{
    if (file) {

        std::ofstream is(file, std::ios_base::binary);
        
        is.write(reinterpret_cast<char*>(points.data()), points.size() * sizeof(CameraPosRot));
        is.close();
    }
}

void CatmullRom::LoadPoints(const char* ffile)
{
    this->file = ffile;
    std::ifstream is(file, std::ios_base::binary);

    is.seekg(0, is.end);
    int length = static_cast<int>(is.tellg());
    is.seekg(0, 0);
    if (length > 4 * sizeof(CameraPosRot)) {
        points.resize(length / sizeof(CameraPosRot));
        is.read((char*)points.data(), length);
        is.close();

        
    }
    else {
        points = {};

        for (int i{ 0 }; i < 100; i++) {
            points.emplace_back(CameraPosRot{
                    glm::vec3((RandomFloat() - 0.5f) * 200, RandomFloat() * 20.f + 50.f, (RandomFloat() - 0.5f) * 200),
                    glm::vec3(RandomFloat() * 2.f - 1.0f, RandomFloat() * 2.f - 1.0f, RandomFloat() * 2.f - 1.0f)
            });
        }

        std::cout << "Loadcatmullrom failed\n";
    }
    LoadPoints(points);
}

float SafeT(float prevT, const glm::vec3& a, const glm::vec3& b, float alpha) {
    float d = glm::distance(a, b);
    return prevT + (d > 1e6f ? std::pow(d, alpha) : 1e-6f);
}

void CatmullRom::LoadPoints(const std::vector<CameraPosRot>& ppoints)
{
    posSegments.clear();
    rotSegments.clear();
    this->points = ppoints;
    // duplicate points so they all get reached

    // need atleast 4 points
    assert(points.size() > 3);
    for (int i{ 0 }; i < points.size() - 4; i++) {
        const glm::vec3& p0 = points[i].camPos;
        const glm::vec3& p1 = points[i + 1].camPos;
        const glm::vec3& p2 = points[i + 2].camPos;
        const glm::vec3& p3 = points[i + 3].camPos;


        const float t0 = 0.f;
        const float t1 = SafeT(t0,p0,p1,alpha);
        const float t2 = SafeT(t1,p1,p2,alpha);
        const float t3 = SafeT(t2,p2,p3,alpha);


        glm::vec3 m1 = (1.0f - tension) * (t2 - t1) *
            ((p1 - p0) / (t1 - t0) - (p2 - p0) / (t2 - t0) + (p2 - p1) / (t2 - t1));
        glm::vec3 m2 = (1.0f - tension) * (t2 - t1) *
            ((p2 - p1) / (t2 - t1) - (p3 - p1) / (t3 - t1) + (p3 - p2) / (t3 - t2));

        Segment seg;
        seg.a = 2.f * (p1 - p2) + m1 + m2;
        seg.b = -3.0f * (p1 - p2) - m1 - m1 - m2;
        seg.c = m1;
        seg.d = p1;

        posSegments.emplace_back(seg);
    }

    for (int i{ 0 }; i < points.size() - 4; i++) {
        const glm::vec3& p0 = points[i].camFront;
        const glm::vec3& p1 = points[i + 1].camFront;
        const glm::vec3& p2 = points[i + 2].camFront;
        const glm::vec3& p3 = points[i + 3].camFront;


        const float t0 = 0.f;
        const float t1 = SafeT(t0, p0, p1, alpha);
        const float t2 = SafeT(t1, p1, p2, alpha);
        const float t3 = SafeT(t2, p2, p3, alpha);


        glm::vec3 m1 = (1.0f - tension) * (t2 - t1) *
            ((p1 - p0) / (t1 - t0) - (p2 - p0) / (t2 - t0) + (p2 - p1) / (t2 - t1));
        glm::vec3 m2 = (1.0f - tension) * (t2 - t1) *
            ((p2 - p1) / (t2 - t1) - (p3 - p1) / (t3 - t1) + (p3 - p2) / (t3 - t2));

        Segment seg;
        seg.a = 2.f * (p1 - p2) + m1 + m2;
        seg.b = -3.0f * (p1 - p2) - m1 - m1 - m2;
        seg.c = m1;
        seg.d = p1;

        rotSegments.emplace_back(seg);
    }
}

CameraPosRot CatmullRom::GetPoint(float t) const
{
    CameraPosRot c;

    const unsigned int index = static_cast<int>(floor(t));
    if (index >= posSegments.size())
        return CameraPosRot{glm::vec3(100.f)};

    t -= floor(t);
    t = std::max(t, 0.0001f);

    const Segment& segp = posSegments[index];
    c.camPos =  segp.a * t * t * t +
                segp.b * t * t +
                segp.c * t +
                segp.d;


    const Segment& segr = rotSegments[index];
    c.camFront = segr.a * t * t * t +
                 segr.b * t * t +
                 segr.c * t +
                 segr.d;

    if (c.camFront == glm::vec3(0.f))
        c.camFront = glm::vec3(1.f, 0.f, 0.f);

    c.camFront = glm::normalize(c.camFront);
    return c;
}