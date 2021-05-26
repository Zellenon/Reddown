//
//  processor.h
//  Reddown
//
//  Created by Zelle Mandez on 9/8/17.
//
//

#ifndef processor_h
#define processor_h
#include <regex>
#include <curl/curl.h>

struct post {
    string path;
    string url;
    string docname;
    string original;
};

class processor: public ofThread{
public:
    
    void setup(){ //-1 for random string
        cout << "Started!" << endl;
        temp = 0;
        f = &temp;
    }
    
    void threadedFunction(){
        while (isThreadRunning()){
            if (!QueuedPosts.empty()) {
                processNext();
            } else {
                cout << "ENDING url processing";
                stopThread();
                //sleep(2000);
            }
        }
    }
    
    void processNext() {
        lock();
        current = QueuedPosts.front();
        unlock();
        
        //curlDownload(current.url, current.docname, current.path);
        curlDownload(current.url, current.docname);
        if (current.original.size() > 4) {
            lock();
            file.open(current.path, ofFile::Append);
            file << current.original << endl;
            file.close();
            unlock();
        }
        
        
        lock();
        QueuedPosts.pop();
        unlock();
    }
    
    void add(post temp) {
        lock();
        cout << "(Processor): Image recieved!" << endl;
        //post temp{url,name};
        QueuedPosts.push(temp);
        unlock();
    }
    
    void stop()
    {
        stopThread();
    }
    
    static size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream)
    {
        size_t written = fwrite(ptr, size, nmemb, (FILE *)stream);
        return written;
    }
    
    int curlDownload(string url1, string name1/*, string path*/) {
        //mutex.lock();
        CURL *curl_handle;
        const char * urlc = url1.c_str();
        name1 = cachePath + name1;
        const char *pagefilename = name1.c_str();
        FILE *pagefile;
        
        /*if(argc < 2) {
         printf("Usage: %s <URL>\n", argv[0]);
         return 1;
         }*/
        
        curl_global_init(CURL_GLOBAL_ALL);
        
        // init the curl session
        curl_handle = curl_easy_init();
        
        // set URL to get here
        //cout << "Used URL: " << urlc << endl << "Used Filename: " << pagefilename << endl;
        curl_easy_setopt(curl_handle, CURLOPT_URL, urlc);
        
        // Switch on full protocol/debug output while testing
        curl_easy_setopt(curl_handle, CURLOPT_VERBOSE, 0L);
        
        // disable progress meter, set to 0L to enable and disable debug output
        curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 0L);
        
        // send all data to this function
        curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_data);
        
        
        // open the file
        pagefile = fopen(pagefilename, "wb");
        
        if(pagefile) {
            
            // write the page body to this file handle
            curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, pagefile);
            
            //curl_easy_setopt(curl_handle, CURLOPT_XFERINFOFUNCTION, &processor::progress_callback);
            //curl_easy_setopt(curl_handle, CURLOPT_PROGRESSFUNCTION, &processor::progress_callback);
            
            // get it!
            curl_easy_perform(curl_handle);
            
            // close the header file
            fclose(pagefile);
        }
        
        // cleanup curl stuff
        curl_easy_cleanup(curl_handle);
        
        image.open(name1, ofFile::ReadWrite);
        if (image.getSize() > 300)
            image.moveTo(path);
        else
            image.remove();
        image.close();
        //mutex.unlock();
        return 0;
    }
    
    int progress_callback(void *clientp,   curl_off_t dltotal,   curl_off_t dlnow,   curl_off_t ultotal,   curl_off_t ulnow) {
        cout << "dltotal " << dltotal << " dlnow " << dlnow << " ultotal " << ultotal << " ulnow " << ulnow << endl;
        //cout << "Percentage Complete: " << dlnow / dltotal << endl;
        return 0;
    }
    
    queue<post> QueuedPosts;
    double* f; // Don't know what this is for. Used by the PROGRESSDATA callback in curl_easy_setopt
    double temp;
    post current;
    ofFile file; //Used for the editing of records
    ofFile image;
    //static double progress;
    //string path = "/Users/Zelle/reddit/pics/";
    string path = "";
    string cachePath = "";
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//<    ><    ><    ><    ><    ><    ><    ><    ><    ><    ><    ><    ><    ><    ><    ><    ><    ><    ><    ><    ><    ><    ><    ><    ><    >
//<    ><    ><    ><    ><    ><    ><    ><    ><    ><    ><    ><    ><    ><    ><    ><    ><    ><    ><    ><    ><    ><    ><    ><    ><    >
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class urlSorter: public ofThread{
public:
    
    void setup(){
        cout << "Started!" << endl;
        file = regex("\\w+\\.(gif|jpg|jpeg|png|mp4)");
        gfv = regex("\\w+\\.(gifv)");
        exp = regex("<img src=\"//(i\\.imgur\\.com/[a-zA-Z0-9]+\\.[a-zA-Z]+)\" alt=\"\" itemprop=\"contentURL\" />");
        alb = regex("imgur\\.com/(a|gallery|album)/([0-9a-zA-Z]+)");
        gfy = regex("gfycat\\.com/([a-zA-Z]+)$");
        extless = regex("imgur\\.com/[a-zA-Z]+");
        http = regex("https:");
        animated = regex("\\w+\\.(gif|mp4)");
        
        initialized = true;
        lockdown = false;
    }
    
    void threadedFunction(){
        while (isThreadRunning()){
            if (!QueuedUrls.empty()) {
                if (!lockdown && ImProc->QueuedPosts.size() < 200)
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
    
    void add(string path, string url, int name) {
        mutex.lock();
        post temp{path, url, to_string(name)};
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
    
    regex file, gfv;
    regex exp;
    regex alb;
    regex gfy; //Finding gfycat links
    regex extless;
    regex http; //Used to change http to https
    regex animated; //Used to seperate still pictures and animated pics and video
    queue<post> QueuedUrls;
    vector<string> filenames;
    bool initialized = false;
    bool lockdown;
    ofxJSONElement albjson;
    processor* ImProc;
    processor* VidProc;
};


#endif /* processor_h */
