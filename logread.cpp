#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm> 
#include <unordered_map>
#include <unistd.h>

using namespace std;

vector<string> splitString(const string& str, char delimiter = ' ') {
    vector<string> tokens;
    stringstream ss(str);
    string token;

    while (getline(ss, token, delimiter)) {
        tokens.push_back(token);
    }

    return tokens;
}

int main(int argc, char* argv[]) {
    // take cmd line args
    int opt;

    string token = "";
    string employeeName = "";
    string guestName = "";
    string logfileName = "";
    bool state = false;
    bool allRoomsByGE = false;

    while((opt = getopt(argc, argv, "K:SRE:G:T:I:")) != -1) {
        switch(opt) {
            case 'K': token = optarg; break;
            case 'S': state = true; break;
            case 'R': allRoomsByGE = true; break;
            case 'E': employeeName = optarg; break;
            case 'G': guestName = optarg; break;
            case 'T':
            case 'I':
            default: {
                cerr << "unsupported" << endl;
                return 255;
            }
        }
    }

    if(optind < argc) {
        logfileName = argv[argc-1];
    }

    // get valid token from log file(1st line)
    string validToken = "";
    ifstream file(logfileName);

    if (!file.is_open()) {
        cerr << "Error opening file: " << logfileName << endl;
        return 255;
    }

    getline(file, validToken);
    file.close();

    // valid or not
    bool invalid = false;
    string errmsg = "";

    if(token != validToken) {invalid = true; errmsg = "Token error";}
    if(invalid) {
        cerr << "Invalid: " << errmsg << endl;
        return 255;
    }


    // For this we need to read all contents of log file
    vector<vector<string> > gallery;
    vector<vector<string> > room;


    string content = "";

    string allRoomsVisited = "";
    string nameToCheck = "";
    if(employeeName!="") {
        nameToCheck = employeeName;
    } else {
        nameToCheck = guestName;
    }

    file.open(logfileName);

    if (!file.is_open()) {
        cerr << "Error opening fileee: " << logfileName << endl;
        return 255;
    }

    getline(file, content);
    while(getline(file, content)) {
        vector<string> contentSplit = splitString(content);
        string gOre = contentSplit[4];
        string roomNo = contentSplit[3];
        string name = contentSplit[2];
        string aOrl = contentSplit[1];

        vector<string> temp;
        temp.push_back(name);
        temp.push_back(gOre);
        if(roomNo == "-1") {
            // Gallery
            if(aOrl == "A") {
                // add to gallery
                gallery.push_back(temp);
            } else {
                // remove from gallery
                for(int i=0; i< gallery.size(); i++) {
                    if(gallery[i][0]==temp[0] && gallery[i][1]==temp[1]) {
                        gallery.erase(gallery.begin()+i);
                        break;
                    }
                }
            }
        } else {
            temp.push_back(roomNo);
            // Room
            if(aOrl == "A") {
                // add to room
                room.push_back(temp);
                if(nameToCheck==name) {
                    allRoomsVisited += roomNo+",";
                }
                // remove from gallery
                for(int i=0; i< gallery.size(); i++) {
                    if(gallery[i][0]==temp[0] && gallery[i][1]==temp[1]) {
                        gallery.erase(gallery.begin()+i);
                        break;
                    }
                }
            } else {
                // remove from room
                for(int i=0; i< room.size(); i++) {
                    if(room[i][0]==temp[0] && room[i][1]==temp[1] && room[i][2]==temp[2]) {
                        room.erase(room.begin()+i);
                        break;
                    }
                }
                // add to gallery
                gallery.push_back(temp);
            }
        }
    }
    file.close();


    if(state) {
        // cout << "Gallery(Name:Guest/Employee)" << endl;
        string guests = "";
        string employees = "";
        for(vector<string> t: gallery) {
            if(t[1]=="1") guests += t[0] + ",";
            else employees += t[0] + ",";
        }
        if(employees=="" && guests=="") cout << "No one in gallery\n";
        else cout << employees << endl << guests << endl;

        // cout << "Rooms(Name:Guest/Employee:RoomNumber)"<< endl;
        unordered_map<string, vector<string> > m; 
        for(vector<string> t: room) {
            if(m.find(t[2]) != m.end()) {
                m[t[2]].push_back(t[0]);
            } else {
                m[t[2]] = {t[0]};
            }
        }

        for(const auto& pair: m) {
            cout << pair.first << " : ";
            for(string t: pair.second) {
                cout << t << ",";
            }
            cout << endl;
        }
    } else if(allRoomsByGE) {
        cout << allRoomsVisited << endl;
    }
}