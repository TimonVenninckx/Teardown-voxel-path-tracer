#pragma once


#include <fstream>
#include <iostream>
#include <cstring>
#include <vector>
#include <unordered_map>

#include <glm/vec3.hpp>
#include <string_view>


// Custom magica-voxel parser
// Do NOT use this one please, (because its questionable)
// Just use singe-header ogt_vox https://github.com/jpaver/opengametools?tab=readme-ov-file
// Or gvox which supports more formats https://github.com/GabeRundlett/gvox

namespace MagicaParser {

    // https://github.com/ephtracy/voxel-model/blob/master/MagicaVoxel-file-format-vox-extension.txt
    typedef unsigned char u8;
    struct FileStructure {
        char vox[4];
        int version;
    };

    struct MainChunkHeader {
        char tag[4];
        int byteCount;
        int childBytes;
        // chunk data
        // children chunks
    };

    struct Voxel {
        u8 x;
        u8 y;
        u8 z;
        u8 materialIndex;
    };

    struct PaletteColor {
        u8 r, g, b, a;
    };

    struct Transform {
        u8 rotation{ 1 << 2 };
        glm::ivec3 offset{};
    };

    struct VoxelMesh {
        glm::ivec3 extent;
        std::vector<Voxel> voxels;
    };

    struct Translation {
        int ID{0};
        u8 rotation{0};
        glm::ivec3 translation{ 0 };
        // idk what this is for tbh
        int frameIndex{};
    };

    struct ModelData {
        int ID;
        // could have frame data
    };

    struct ModelInstance {
        int ID;
        glm::vec3 position{};     // MV voxel-integer units, MV Z-up space
        glm::mat3 rotation{ 1.f };// pure rotation, MV Z-up space
    };


    struct Node {
        //int id;
        enum Type { TRN, GRP, SHP } type;
        int child = -1;                 // TRN
        std::vector<int> childNodes{}; // GRP
        std::vector<ModelData> models{};  // SHP

        Transform transform{};
    };

    enum MATLType {
        diffuse = 0,
        metal   = 1,
        glass   = 2,
        emit    = 3
    };

    struct MATL {
        int id{};
        MATLType type;
        float weight{};
        float emit{};
        float rough{};
        float spec{};
        float ior{};
        float flux{};
        float att{};
        float plastic{};
        float alpha{};
        float trans{};
        float d{};
        float g{};
        float sp{};
        float metal{};
        float media{};
        float ldr{};
    };



    struct Scene {
        std::vector<VoxelMesh> meshes{};
        std::vector<ModelInstance> instances{};
        std::vector<MATL> materials{};
        PaletteColor palette[256];
    };


    glm::mat4 RotationToMat(u8 rotation) {

        uint c0 = rotation & 3;
        uint c1 = (rotation >> 2) & 3;
        // process of elimination to determine row 2 index based on row0/row1 being one of {0,1,2} choose 2.
        // FROM actual good vox parser ogt_vox
        static const uint k_row2_index[] = { UINT32_MAX, UINT32_MAX, UINT32_MAX, 2, UINT32_MAX, 1, 0, UINT32_MAX };
        uint c2 = k_row2_index[(1 << c0) | (1 << c1)];

        float s0 = ((rotation >> 4) & 1) ? -1.f : +1.f;
        float s1 = ((rotation >> 5) & 1) ? -1.f : +1.f;
        float s2 = ((rotation >> 6) & 1) ? -1.f : +1.f;

        glm::mat4 matrix = glm::mat4(0.f);
        matrix[c0][0] = s0;
        matrix[c1][1] = s1;
        matrix[c2][2] = s2;
        matrix[3][3] = 1.f;
        return matrix;
    }

    glm::mat4 RotationToMatYUpSystem(u8 rotation) {
        
        glm::mat4 swapYZ(
            1, 0, 0, 0,
            0, 0, 1, 0,
            0,-1, 0, 0,
            0, 0, 0, 1
        );
        return swapYZ * RotationToMat(rotation);
    }


    glm::mat4 ConvertRotationZupToYup(u8 rotation) {
        // First get the Z-up rotation matrix from MagicaVoxel
        glm::mat4 zUpMatrix = MagicaParser::RotationToMat(rotation);

        // Conversion matrix from Z-up to Y-up coordinate system
        // This swaps Y and Z axes and flips Z
        glm::mat4 zUpToYUp(
            1, 0, 0, 0,  // X stays X
            0, 0, 1, 0,  // Z becomes Y
            0, -1, 0, 0,  // -Y becomes Z
            0, 0, 0, 1
        );

        // Apply the conversion
        return zUpToYUp * zUpMatrix * glm::transpose(zUpToYUp);
    }


