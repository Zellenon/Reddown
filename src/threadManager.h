//
//  threadManager.h
//  Reddown
//
//  Created by Zelle Mandez on 10/7/17.
//
//

#ifndef threadManager_h
#define threadManager_h
#include <regex>

#define RECORDLENGTH 300
#define SEARCHSIZE 150

struct order {
    int type; //SELECT = 0, LIST = 1, UPDATE = 2, RELOAD = 3, PURGE = 4, UPDATE SPECIFIC = 5
    string arg1;
    int arg2;
};

class threadManager : public ofThread {
public:
    //--------------------------------------------------------------
    void setup() {
        string currentUser = getenv("USER");
        resourcePath = "/Users/" + currentUser + "/Documents/Reddown/";
        subListPath = resourcePath + "subreddits.txt";
        recordsPath = resourcePath + "records/";
        cachePath = resourcePath + ".cache/";
        ofDirectory resourceBin1(resourcePath); if(!resourceBin1.exists()){ resourceBin1.create(true); }
        ofDirectory recordsBin1(recordsPath); if(!recordsBin1.exists()){ recordsBin1.create(true); }
        ofDirectory picsBin1(resourcePath+"pics/"); if(!picsBin1.exists()){ picsBin1.create(true); }
        ofDirectory animBin1(resourcePath+"animated/"); if(!animBin1.exists()){ animBin1.create(true); }
        ofDirectory keeperBin1(resourcePath+"keepers/"); if(!keeperBin1.exists()){ keeperBin1.create(true); }
        ofDirectory trashBin1(resourcePath+"tossers/"); if(!trashBin1.exists()){ trashBin1.create(true); }
        ofDirectory cacheBin1(cachePath); if(cacheBin1.exists()){ cacheBin1.remove(true); }
        cacheBin1.create(true);
        
        resourceBin = resourceBin1;
        recordsBin = recordsBin1;
        picsBin = picsBin1;
        animBin = animBin1;
        keeperBin = keeperBin1;
        trashBin = trashBin1;
        cacheBin = cacheBin1;
        
        if (!ofFile::doesFileExist(subListPath)) {
            ofFile file;
            file.open(subListPath, ofFile::Append);
            file << "*Put a list of subreddits here, on different lines!" << endl;
            file << "*Anything with an asterisk will be ignored." << endl;
            file << "cats" << endl << "dogs" << endl << "awww" << endl;
            file.close();
        }
        
        //--------------------------------------------------------------Truncate Records
        ofBuffer subbuffer = ofBufferFromFile(subListPath);
        for (ofBuffer::Line it = subbuffer.getLines().begin(), end = subbuffer.getLines().end(); it != end; ++it) {
            string sub = *it;
            if(sub.empty() == false) {
                if (ofFile::doesFileExist(recordsPath+sub+".txt")) {
                    int number_of_lines = 0;
                    std::string line;
                    std::ifstream myfile(recordsPath+sub+".txt");
                    
                    while (std::getline(myfile, line))
                        ++number_of_lines;
                    //cout << number_of_lines << endl;
                    if (number_of_lines > RECORDLENGTH) {
                        int diff = number_of_lines - RECORDLENGTH;
                        ofBuffer postbuffer = ofBufferFromFile(recordsPath+sub+".txt");
                        ofBuffer replacementBuffer = *new ofBuffer();
                        int i = 0;
                        for (ofBuffer::Line it2 = postbuffer.getLines().begin(), end2 = postbuffer.getLines().end(); it2 != end2; ++it2, ++i) { //find out what an acceptable size for this is and fix it
                            string filler = *it2;
                            if (!filler.empty())
                                filler += "\n";
                            if (i > diff)
                                replacementBuffer.append(filler);
                        }
                        ofBufferToFile(recordsPath+sub+".txt", replacementBuffer);
                    }
                }
            }
        }
        
        ImProc = new processor();
        ImProc->setup();
        ImProc->path = picsBin.getAbsolutePath() + "/";
        ImProc->cachePath = cachePath;
        ImProc->startThread();
        
        VidProc = new processor();
        VidProc->setup();
        VidProc->path = animBin.getAbsolutePath() + "/";
        VidProc->cachePath = cachePath;
        VidProc->startThread();
        
        Sorter = new urlSorter();
        Sorter->ImProc = ImProc;
        Sorter->VidProc = VidProc;
        Sorter->setup();
        Sorter->startThread();
        
        //DonSorter = new donmaiUrlSorter();
    }
    
