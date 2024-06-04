//Project Ident: AD48FB4835AF347EB0CA8009E24C3B13F8519882

#include <getopt.h>
#include "P2random.h"
#include <string>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <vector>
#include <queue>
#include <utility>

using namespace std;

struct Deployment {
    uint32_t fSense = 0;
    uint32_t ID = 0;
    uint32_t quantity = 0;
    uint32_t genID = 0;
    uint32_t planetID = 0;
};

struct CustomLess { // max pq puts the largest element to top using this
    bool operator() (const Deployment& a, Deployment& b) const { 
        if (a.fSense < b.fSense) {
            return true;
        }
        else if (a.fSense > b.fSense) {
            return false;
        }
        else {
            return a.ID > b.ID;
        }
    }
};

struct CustomGreater {
    bool operator() (const Deployment& a, Deployment& b) const {
        if (a.fSense > b.fSense) {
            return true;
        }
        else if (a.fSense < b.fSense) {
            return false;
        }
        else {
            return a.ID > b.ID;
        }
    }
};

struct General {
    uint32_t totJedi = 0;
    uint32_t totSith = 0;
    uint32_t totSurvived = 0;
};

enum class State { None, Ready, Fight };
// Sith, Sith attack, Jedi, Jedi weakest


struct Watcher { // When Sith was on the planet first

    State state = State::None;
    uint32_t jediTime = 0; //weakest
    uint32_t sithTime = 0; //strongest
    uint32_t maybeTime = 0;

    uint32_t jediForce = 0; //weakest
    uint32_t sithForce = 0; //strongest
    uint32_t maybeForce = 0;

};



class Planet {
    friend class Galaxy;
private:
    priority_queue<Deployment, vector<Deployment>, CustomLess> sithPQ; // max pq
    priority_queue<Deployment, vector<Deployment>, CustomGreater> jediPQ; // min pq
    Deployment sith;
    Deployment jedi;

    priority_queue<uint32_t, vector<uint32_t>> maxPQ;
    priority_queue<uint32_t, vector<uint32_t>, greater<uint32_t>> minPQ;

    Watcher attacker;
    Watcher ambusher;


    bool battle = false;

    

public:
    void battler(vector<General>& generals, uint32_t& totBattles, vector<bool>& modes) {
        if (sithPQ.empty() || jediPQ.empty()) {
            return;
        }
        sith = sithPQ.top();
        jedi = jediPQ.top();
        
        while (sith.fSense >= jedi.fSense) {
            sithPQ.pop();
            jediPQ.pop();
            if (sith.quantity > jedi.quantity) {
                sith.quantity -= jedi.quantity;
                outputHelper(generals, modes, jedi);
                jedi.quantity = 0;
                totBattles++;

            }
            else if (sith.quantity < jedi.quantity) {
                jedi.quantity -= sith.quantity;
                outputHelper(generals, modes, sith);
                sith.quantity = 0;
                totBattles++;
            }
            else {
                outputHelper(generals, modes, jedi);
                jedi.quantity = sith.quantity = 0;
                totBattles++;
            }
            if (sith.quantity > 0) {
                sithPQ.push(sith);
            }
            if (jedi.quantity > 0) {
                jediPQ.push(jedi);
            }
            battle = true;
            if (sithPQ.empty() || jediPQ.empty()) {
                return;
            }
            sith = sithPQ.top();
            jedi = jediPQ.top();
        }
    }

    void outputHelper(vector<General>& generals, vector<bool>& modes, Deployment& dep) {
        if (modes[0]) {
            generals[sith.genID].totSurvived -= dep.quantity;
            generals[jedi.genID].totSurvived -= dep.quantity;
        }
        if (modes[2]) {
            cout << "General " << sith.genID << "'s battalion attacked "
                << "General " << jedi.genID << "'s battalion on planet "
                << sith.planetID << ". " << dep.quantity * 2 << " troops were lost.\n";
        }
        if (modes[1]) {
            medianPQ(dep.quantity * 2);
        }
    }

