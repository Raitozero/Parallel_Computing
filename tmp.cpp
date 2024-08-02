#include <random>
#include <thread>
#include <cstring>

#include "Force.h"

const double G = 6.674e-11;

float range = 1600.0, threshold = 0.5f;
int particleNum = 100000, unitTime = 100, threadNum = 4;

//particle pos ranges from (0,0) to (.1* range, .9 * range) for simplicity. For now.
vector<shared_ptr<Particle>> generateParticles(int num){
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<> massRand(5*10e8, 10e9);
    uniform_real_distribution<> posRand(0.1 * range, 0.9 * range);
    vector<shared_ptr<Particle>> ans;
    for(size_t i = 0; i < num; i++){
        Point position{Point(posRand(gen), posRand(gen))};
        ans.emplace_back(make_shared<Particle>(i, massRand(gen), position, Point(0, 0)));
    }
    return ans;
}

//For debug use, Print Node Structure.
// void printNode(ofstream& outfile, const unique_ptr<Node>& node, int depth = 0) {
//     string padding(depth * 4, ' ');
//     if (node->isLeaf && node->particle) {
//         outfile << padding << "P: " << node->particle->id << endl;
//     } else {
//         outfile << padding << "Int" << endl;
//     }
// }

// void printQuadTree(ofstream& outfile, const unique_ptr<Node>& node, int depth = 0) {
//     if (!node) return;
//     printNode(outfile, node, depth);
//     if (!node->isLeaf) {
//         for (const auto& child : node->children) {
//             printQuadTree(outfile, child, depth + 1);
//         }
//     }
// }

int main(int argc, char** argv){

    //Command Flags
    //"-r" change the image size, "-n" change the number of particles, "-t" change the unitTime
    for (int i = 1; i < argc; i++) {
        if (i < argc - 1) {
            if (strcmp(argv[i], "-r") == 0) range = stoi(argv[i + 1]);
            else if (strcmp(argv[i], "-n") == 0) particleNum= stof(argv[i + 1]);
            else if (strcmp(argv[i], "-t") == 0) unitTime = stoi(argv[i + 1]);
        }
    }

    //Start the program
    cout << "...............................Starting............................" << endl;
    cout << "Particle Number: " << particleNum << "  " << "Iteration times: " << "10 " << "Time unit: " << unitTime << endl;

    //generate particles
    vector<shared_ptr<Particle>> particles = generateParticles(particleNum);

    //make duplicates to ensure the sequential and parallel approaches have the same results
    vector<shared_ptr<Particle>> duplicates;
    for(auto& p:particles) duplicates.push_back(make_shared<Particle>(*p));

    //Sequential
    {
        cout << "---------------Sequential Simulation Starts---------------" << endl;
        Timer SeqTimer;
        for(size_t i = 0; i < 20; i++){
            //1. build and trim the quadtree 
            Timer treeBuildTimer;
            Bitmap image(range, range);
            QuadTree qt1(Point(0, 0), range);
            qt1.buildQuadTree_seq(particles);
            int64_t treeBuildTime= treeBuildTimer.dur_ms();

            //2. calculate the gravity and simulate next postion of each particle
            Timer simulationTimer;
            vector<shared_ptr<Particle>> newParticles = updateGenerateNew(qt1, particles);
            int64_t simulationTime= simulationTimer.dur_ms();
            particles = newParticles;

            //Mark out two particles to observe their movements
            for (const auto& particle : particles) {
                if(particle->id == 1 || particle->id == 10) image.drawBigDot(particle->position.x, particle->position.y, 15, 255, 0, 0);
                //image.setPixel(particle->position.x, particle->position.y, 255, 255, 255);
                image.drawBigDot(particle->position.x, particle->position.y, 5, 255, 255, 255);//make the dots bigger for observation
            }

            // Print image and information
            string filename = "seq" + to_string(i+1) + ".bmp";
            image.save(filename);
            cout << "Iteration " << i+1  << ": " << endl;
            cout << "QuadTree Build Time: " << treeBuildTime << "ms, " << "Simulation Time: " <<  simulationTime << "ms." << endl;
        }
        cout << "Sequential Overall Time: " << SeqTimer.dur_ms() << "ms." << endl;
        cout << "---------------Sequential Simulation Ends---------------" << endl;
    }
    //Parallel
    {
        cout << "---------------Parallel Simulation Starts---------------" << endl;
        Timer ParallelTimer;
        vector<thread> threads(threadNum);
        for(size_t i = 0; i < 20; i++){
            Timer treeBuildTimer;
            Bitmap image(range, range);

            //Tree Build
            QuadTree qt2(Point(0, 0), range);
            for(size_t i = 1; i <= threadNum; i++){
                threads[i-1] = thread(&QuadTree::buildQuadTree_Parallel, ref(qt2), ref(duplicates), (i-1)* particleNum/threadNum, i*particleNum/threadNum);
            }
            for(auto& thread: threads) thread.join();

            //Tree Trim
            for(size_t i = 0; i < threadNum; i++){
                threads[i] = thread(quadTreeTrim, ref(qt2.root->children[i]));
            }
            for(auto& thread: threads) thread.join();
            int64_t treeBuildTime= treeBuildTimer.dur_ms();
            

            //Simulate
            Timer simulationTimer;
            vector<shared_ptr<Particle>> newParticles(particleNum);
            for(size_t i = 1; i <= threadNum; i++){
                threads[i-1] =  thread(updateGenerateNew_parallel, ref(qt2), ref(duplicates), ref(newParticles), (i-1)* particleNum/threadNum, i * particleNum/threadNum);
            }
            for(auto& thread: threads) thread.join();
            int64_t simulationTime= simulationTimer.dur_ms();

            duplicates = newParticles;
            //write to image
            for (const auto& particle : duplicates) {
                if(particle->id == 1 || particle->id == 10) image.drawBigDot(particle->position.x, particle->position.y, 15, 255, 0, 0);
                image.drawBigDot(particle->position.x, particle->position.y, 5, 255, 255, 255);
            }
            string filename = "parallel" + to_string(i+1) + ".bmp";
            image.save(filename);
            cout << "Iteration " << i+1  << ": " << endl;
            cout << "QuadTree Build Time: " << treeBuildTime << "ms, " << "Simulation Time: " <<  simulationTime << "ms." << endl;
        }
        cout << "Parallel Overall Time: " << ParallelTimer.dur_ms() << "ms." << endl;
        cout << "---------------Parallel Simulation Ends---------------" << endl;
    }
    return 0;
}