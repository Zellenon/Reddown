//
//  omniVidLoader.h
//  Reddown
//
//  Created by Zelle Mandez on 11/12/17.
//
//

#ifndef omniVidLoader_h
#define omniVidLoader_h

#define SIZE_DESIRED 3
//#define DISPLAYGIF gifList[gifList.size()-1]
//#define DISPLAYVID vidList[vidList.size()-1]
#define DISPLAYGIF gifList[0]
#define DISPLAYVID vidList[0]

#include "ofxThreadedGif/ofxThreadedGifLoader.h"
struct vidFile {
    ofVideoPlayer video;
    ofFile source;
};

struct gifFile {
    ofxThreadedGifFile gif;
    ofFile source;
};

class omniVidLoader : public ofThread {
public:
    
    omniVidLoader(string animPath, string keeperPath) {
        vidBin = *new ofDirectory(animPath);
        vidBin.allowExt("mp4");
        vidBin.allowExt("mov");
        
        gifBin = *new ofDirectory(animPath);
        gifBin.allowExt("gif");
        
        keeperBin = *new ofDirectory(keeperPath);
    }
    
    void setup() {
        ofAddListener(ofxGifLoadedEvent::events, this, &omniVidLoader::gifLoaded); //Used by the library that helps load animated gifs
        //ofAddListener(ofEvents().update, this, &omniVidLoader::update);            //Tells us when to run our upkeep routine //Uncomment this to use other update method
        loader.startThread();                                                      //Starts the gifLoader
        
        string currentUser = getenv("USER");
        string cachePath = "/Users/" + currentUser + "/Documents/Reddown/.reviewCache/";
        cacheBin = *new ofDirectory(cachePath);
        cacheBin.listDir();
        if (cacheBin.isDirectory()) {                                              //If there's stuff in the reviewCache folder, empty it out into the animfolder
            for (int i = 0; i< cacheBin.size(); ++i) {
                ofFile temp = cacheBin.getFile(i);
                temp.moveTo(gifBin.getAbsolutePath());
            }
        } else
            ofDirectory::createDirectory(cachePath,false,false);
        
        fileMode = true;
        gifCount=0;
    }
    
    void stop() {
        if (cacheBin.isDirectory()) {                                              //If there's stuff in the reviewCache folder, empty it out into the animfolder
            for (int i = 0; i< cacheBin.size(); ++i) {
                ofFile temp = cacheBin.getFile(i);
                temp.moveTo(gifBin.getAbsolutePath());
                //cout << "Moved File";
            }
        }
        stopThread();
    }
    
    void threadedFunction() {
        while (isThreadRunning()){
            update2();
        }
    }
    
    void update(ofEventArgs & a) {
        cacheBin.listDir();
        toDoSize = cacheBin.size() + vidBin.size();
        //loadGifs();
        loadVids();
        if (running) {
            if (fileMode && vidList.size() > 0) DISPLAYVID.video.update();
            if (!fileMode && gifList.size() > 0) frame++;
        
            if (vidList.size() == 0) fileMode = false;
        }
    }
    
    void update2() {
        cacheBin.listDir();
        toDoSize = cacheBin.size() + vidBin.size();
        //loadGifs();
        loadVids();
        if (unloadedList.size() > 0) shiftVids();
        if (running) {
            //if (fileMode && vidList.size() > 0) DISPLAYVID.video.update();
            if (!fileMode && gifList.size() > 0) frame++;
            if (vidList.size() == 0 && gifList.size() > 0) fileMode = false;
        }
    }
    
    void draw() {
        if (canDraw()) {
            if (fileMode) {
                DISPLAYVID.video.update();
                DISPLAYVID.video.draw(ofGetWidth()/2 - getFrameWidth()/2, ofGetHeight()/2 - getFrameHeight()/2);
                /*ofImage image;
                image.setFromPixels(getFramePixels());
                image.draw(ofGetWidth()/2 - getFrameWidth()/2, ofGetHeight()/2 - getFrameHeight()/2);*/
            } else {
                if (gifList.size() > 0) {
                    frame++;
                    int frameCalc = ((int)(frame / (DISPLAYGIF.gif.getFrameAt(1)->getDuration() * 30))) % DISPLAYGIF.gif.getNumFrames();
                    //cout << "Frame Dur: " << to_string(DISPLAYGIF.gif.getFrameAt(frameCalc)->getDuration()) << endl;
                    //cout << "Frame Dur: " << to_string(DISPLAYGIF.gif.getDuration()) << endl;
                    return *DISPLAYGIF.gif.getFrameAt(frameCalc)->getRawPixels();
                }
            }
        }
    }
    
    void loadGifs() {
        lock();
        //cout << "gifBin size: " << to_string(gifBin.size()) << " gifList size: " << to_string(gifList.size()) << " gifCount " << to_string(gifCount) << endl;
        gifBin.listDir();
        if ((gifList.size() < SIZE_DESIRED && gifCount < SIZE_DESIRED) && gifBin.size() > 0) {
            cout << "LOADING GIF; gifBin size: " << to_string(gifBin.size()) << endl;
            ofFile temp;
            gifBin.listDir();
            temp = gifBin.getFile(0);
            temp.moveTo(cacheBin.getAbsolutePath());
            loader.loadFromDisk(temp.getAbsolutePath());
            temp.close();
            gifCount++;
        }
        unlock();
    }
    
