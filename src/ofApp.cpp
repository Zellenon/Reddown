#include "ofApp.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <curl/curl.h>

using namespace std;

/*--------------------------------------------------------------
**TODO
 * Make a OmniVidPlayer that works with both mp4 and gif files.
 https://github.com/mactkg/ofxThreadedGifLoader/blob/master/example/src/ofApp.cpp
 https://github.com/fluaten/Pinacotek
 *MAKE IT SO IMGUR ALBUMS DONT WIPE RECORDS
 *Fix Mutexes
 *Get that damn progress bar working
 
 *Maybe add push notifications
*/

void ofApp::setup(){
    ofSetWindowShape(1440, 900);
    ofSetWindowPosition(20, 20);
    ofSetFrameRate(30); //Find a better way of limiting background overrun
    
//  Initializers
    modeGuiInit();
    downGuiInit();
    reviewGuiInit();
    reviewAnimGuiInit();
    
//  Thread Setup
    Manager = new threadManager();
    Manager->setup();
    Manager->startThread();
    
    ofSleepMillis(100);
    vidLoader = new omniVidLoader(Manager->animBin.getAbsolutePath(), Manager->keeperBin.getAbsolutePath());
    vidLoader->setup();
    
    operationMode = 2;
    windowMode = 0;
    changeMode();
}

//  Sets up the master GUI responsible for changing operation modes.
void ofApp::modeGuiInit() {
    ofxDatGuiComponent* component;
    
    modeGui = new ofxDatGui(ofxDatGuiAnchor::TOP_RIGHT);
    //modeGui->setPosition(ofGetWidth()-200,0);
    
    component = modeGui->addButton("d o w n l o a d   m o d e"); //
    component->onButtonEvent(this, &ofApp::DownloadModeButtonEvent);
    
    component = modeGui->addButton("r e v i e w   m o d e");
    component->onButtonEvent(this, &ofApp::ReviewModeButtonEvent);
    
    component = modeGui->addButton("r e v i e w   a n i m   m o d e");
    component->onButtonEvent(this, &ofApp::ReviewAnimModeButtonEvent);
}

//  Sets up the GUI the user interfaces with to order downloads
void ofApp::downGuiInit() {
    ofxDatGuiComponent* component;
    
    downGui = new ofxDatGui();
    downGui->setPosition(50,50);
    
    component = downGui->addButton("B e g i n");
    component->onButtonEvent(this, &ofApp::DownloadButtonEvent);
    
    vector<string> options = {"Select", "List", "Update", "Refresh", "Purge", "Update Specific", "DONMAI"};//SELECT = 0, LIST = 1, UPDATE = 2, RELOAD = 3, PURGE = 4, UPDATE SPECIFIC = 5, DONMAI = 6
    component = downGui->addDropdown("Operation Modes", options);
    component->onDropdownEvent(this, &ofApp::onDropdownEvent);
    
    component = downGui->addLabel("Options:");
    
    redname = downGui->addTextInput("Sub Selection", "Subreddit Name");
    redname->setWidth(270, .4);
    postCount = downGui->addSlider("Posts", 1, 200);
    postCount->setWidth(270, 0.2);
    postCount->setPrecision(0);
    postCount->setValue(20);
    postCount->setVisible(true);
}

//  Sets up the GUI for reviewing images downloaded
void ofApp::reviewGuiInit() {
    ofxDatGuiComponent* component;
    
    reviewGui = new ofxDatGui();
    reviewGui->setPosition(ofGetWidth()/2-50, 25);
    reviewCullButton = reviewGui->addButton("C u l l   B a t c h");
    reviewCullButton->onButtonEvent(this, &ofApp::CullButtonEvent);
    //reviewCullButton->setPosition(ofGetWidth()/2-50, 20);
}

//  Sets up the GUI for reviewing videos downloaded
void ofApp::reviewAnimGuiInit() {
    ofxDatGuiComponent* component;
    
    reviewAnimGui = new ofxDatGui();
    reviewAnimGui->setPosition(ofGetWidth()/2-50, 25);
    reviewAnimCullButton = reviewAnimGui->addButton("C u l l   B a t c h");
    reviewAnimCullButton->onButtonEvent(this, &ofApp::CullAnimButtonEvent);
    //reviewCullButton->setPosition(ofGetWidth()/2-50, 20);
}

//--------------------------------------------------------------
void ofApp::update(){
    for (int i = 0; i < vidLoader->unloadedList.size() > 0; i++)
        if (!vidLoader->unloadedList[i].video.isLoaded())
            vidLoader->unloadedList[i].video.loadAsync(vidLoader->unloadedList[i].source.getAbsolutePath());
}