    void medianPQ(uint32_t x) {
        if (minPQ.size() == maxPQ.size()) {
            if (maxPQ.size() == 0) {
                maxPQ.push(x);
                return;
            }
            if (x < maxPQ.top()) {
                maxPQ.push(x);
            }
            else {
                minPQ.push(x);
            }
        }
        else if (maxPQ.size() < minPQ.size()) {
            if (x <= minPQ.top()) {
                maxPQ.push(x);
            }
            else {
                uint32_t temp = minPQ.top();
                minPQ.pop();
                maxPQ.push(temp);
                minPQ.push(x);
            }
        }
        else {
            if (x >= maxPQ.top()) {
                minPQ.push(x);
            }
            else {
                uint32_t temp = maxPQ.top();
                maxPQ.pop();
                minPQ.push(temp);
                maxPQ.push(x);
            }
        }
    }

    uint32_t medianFinder() {
        if (minPQ.size() == maxPQ.size()) {
            return (minPQ.top() + maxPQ.top()) / 2;
        }
        else if (maxPQ.size() > minPQ.size()) {
            return maxPQ.top();
        }
        else {
            return minPQ.top();
        }
    }
};

class Galaxy {
private:
    vector<Planet> planets;
    vector<General> generals;
    vector<bool> modes; // stores bools for gmvw in this order
    uint32_t numGen;
    uint32_t numPlanet;
    uint32_t totBattles = 0;

    

public:
    Galaxy(uint32_t numPlanetIn, uint32_t numGenIn) {
        numGen = numGenIn;
        numPlanet = numPlanetIn;
        planets.resize(numPlanet);
        modes.resize(4, false);
    }

    void genResize() {
        generals.resize(numGen);
    }

    void setMode(char mode) {
        if (mode == 'g') {
            modes[0] = true;
            genResize();
        }
        else if (mode == 'm') {
            modes[1] = true;
        }
        else if (mode == 'v') {
            modes[2] = true;
        }
        else {
            modes[3] = true;
        }
    }

    void reader(string& inMode) {
        stringstream ss;
        string junkS;
        char junkC;

        if (inMode == "PR") {
            uint32_t seed = 0;
            uint32_t numDep = 0;
            uint32_t arrival = 0;
            cin >> junkS >> seed >> junkS 
                >> numDep >> junkS >> arrival;

            P2random::PR_init(ss, seed, numGen, numPlanet, numDep, arrival);
        }

        istream& inputStream = inMode == "PR" ? ss : cin;

        int timestamp = 0;
        int pTimestamp = 0;
        string side;
        int genID = 0;
        int planetID = 0;
        int fSense = 0;
        int quantity = 0;

        uint32_t ID = 0;

        cout << "Deploying troops...\n";
        while (inputStream >> timestamp >> side
            >> junkC >> genID >> junkC >> planetID
            >> junkC >> fSense >> junkC >> quantity) {

            if (timestamp < pTimestamp) {
                cerr << "Invalid decreasing timestamp" << endl;
                exit(1);
            }

            if (genID < 0 || genID >= static_cast<int>(numGen)) {
                cerr << "Invalid general ID" << endl;
                exit(1);
            }

            if (planetID < 0 || planetID >= static_cast<int>(numPlanet)) {
                cerr << "Invalid planet ID" << endl;
                exit(1);
            }

            if (fSense <= 0) {
                cerr << "Invalid force sensntivity level" << endl;
                exit(1);
            }

            if (quantity <= 0) {
                cerr << "Invalid number of troops" << endl;
                exit(1);
            }

            Deployment dep = { static_cast<uint32_t>(fSense), ID,
            static_cast<uint32_t>(quantity), static_cast<uint32_t>(genID),
            static_cast<uint32_t>(planetID) };
            ID++;

            if (modes[0]) {
                if (side == "SITH") {
                    planets[planetID].sithPQ.push(dep);
                    generals[genID].totSith += dep.quantity;
                }
                else {
                    planets[planetID].jediPQ.push(dep);
                    generals[genID].totJedi += dep.quantity;
                }

                generals[genID].totSurvived += dep.quantity;
            }
            else {
                if (side == "SITH") {
                    planets[planetID].sithPQ.push(dep);
                }
                else {
                    planets[planetID].jediPQ.push(dep);
                }
            }
            if (modes[1]) {
                outputHelper2(timestamp, pTimestamp);
            }
            planets[planetID].battler(generals, totBattles, modes);
            pTimestamp = timestamp;
            uint32_t fSenseUint = static_cast<uint32_t>(fSense);

            if (modes[3]) {
                movieWatcherAtt(planetID, side, fSenseUint, timestamp);
                movieWatcherAmb(planetID, side, fSenseUint, timestamp);
            }

        } // while
        if (modes[1]) {
            outputHelper2(timestamp+1, pTimestamp);
        }
        cout << "---End of Day---\n"
            << "Battles: " << totBattles << "\n";

        if (modes[0]) {
            genEvalOut();
        }

        if (modes[3]) {
            movieWatcherOut();
        }
    }