    void ParseEmptyDict(std::ifstream& file) {
        // just read the dict dont actually care whats inside;

        
        int numPairs;
        file.read((char*)&numPairs, sizeof(int));
        for (int i{ 0 }; i < numPairs; i++) {
            int keySize;
            file.read((char*)&keySize, sizeof(int));
            std::vector<char> keyBuffer(keySize);
            file.read((char*)keyBuffer.data(), keySize);

            int valSize;
            file.read((char*)&valSize, sizeof(int));
            std::vector<char> valBuffer(valSize);
            file.read((char*)valBuffer.data(), valSize);
        }
    }

    void DebugBuffer(std::ifstream& file) {

        std::streampos loc = file.tellg();
        loc -= 20;
        file.seekg(loc);
        std::vector<char> bullshitBuffer(200);
        file.read(bullshitBuffer.data(), 200);
    }



    void ParseNTRN(std::ifstream& file, std::unordered_map<int,Node>& nodes,MainChunkHeader head){
        std::streamoff endPos = file.tellg();
        endPos += head.byteCount;

        Node node;
        int nodeID;
        file.read((char*)&nodeID, sizeof(int));

        ParseEmptyDict(file);
        
        struct nTRN { int childNodeID, ReservedID, layerID, numFrames; };
        nTRN n;
        file.read((char*)&n, sizeof(nTRN));
        
        node.type = Node::TRN;
        node.child = n.childNodeID;
        
        Translation trans{};
        for (int f{ 0 }; f < n.numFrames; f++) {
            int numPairs;
            file.read((char*)&numPairs, sizeof(int));
            for (int p{ 0 }; p < numPairs; p++){
                int keyBufferSize;
                file.read((char*)&keyBufferSize, sizeof(int));
                // IS ALWAYS 2
                assert(keyBufferSize == 2);

                std::vector<char> name(keyBufferSize);
                file.read((char*)name.data(), keyBufferSize);

                int valueBufferSize;
                file.read((char*)&valueBufferSize, sizeof(int));

                if (std::memcmp(name.data(), "_r", 2) == 0) {
                    std::vector<char> buffer(valueBufferSize);
                    file.read(buffer.data(), valueBufferSize); // only read the first byte
                    
                    std::string s(buffer.begin(), buffer.end());
                    trans.rotation = (u8)std::stoi(s);
                    //std::cout << "Rotation parsed: " << (int)trans.rotation;
                }
                else if (std::memcmp(name.data(), "_t", 2) == 0) {
                    // for some reason stored as string :)
                    std::vector<char> transform(valueBufferSize);
                    file.read(transform.data(), valueBufferSize);
                    int axis{ 0 };
                    std::string str{};
                    for (char c : transform) {
                        // if empty go to next number
                        if (c == ' ') {
                            trans.translation[axis] = std::stoi(str);
                            str = std::string();
                            axis++;
                            if (axis > 2)
                                break;
                        }
                        else {
                            str += c;
                        }
                    }
                    if(str.size())
                        trans.translation[2] = std::stoi(str);

                    //std::cout << "Translation:: " << valueBufferSize << " big buffer Values\nx:" << trans.translation[0] << "\ty:" << trans.translation[1] << "\tz:" << trans.translation[2] << '\n';
                }
                else if (std::memcmp(name.data(), "_f", 2) == 0) {
                    //assert(valueBufferSize <= 4);
                    //file.read((char*)&trans.frameIndex, valueBufferSize);
                    std::vector<char> buffer(valueBufferSize);
                    file.read(buffer.data(), valueBufferSize);
                }
            }
        }

        if (n.numFrames) {
            // set last row of matrix
            //node.matrix = glm::translate(node.matrix, glm::vec3(trans.translation) * voxelSize);
            node.transform.offset = trans.translation;
            if(trans.rotation)
                node.transform.rotation = trans.rotation;

            nodes[nodeID] = node;
            //data.translations.push_back(trans);
        }

        // just making sure
        file.seekg(endPos);
    }


    float ParseFloat(std::ifstream& file, int floatSize) {

        std::vector<char> fl(floatSize);
        file.read(fl.data(), floatSize);
        std::string s(fl.begin(), fl.end());

        return static_cast<float>(atof(s.c_str()));
    }