//--------------------------------------------------------------
void ofApp::draw()
{
    ofBackground(200);
    messages = "Orders to process:" + to_string(Manager->QueuedOrders.size());
    messages +="\nFPS: " + to_string(ofGetFrameRate());
    switch(windowMode) {
        case 0:
            downDraw();
            break;
        case 1:
            reviewDraw();
            break;
        case 2:
            reviewAnimDraw();
            break;
    }
    ofDrawBitmapString(messages, 300, 20);
}

void ofApp::downDraw() {
    ofSetColor(ofColor::black);
    messages = messages + ofToString("\nUrls to be sorted: " + to_string(Manager->Sorter->QueuedUrls.size()));
    messages = messages + ofToString("\nBuffered Image Links: " + to_string(Manager->ImProc->QueuedPosts.size()));
    messages = messages + ofToString("\nBuffered Video Links: " + to_string(Manager->VidProc->QueuedPosts.size()));
}

//--------------------------------------------------------------
void ofApp::exit(){
    Manager->stop();
    loader.stopThread();
    vidLoader->stop();
}

//--------------------------------------------------------------
void ofApp::DownloadButtonEvent(ofxDatGuiButtonEvent e) { //e.target gives you the button
    switch(operationMode) {
        case 0:
            Manager->add(0, redname->getText(), postCount->getValue());
            cout << "Got to the case switch" << endl;
            //redditS(redname->getText(), postCount->getValue());
            break;
        case 1:
            Manager->add(1,"",postCount->getValue());
            break;
        case 2:
            Manager->add(2,"",0);
            break;
        case 6:
            Manager->add(6, redname->getText(), postCount->getValue());
            break;
    }
}
//--------------------------------------------------------------
void ofApp::DownloadModeButtonEvent(ofxDatGuiButtonEvent e) { //e.target gives you the button
    windowMode = 0;
    operationMode = 2;
    changeMode();
}
//--------------------------------------------------------------
void ofApp::ReviewModeButtonEvent(ofxDatGuiButtonEvent e) { //e.target gives you the button
    windowMode = 1;
    changeMode();
}
//--------------------------------------------------------------
void ofApp::ReviewAnimModeButtonEvent(ofxDatGuiButtonEvent e) { //e.target gives you the button
    windowMode = 2;
    changeMode();
}
//--------------------------------------------------------------
void ofApp::onDropdownEvent(ofxDatGuiDropdownEvent e) {
    operationMode = e.child;
    changeMode();
}
//--------------------------------------------------------------
void ofApp::changeMode() {
    switch(windowMode) {
        case 0: //<<<<<<<<<<<<<<<<<<<<<<<-=-=-=-=-=-=-=-=-=-=-=- DOWNLOAD MODE
            ofSetFrameRate(30);
            downGui->setVisible(true);
            reviewGui->setVisible(false);
            reviewAnimGui->setVisible(false);
            //vidLoader->running = false;
            vidLoader->stopThread();
            postCount->setMax(100);
            switch(operationMode) {
                case 0: //SELECT = 0
                    postCount->setVisible(true);
                    redname->setVisible(true);
                    break;
                case 1: //LIST = 1
                    postCount->setVisible(true);
                    redname->setVisible(false);
                    break;
                case 2: //UPDATE = 2
                    postCount->setVisible(false);
                    redname->setVisible(false);
                    break;
                case 3: //RELOAD = 3
                    postCount->setVisible(false);
                    redname->setVisible(false);
                    break;
                case 4: //PURGE = 4
                    postCount->setVisible(false);
                    redname->setVisible(true);
                    break;
                case 5: //UPDATE SPECIFIC = 5
                    postCount->setVisible(false);
                    redname->setVisible(true);
                    break;
                case 6: //DONMAI = 6
                    postCount->setMax(2000);
                    postCount->setVisible(true);
                    redname->setVisible(true);
                    break;
            }
            break;
        case 1: //<<<<<<<<<<<<<<<<<<<<<<<-=-=-=-=-=-=-=-=-=-=-=- REVIEW MODE
            ofSetFrameRate(30);
            //vidLoader->running = false;
            vidLoader->stopThread();
            downGui->setVisible(false);
            reviewGui->setVisible(true);
            reviewAnimGui->setVisible(false);
            reviewLoad();
            break;
        case 2: //<<<<<<<<<<<<<<<<<<<<<<<-=-=-=-=-=-=-=-=-=-=-=- REVIEW ANIM MODE
            ofSetFrameRate(30);
            downGui->setVisible(false);
            reviewAnimGui->setVisible(true);
            reviewGui->setVisible(false);
            vidLoader->running = true;
            reviewAnimLoad();
            break;
    }
}
