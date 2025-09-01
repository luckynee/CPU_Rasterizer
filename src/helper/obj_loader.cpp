#include <sstream>
#include <fstream>

#include "helper/obj_loader.hpp"

namespace helper
{
    std::vector<rasterizer::vector3f> load_obj(const std::string &filename)
    {
        std::vector<rasterizer::vector3f> points;
        std::vector<rasterizer::vector3f> trianglesPoints;

        std::ifstream file(filename);
        std::string line;

        while (std::getline(file, line))
        {
            std::istringstream iss(line);
            std::string prefix;
            iss >> prefix;

            if (prefix == "v")
            {
                rasterizer::vector3f vertex;
                iss >> vertex.x >> vertex.y >> vertex.z;
                points.push_back(vertex);
            }
            else if (prefix == "f")
            {
                std::string vertex_str;
                std::vector<int> indices;
                while (iss >> vertex_str)
                {
                    size_t pos = vertex_str.find("//");
                    int vertex_index = 0;
                    if (pos != std::string::npos)
                        vertex_index = std::stoi(vertex_str.substr(0, pos));
                    else
                        vertex_index = std::stoi(vertex_str);

                    // Obj is 1-based
                    indices.push_back(vertex_index - 1);
                }
                // Triangulate (assuming triangles)
                if (indices.size() == 3)
                {
                    trianglesPoints.push_back(points[indices[0]]);
                    trianglesPoints.push_back(points[indices[1]]);
                    trianglesPoints.push_back(points[indices[2]]);
                }
            }
        }

        return trianglesPoints;
    }
}