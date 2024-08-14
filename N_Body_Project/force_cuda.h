#include <cuda.h>
#include <cuda_runtime.h>


struct Particle {
    float3 position;
    float3 velocity;
    float mass;
    bool inRange;
};

struct Node {
    float3 com;  // Center of mass
    float mass;
    float length;
    bool isLeaf;
    int particleIndex;  // Index of the particle in the array if it's a leaf
    int children[4];    // Indices of the child nodes
};