    //--------------------------------------------------------------
    void threadedFunction(){
        while (isThreadRunning()){
            //int load = ImProc->QueuedPosts.size() + VidProc->QueuedPosts.size() + Sorter->QueuedUrls.size();
            int load = 0;
            if (!QueuedOrders.empty() && load < 250) {
                processNext();
            } else {
                //stopThread();
                sleep(1000);
            }
            if (!ImProc->isThreadRunning() && ImProc->QueuedPosts.size() > 0) ImProc->startThread();
            if (!VidProc->isThreadRunning() && VidProc->QueuedPosts.size() > 0) VidProc->startThread();
            //if (!Sorter->isThreadRunning() && Sorter->QueuedPosts.size() > 0) Sorter->startThread();
        }
    }
    
    //--------------------------------------------------------------
    void processNext() {
        mutex.lock();
        order temp = QueuedOrders.front();
        switch (temp.type) {
            case 0:
                redditS(temp.arg1, temp.arg2);
                break;
            case 1:
                redditL(temp.arg2);
                break;
            case 2:
                redditU();
                break;
            case 3:
                redditR();
                break;
            case 4:
                redditP(temp.arg1);
                break;
            case 5:
                redditUS(temp.arg1);
                break;
            case 6:
                donmai(temp.arg1, temp.arg2);
                break;
        }
        QueuedOrders.pop();
        mutex.unlock();
    }
    
    //--------------------------------------------------------------
    void add(int type, string arg_1, int arg_2) {
        //mutex.lock(); //YEAH WE NEED TO FIX THIS
        order temp{type,arg_1,arg_2};
        QueuedOrders.push(temp);
        //mutex.unlock();
    }
    
    //--------------------------------------------------------------
    void stop()
    {
        ImProc->stop();
        VidProc->stop();
        Sorter->stop();
        stopThread();
    }
    
    //--------------------------------------------------------------
    void redditS(string sub, int posts) {
        Sorter->lockdown = true;
        bool parsingSuccessful = json.open("https://www.reddit.com/r/" + sub + "/.json?limit=" + ofToString(posts));
        
        if (parsingSuccessful) {
            for (Json::ArrayIndex i = 0; i < json["data"]["children"].size(); ++i) {
                string url = json["data"]["children"][i]["data"]["url"].asString();
                Sorter->add(recordsPath+sub+".txt",url,i);
            }
        } else {
            ofLogNotice("ofApp::setup") << "Failed to parse JSON.";
        }
        Sorter->lockdown = false;
    }
    
    //--------------------------------------------------------------
    void redditL(int posts) {
        //Sorter->lockdown = true;
        ofBuffer buffer = ofBufferFromFile(subListPath);
        ofBuffer::Lines lines = buffer.getLines();
        for (ofBuffer::Line it = buffer.getLines().begin(), end = buffer.getLines().end(); it != end; ++it) {
            string sub = *it;
            if(sub.empty() == false && !(sub.find("*") != std::string::npos)) {
                cout << sub << endl;
                bool parsingSuccessful = json.open("https://www.reddit.com/r/" + sub + "/.json?limit=" + ofToString(posts));
                if (parsingSuccessful) {
                    for (Json::ArrayIndex i = 0; i < json["data"]["children"].size(); ++i) {
                        string url = json["data"]["children"][i]["data"]["url"].asString();
                        Sorter->add(recordsPath+sub+".txt",url,i);
                    }
                } else {
                    ofLogNotice("ofApp::setup") << "Failed to parse JSON.";
                }
            }
        }
        Sorter->lockdown = false;
    }
    
