#include <random>
#include <thread>
#include <fstream>

#include "QuadTree.cpp"
#include "image.cpp"
#include "force.cpp"

float range = 800.0;



//particle pos ranges from (0,0) to (range,range) for simplicity. For now.
vector<unique_ptr<Particle>> generateParticles(int num){
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<> massRand(100.0, 1000.0);
    uniform_real_distribution<> posRand(0.0, range);
    vector<unique_ptr<Particle>> ans;
    for(size_t i = 0; i < num; i++){
        Point position{Point(posRand(gen), posRand(gen))};
        ans.emplace_back(make_unique<Particle>(i, massRand(gen), position, Point(0.1, 0.1)));
    }
    return ans;
}

// void printNode(const unique_ptr<Node>& node, int depth = 0) {
//     string padding(depth * 4, ' ');
//     if(node->isLeaf && node->particle) {
//         cout << padding << "P: " << node->particle->id << endl;
//     } else {
//         cout << padding << "Itn" << endl;
//     }
// }


// void printQuadTree(const unique_ptr<Node>& node, int depth = 0) {
//     if(!node) return;
//     printNode(node, depth);
//     if(!node->isLeaf) {
//         for(const auto& child : node->children) {
//             printQuadTree(child, depth + 1);
//         }
//     }
// }


void printNode(ofstream& outfile, const unique_ptr<Node>& node, int depth = 0) {
    string padding(depth * 4, ' ');
    if (node->isLeaf && node->particle) {
        outfile << padding << "P: " << node->particle->id << endl;
    } else {
        outfile << padding << "Int" << endl;
    }
}

void printQuadTree(ofstream& outfile, const unique_ptr<Node>& node, int depth = 0) {
    if (!node) return;
    printNode(outfile, node, depth);
    if (!node->isLeaf) {
        for (const auto& child : node->children) {
            printQuadTree(outfile, child, depth + 1);
        }
    }
}



int main(){
    Bitmap image1(range, range);
    Bitmap image2(range, range);
    int particleNum = 200;
    vector<unique_ptr<Particle>> particles = generateParticles(particleNum);
    vector<unique_ptr<Particle>> duplicates;
    for(auto& p:particles) duplicates.push_back(make_unique<Particle>(*p));
    for (const auto& particle : particles) {
        image1.setPixel(particle->position.x, particle->position.y, 255, 255, 255);
    }
    image1.save("particles1.bmp");

    //ofstream outfile1("quadtree1.txt");
    //ofstream outfile2("quadtree2.txt");

    // //Test 1
    // //QuadTree build time
    // {
    //     Timer t1;
    //     QuadTree qt1(Point(0, 0), range);
    //     qt1.buildQuadTree_seq(particles);
    //     //printQuadTree(outfile1, qt1.root);
    //     cout << "t1: ";
    // }

    // {
    //     Timer t2;
    //     QuadTree qt2(Point(0, 0), range);
    //     int num = 4;
    //     vector<thread> v(num);
    //     for(size_t i = 1; i <= num; i++){
    //         v[i-1] = thread(&QuadTree::buildQuadTree_Parallel, ref(qt2), ref(duplicates), (i-1)* particleNum/num, i*particleNum/num);
    //     }
    //     for(auto& p: v) p.join();
    //     for(size_t i = 0; i < 4; i++){
    //         v[i] = thread(quadTreeTrim, ref(qt2.root->children[i]));
    //     }
    //     for(auto& p: v) p.join();
    //     //printQuadTree(outfile2, qt2.root);
    //     cout << "t2: ";
    // }
    // //Test1 end

    //Test 2 compute Force
    QuadTree qt1(Point(0, 0), range);
    qt1.buildQuadTree_seq(particles);
    vector<unique_ptr<Particle>> newParticles = update(qt1, duplicates);
    for (const auto& particle : newParticles) {
        image2.setPixel(particle->position.x, particle->position.y, 255, 255, 255);
    }
    image2.save("particles2.bmp");
    cout << "Done" << endl;
    return 0;
}