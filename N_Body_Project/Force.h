#ifndef FORCE_H
#define FORCE_H

#include "NBody.h"
#include <cmath>

extern const double G;
extern float range, threshold;
extern int particleNum, unitTime;

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

float distance(const shared_ptr<Particle>& particle, const unique_ptr<Node>& node);

//To determine if a node is sufficiently far away, compute the quotient s / d, 
//where s is the width of the region represented by the internal node, and d is the distance between the body and the node’s center-of-mass. 
//compare s/d against a threshold value θ. If s / d < θ, then the internal node is sufficiently far away.


//calculate the gravity exerted by p2 on p1
Force computeForceHelper(const shared_ptr<Particle>& p1, const shared_ptr<Particle>& p2);

//if a particle is faraway from a node, consider all particles in this node's children to be one single body
Force computeForceFaraway(const shared_ptr<Particle>& p1, const unique_ptr<Node>& node);

void computeForce(const unique_ptr<Node>& node, const shared_ptr<Particle>& particle, Force& force);

shared_ptr<Particle> updateParticle(const QuadTree& qt, const shared_ptr<Particle>& particle);

vector<shared_ptr<Particle>> updateGenerateNew(const QuadTree& qt, const vector<shared_ptr<Particle>>& particles);

void updateGenerateNew_parallel(const QuadTree& qt, const vector<shared_ptr<Particle>>& origin, vector<shared_ptr<Particle>>& ans, int start, int end);

#endif

