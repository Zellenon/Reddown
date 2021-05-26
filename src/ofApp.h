#pragma once

#include "ofMain.h"
#include "ofxDatGui.h"
#include "ofxJSON.h"
#include "ofxGui.h"
#include "ofxThreadedImageLoader.h"

#include "processor.h"
#include "threadManager.h"
#include "omniVidLoader.h"

struct coord {
    int x,y;
};

class ofApp : public ofBaseApp{
    
private:
    void setup();
    void update();
    void draw();
    void exit();
    
    threadManager* Manager;
    int operationMode; //SELECT = 0, LIST = 1, UPDATE = 2, RELOAD = 3, PURGE = 4, UPDATE SPECIFIC = 5
    int windowMode; //DOWNLOAD = 0, REVIEW = 1, , REVIEW ANIMATIONS = 2, LIST = 3
    
    ofxDatGui* modeGui;
    void DownloadModeButtonEvent(ofxDatGuiButtonEvent e);
    void ReviewModeButtonEvent(ofxDatGuiButtonEvent e);
    void ReviewAnimModeButtonEvent(ofxDatGuiButtonEvent e);
    void modeGuiInit();
    string messages;
    
    ofxDatGui* downGui;
    void DownloadButtonEvent(ofxDatGuiButtonEvent e);
    void onDropdownEvent(ofxDatGuiDropdownEvent e);
    void changeMode();
    ofxDatGuiTextInput* redname;
    ofxDatGuiSlider* postCount;
    void downGuiInit();
    void downDraw();
    
    ofxDatGui* reviewGui;
    ofxDatGuiButton* reviewCullButton;
    ofDirectory picsBin;
    ofDirectory keeperBin;
    ofDirectory trashBin;
    const static int rows = 2, cols = 5;
    int colWidth, rowHeight;
    void reviewGuiInit();
    void positionItems();
    void reviewLoad();
    void reviewDraw();
    void loadBatch();
    void CullButtonEvent(ofxDatGuiButtonEvent e);
    void mousePressedReview(int x, int y, int button);
    ofxThreadedImageLoader loader;
    vector<ofImage> images;
    vector<bool> keepers;
    vector<string> imageOriginals;
    vector<ofImage> images2;
    vector<string> imageOriginals2;
    coord getCfromP(int x, int y);
    coord getPfromC(int x, int y);
    
    ofxDatGui* reviewAnimGui;
    ofxDatGuiButton* reviewAnimCullButton;
    ofDirectory animBin;
    void reviewAnimGuiInit();
    void reviewAnimLoad();
    void reviewAnimDraw();
    void CullAnimButtonEvent(ofxDatGuiButtonEvent e);
    void mousePressedAnimReview(int x, int y, int button);
    bool keeper;
    omniVidLoader* vidLoader;
    ofImage display;
    
    
    
    
    
    
    //                                  JUNC FUNC
    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void mouseEntered(int x, int y);
    void mouseExited(int x, int y);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);
};
