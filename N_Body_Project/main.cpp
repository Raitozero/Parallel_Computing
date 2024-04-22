#include <random>
#include <thread>
#include <fstream>

#include "QuadTree.cpp"
#include "image.cpp"
#include "force.cpp"




//particle pos ranges from (0,0) to (range,range) for simplicity. For now.
vector<shared_ptr<Particle>> generateParticles(int num){
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<> massRand(10e7, 10e9);
    uniform_real_distribution<> posRand(0.0, range);
    vector<shared_ptr<Particle>> ans;
    for(size_t i = 0; i < num; i++){
        Point position{Point(posRand(gen), posRand(gen))};
        ans.emplace_back(make_shared<Particle>(i, massRand(gen), position, Point(0, 0)));
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
    vector<shared_ptr<Particle>> particles = generateParticles(particleNum);
    vector<shared_ptr<Particle>> duplicates;
    for(auto& p:particles) duplicates.push_back(make_shared<Particle>(*p));
    //ofstream outfile1("quadtree1.txt");
    //ofstream outfile2("quadtree2.txt");

    //Test 1
    //QuadTree build time
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
    //     vector<thread> v(threadNum);
    //     for(size_t i = 1; i <= threadNum; i++){
    //         v[i-1] = thread(&QuadTree::buildQuadTree_Parallel, ref(qt2), ref(duplicates), (i-1)* particleNum/threadNum, i*particleNum/threadNum);
    //     }
    //     for(auto& p: v) p.join();
    //     for(size_t i = 0; i < threadNum; i++){
    //         v[i] = thread(quadTreeTrim, ref(qt2.root->children[i]));
    //     }
    //     for(auto& p: v) p.join();
    //     //printQuadTree(outfile2, qt2.root);
    //     cout << "t2: ";
    // }
    //Test1 end

    //Test 2 compute Force
    {
        Timer t1;
        for(size_t i = 0; i < 10; i++){
            Bitmap image(range, range);
            QuadTree qt1(Point(0, 0), range);
            qt1.buildQuadTree_seq(particles);
            vector<shared_ptr<Particle>> newParticles = updateGenerateNew(qt1, particles);
            for (const auto& particle : newParticles) {
                if(particle->id == 1 || particle->id == 10) image.drawBigDot(particle->position.x, particle->position.y, 15, 255, 0, 0);
                image.drawBigDot(particle->position.x, particle->position.y, 5, 255, 255, 255);
            }
            string filename = "seq" + to_string(i) + ".bmp";
            image.save(filename);
            particles = newParticles;
        }
        cout << "t1: ";
    }

    {
        Timer t2;
        for(size_t i = 0; i < 10; i++){
            Bitmap image(range, range);
            QuadTree qt2(Point(0, 0), range);
            qt2.buildQuadTree_seq(duplicates);
            vector<shared_ptr<Particle>> newParticles(particleNum);
            vector<thread> threads(threadNum);
            for(size_t i = 1; i < threadNum+1; i++){
                threads[i-1] =  thread(updateGenerateNew_parallel, ref(qt2), ref(duplicates), ref(newParticles), (i-1)* particleNum/threadNum, i * particleNum/threadNum);
            }
            for(auto& thread: threads) thread.join();
            for (const auto& particle : newParticles) {
                if(particle->id == 1 || particle->id == 10) image.drawBigDot(particle->position.x, particle->position.y, 15, 255, 0, 0);
                image.drawBigDot(particle->position.x, particle->position.y, 5, 255, 255, 255);
            }
            string filename = "parallel" + to_string(i) + ".bmp";
            image.save(filename);
            duplicates = newParticles;
        }
        cout << "t2: ";
    }

    cout << "Done" << endl;
    return 0;
}