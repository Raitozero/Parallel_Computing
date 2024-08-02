#include "Force.h"
#include <cmath>

float distance(const shared_ptr<Particle>& particle, const unique_ptr<Node>& node){
   return sqrt(pow((particle->position.x - node->com.x),2) + pow((particle->position.y - node->com.y),2));
}

//To determine if a node is sufficiently far away, compute the quotient s / d, 
//where s is the width of the region represented by the internal node, and d is the distance between the body and the node’s center-of-mass. 
//compare s/d against a threshold value θ. If s / d < θ, then the internal node is sufficiently far away.


//calculate the gravity exerted by p2 on p1
Force computeForceHelper(const shared_ptr<Particle>& p1, const shared_ptr<Particle>& p2){
    Force f;
    float dx = p2->position.x-p1->position.x, dy = p2->position.y-p1->position.y;
    float distSquared = dx * dx + dy * dy, dist = sqrt(distSquared);
    if(!distSquared) return f;
    float gravity = G * p1->mass * p2->mass / distSquared;
    f.x = gravity * dx/dist;
    f.y = gravity * dy/dist;
    return f;
}

Force computeForceFaraway(const shared_ptr<Particle>& p1, const unique_ptr<Node>& node){
    Force f;
    float dx = node->com.x-p1->position.x, dy = node->com.y-p1->position.y;
    float distSquared = dx * dx + dy * dy, dist = sqrt(distSquared);
    if(!distSquared) return f;
    float gravity = G * p1->mass * node->mass / distSquared;
    f.x = gravity * dx/dist;
    f.y = gravity * dy/dist;
    return f;
}

void computeForce(const unique_ptr<Node>& node, const shared_ptr<Particle>& particle, Force& force){
    float s = node->length, d = distance(particle, node);
    //not far away: perform the process recursively on each of the root’s children
    if(s/d >= threshold){
        if(node->isLeaf) force += computeForceHelper(particle, node->particle);
        else{
            for(auto& child: node->children){
                if(child) computeForce(child, particle, force);
            }
        }
    }
    else force += computeForceFaraway(particle, node);
}

shared_ptr<Particle> updateParticle(const QuadTree& qt, const shared_ptr<Particle>& particle){
    shared_ptr<Particle> newParticle = make_shared<Particle>(*particle);
    Force force;
    computeForce(qt.root, particle, force);
    float accX = force.x/particle->mass, accY = force.y/particle->mass;
    //s = vt+1/2*a*t^2
    newParticle->position.x = particle->position.x + particle->velocity.x * unitTime + 0.5 * accX * unitTime * unitTime;
    newParticle->position.y = particle->position.y + particle->velocity.y * unitTime + 0.5 * accY * unitTime * unitTime;
    if(newParticle->position.x < 0 | newParticle->position.x > range | newParticle->position.y < 0 | newParticle->position.y > range) newParticle->inRange = false;
    // v' = v + a*t;
    newParticle->velocity.x = particle->velocity.x + accX * unitTime;
    newParticle->velocity.y = particle->velocity.y + accY * unitTime;
    return newParticle;
} 

vector<shared_ptr<Particle>> updateGenerateNew(const QuadTree& qt, const vector<shared_ptr<Particle>>& particles){
    vector<shared_ptr<Particle>> ans(particles.size());
    for(size_t i = 0; i < particles.size(); i++){
        ans[i] = updateParticle(qt, particles[i]);
    }
    return ans;
}


void updateGenerateNew_parallel(const QuadTree& qt, const vector<shared_ptr<Particle>>& origin, vector<shared_ptr<Particle>>& ans, int start, int end){
    for(size_t i = start; i < end; i++){
        ans[i] = updateParticle(qt, origin[i]);
    }
}


