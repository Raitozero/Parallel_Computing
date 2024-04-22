#ifndef QUAD_TREE_H
#define QUAD_TREE_H


#include <iostream>
#include <vector>
#include <memory>
#include <chrono>
#include <mutex>

using namespace std;


class Point{
public:
    float x, y;
    Point(): x(0), y(0){}
    Point(float f1, float f2): x(f1), y(f2){}
    Point operator+(const Point& p){
        return Point(x+p.x, y+p.y);
    }
};

class Particle {
public:
    int id;
    float mass;
    Point position;
    Point velocity;
    Particle(int idd, float mas, Point pos, Point vel): id(idd), mass(mas), position(pos), velocity(vel) {}
    Particle(const Particle& p): id(p.id), mass(p.mass), position(p.position), velocity(p.velocity){};
};

class Node {
/*     (x,y) --------------- 
      |           |           |
      |children[0]|children[1]|
      | ----------+---------  |
      |children[2]|children[3]|
      |           |           |
          -----------------  
              length
*/
public:
    Point topLeft, com; //the coordinates of the topleft point; center of mass
    float length, mass; // length of each side
    bool isLeaf;
    int count;
    mutex m;
    vector<unique_ptr<Node>> children;
    unique_ptr<Particle> particle;
    //vector<unique_ptr<Particle>> particles;
    // Node(){
    //     topLeft = Point(-1,-1);
    //     com = topLeft;
    //     length = 0;
    //     mass = 0;
    //     count = 0;
    //     particle = nullptr;
    //     isLeaf = true;
    //     childid = -1;
    // }

    Node(Point p, float len){
        topLeft = p;
        length = len;
        com = topLeft;
        mass = 0;
        count = 0;
        particle = nullptr;
        isLeaf = true;
    }
    void add(const unique_ptr<Particle>& p);//update the mass, com of curr node when adding a new particle to this node or its children
    void split();//initiate children
};

class QuadTree {
public:
    unique_ptr<Node> root;
    // QuadTree(){root = make_unique<Node>();}
    QuadTree(Point topleft, float length){
        root = make_unique<Node>(topleft, length);
    }
    void buildQuadTree_seq(vector<unique_ptr<Particle>>& particles);
    void buildQuadTree_Parallel(vector<unique_ptr<Particle>>& particles, int start, int end);
};

class Timer{
public:
	chrono::system_clock::time_point Begin;
	chrono::system_clock::time_point End;
	chrono::system_clock::duration RunTime;
	Timer(){
		Begin = chrono::system_clock::now();
	}
	~Timer(){
		End = chrono::system_clock::now();
		RunTime = End - Begin;
		cout << "Run Time is " << chrono::duration_cast<chrono::milliseconds>(RunTime).count() << "ms" << endl;
	}
};


#endif