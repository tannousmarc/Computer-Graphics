#include <iostream>
#include <fstream>
#include <glm/glm.hpp>
#include <stdint.h>
#include "definitions.cpp"

void LoadObject(const char* file, std::vector<Triangle>& triangles)
{
    char path[512];
    path[0] = '\0';
    strcat(path, "Objects/");
    strcat(path, file);
    
    vector<Vertex> vertices;

    vector<vec4> normals;

    string content;
    ifstream object(path);
    while(object >> content) {
        // Reading a vertex
        if(content == "v"){
            string a,b,c,d;
            object >> a >> b >> c;
            Vertex temp;
            temp.position = vec4(strtof(a.c_str(), 0), strtof(b.c_str(), 0), strtof(c.c_str(), 0), 1);
            vertices.push_back(temp);
        }
        // Reading a normal
        if(content == "vn"){
            string a,b,c;
            object >> a >> b >> c;

            normals.push_back(vec4(strtof(a.c_str(), 0), strtof(b.c_str(), 0), strtof(c.c_str(), 0), 1));
        }
        // Reading a triangle
        if(content == "f"){
            string a,b,c;
            object >> a >> b >> c;

            string tokena;
            vector<string> parsedA;
            while(tokena != a){
                tokena = a.substr(0,a.find_first_of("/"));
                a = a.substr(a.find_first_of("/") + 1);
                parsedA.push_back(tokena.c_str());
            }
            string tokenb;
            vector<string> parsedB;
            while(tokenb != b){
                tokenb = b.substr(0,b.find_first_of("/"));
                b = b.substr(b.find_first_of("/") + 1);
                parsedB.push_back(tokenb.c_str());
            }
            string tokenc;
            vector<string> parsedC;
            while(tokenc != c){
                tokenc = c.substr(0,c.find_first_of("/"));
                c = c.substr(c.find_first_of("/") + 1);
                parsedC.push_back(tokenc.c_str());
            }


            if(2 < parsedA.size()){
                triangles.push_back(Triangle(vertices[stoi(parsedA[0].c_str()) - 1].position, 
                                            vertices[stoi(parsedB[0].c_str()) - 1].position, 
                                            vertices[stoi(parsedC[0].c_str()) - 1].position, 
                                            vec3(1, 1, 1),
                                            normals[stoi(parsedA[2].c_str()) - 1]
                                            ));
            }
            else{
                triangles.push_back(Triangle(vertices[stoi(parsedA[0].c_str()) - 1].position, 
                                            vertices[stoi(parsedB[0].c_str()) - 1].position, 
                                            vertices[stoi(parsedC[0].c_str()) - 1].position, 
                                            vec3(0.71, 0.71, 0.71),
                                            1
                                            ));
            }
        }
    }

    object.close();
}