    void outputHelper2(uint32_t timestamp, uint32_t pTimestamp) {
        if (timestamp > pTimestamp) {
            uint32_t i = 0;
            for ( ; i < numPlanet; i++) {
                if (planets[i].battle == true) {
                    cout << "Median troops lost on planet " << i << " at time "
                        << pTimestamp << " is " << planets[i].medianFinder() << ".\n";
                }
            }
        }
    }

    void genEvalOut() {
        cout << "---General Evaluation---\n";
        for (uint32_t i = 0; i < generals.size(); i++) {
            cout << "General " << i << " deployed " << generals[i].totJedi << " Jedi troops and "
                << generals[i].totSith << " Sith troops, and " << generals[i].totSurvived << "/"
                << generals[i].totSith + generals[i].totJedi << " troops survived.\n";
        }
        
    }

    void movieWatcherAtt(uint32_t planetID, string& side, uint32_t fSenseUint, uint32_t timestamp) {
        if (planets[planetID].attacker.state == State::None && side == "SITH") { // Jedi wasn't seen yet
            // don't save the sith
        }
        else if (planets[planetID].attacker.state == State::None && side == "JEDI") {
            // ready for attack
            planets[planetID].attacker.state = State::Ready; // JEDI Seen
            planets[planetID].attacker.jediForce = fSenseUint; // save as weakest JEDI
            planets[planetID].attacker.jediTime = timestamp;
        }
        else if (planets[planetID].attacker.state == State::Ready && side == "SITH") {
            // attack possible if JEDI is weak enough
            if (planets[planetID].attacker.jediForce <= fSenseUint) {
                planets[planetID].attacker.state = State::Fight;
                planets[planetID].attacker.sithForce = fSenseUint;
                planets[planetID].attacker.sithTime = timestamp;
            }
        }
        else if (planets[planetID].attacker.state == State::Ready && side == "JEDI") {
            // save JEDI if weaker than the current one
            if (fSenseUint < planets[planetID].attacker.jediForce) {
                planets[planetID].attacker.jediForce = fSenseUint;
                planets[planetID].attacker.jediTime = timestamp;
            }
        }
        else if (planets[planetID].attacker.state == State::Fight && side == "SITH") {
            // save SITH if difference is more than current situation in the planet watcher
            if (planets[planetID].attacker.maybeForce != 0) {
                uint32_t curDiff = planets[planetID].attacker.sithForce - planets[planetID].attacker.jediForce;
                if (fSenseUint > planets[planetID].attacker.jediForce) {
                    uint32_t newDiff = fSenseUint - planets[planetID].attacker.jediForce;
                    if (newDiff > curDiff) {
                        planets[planetID].attacker.sithForce = fSenseUint;
                        planets[planetID].attacker.sithTime = timestamp;
                    }
                }
                curDiff = planets[planetID].attacker.sithForce - planets[planetID].attacker.jediForce;
                if (fSenseUint > planets[planetID].attacker.maybeForce) { //maybeForce is JEDI
                    uint32_t newDiff = fSenseUint - planets[planetID].attacker.maybeForce;
                    if (newDiff > curDiff) {
                        planets[planetID].attacker.jediForce = planets[planetID].attacker.maybeForce;
                        planets[planetID].attacker.jediTime = planets[planetID].attacker.maybeTime;
                        planets[planetID].attacker.sithForce = fSenseUint;
                        planets[planetID].attacker.sithTime = timestamp;
                    }
                }
            }
            else {
                if (fSenseUint > planets[planetID].attacker.sithForce) {
                    planets[planetID].attacker.sithForce = fSenseUint;
                    planets[planetID].attacker.sithTime = timestamp;
                }
            }
        }
        else if (planets[planetID].attacker.state == State::Fight && side == "JEDI") {
            // save JEDI regardless because the next SITH will determine what happens
            if (planets[planetID].attacker.maybeForce == 0) {
                planets[planetID].attacker.maybeForce = fSenseUint;
                planets[planetID].attacker.maybeTime = timestamp;
            }
            else if (fSenseUint < planets[planetID].attacker.maybeForce) {
                planets[planetID].attacker.maybeForce = fSenseUint;
                planets[planetID].attacker.maybeTime = timestamp;
            }
        }
    }