    /*void loadVids() {
        vidBin.listDir();
        if (vidList.size() < SIZE_DESIRED && vidBin.size() > 0) {
            lock();
            vidFile temp;
            temp.source = vidBin.getFile(0);
            temp.source.moveTo(cacheBin.getAbsolutePath());
            temp.video.loadAsync(temp.source.getAbsolutePath());
            temp.video.setLoopState(OF_LOOP_NORMAL);
            temp.video.play();
            vidList.push_back(temp);
            unlock();
        }
    }*/
    void loadVids() {
        vidBin.listDir();
        if (vidList.size() + unloadedList.size() < SIZE_DESIRED && vidBin.size() > 0) {
            lock();
            vidFile temp;
            temp.source = vidBin.getFile(0);
            temp.source.moveTo(cacheBin.getAbsolutePath());
            //temp.video.loadAsync(temp.source.getAbsolutePath());
            //temp.video.play();
            unloadedList.push_back(temp);
            unlock();
        }
    }
    
    void shiftVids() {
        for (int i = 0; i<unloadedList.size();) {
            if (unloadedList[i].video.isLoaded()) {
                unloadedList[i].video.play();
                unloadedList[i].video.setLoopState(OF_LOOP_NORMAL);
                vidList.push_back(unloadedList[i]);
                cout << "list size: " << unloadedList.size() << " i value " << i;
                unloadedList.erase(unloadedList.begin()+i);
            } else
                i++;
        }
    }
    
    void gifLoaded(ofxGifLoadedEvent &e){
        lock();
        ofFile temp;
        temp.open(e.path);
        gifList.push_back(gifFile{e.gif,temp});
        unlock();
    }
    
    ofPixels & getFramePixels() {
        if (fileMode) {
            if (vidList.size() > 0 && DISPLAYVID.video.isLoaded())
                return DISPLAYVID.video.getPixels();
        } else {
            if (gifList.size() > 0) {
                int frameCalc = ((int)(frame / (DISPLAYGIF.gif.getFrameAt(1)->getDuration() * 30))) % DISPLAYGIF.gif.getNumFrames();
                //cout << "Frame Dur: " << to_string(DISPLAYGIF.gif.getFrameAt(frameCalc)->getDuration()) << endl;
                //cout << "Frame Dur: " << to_string(DISPLAYGIF.gif.getDuration()) << endl;
                return *DISPLAYGIF.gif.getFrameAt(frameCalc)->getRawPixels();
            }
        }
    }
    
    int getFrameWidth() {
        if (fileMode) {
            if (vidList.size() > 0 && DISPLAYVID.video.isLoaded())
                return DISPLAYVID.video.getWidth();
        } else {
            if (gifList.size() > 0)
                return DISPLAYGIF.gif.getWidth();
        }
        return -1;
    }
    
    int getFrameHeight() {
        if (fileMode) {
            if (vidList.size() > 0 && DISPLAYVID.video.isLoaded())
                return DISPLAYVID.video.getHeight();
        } else {
            if (gifList.size() > 0)
                return DISPLAYGIF.gif.getHeight();
        }
        return -1;
    }
    
    bool canDraw() {
        if (fileMode) {
            if (vidList.size() > 0 && DISPLAYVID.video.isLoaded())
                return DISPLAYVID.video.isLoaded();
        } else {
            if (gifList.size() > 0)
                return true;
        }
        return false;
    }
    
    void keep() {
        cout << "KEEP IT" << endl;
        if (fileMode) {
            if (vidList.size() > 0) {
                DISPLAYVID.source.moveTo(keeperBin.getAbsolutePath());
                vidList.erase(vidList.begin());
                loadFrontVid();
            }
        } else {
            if (gifList.size() > 0) {
                DISPLAYGIF.source.moveTo(keeperBin.getAbsolutePath());
                gifList.erase(gifList.begin());
                gifCount--;
            }
        }
        if (gifList.size() != 0) fileMode = false; else fileMode = true;
        frame = 0;  
    }
    
    void toss() {
        cout << "TOSS IT" << endl;
        if (fileMode) {
            if (vidList.size() > 0) {
                DISPLAYVID.source.remove();
                vidList.erase(vidList.begin());
                if (vidList.size() > 0)
                    loadFrontVid();
            }
        } else {
            if (gifList.size() > 0) {
                DISPLAYGIF.source.remove();
                gifList.erase(gifList.begin());
                gifCount--;
            }
        }
        if (gifList.size() != 0) fileMode = false; else fileMode = true;
        frame = 0;
    }
    
    void loadFrontVid() {
        vidList[0].video.play();
        vidList[0].video.setLoopState(OF_LOOP_NORMAL);
        vidList[0].video.setFrame(0);
    };
    
    ofxThreadedGifLoader loader;
    vector<gifFile> gifList;
    vector<vidFile> vidList;
    vector<vidFile> unloadedList;
    ofDirectory vidBin;
    ofDirectory gifBin;
    ofDirectory keeperBin;
    ofDirectory cacheBin;
    int frame;
    bool fileMode; //True = Vids; False = Gifs
    int toDoSize;
    int gifCount;
    bool running;
};

#endif /* omniVidLoader_h */
