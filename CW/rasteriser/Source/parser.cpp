#include <iostream>
#include <fstream>
#include <glm/glm.hpp>
#include <stdint.h>
#include "definitions.cpp"

glm::vec4 toVec4(glm::vec3 ceva){
    return vec4(ceva.x, ceva.y, ceva.z, 1);
}

inline void split(const std::string &input,
			std::vector<std::string> &output,
			std::string token){
    output.clear();

    std::string temp;

    for (size_t i = 0; i < input.size(); i++){
        if (input.substr(i, token.size()) == token){
            if (!temp.empty()){
                output.push_back(temp);
                temp.clear();
                i += token.size() - 1;
            }
            else{
                output.push_back("");
            }
        }
        else if (i + token.size() >= input.size()){
            temp += input.substr(i, token.size());
            output.push_back(temp);
            break;
        }
        else{
            temp += input[i];
        }
    }
}

inline std::string tail(const std::string &in){
    size_t token_start = in.find_first_not_of(" \t");
    size_t space_start = in.find_first_of(" \t", token_start);
    size_t tail_start = in.find_first_not_of(" \t", space_start);
    size_t tail_end = in.find_last_not_of(" \t");
    if (tail_start != std::string::npos && tail_end != std::string::npos){
        return in.substr(tail_start, tail_end - tail_start + 1);
    }
    else if (tail_start != std::string::npos){
        return in.substr(tail_start);
    }
    return "";
}

inline std::string firstToken(const std::string &in){
    if (!in.empty()){
        size_t token_start = in.find_first_not_of(" \t");
        size_t token_end = in.find_first_of(" \t", token_start);
        if (token_start != std::string::npos && token_end != std::string::npos){
            return in.substr(token_start, token_end - token_start);
        }
        else if (token_start != std::string::npos){
            return in.substr(token_start);
        }
    }
    return "";
}

bool loadGivenFile(std::string Path, vector<Triangle>& triangles){
			if (Path.substr(Path.size() - 4, 4) != ".obj")
				return false;

			std::ifstream file(Path);

			if (!file.is_open())
				return false;


			std::vector<vec3> positions;
			std::vector<vec2> textureCoords;
			std::vector<vec3> normals;

			std::string currentLine;
			
			while (std::getline(file, currentLine)){

				if (firstToken(currentLine) == "v"){
					std::vector<std::string> spos;
					vec3 vpos;
					split(tail(currentLine), spos, " ");

					vpos.x = std::stof(spos[0]);
					vpos.y = std::stof(spos[1]);
					vpos.z = std::stof(spos[2]);

					positions.push_back(vpos);
				}
				if (firstToken(currentLine) == "vt"){
					std::vector<std::string> stex;
					vec2 vtex;
					split(tail(currentLine), stex, " ");

					vtex.x = std::stof(stex[0]);
					vtex.y = std::stof(stex[1]);

					textureCoords.push_back(vtex);
				}
				if (firstToken(currentLine) == "vn"){
					std::vector<std::string> snor;
					vec3 vnor;
					split(tail(currentLine), snor, " ");

					vnor.x = std::stof(snor[0]);
					vnor.y = std::stof(snor[1]);
					vnor.z = std::stof(snor[2]);

					normals.push_back(vnor);
				}
				if (firstToken(currentLine) == "f"){
					std::vector<vec3> vVerts;
                    std::vector<std::string> afterSplitting;
                    split(tail(currentLine), afterSplitting, " ");
                    std::vector<vec3> verticesPositions;
                    std::vector<vec2> texturesPositions;
                    std::vector<vec3> normalsPositions;

                    for(size_t i = 0; i < afterSplitting.size(); i++){
                        std::vector<std::string> splitElems;
                        split(afterSplitting[i], splitElems, "/");
                        if(splitElems.size() == 1){
                            verticesPositions.push_back(positions[std::stoi(splitElems[0]) - 1]);
                            texturesPositions.push_back(vec2(0,0));
                            normalsPositions.push_back(vec3(1,1,1));
                        }else if(splitElems.size() == 2){
                            verticesPositions.push_back(positions[std::stoi(splitElems[0]) - 1]);
                            texturesPositions.push_back(textureCoords[std::stoi(splitElems[1]) - 1]);
                            normalsPositions.push_back(vec3(1,1,1));
                        }else if(splitElems.size() == 3 && splitElems[1] == ""){
                            verticesPositions.push_back(positions[std::stoi(splitElems[0]) - 1]);
                            texturesPositions.push_back(vec2(0,0));
                            normalsPositions.push_back(normals[std::stoi(splitElems[2]) - 1]);
                        }else if(splitElems.size() == 3 && splitElems[1] != ""){

                            verticesPositions.push_back(positions[std::stoi(splitElems[0]) - 1]);
                            texturesPositions.push_back(textureCoords[std::stoi(splitElems[1]) - 1]);
                            normalsPositions.push_back(normals[std::stoi(splitElems[2]) - 1]);
                        }
                    }

                    if(verticesPositions.size() == 3){
                        Triangle currentTriangle(toVec4(verticesPositions[0]), toVec4(verticesPositions[1]), toVec4(verticesPositions[2]), vec3(1,1,1), vec4(normalsPositions[0].x, normalsPositions[0].y, normalsPositions[0].z, 1));
                        currentTriangle.set_uvs(texturesPositions[0], texturesPositions[1], texturesPositions[2]);
                        currentTriangle.hasTexture = true;
                        triangles.push_back(currentTriangle);
                    }
					
				}
			
			}

			
            return true;
}

RenderedObject LoadObject(string path){
    vector<Triangle> triangles;
    loadGivenFile(path, triangles);
    RenderedObject returnedObject;
    returnedObject.triangles = triangles;
    return returnedObject;
}