    void movieWatcherAmb(uint32_t planetID, string& side, uint32_t fSenseUint, uint32_t timestamp) {
        if (planets[planetID].ambusher.state == State::None && side == "JEDI") { // JEDI wasn't seen yet
            // don't save the JEDI
        }
        else if (planets[planetID].ambusher.state == State::None && side == "SITH") {
            // ready for ambush
            planets[planetID].ambusher.state = State::Ready; // SITH Seen
            planets[planetID].ambusher.sithForce = fSenseUint; // save as strongest SITH
            planets[planetID].ambusher.sithTime = timestamp;
        }
        else if (planets[planetID].ambusher.state == State::Ready && side == "JEDI") {
            // ambush possible if SITH is strong enough
            if (planets[planetID].ambusher.sithForce >= fSenseUint) {
                planets[planetID].ambusher.state = State::Fight;
                planets[planetID].ambusher.jediForce = fSenseUint;
                planets[planetID].ambusher.jediTime = timestamp;
            }
        }
        else if (planets[planetID].ambusher.state == State::Ready && side == "SITH") {
            // save SITH if stronger than the current one
            if (fSenseUint > planets[planetID].ambusher.sithForce) {
                planets[planetID].ambusher.sithForce = fSenseUint;
                planets[planetID].ambusher.sithTime = timestamp;
            }
        }
        else if (planets[planetID].ambusher.state == State::Fight && side == "JEDI") {
            // save JEDI if difference is more than current situation in the planet watcher
            if (planets[planetID].ambusher.maybeForce != 0) {
                uint32_t curDiff = planets[planetID].ambusher.sithForce - planets[planetID].ambusher.jediForce;
                if (fSenseUint < planets[planetID].ambusher.sithForce) {
                    uint32_t newDiff = planets[planetID].ambusher.sithForce - fSenseUint;
                    if (newDiff > curDiff) {
                        planets[planetID].ambusher.jediForce = fSenseUint;
                        planets[planetID].ambusher.jediTime = timestamp;
                    }
                }
                curDiff = planets[planetID].ambusher.sithForce - planets[planetID].ambusher.jediForce;
                if (fSenseUint < planets[planetID].ambusher.maybeForce) { // maybeForce is SITH
                    uint32_t newDiff = planets[planetID].ambusher.maybeForce - fSenseUint;
                    if (newDiff > curDiff) {
                        planets[planetID].ambusher.sithForce = planets[planetID].ambusher.maybeForce;
                        planets[planetID].ambusher.sithTime = planets[planetID].ambusher.maybeTime;
                        planets[planetID].ambusher.jediForce = fSenseUint;
                        planets[planetID].ambusher.jediTime = timestamp;
                    }
                }
            }
            else {
                if (fSenseUint < planets[planetID].ambusher.jediForce) {
                    planets[planetID].ambusher.jediForce = fSenseUint;
                    planets[planetID].ambusher.jediTime = timestamp;
                }
            }
        }
        else if (planets[planetID].ambusher.state == State::Fight && side == "SITH") {
            // save SITH regardless because the next Jedi will determine what happens
            // save JEDI regardless because the next SITH will determine what happens
            if (planets[planetID].ambusher.maybeForce == 0) {
                planets[planetID].ambusher.maybeForce = fSenseUint;
                planets[planetID].ambusher.maybeTime = timestamp;
            }
            else if (fSenseUint > planets[planetID].ambusher.maybeForce) {
                planets[planetID].ambusher.maybeForce = fSenseUint;
                planets[planetID].ambusher.maybeTime = timestamp;
            }
            
        }
    }