    void ParseMATL(std::ifstream& file, std::vector<MATL>& materials, MainChunkHeader head) {

        MATL mat{};
        
        file.read((char*)&mat.id, sizeof(int));
        head.byteCount -= sizeof(int);
        std::streamoff pos = file.tellg();


        int numValues;
        file.read((char*)&numValues, sizeof(int));
        for (int p{ 0 }; p < numValues; p++) {

            int keyBufferSize;
            file.read((char*)&keyBufferSize, sizeof(int));

            if (keyBufferSize <= 0 || keyBufferSize > 100) {
                std::cout << "ERROR: Invalid keyBufferSize: " << keyBufferSize
                    << " at property " << p << "\n";
                break;
            }

            std::vector<char> name(keyBufferSize);
            file.read((char*)name.data(), keyBufferSize);
            std::string_view keyStr(name.data(), keyBufferSize);


            int valueBufferSize;
            file.read((char*)&valueBufferSize, sizeof(int));
            if (valueBufferSize < 0 || valueBufferSize > 1000) {
                std::cout << "ERROR: Invalid valueBufferSize: " << valueBufferSize
                    << " for key '" << keyStr << "'\n";
                break;
            }

            


            if (keyStr == "_type") {
                std::vector<char> buffer(valueBufferSize);
                file.read(buffer.data(), valueBufferSize);
                std::string_view valueBuffer(buffer.data(), valueBufferSize);

                if (valueBuffer ==  "_diffuse") {
                    mat.type = MATLType::diffuse;
                }
                else if (valueBuffer == "_metal") {
                    mat.type = MATLType::metal;
                }
                else if (valueBuffer == "_glass") {
                    mat.type = MATLType::glass;
                }
                else if (valueBuffer == "_emit") {
                    mat.type = MATLType::emit;
                }
                else {
                    std::cout << "MATL::Coulnd't parse material type\n";
                }
            }
            else if (keyStr == "_emit") {
                mat.emit = ParseFloat(file, valueBufferSize);
            }
            else if (keyStr == "_weight") {
                mat.weight = ParseFloat(file, valueBufferSize);
            }
            else if (keyStr == "_rough") {
                mat.rough = ParseFloat(file, valueBufferSize);
            }
            else if (keyStr == "_spec") {
                mat.spec = ParseFloat(file, valueBufferSize);
            }
            else if (keyStr == "_ior") {
                mat.ior = ParseFloat(file, valueBufferSize);
            }
            else if (keyStr == "_att") {
                mat.att = ParseFloat(file, valueBufferSize);
            }
            else if (keyStr == "_flux") {
                mat.flux = ParseFloat(file, valueBufferSize);
            }
            else if (keyStr == "_plastic") {
                mat.plastic = ParseFloat(file, valueBufferSize);
            }
            else if (keyStr == "_alpha") {
                mat.alpha = ParseFloat(file, valueBufferSize);
            }
            else if (keyStr == "_trans") {
                mat.trans = ParseFloat(file, valueBufferSize);
            }
            else if (keyStr == "_d") {
                mat.d = ParseFloat(file, valueBufferSize);
            }
            else if (keyStr == "_g") {
                mat.g = ParseFloat(file, valueBufferSize);
            }
            else if (keyStr == "_sp") {
                mat.sp = ParseFloat(file, valueBufferSize);
            }
            else if (keyStr == "_metal") {
                mat.metal = ParseFloat(file, valueBufferSize);
            }
            else if (keyStr == "_media") {
                mat.media = ParseFloat(file, valueBufferSize);
            }
            else if (keyStr == "_ri") {
                mat.ior = ParseFloat(file, valueBufferSize);
            }
            else if (keyStr == "_ldr") {
                mat.ldr = ParseFloat(file, valueBufferSize);
            }
            else {
                std::vector<char> valueData(valueBufferSize);
                file.read(valueData.data(), valueBufferSize);

                std::string readkeyStr(keyStr.data(), keyStr.size());
                std::string valueStr(valueData.data(), valueData.size());

                std::cout << "MATL::Couldn't parse key: '" << readkeyStr
                    << "' with value: '" << valueStr << "' (size: "
                    << valueBufferSize << ")\n";
            }
        }

        materials.emplace_back(mat);
        
        file.seekg(pos + head.byteCount);
    }

    inline bool TagEqual(const char* tag1, const char* tag2) {
        return std::memcmp(tag1, tag2, 4) == 0;
    }

    // Replace TraverseNodes:
    void TraverseNodes(int nodeID, std::unordered_map<int, Node>& nodes,
        Scene& scene, const glm::mat4& parentTransform)
    {
        Node& n = nodes[nodeID];
        switch (n.type) {
            case Node::TRN: {
                if (n.child > 0) {
                    glm::mat4 local = RotationToMat(n.transform.rotation);
                    local[3] = glm::vec4(glm::vec3(n.transform.offset), 1.f);
                    TraverseNodes(n.child, nodes, scene, parentTransform * local);
                }
                break;
            }
            case Node::GRP: {
                for (int c : n.childNodes)
                    TraverseNodes(c, nodes, scene, parentTransform);
                break;
            }
            case Node::SHP: {
                for (ModelData& m : n.models) {
                    ModelInstance inst;
                    inst.ID = m.ID;
                    inst.position = glm::vec3(parentTransform[3]);  // translation column
                    inst.rotation = glm::mat3(parentTransform);     // upper-left 3x3
                    scene.instances.emplace_back(inst);
                }
                break;
            }
        }
    }


        


