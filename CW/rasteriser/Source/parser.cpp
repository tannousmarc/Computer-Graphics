#include <iostream>
#include <fstream>
#include <glm/glm.hpp>
#include <stdint.h>
#include "definitions.cpp"
#include "OBJ_Loader.h"

glm::vec4 toVec4(glm::vec3 ceva){
    return vec4(ceva.x, ceva.y, ceva.z, 1);
}
namespace algorithm{
		// Split a String into a string array at a given token
		inline void split(const std::string &in,
			std::vector<std::string> &out,
			std::string token)
		{
			out.clear();

			std::string temp;

			for (int i = 0; i < int(in.size()); i++)
			{
				std::string test = in.substr(i, token.size());

				if (test == token)
				{
					if (!temp.empty())
					{
						out.push_back(temp);
						temp.clear();
						i += (int)token.size() - 1;
					}
					else
					{
						out.push_back("");
					}
				}
				else if (i + token.size() >= in.size())
				{
					temp += in.substr(i, token.size());
					out.push_back(temp);
					break;
				}
				else
				{
					temp += in[i];
				}
			}
		}

		// Get tail of string after first token and possibly following spaces
		inline std::string tail(const std::string &in)
		{
			size_t token_start = in.find_first_not_of(" \t");
			size_t space_start = in.find_first_of(" \t", token_start);
			size_t tail_start = in.find_first_not_of(" \t", space_start);
			size_t tail_end = in.find_last_not_of(" \t");
			if (tail_start != std::string::npos && tail_end != std::string::npos)
			{
				return in.substr(tail_start, tail_end - tail_start + 1);
			}
			else if (tail_start != std::string::npos)
			{
				return in.substr(tail_start);
			}
			return "";
		}

		// Get first token of string
		inline std::string firstToken(const std::string &in)
		{
			if (!in.empty())
			{
				size_t token_start = in.find_first_not_of(" \t");
				size_t token_end = in.find_first_of(" \t", token_start);
				if (token_start != std::string::npos && token_end != std::string::npos)
				{
					return in.substr(token_start, token_end - token_start);
				}
				else if (token_start != std::string::npos)
				{
					return in.substr(token_start);
				}
			}
			return "";
		}
	}

