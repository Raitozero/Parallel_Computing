#ifndef NBODY_H
#define NBODY_H

#include <iostream>
#include <vector>
#include <memory>
#include <chrono>
#include <mutex>
#include <fstream>

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
    bool inRange = true;
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
    shared_ptr<Particle> particle;
    //int childid; //for debug use
    //vector<shared_ptr<Particle>> particles;
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
    void add(const shared_ptr<Particle>& p);//update the mass, com of curr node when adding a new particle to this node or its children
    void split();//initiate children
};

class QuadTree {
public:
    unique_ptr<Node> root;
    // QuadTree(){root = make_unique<Node>();}
    QuadTree(Point topleft, float length){
        root = make_unique<Node>(topleft, length);
    }
    void buildQuadTree_seq(vector<shared_ptr<Particle>>& particles);
    void buildQuadTree_Parallel(vector<shared_ptr<Particle>>& particles, int start, int end);
};

class Bitmap {
private:
    int width, height;
    vector<unsigned char> image;

    int getFileSize() const {
        return 54 + 3 * width * height; // 54 bytes for header, 3 bytes per pixel
    }

public:
    Bitmap(int w, int h) : width(w), height(h) {
        image.resize(3 * width * height);
        fill(image.begin(), image.end(), 0); // Start with a black image
    }
    void setPixel(int x, int y, unsigned char r, unsigned char g, unsigned char b);
    void drawBigDot(int centerX, int centerY, int dotSize, unsigned char r, unsigned char g, unsigned char b);
    void save(const string& filename);
};


class Timer{
public:
	chrono::system_clock::time_point Begin;
	chrono::system_clock::time_point End;
	chrono::system_clock::duration RunTime;
	Timer(){
		Begin = chrono::system_clock::now();
	}
	// ~Timer(){
	// 	End = chrono::system_clock::now();
	// 	RunTime = End - Begin;
	// 	cout << "Run Time is " << chrono::duration_cast<chrono::milliseconds>(RunTime).count() << "ms" << endl;
	// }
    int64_t dur_ms(){
        return chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now() - Begin).count();
    }
};


int findSection(const unique_ptr<Node>& node, const shared_ptr<Particle>& p);
void quadInsert(unique_ptr<Node>& node, shared_ptr<Particle>& p);
void quadInsert_parallel(unique_ptr<Node>& node, const shared_ptr<Particle>& p);
void quadTreeTrim(unique_ptr<Node>& node);

#endif