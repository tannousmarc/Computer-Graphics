#include <iostream>
#include <fstream>
#include <glm/glm.hpp>
#include <stdint.h>
#include "definitions.cpp"

struct MTL{
    string name;
    float Ns;
    float Ni;
    float d;
    float Tr;
    vec3 Tf;
    float illum;
    vec3 Ka;
    vec3 Kd;
    vec3 Ks;
    vec3 Ke;
    string map_Ka;
    string map_Kd;
};

std::vector<std::string> split(std::string str,std::string sep){
    char* cstr=const_cast<char*>(str.c_str());
    char* current;
    std::vector<std::string> arr;
    current=strtok(cstr,sep.c_str());
    while(current!=NULL){
        arr.push_back(current);
        current=strtok(NULL,sep.c_str());
    }
    return arr;
}

MTL parseMTL(string file){
    // char path[512];
    // path[0] = 'M';
    // path[1] = 'T';
    // path[2] = 'L';
    // path[3] = 's';
    // path[4] = '\0';
    // // path = "MTLs";
    // // strcat(path, "MTLs/");
    // strcat(path, file);
    // cout << "INSIDE PARSE MTL" << endl;
    string path = "MTLs/12228_Dog_v1_L2.mtl";
    // cout << "WHAT " << path << " ??? ";
    ifstream object(path);

    MTL toReturn;
    
    for( std::string line; getline( object, line ); ){
        // cout << "READ LINE" << endl;
        vector<string> splitted = split(line, " ");

        // for(int i = 0; i < splitted.size(); i++){
        //     cout << splitted[i] << " ";
        // }
        // cout << endl;

        if(splitted.size() == 0)
            continue;
        if(splitted[0] == "newmtl"){
            toReturn.name = splitted[1];
        }else if(splitted[0] == "Ns"){
            toReturn.Ns = stof(splitted[1]);
        }else if(splitted[0] == "Ni"){
            toReturn.Ni = stof(splitted[1]);
        }else if(splitted[0] == "d"){
            toReturn.d = stof(splitted[1]);
        }else if(splitted[0] == "Tr"){
            toReturn.Tr = stof(splitted[1]);
        }else if(splitted[0] == "Tf"){
            float x = stof(splitted[1]);
            float y = stof(splitted[2]);
            float z = stof(splitted[3]);
            toReturn.Tf = vec3(x, y, z);
        }else if(splitted[0] == "illum"){
            toReturn.illum = stof(splitted[1]);
        }else if(splitted[0] == "Ka"){
            float x = stof(splitted[1]);
            float y = stof(splitted[2]);
            float z = stof(splitted[3]);
            toReturn.Ka = vec3(x, y, z);
        }else if(splitted[0] == "Kd"){
            float x = stof(splitted[1]);
            float y = stof(splitted[2]);
            float z = stof(splitted[3]);
            toReturn.Kd = vec3(x, y, z);
        }else if(splitted[0] == "Ks"){
            float x = stof(splitted[1]);
            float y = stof(splitted[2]);
            float z = stof(splitted[3]);
            toReturn.Ks = vec3(x, y, z);
        }else if(splitted[0] == "Ke"){
            float x = stof(splitted[1]);
            float y = stof(splitted[2]);
            float z = stof(splitted[3]);
            toReturn.Ke = vec3(x, y, z);
        }else if(splitted[0] == "map_Ka"){
            toReturn.map_Ka = splitted[1];
        }else if(splitted[0] == "map_Kd"){
            toReturn.map_Kd = splitted[1];
        }
    }

    return toReturn;

}


void LoadObject(string file, std::vector<Triangle>& triangles){
    // char path[512];
    // path[0] = '\0';
    // strcat(path, "Objects/");
    // strcat(path, file);
    string path = "Objects/" + file;
    ifstream object(path);

    vector<Vertex> vertices;
    vector<vec4> normals;
    vector<Texture> textures;
    MTL loadedMTL;

    bool hasMTL = false;

    for( std::string line; getline( object, line ); ){
        vector<string> splitted = split(line, " ");
        if(splitted.size() == 0)
            continue;

        for(int i = 0; i < splitted.size(); i++){
            cout << splitted[i] << " ";
        }
        cout << endl;
        if(splitted[0] == "mtllib"){
            cout << "MTLLIB" << endl;
            hasMTL = true;
            cout << splitted[1] << endl;
            loadedMTL = parseMTL(splitted[1]);
            // cout << "EXITED";
            cout << loadedMTL.map_Kd << " " << loadedMTL.illum << endl;
        }else if(splitted[0] == "v"){
            // cout << "V" << endl;
            float x = stof(splitted[1]);
            float y = stof(splitted[2]);
            float z = stof(splitted[3]);
            Vertex toPush;
            toPush.position = vec4(x, y, z, 1);
            vertices.push_back(toPush);
        }else if(splitted[0] == "vt"){
            // cout << "VT" << endl;
            float u = stof(splitted[1]);
            float v = stof(splitted[2]);
            Texture texture;
            texture.mapping = vec2(u,v);
            textures.push_back(texture);
        }else if(splitted[0] == "vn"){
            // cout << "VN" << endl;
            float x = stof(splitted[1]);
            float y = stof(splitted[2]);
            float z = stof(splitted[3]);
            normals.push_back(vec4(x, y, z, 1));
        }else if(splitted[0] == "f"){

        };
    }
}
/*
void LoadObject(const char* file, std::vector<Triangle>& triangles)
{
    char path[512];
    path[0] = '\0';
    strcat(path, "Objects/");
    strcat(path, file);
    
    vector<Vertex> vertices;

    vector<vec4> normals;
    vector<vec2> textures;

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

        // Reading a texture
        if(content == "vt"){
            string a,b,c;
            object >> a >> b >> c;
            textures.push_back(vec2(strtof(a.c_str(), 0), strtof(b.c_str(), 0)));
            cout << textures[textures.size()-1].x << " " << textures[textures.size()-1].y << endl;
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
                // printf('tokena is %s , a is %s \n', tokena, a);
                cout << "token a is " << tokena << " , a is " << a << endl;
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
*/