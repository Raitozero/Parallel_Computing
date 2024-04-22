#include "QuadTree.h"
#include <cmath>

const double G = 6.674e-11;
float threshold = 0.5f;
int unitTime = 1000;

class Force{
public:
    float x, y;
    Force(){x=0; y =0;}
    Force(float xx, float yy){x=xx; y=yy;}
    Force operator+(const Force& ff){
        return Force(x+ff.x, y+ff.y);
    }
    Force& operator+=(const Force& ff) {
        x += ff.x;
        y += ff.y;
        return *this;
    }
};

float distance(unique_ptr<Particle>& particle, unique_ptr<Node>& node){
   return sqrt(pow((particle->position.x - node->com.x),2) + pow((particle->position.y - node->com.y),2));
}

//To determine if a node is sufficiently far away, compute the quotient s / d, 
//where s is the width of the region represented by the internal node, and d is the distance between the body and the node’s center-of-mass. 
//compare s/d against a threshold value θ. If s / d < θ, then the internal node is sufficiently far away.


//calculate the gravity exerted by p2 on p1
Force computeForceHelper(unique_ptr<Particle>& p1, unique_ptr<Particle>& p2){
    Force f;
    float dx = p2->position.x-p1->position.x, dy = p2->position.y-p1->position.y;
    float distSquared = dx * dx + dy * dy, dist = sqrt(distSquared);
    if(!distSquared) return f;
    float gravity = G * p1->mass * p2->mass / distSquared;
    f.x = gravity * dx/dist;
    f.y = gravity * dy/dist;
    return f;
}


void computeForce(unique_ptr<Node>& node, unique_ptr<Particle>& particle, Force& force){
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
    //else it's far away, we can ignore
}

unique_ptr<Particle> updateParticle(QuadTree& qt, unique_ptr<Particle>& particle){
    unique_ptr<Particle> newParticle = make_unique<Particle>(*particle);
    Force force;
    computeForce(qt.root, particle, force);
    float accX = force.x/particle->mass, accY = force.y/particle->mass;
    //s = vt+1/2*a*t^2
    newParticle->position.x = particle->position.x + particle->velocity.x * unitTime + 0.5 * accX * unitTime * unitTime;
    newParticle->position.y = particle->position.y + particle->velocity.y * unitTime + 0.5 * accY * unitTime * unitTime;
    // v' = v + a*t;
    newParticle->velocity.x = particle->velocity.x + accX * unitTime;
    newParticle->velocity.y = particle->velocity.y + accY * unitTime;
    return move(newParticle);
}

vector<unique_ptr<Particle>> update(QuadTree& qt, vector<unique_ptr<Particle>>& particles){
    vector<unique_ptr<Particle>> ans(particles.size());
    for(size_t i = 0; i < particles.size(); i++){
        ans[i] = updateParticle(qt, particles[i]);
    }
    return ans;
}

void computeForce_parallel(unique_ptr<Node>& node, vector<unique_ptr<Particle>>& particles){
}