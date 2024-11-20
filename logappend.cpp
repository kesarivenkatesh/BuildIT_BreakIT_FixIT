#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm> 
#include <unistd.h>

using namespace std;

bool isAlphabetic(const string& str) {
    for (char c : str) {
        if (!isalpha(c)) {
            return false;
        }
    }
    return true;
}

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

    int timestamp = -1;
    string token = "";
    string employeeName = "";
    string guestName = "";
    bool arrived = false;
    bool left = false;
    bool roomGiven = false;
    int roomNo = -1;
    string logfileName = "";

    while((opt = getopt(argc, argv, "T:K:E:G:ALR:")) != -1) {
        switch(opt) {
            case 'T': timestamp = stoi(optarg); break;
            case 'K': token = optarg; break;
            case 'E': employeeName = optarg; break;
            case 'G': guestName = optarg; break;
            case 'A': arrived = true; break;
            case 'L': left = true; break;
            case 'R': roomGiven = true; roomNo = stoi(optarg); break;
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
    
    // 1. check E and G both not present
    if(employeeName!="" && guestName!="") {invalid=true; errmsg = "both emp, guest name";}
    // 2. check A and L both not present
    else if(arrived && left) {invalid = true; errmsg = "A and L";}
    // 3. T > 0
    else if(timestamp <= 0) {invalid = true; errmsg = "T <= 0";}
    // 4. 0 <= R <= 1073741823
    else if( roomGiven && (roomNo < 0 || roomNo > 1073741823)) {invalid = true; errmsg = "room No";}
    // 5. check if token is valid
    else if(token != validToken) {invalid = true; errmsg = "Token invalid";}
    // 6. check if emp/guest name has only [a-zA-Z]
    else if(!isAlphabetic(employeeName!="" ? employeeName:guestName)) {invalid = true; errmsg = "name not chars";}

    if(invalid) {
        cerr << "Unsupported " << errmsg << endl;
        return 255;
    }

    // Now check timestamp is greater than recent one in log file(since time should only increase)
    string lastLine;
    string content;
    int line=0;

    file.open(logfileName);

    if (!file.is_open()) {
        cerr << "Error opening filee: " << logfileName << endl;
        return 255;
    }

    while(getline(file, content)) {
        line++;
        lastLine = content;
    }
    file.close();

    if(line>1) {
        vector<string> lastLineSplit = splitString(lastLine);
        int recenttimestamp = stoi(lastLineSplit[0]);
        if(timestamp <= recenttimestamp) {
            cerr << "Timestamp is not valid" << endl;
            return 255;
        }
    }
    
    // Good, we append after checking
    // 1. If already Arrived, no need to arrive
    // 2. If Arrived in gallery, then only enter room
    // 3. If entering room, then check if he is already in other room

    // For this we need to read all contents of log file
    vector<vector<string> > gallery;
    vector<vector<string> > room;


    content = "";

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

    // now trying to append
    string arrivedOrLeft = arrived ? "A" : "L";
    string name = guestName != "" ? guestName : employeeName;
    string guest = guestName != "" ? "1" : "0";
    string lineToAppend = to_string(timestamp) + " " + arrivedOrLeft + " " + name + " " + to_string(roomNo) + " " + guest;

    ofstream outfile(logfileName, ios::app);

    if (!outfile.is_open()) {
        cerr << "Error opening file: " << logfileName << endl;
        return 1;
    }

    // Arrival cases
    if(arrivedOrLeft == "A") {
        // Room No is not given, then add to gallery if not already in the gallery and in any of the rooms
        if(roomNo==-1) {
            for(vector<string> t: gallery) {
                if(t[0]==name && t[1]==guest) {
                    cerr << "Invalid: "+ name +" Already in Gallery" << endl;
                    return 255;
                }
            }
            for(vector<string> t: room) {
                if(t[0]==name && t[1]==guest) {
                    cerr << "Invalid: "+ name +" Already in a Room" << endl;
                    return 255;
                }
            }
            gallery.push_back({name, guest});
        }
        // Room No is given, then
        // * check if he is in gallery
        //      -> if yes, then add to room and remove from gallery
        //      -> if no, then 2 cases 1. Not arrived yet 2. Already in the room
        else if(roomNo!=-1) {
            bool inGallery = false;
            for(int i=0; i<gallery.size(); i++) {
                if(gallery[i][0]==name && gallery[i][1]==guest) {
                    inGallery = true;
                    break;
                }
            }
            if(!inGallery) {
                // Not in gallery(2 cases: must be in a room OR Not arrived yet)
                // check if in any room
                bool inAnyRoom = false;
                string presentRoomNo = "";
                for(int i=0; i<room.size(); i++) {
                    if(room[i][0]==name && room[i][1]==guest) {
                        inAnyRoom = true;
                        presentRoomNo = room[i][2];
                        break;
                    }
                }
                if(inAnyRoom) {
                    cerr << "Invalid: " << name + " need to leave room " + presentRoomNo + " first" << endl;
                } else {
                    cerr << "Invalid: " << name + " need to enter gallery first" << endl;
                }
                return 255;
            } else {
                // in the gallery, now can enter the room
                room.push_back({name, guest, to_string(roomNo)});
                // remove from gallery
                for(int i=0; i< gallery.size(); i++) {
                    if(gallery[i][0]==name && gallery[i][1]==guest) {
                        gallery.erase(gallery.begin()+i);
                        break;
                    }
                }
            }
        }
    }
    // Leaving cases
    else if(arrivedOrLeft == "L") {
        // Room No is not given, then check in gallery
        //   -> if present, remove from gallery
        //   -> if not present, 2 cases: 1. Already left OR 2. In a room
        if(roomNo==-1) {
            bool inGallery = false;
            for(int i=0; i< gallery.size(); i++) {
                if(gallery[i][0]==name && gallery[i][1]==guest) {
                    inGallery = true;
                    gallery.erase(gallery.begin()+i);
                    break;
                }
            }
            if(!inGallery) {
                // check in rooms
                bool inAnyRoom = false;
                for(int i=0; i< room.size(); i++) {
                    if(room[i][0]==name && room[i][1]==guest) {
                        inAnyRoom = true;
                        break;
                    }
                }
                if(inAnyRoom) {
                    cerr << "Invalid: In a room" << endl;
                } else {
                    cerr << "Invalid: Not in gallery or a room" << endl;
                }
                return 255;
            }
        }
        // Room No is given
        // * check rooms to find the guest
        //    -> if in given roomNo, remove from rooms and add to gallery
        //    -> if not, then return 255
        else {
            bool inGivenRoom = false;
            for(int i=0; i< room.size(); i++) {
                if(room[i][0]==name && room[i][1]==guest && room[i][2]==to_string(roomNo)) {
                    inGivenRoom = true;
                    room.erase(room.begin()+i);
                    break;
                }
            }
            if(inGivenRoom) {
                gallery.push_back({name, guest});
            } else {
                cerr << "Invalid: Not in given room" << endl;
                return 255;
            }
        }
    }
    

    outfile << lineToAppend << endl;

    outfile.close();

    // just printing
    cout << "Gallery(Name:Guest/Employee)" << endl;
    for(vector<string> t: gallery) {
        if(t[1]=="1") cout << t[0] << " Guest" << endl;
        else cout << t[0] << " Employee" << endl;
    }
    cout << "Rooms(Name:Guest/Employee:RoomNumber)"<< endl;
    for(vector<string> t: room) {
        if(t[1]=="1") cout << t[0] << " Guest " << t[2] << endl;
        else cout << t[0] << " Employee " << t[2] << endl;
    }
    return 0;
}