    void movieWatcherOut() {
        cout << "---Movie Watcher---\n";
        for (uint32_t i = 0; i < planets.size(); i++) {
            if (planets[i].ambusher.state == State::Fight) {
                cout << "A movie watcher would enjoy an ambush on planet " 
                    << i << " with Sith at time " << planets[i].ambusher.sithTime 
                    << " and Jedi at time " << planets[i].ambusher.jediTime 
                    << " with a force difference of " << planets[i].ambusher.sithForce 
                    - planets[i].ambusher.jediForce << ".\n";
            }
            else {
                cout << "A movie watcher would not see an interesting ambush on planet "
                    << i << ".\n";
            }

            if (planets[i].attacker.state == State::Fight) {
                cout << "A movie watcher would enjoy an attack on planet "
                    << i << " with Jedi at time " << planets[i].attacker.jediTime
                    << " and Sith at time " << planets[i].attacker.sithTime
                    << " with a force difference of " << planets[i].attacker.sithForce
                    - planets[i].attacker.jediForce << ".\n";
            }
            else {
                cout << "A movie watcher would not see an interesting attack on planet "
                    << i << ".\n";
            }
        }
    }
};

void getMode(int argc, char* argv[], Galaxy& galaxy) {
    // These are used with getopt_long()
    opterr = false; // Let us handle all error output for command line options
    int choice;
    int index = 0;
    struct option long_options[] = {
        // Fill in two lines, for the "mode" ('m') and
        // the "help" ('h') options.
        // ./project0 -m nosize
        // ./project0 --help
        { "verbose",        no_argument, nullptr, 'v'  },
        { "median",         no_argument, nullptr, 'm'  },
        { "general-eval",   no_argument, nullptr, 'g'  },
        { "watcher",        no_argument, nullptr, 'w'  },
        { nullptr,          0,           nullptr, '\0' },
    };  // long_options[]

    // Fill in the double quotes, to match the mode and help options.
    while ((choice = getopt_long(argc, argv, "vmgw", long_options, &index)) != -1) {
        switch (choice) {

        case 'v':
            // verbose output
            galaxy.setMode('v');
            break;

        case 'm':
            // median output
            galaxy.setMode('m');
            break;

        case 'g':
            // general output
            galaxy.setMode('g');
            break;

        case 'w': 
            // watcher output
            galaxy.setMode('w');
            break;
        }  // switch ..choice
    }  // while
}  // getMode()



int main(int argc, char* argv[]) {
    std::ios_base::sync_with_stdio(false);
    string junk;
    string inMode;
    uint32_t numGen = 0;
    uint32_t numPlanet = 0;
    getline(cin, junk);
    cin >> junk >> inMode >> junk 
        >> numGen >> junk >> numPlanet;

    Galaxy galaxy(numPlanet, numGen);
    getMode(argc, argv, galaxy);
    galaxy.reader(inMode);
}