    //--------------------------------------------------------------
    void redditU() {
        cout << "UPDATING";
        //Sorter->lockdown = true;
        ofBuffer subbuffer = ofBufferFromFile(subListPath);
        for (ofBuffer::Line it = subbuffer.getLines().begin(), end = subbuffer.getLines().end(); it != end; ++it) {
            //while (Sorter->QueuedUrls.size() > 250)// Move These two lines to the threadedFunction function after breaking this into redditUS
            //    sleep(5000);// Move these two lines to the threadedFunction function after breaking this into redditUS
            string sub = *it;
            if(sub.empty() == false && !(sub.find("*") != std::string::npos)) {
                cout << "Queuing Update Order: " << sub << endl;
                add(5,sub,0);
            } else {
                cout << "Asterisk found" << endl;
            }
            //sleep(10); // Move this to the threadedFunction function after breaking this into redditUS
        }
        Sorter->lockdown = false;
    }
    
    //--------------------------------------------------------------
    void redditUS(string sub) {
        cout << sub << endl;
        bool parsingSuccessful = json.open("https://www.reddit.com/r/" + sub + "/.json?limit=" + ofToString(SEARCHSIZE));
        cout << "JSon obtained" << endl;
        
        
        bool checkable = ofFile::doesFileExist(recordsPath+sub+".txt");
        if (ofFile::doesFileExist(recordsPath+sub+".txt")) {
            ofBuffer postbuffer = ofBufferFromFile(recordsPath+sub+".txt");
            checkable = true;
        }
        cout << "Record file found: " << checkable;
        if (parsingSuccessful) {
            ofBuffer postbuffer;
            for (Json::ArrayIndex i = 0; i < json["data"]["children"].size(); ++i) {
                string url = json["data"]["children"][i]["data"]["url"].asString();
                if (checkable) {
                    postbuffer = ofBufferFromFile(recordsPath+sub+".txt");
                    bool found = false;
                    for (ofBuffer::Line it2 = postbuffer.getLines().begin(), end2 = postbuffer.getLines().end(); it2 != end2; ++it2) {
                        string toCheckAgainst(*it2);
                        if (url.compare(toCheckAgainst) == 0) {
                            found = true;
                            break;
                        }
                    }
                    if (!found) {
                        Sorter->add(recordsPath+sub+".txt",url,i);
                    }
                } else {
                    Sorter->add(recordsPath+sub+".txt",url,i);
                }
            }
        } else {
            ofLogNotice("ofApp::setup") << "Failed to parse JSON.";
        }
    }
    //--------------------------------------------------------------
    // http://danbooru.donmai.us/posts.json?page=1&tags=cats+rating%3Asafe
    // http://danbooru.donmai.us/posts.json?tags=cats+long_hair+rating%3Asafe.json+status%3Adeleted&ms=1
    void donmai(string tagString, int pages) {
        vector<string> tags = tagStringToArray(tagString);
        pages = pages / 20;
        //bool parsingSuccessful = json.open("https://www.reddit.com/r/" + sub + "/.json?limit=" + ofToString(SEARCHSIZE));
        for (int j=0; j<pages; j++) {
            cout << "Page " << j << endl;
            bool parsingSuccessful;
            if (tags.size() == 1)
                parsingSuccessful = json.open("http://danbooru.donmai.us/posts.json?page=" + ofToString(j) + "&tags=" + tags[0]);
            else
                parsingSuccessful = json.open("http://danbooru.donmai.us/posts.json?page=" + ofToString(j) + "&tags=" + tags[0] + "+" + tags[1]);
            //                      TODO: Implement Recordkeeping
            cout << "Parsing: " << parsingSuccessful << endl;
            if (parsingSuccessful) {
                for (Json::ArrayIndex i = 0; i < json.size(); ++i) {
                    vector<string> postTags = tagStringToArray(json[i]["tag_string"].asString());
                    if (hasStrings(tags,postTags)) {
                        string url = "http://danbooru.donmai.us" + json[i]["file_url"].asString();
                        //DonSorter->add(url,i);
                        Sorter->add(recordsPath+"donmai.txt",url,i);
                    }
                }
            }
        }
    }
    //--------------------------------------------------------------
    vector<string> tagStringToArray(string tagString) {
        vector<string> v;
        //boost::split(v, tagString, ::isspace);
        v = split(tagString,' ');
        return v;
    }
    
