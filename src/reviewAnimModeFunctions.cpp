//
//  reviewAnimModeFunctions.cpp
//  Reddown
//
//  Created by Zelle Mandez on 11/10/17.
//
//

#include "ofApp.h"

void ofApp::reviewAnimLoad() {
    keeper = false;
    vidLoader->startThread();
}

void ofApp::reviewAnimDraw() {
    if (vidLoader->canDraw()) {
        if (keeper)
            ofSetColor(50,255,50);
        else
            ofSetColor(255);
        //display.setFromPixels(vidLoader->getFramePixels());
        //display.update();
        //display.draw(ofGetWidth()/2 - vidLoader->getFrameWidth()/2, ofGetHeight()/2 - vidLoader->getFrameHeight()/2);
        //vidLoader->vidList[0].video.update();
        vidLoader->draw();
        //cout << "drawn" << endl;
    }
    
    messages = messages + "\nVid remaining: " + to_string(vidLoader->toDoSize);
}

void ofApp::mousePressedAnimReview(int x, int y, int button) {
    if (y < 60)
        return;
    keeper = !keeper;
}

void ofApp::CullAnimButtonEvent(ofxDatGuiButtonEvent e) {
    if (keeper) {
        vidLoader->keep();
    } else {
        vidLoader->toss();
    }
    keeper = false;
}
