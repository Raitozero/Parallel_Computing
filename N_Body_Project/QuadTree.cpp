#include "QuadTree.h"

//add particle to this node or its children
void Node::add(const shared_ptr<Particle>& p){
    this->com = Point((this->com.x * this->mass + p->position.x * p->mass)/(this->mass + p->mass),
    (this->com.y * this->mass + p->position.y * p->mass)/(this->mass + p->mass));//update 'center of mass' according to definition
    this->mass += p->mass;
    //this->particles.push_back(p);
    this->count++;
}


void Node::split() {
    float newLength = length / 2;
    this->children.emplace_back(make_unique<Node>(Point(topLeft.x, topLeft.y), newLength));
    this->children.emplace_back(make_unique<Node>(Point(topLeft.x + newLength, topLeft.y), newLength));
    this->children.emplace_back(make_unique<Node>(Point(topLeft.x, topLeft.y + newLength), newLength));
    this->children.emplace_back(make_unique<Node>(Point(topLeft.x + newLength, topLeft.y + newLength), newLength));
    isLeaf = false; // Now this Node is no longer a leaf since it has children
}

/*
insert a new particle to quadtree
if curr node is a leaf, then insert to 
*/

int findSection(const unique_ptr<Node>& node, const shared_ptr<Particle>& p){
    auto [x0, y0] = node->topLeft;
    auto [x, y]= p->position;
    int ans = 0;
    ans += x > x0 + node->length/2 ? 1 : 0;
    ans += y > y0 + node->length/2 ? 2 : 0;
    return ans;
}


void quadInsert(unique_ptr<Node>& node, shared_ptr<Particle>& p){
    if(!p->inRange) return;
    node->add(p);
    //case1: leaf & occupied
    if(node->isLeaf){
        if(node->particle){ 
            //if this node has a particle, we need to split and store this particle and new particle to its children
            shared_ptr<Particle> tmp = node->particle;
            node->particle.reset();
            node->split();
            quadInsert(node->children[findSection(node, tmp)], tmp);
            quadInsert(node->children[findSection(node, p)], p);
        }
        else{ // leaf & vacant 
            node->particle = p;
        }
    }
    else{
        quadInsert(node->children[findSection(node, p)], p);
    }
}

void quadInsert_parallel(unique_ptr<Node>& node, shared_ptr<Particle>& p){
    if(!p->inRange) return;
    unique_lock<mutex> lck(node->m);
    node->add(p);
    //case1: leaf & occupied
    if(node->isLeaf){
        if(node->particle){
            //if this node has a particle, we need to split and store this particle and new particle to its children
            shared_ptr<Particle> tmp = node->particle;
            node->particle.reset();
            node->split();
            lck.unlock();
            quadInsert_parallel(node->children[findSection(node, tmp)], tmp);
            quadInsert_parallel(node->children[findSection(node, p)], p);
        }
        else{ // leaf & vacant 
            node->particle = p;
        }
    }
    else{
        lck.unlock();
        quadInsert_parallel(node->children[findSection(node, p)], p);
    }
}

void quadTreeTrim(unique_ptr<Node>& node){
    if(!node) return;
    for(auto& child: node->children) quadTreeTrim(child);
    if(node->isLeaf && !node->particle) node.reset();
}

void QuadTree::buildQuadTree_seq(vector<shared_ptr<Particle>>& particles){
    for(auto& p: particles) quadInsert(this->root, p);
    quadTreeTrim(this->root);
}

void QuadTree::buildQuadTree_Parallel(vector<shared_ptr<Particle>>& particles, int start, int end){
    for(size_t i = start; i < end; i++) quadInsert_parallel(this->root, particles[i]);
    return;
}

