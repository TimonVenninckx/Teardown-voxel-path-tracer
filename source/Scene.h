#pragma once


#include <glm/vec3.hpp>
#include <string>
#include <vector>

struct Scene {
    struct Model {
        std::string fileLocation{};
        glm::vec3 worldOrigin{0.f};
    };
    std::string name{};
    std::vector<Model> models{};
};



inline std::vector<Scene> ALLSCENES{
    Scene {
        "HugeScene",
        {
            {"assets/castle.vox",                                glm::vec3(0.f,   6.f,  130.f)},
            {"assets/Spaceships.vox",                            glm::vec3(-20.f, 10.f,  40.f)},
            {"assets/nuke.vox",                                  glm::vec3(0.f,   0.f,   0.f)},
            {"assets/recordPlayer.vox",                          glm::vec3(30.f,  10.f,  50.f)},
            {"assets/menger.vox",                                glm::vec3(0.f,   20.f,  70.f)},
            {"assets/FULL VERSION AVAILABLE ON MY GUMROAD.vox",  glm::vec3(70.f,  20.f,  70.f)},
        }
    },
    Scene{
        "Church",
        {{"assets/Church_Of_St_Sophia.vox"}}
    },
    Scene{
        "CastleScene",
        {{"assets/castle.vox", glm::vec3(0.f, 0.f, 0.f)}}
    },
    Scene{
        "Nuke",
        {{"assets/nuke.vox"}}
    },
    Scene{
        "OasisScene",
        {{"assets/Oasis_Hard_Cover.vox", glm::vec3(0.f, 0.f, 0.f)}}
    },
    Scene{
        "GITestScene",
        {{"assets/gitest.vox"}}
    },
    Scene{
        "SponzaScene",
        {{"assets/sponza.vox", glm::vec3(0.f, 0.f, 0.f)}}
    },
    Scene{
        "Minecraft",
        {{"assets/custom.vox", glm::vec3(0.f, 0.f, 0.f)}}
    },
};