    bool hasStrings(vector<string> requirements, vector<string> pool) {
        for (int i = 0; i < requirements.size(); i++) {
            bool found = false;
            for (int j = 0; j < pool.size(); j++) {
                if (requirements[i] == pool[j])
                    found = true;
            }
            if (!found)
                return false;
        }
        return true;
    }
    
    template<typename Out>
    void split(const std::string &s, char delim, Out result) {
        std::stringstream ss(s);
        std::string item;
        while (std::getline(ss, item, delim)) {
            *(result++) = item;
        }
    }
    
    std::vector<std::string> split(const std::string &s, char delim) {
        std::vector<std::string> elems;
        split(s, delim, std::back_inserter(elems));
        return elems;
    }
    //--------------------------------------------------------------
    void redditR() {
        
    }
    
    //--------------------------------------------------------------
    void redditP(string phrase) {
        
    }
    
    queue<order> QueuedOrders;
    processor* ImProc;
    processor* VidProc;
    urlSorter* Sorter;
    
    string resourcePath;
    string subListPath;
    string recordsPath;
    string cachePath;
    vector<string> subNames;
    ofDirectory recordsBin;
    ofDirectory resourceBin;
    ofDirectory picsBin;
    ofDirectory animBin;
    ofDirectory cacheBin;
    ofDirectory keeperBin;
    ofDirectory trashBin;
    
    ofxJSONElement json;
};


#endif /* threadManager_h */





/*void redditU() {     //---------------- OLD REDDIT UPDATE FUNCTION
 //Sorter->lockdown = true;
 ofBuffer subbuffer = ofBufferFromFile(subListPath);
 for (ofBuffer::Line it = subbuffer.getLines().begin(), end = subbuffer.getLines().end(); it != end; ++it) {
 //while (Sorter->QueuedUrls.size() > 250)// Move These two lines to the threadedFunction function after breaking this into redditUS
 //    sleep(5000);// Move these two lines to the threadedFunction function after breaking this into redditUS
 string sub = *it;
 if(sub.empty() == false && !(sub.find("*") != std::string::npos)) {
 cout << sub << endl;
 bool parsingSuccessful = json.open("https://www.reddit.com/r/" + sub + "/.json?limit=70");
 
 bool checkable = ofFile::doesFileExist(recordsPath+sub+".txt");
 if (ofFile::doesFileExist(recordsPath+sub+".txt")) {
 ofBuffer postbuffer = ofBufferFromFile(recordsPath+sub+".txt");
 checkable = true;
 }
 cout << "Record file found: " << checkable;
 if (parsingSuccessful) {
 ofBuffer postbuffer;
 for (Json::ArrayIndex i = 0; i < json["data"]["children"].size(); ++i) {
 string url = json["data"]["children"][i]["data"]["url"].asString();
 if (checkable) {
 postbuffer = ofBufferFromFile(recordsPath+sub+".txt");
 bool found = false;
 for (ofBuffer::Line it2 = postbuffer.getLines().begin(), end2 = postbuffer.getLines().end(); it2 != end2; ++it2) {
 string toCheckAgainst(*it2);
 if (url.compare(toCheckAgainst) == 0) {
 found = true;
 break;
 }
 }
 if (!found) {
 Sorter->add(recordsPath+sub+".txt",url,i);
 }
 } else {
 Sorter->add(recordsPath+sub+".txt",url,i);
 }
 }
 } else {
 ofLogNotice("ofApp::setup") << "Failed to parse JSON.";
 }
 } else cout << "Asterisk found" << endl;
 //sleep(10); // Move this to the threadedFunction function after breaking this into redditUS
 }
 Sorter->lockdown = false;
 }*/
