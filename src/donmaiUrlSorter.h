//
//  donmaiUrlSorter.h
//  Reddown
//
//  Created by Zelle Mandez on 1/8/18.
//
//


/*API DOC Mini:
* http://danbooru.donmai.us/posts.json?page=1&tags=cats+rating%3Asafe
*/
#ifndef donmaiUrlSorter_h
#define donmaiUrlSorter_h

class donmaiUrlSorter: public ofThread{
public:
    
    void setup(){
        cout << "Started!" << endl;
        file = regex("\\w+\\.(gif|jpg|jpeg|png|mp4)");
        gfv = regex("\\w+\\.(gifv)");
        extless = regex("imgur\\.com/[a-zA-Z]+");
        http = regex("https:");
        animated = regex("\\w+\\.(gif|mp4)");
        
        initialized = true;
        lockdown = false;
    }
    
    void threadedFunction(){
        while (isThreadRunning()){
            if (!QueuedUrls.empty() && !lockdown) {
                if (ImProc->QueuedPosts.size() < 200)
                    processNext();
                else
                    sleep(500);
            } else {
                //stopThread();
                sleep(2000);
            }
        }
    }
    
    void processNext() {
        lock();
        post temp = QueuedUrls.front();
        unlock();
        cout << "(Sorter): Processing file " << temp.docname << endl;
        process(temp.path,temp.url,ofToInt(temp.docname));
        cout << "(Sorter): Completed Processing" << temp.docname << endl;
        lock();
        QueuedUrls.pop();
        unlock();
    }
    
    void add(string url, int name) {
        mutex.lock();
        post temp{"donmai", url, to_string(name)};
        QueuedUrls.push(temp);
        cout << "Queued " << url << endl;
        mutex.unlock();
    }
    
    void send(post toSend, bool animated) {
        //ImProc->add(temp);
        filenames.push_back(toSend.docname);
        if (animated) {
            VidProc->lock();
            VidProc->QueuedPosts.push(toSend);
            VidProc->unlock();
        } else {
            ImProc->lock();
            ImProc->QueuedPosts.push(toSend);
            ImProc->unlock();
        }
    }
    
    void stop()
    {
        stopThread();
    }
    
    void process(string path, string url, int id) {
        string original = url;
        cout << "(Sorter): Inside processing function" << endl;
        url = regex_replace(url,http,"https:");
        smatch m;
        smatch vidExt;
        string name;
        if (regex_search(url, m, file)) {             // Protocol for a Standard Image file, i.e. ending in a recognized image extension.
            name = m[0].str();
            if (regex_search(url,m,gfv)) url = url.substr(0,url.length()-1);
            cout << "Standard Image Found" << endl;
            
        } else if (regex_search(url, m, alb)) {       // Protocol for imgur album processing.
            cout << "Album Found " << m[2].str() << endl;
            ofHttpResponse resp = ofLoadURL("https://imgur.com/a/" + m[2].str() + "/layout/blog");
            smatch res;
            string str = resp.data;
            
            string::const_iterator searchStart( str.cbegin() );
            while ( regex_search( searchStart, str.cend(), res, exp ) )
            {
                //cout << ( searchStart == str.cbegin() ? "" : " " ) << res[1];
                searchStart += res.position() + res.length();
                //ImProc->add(url, name);
                smatch fileNameFinder;
                regex_search(url, fileNameFinder, file);
                name = fileNameFinder[0].str();
                url = res[1].str();
                post temp {path,url,name,""};
                bool vid = regex_search(url,vidExt,animated);
                send(temp,vid);
            }
            ofFile file;
            lock();
            file.open(path, ofFile::Append);
            file << original << endl;
            file.close();
            unlock();
            cout << endl;
            return;
            
        } else if (regex_search(url, m, extless)) {// Protocol for non-album imgur links without recognized image extensions.
            name = m[0].str() + ".jpg";
            url = url + ".jpg";
            cout << "Extensionless Imgur file Found" << endl;
            
        } else if (regex_search(url, m, gfy)) { // Protocol for Gfycat links. Downloads them as .mp4 files.
            cout << "Gfycat File Found " << m[1].str() << endl;
            bool parsingSuccessful = albjson.open("https://gfycat.com/cajax/get/" + m[1].str() + ".json");
            if (parsingSuccessful) {
                url = albjson["gfyItem"]["mp4Url"].asString();
                if (url.length() < 2)
                    url = "https://giant.gfycat.com/" + m[1].str() + ".mp4";
                name = m[1].str() + ".mp4";
            }
        } else {
            return;
        }
        post temp{path,url,name,original};
        bool vid = regex_search(url,vidExt,animated);
        send(temp,vid);
    }
    /*
    vector<string> tagStringToArray(string tagString) {
        vector<string> v;
        boost::split(v, tagString, ::isspace);
        return v;
    }*/
    
    regex file, gfv;
    regex exp;
    regex alb;
    regex gfy; //Finding gfycat links
    regex extless;
    regex http; //Used to change http to https
    regex animated; //Used to seperate still pictures and animated pics and video based on extension
    
    queue<post> QueuedUrls;
    vector<string> filenames;
    bool initialized = false;
    bool lockdown;
    ofxJSONElement albjson;
    processor* ImProc;
    processor* VidProc;
};

#endif /* donmaiUrlSorter_h */
