#include <iostream>
#include <memory>
#include <vector>

class Vector {
public:
    float x, y;

    Vector(float x, float y) : x(x), y(y) {}

    void add(const Vector& v) {
        x += v.x;
        y += v.y;
    }
};

class Particle {
public:
    Vector pos;
    // Assume there's a mass property, though your Java code doesn't define it
    float mass;

    Particle(Vector p, float m) : pos(p), mass(m) {}
};

class TreeNode {
public:
    float x, y, w;
    std::unique_ptr<TreeNode> children; // Children
    bool leaf;
    std::unique_ptr<Particle> particle;
  
    Vector totalCenter; // "Total" center of mass
    std::unique_ptr<Vector> center;
    float totalMass; // Total mass
    int count; // Number of particles

    TreeNode(float x, float y, float w)
    : x(x), y(y), w(w), leaf(true), particle(nullptr), children(nullptr),
      totalCenter(0, 0), center(nullptr), totalMass(0), count(0) {
        children = std::make_unique<TreeNode>(4);
    }

    void split() {
        float newWidth = w * 0.5f;
        children[0] = TreeNode(x, y, newWidth); // nw
        children[1] = TreeNode(x + newWidth, y, newWidth); // ne
        children[2] = TreeNode(x, y + newWidth, newWidth); // sw
        children[3] = TreeNode(x + newWidth, y + newWidth, newWidth); // se
        leaf = false;
    }

    int which(const Vector& v) {
        float halfWidth = w * 0.5f;
        if (v.y < y + halfWidth) {
            return v.x < x + halfWidth ? 0 : 1;
        }
        return v.x < x + halfWidth ? 2 : 3;
    }

    void insert(Particle newP) {
        Particle a = *particle; // Backup the existing particle

        if (leaf) {
            if (particle) {
                totalCenter.add(newP.pos);
                totalMass += newP.mass;
                count++;

                TreeNode* cur = this;
                int qA = cur->which(a.pos);
                int qB = cur->which(newP.pos);
                while (qA == qB) {
                    cur->split();
                    cur = &(cur->children[qA]);
                    qA = cur->which(a.pos);
                    qB = cur->which(newP.pos);
                    
                    cur->totalCenter.add(a.pos);
                    cur->totalCenter.add(newP.pos);
                    cur->totalMass += newP.mass * 2;
                    cur->count += 2;
                }

                cur->split();
                cur->children[qA].particle = std::make_unique<Particle>(a);
                cur->children[qB].particle = std::make_unique<Particle>(newP);

                cur->children[qA].totalCenter.add(a.pos);
                cur->children[qB].totalCenter.add(newP.pos);
                cur->children[qA].totalMass += a.mass;
                cur->children[qB].totalMass += newP.mass;
                cur->children[qA].count++;
                cur->children[qB].count++;

                particle.release();
            } else {
                particle = std::make_unique<Particle>(newP);
                totalCenter.add(newP.pos);
                totalMass += newP.mass;
                count++;
            }
        } else {
            totalCenter.add(newP.pos);
            totalMass += newP.mass;
            count++;
            children[which(newP.pos)].insert(newP);
        }
    }

    // Placeholder for `display` function as C++ typically does not handle graphics directly.
    // This function would need to integrate with a graphics library to provide equivalent functionality.
    void display() {
        // Integration with a graphical library required.
    }
};

int main() {
    // Example of usage
    TreeNode root(0, 0, 400);
    Particle p(Vector(100, 100), 1.0f);
    root.insert(p);
    // root.display(); // Called when integrated with a graphics library.
    return 0;
}