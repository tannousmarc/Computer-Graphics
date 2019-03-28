#include <iostream>
#include <fstream>
#include <glm/glm.hpp>
#include <stdint.h>
#include "definitions.cpp"
#include "OBJ_Loader.h"

RenderedObject LoadObject(string path){
    objl::Loader Loader;
    bool loadout = Loader.LoadFile(path);
    vector<Triangle> triangles;
    RenderedObject returnedObject;
    if(loadout){
        for (int i = 0; i < Loader.LoadedMeshes.size(); i++){
            objl::Mesh curMesh = Loader.LoadedMeshes[i];
            // for (int j = 0; j < curMesh.Vertices.size(); j++)
			// {
			// 	cout << "V" << j << ": " <<
			// 		"P(" << curMesh.Vertices[j].Position.X << ", " << curMesh.Vertices[j].Position.Y << ", " << curMesh.Vertices[j].Position.Z << ") " <<
			// 		"N(" << curMesh.Vertices[j].Normal.X << ", " << curMesh.Vertices[j].Normal.Y << ", " << curMesh.Vertices[j].Normal.Z << ") " <<
			// 		"TC(" << curMesh.Vertices[j].TextureCoordinate.X << ", " << curMesh.Vertices[j].TextureCoordinate.Y << ")\n";
			// }
            // cout << "Indices:\n";

			// Go through every 3rd index and print the
			//	triangle that these indices represent
			for (int j = 0; j < curMesh.Indices.size(); j += 3)
			{
				// cout << "T" << j / 3 << ": " << curMesh.Indices[j] << ", " << curMesh.Indices[j + 1] << ", " << curMesh.Indices[j + 2] << "\n";
                auto currentV1 = curMesh.Vertices[curMesh.Indices[j]];
                auto currentV2 = curMesh.Vertices[curMesh.Indices[j+1]];
                auto currentV3 = curMesh.Vertices[curMesh.Indices[j+2]];

                vec4 positionV1(currentV1.Position.X, currentV1.Position.Y, currentV1.Position.Z, 1);
                vec4 positionV2(currentV2.Position.X, currentV2.Position.Y, currentV2.Position.Z, 1);
                vec4 positionV3(currentV3.Position.X, currentV3.Position.Y, currentV3.Position.Z, 1);

                vec4 normal(currentV1.Normal.X, currentV1.Normal.Y, currentV2.Normal.Z, 1);
                Triangle currentTriangle(positionV1, positionV2, positionV3, vec3(1,1,1), normal);

                vec2 textureV1(currentV1.TextureCoordinate.X, currentV1.TextureCoordinate.Y);
                vec2 textureV2(currentV2.TextureCoordinate.X, currentV2.TextureCoordinate.Y);
                vec2 textureV3(currentV3.TextureCoordinate.X, currentV3.TextureCoordinate.Y);

                if(textureV1.x != 0 || textureV2.y != 0){
                    currentTriangle.set_uvs(textureV1, textureV2, textureV3);
                    // cout << textureV3.x << " " << textureV3.y << endl;
                    currentTriangle.hasTexture = true;
                }else{
                    currentTriangle.hasTexture = false;
                }

                triangles.push_back(currentTriangle);
                // triangles.push_back(Triangle(positionV1, positionV2, positionV3, vec3(1,1,1), normal));
                // triangles.push_back(Triangle(vertices[stoi(parsedA[0].c_str()) - 1].position, 
                //                                             vertices[stoi(parsedB[0].c_str()) - 1].position, 
                //                                             vertices[stoi(parsedC[0].c_str()) - 1].position, 
                //                                             vec3(1, 1, 1),
                //                                             normals[stoi(parsedA[2].c_str()) - 1]
                //                                             ));
			}
        }
    }
    returnedObject.triangles = triangles;
    return returnedObject;
}