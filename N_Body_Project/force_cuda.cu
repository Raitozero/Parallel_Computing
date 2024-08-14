#include "Force.h"
#include "force_cuda.h"
#include <math.h>

__device__ float distance(const Particle& p, const Node& node) {
    return sqrt(pow((p.position.x - node.com.x), 2) + pow((p.position.y - node.com.y), 2));
}

__device__ Force computeForceHelper(const Particle& p1, const Particle& p2) {
    Force f = {0.0f, 0.0f};
    float dx = p2.position.x - p1.position.x;
    float dy = p2.position.y - p1.position.y;
    float distSquared = dx * dx + dy * dy;
    float dist = sqrt(distSquared);
    if (distSquared == 0) return f;
    float gravity = G * p1.mass * p2.mass / distSquared;
    f.x = gravity * dx / dist;
    f.y = gravity * dy / dist;
    return f;
}

__device__ Force computeForceFaraway(const Particle& p, const Node& node) {
    Force f = {0.0f, 0.0f};
    float dx = node.com.x - p.position.x;
    float dy = node.com.y - p.position.y;
    float distSquared = dx * dx + dy * dy;
    float dist = sqrt(distSquared);
    if (distSquared == 0) return f;
    float gravity = G * p.mass * node.mass / distSquared;
    f.x = gravity * dx / dist;
    f.y = gravity * dy / dist;
    return f;
}

__global__ void computeForces(Particle* particles, Force* forces, Node* nodes, int numParticles, float threshold) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx >= numParticles) return;

    Particle p = particles[idx];
    Force totalForce = {0.0f, 0.0f};

    // Traverse the quadtree and calculate forces
    for (int i = 0; i < numNodes; i++) {
        Node node = nodes[i];
        float s = node.length;
        float d = distance(p, node);

        if (s / d >= threshold) {
            if (node.isLeaf) {
                int particleIdx = node.particleIndex;
                Force f = computeForceHelper(p, particles[particleIdx]);
                totalForce.x += f.x;
                totalForce.y += f.y;
            } else {
                // Traverse children (handled by next iterations)
            }
        } else {
            Force f = computeForceFaraway(p, node);
            totalForce.x += f.x;
            totalForce.y += f.y;
        }
    }

    forces[idx] = totalForce;
}

void launchForceComputation(Particle* h_particles, Force* h_forces, Node* h_nodes, int numParticles, int numNodes, float threshold) {
    // Allocate device memory
    Particle* d_particles;
    Force* d_forces;
    Node* d_nodes;
    cudaMalloc(&d_particles, numParticles * sizeof(Particle));
    cudaMalloc(&d_forces, numParticles * sizeof(Force));
    cudaMalloc(&d_nodes, numNodes * sizeof(Node));

    // Copy data from host to device
    cudaMemcpy(d_particles, h_particles, numParticles * sizeof(Particle), cudaMemcpyHostToDevice);
    cudaMemcpy(d_nodes, h_nodes, numNodes * sizeof(Node), cudaMemcpyHostToDevice);

    // Launch the kernel
    int blockSize = 256;
    int numBlocks = (numParticles + blockSize - 1) / blockSize;
    computeForces<<<numBlocks, blockSize>>>(d_particles, d_forces, d_nodes, numParticles, threshold);

    // Copy results back to host
    cudaMemcpy(h_forces, d_forces, numParticles * sizeof(Force), cudaMemcpyDeviceToHost);

    // Free device memory
    cudaFree(d_particles);
    cudaFree(d_forces);
    cudaFree(d_nodes);
}

vector<shared_ptr<Particle>> updateGenerateNew(const QuadTree& qt, const vector<shared_ptr<Particle>>& particles) {
    int numParticles = particles.size();
    vector<Particle> h_particles(numParticles);
    vector<Force> h_forces(numParticles);
    vector<Node> h_nodes; // Convert your quadtree to a flat array of nodes

    // Copy particles to the flat array
    for (int i = 0; i < numParticles; i++) {
        h_particles[i] = *particles[i];
    }

    // Launch CUDA kernel
    launchForceComputation(h_particles.data(), h_forces.data(), h_nodes.data(), numParticles, h_nodes.size(), threshold);

    // Update particle positions and velocities
    vector<shared_ptr<Particle>> newParticles(numParticles);
    for (int i = 0; i < numParticles; i++) {
        newParticles[i] = make_shared<Particle>(h_particles[i]);
        float accX = h_forces[i].x / h_particles[i].mass;
        float accY = h_forces[i].y / h_particles[i].mass;
        newParticles[i]->position.x += h_particles[i].velocity.x * unitTime + 0.5 * accX * unitTime * unitTime;
        newParticles[i]->position.y += h_particles[i].velocity.y * unitTime + 0.5 * accY * unitTime * unitTime;
        newParticles[i]->velocity.x += accX * unitTime;
        newParticles[i]->velocity.y += accY * unitTime;
    }

    return newParticles;
}
