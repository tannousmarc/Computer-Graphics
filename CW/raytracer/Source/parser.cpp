#include <iostream>
#include <fstream>
#include <glm/glm.hpp>
#include <stdint.h>
#include "definitions.h"
#include "OBJ_Loader.h"

void LoadObject(string path, vector<Triangle>& triangles){
    objl::Loader Loader;
    bool loadout = Loader.LoadFile(path);
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

                triangles.push_back(currentTriangle);
			}
        }
    }
}
