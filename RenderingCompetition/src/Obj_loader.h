#pragma once

#include <string>
#include <vector>

#include "base.h"
#include "Material.h"

using namespace std;

class ObjLoader {
   public:
    bool load(const string &fname,
              vector<Triangle> &triangles,
              vector<Material> &materials);
};