    Scene ParseModel(const char* fileName) {
        std::ifstream file(fileName, std::ios::binary);

        FileStructure structure;
        file.read((char*) & structure, sizeof(FileStructure));

        if (std::memcmp(structure.vox, "VOX ",4) != 0) {
            std::cout << "Not a magica file. file starts with:\t" << structure.vox << '\n';
        }
        std::cout << "Model Loaded:)\n";


        Scene scene;
        std::unordered_map<int,Node> nodes;

        std::streampos endOfFile;
        while (file.good()) {
            MainChunkHeader head;
            file.read((char*)&head, sizeof(MainChunkHeader));

            if (TagEqual(head.tag, "MAIN"))
                continue;
                //std::cout << "Main start\n";
            else if (std::memcmp(head.tag, "SIZE", 4) == 0) {
                VoxelMesh mesh{};
                file.read((char*)&mesh.extent, 3 * sizeof(int));
                scene.meshes.emplace_back(mesh);
            }
            else if (TagEqual(head.tag, "XYZI")) {
                int numVoxels;
                file.read((char*)&numVoxels, sizeof(int));
                head.byteCount -= sizeof(int);
                assert(scene.meshes.size());
                VoxelMesh& mesh = scene.meshes.back();
                mesh.voxels.resize(head.byteCount / sizeof(Voxel));
                file.read((char*)mesh.voxels.data(), head.byteCount);
            }
            else if (TagEqual(head.tag, "RGBA")) {
                if (head.byteCount != 1024) {
                    std::cout << "MagicaParser::INVALID RGBA DATA\n";
                    continue;
                }
                // https://github.com/ephtracy/voxel-model/blob/master/MagicaVoxel-file-format-vox-extension.txt
                // INDEX 0 IS UNUSED IDK WHY SO THROW AWAY FIRST 4?
                // start from pallette index 1 so skip first 4 bytes
                file.read(sizeof(PaletteColor) + (char*)scene.palette, 1020);
                //read last remaing 4 bytes into byte 0 (SHOULD NOT BE USED ANYWAY);
                file.read((char*)scene.palette, sizeof(PaletteColor));
            }
            else if (TagEqual(head.tag, "nTRN")) {
                ParseNTRN(file, nodes, head);
            }
            else if (TagEqual(head.tag, "nGRP")) {
                Node node;
                node.type = Node::GRP;
                int nodeID;
                file.read((char*)&nodeID, sizeof(int));
                head.byteCount -= sizeof(int);
                std::streamoff cursorPos = file.tellg();
                // parse enmpty dict
                ParseEmptyDict(file);

                int numChildrenNodes;
                file.read((char*)&numChildrenNodes, sizeof(int));

                node.childNodes.resize(numChildrenNodes);
                file.read((char*)node.childNodes.data(), numChildrenNodes * sizeof(int));

                nodes[nodeID] = node;
                file.seekg(cursorPos + head.byteCount);
            }
            else if (TagEqual(head.tag, "nSHP")) {
                Node node;
                node.type = Node::SHP;
                int nodeID;
                file.read((char*)&nodeID, sizeof(int));
                head.byteCount -= sizeof(int);
                std::streamoff cursorPos = file.tellg();

                ParseEmptyDict(file);

                int numModels;
                file.read((char*)&numModels, sizeof(int));

                for (int i{ 0 }; i < numModels; i++) {
                    ModelData m;
                    file.read((char*)&m.ID, sizeof(int));
                    ParseEmptyDict(file);
                    node.models.push_back(m);
                }

                nodes[nodeID] = node;
                file.seekg(cursorPos + head.byteCount);
            }
            else if (TagEqual(head.tag, "MATL")) {
                ParseMATL(file, scene.materials, head);
            }

            /*else if (TagEqual(head.tag, "NOTE")) {
                std::vector<char> note(head.byteCount);

                file.read(note.data(), head.byteCount);
                std::cout << "NOTE\t " << note.data() << '\n';
            }*/
            else {
                //std::cout << "Couldn't parse: " << head.tag << "\n";
                std::streamoff filePos = file.tellg();
                file.seekg(filePos + head.byteCount + head.childBytes);// +head.childBytes);
                
                if (filePos == -1) {
                    std::cout << "MagicaParser::Stop\n";
                }
            }
            if (head.childBytes > 0)
                std::cout << "Skipped child?\n";
        }
        std::cout << "Done parsing :0\n";
        // now we actually build the scene

        
        Transform t{};
        TraverseNodes(0, nodes, scene, glm::mat4(1.f));

        return scene;;
    }
}