bool loadGivenFile(std::string Path, vector<Triangle>& triangles){
			cout << "A";

			if (Path.substr(Path.size() - 4, 4) != ".obj")
				return false;

			cout << "B";
			std::ifstream file(Path);

			if (!file.is_open())
				return false;

			cout << "C";

			std::vector<vec3> positions;
			std::vector<vec2> textureCoords;
			std::vector<vec3> normals;

			std::string curline;
			
			while (std::getline(file, curline)){
                
				if (algorithm::firstToken(curline) == "v"){
					std::vector<std::string> spos;
					vec3 vpos;
					algorithm::split(algorithm::tail(curline), spos, " ");

					vpos.x = std::stof(spos[0]);
					vpos.y = std::stof(spos[1]);
					vpos.z = std::stof(spos[2]);

					positions.push_back(vpos);
				}
			// 	// Generate a Vertex Texture Coordinate
				if (algorithm::firstToken(curline) == "vt"){
					std::vector<std::string> stex;
					vec2 vtex;
					algorithm::split(algorithm::tail(curline), stex, " ");

					vtex.x = std::stof(stex[0]);
					vtex.y = std::stof(stex[1]);

					textureCoords.push_back(vtex);
				}
			// 	// Generate a Vertex Normal;
				if (algorithm::firstToken(curline) == "vn"){
					std::vector<std::string> snor;
					vec3 vnor;
					algorithm::split(algorithm::tail(curline), snor, " ");

					vnor.x = std::stof(snor[0]);
					vnor.y = std::stof(snor[1]);
					vnor.z = std::stof(snor[2]);

					normals.push_back(vnor);
				}
			// 	// Generate a Face (vertices & indices)
				if (algorithm::firstToken(curline) == "f")
				{
					// Generate the vertices
					std::vector<vec3> vVerts;
					// GenVerticesFromRawOBJ(vVerts, Positions, TCoords, Normals, curline);
                    std::vector<std::string> afterSplitting;
                    algorithm::split(algorithm::tail(curline), afterSplitting, " ");
                    std::vector<vec3> verticesPositions;
                    std::vector<vec2> texturesPositions;
                    std::vector<vec3> normalsPositions;

                    for(int i = 0; i < afterSplitting.size(); i++){
                        std::vector<std::string> splitElems;
                        algorithm::split(afterSplitting[i], splitElems, "/");
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
    bool ceva = loadGivenFile(path, triangles);
    RenderedObject returnedObject;
    // if(loadout){
    //     for (size_t i = 0; i < Loader.LoadedMeshes.size(); i++){
    //         objl::Mesh curMesh = Loader.LoadedMeshes[i];
    //         // for (int j = 0; j < curMesh.Vertices.size(); j++)
	// 		// {
	// 		// 	cout << "V" << j << ": " <<
	// 		// 		"P(" << curMesh.Vertices[j].Position.X << ", " << curMesh.Vertices[j].Position.Y << ", " << curMesh.Vertices[j].Position.Z << ") " <<
	// 		// 		"N(" << curMesh.Vertices[j].Normal.X << ", " << curMesh.Vertices[j].Normal.Y << ", " << curMesh.Vertices[j].Normal.Z << ") " <<
	// 		// 		"TC(" << curMesh.Vertices[j].TextureCoordinate.X << ", " << curMesh.Vertices[j].TextureCoordinate.Y << ")\n";
	// 		// }
    //         // cout << "Indices:\n";

	// 		// Go through every 3rd index and print the
	// 		//	triangle that these indices represent
	// 		for (size_t j = 0; j < curMesh.Indices.size(); j += 3)
	// 		{
	// 			// cout << "T" << j / 3 << ": " << curMesh.Indices[j] << ", " << curMesh.Indices[j + 1] << ", " << curMesh.Indices[j + 2] << "\n";
    //             auto currentV1 = curMesh.Vertices[curMesh.Indices[j]];
    //             auto currentV2 = curMesh.Vertices[curMesh.Indices[j+1]];
    //             auto currentV3 = curMesh.Vertices[curMesh.Indices[j+2]];

    //             vec4 positionV1(currentV1.Position.X, currentV1.Position.Y, currentV1.Position.Z, 1);
    //             vec4 positionV2(currentV2.Position.X, currentV2.Position.Y, currentV2.Position.Z, 1);
    //             vec4 positionV3(currentV3.Position.X, currentV3.Position.Y, currentV3.Position.Z, 1);

    //             vec4 normal(currentV1.Normal.X, currentV1.Normal.Y, currentV2.Normal.Z, 1);
    //             Triangle currentTriangle(positionV1, positionV2, positionV3, vec3(1,1,1), normal);

    //             vec2 textureV1(currentV1.TextureCoordinate.X, currentV1.TextureCoordinate.Y);
    //             vec2 textureV2(currentV2.TextureCoordinate.X, currentV2.TextureCoordinate.Y);
    //             vec2 textureV3(currentV3.TextureCoordinate.X, currentV3.TextureCoordinate.Y);

    //             if(textureV1.x != 0 || textureV2.y != 0){
    //                 currentTriangle.set_uvs(textureV1, textureV2, textureV3);
    //                 // cout << textureV3.x << " " << textureV3.y << endl;
    //                 currentTriangle.hasTexture = true;
    //             }else{
    //                 currentTriangle.hasTexture = false;
    //             }

    //             triangles.push_back(currentTriangle);
    //             // triangles.push_back(Triangle(positionV1, positionV2, positionV3, vec3(1,1,1), normal));
    //             // triangles.push_back(Triangle(vertices[stoi(parsedA[0].c_str()) - 1].position, 
    //             //                                             vertices[stoi(parsedB[0].c_str()) - 1].position, 
    //             //                                             vertices[stoi(parsedC[0].c_str()) - 1].position, 
    //             //                                             vec3(1, 1, 1),
    //             //                                             normals[stoi(parsedA[2].c_str()) - 1]
    //             //                                             ));
	// 		}
    //     }
    // }
    returnedObject.triangles = triangles;
    return returnedObject;
}