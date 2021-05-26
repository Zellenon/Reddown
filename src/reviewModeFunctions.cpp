//
//  reviewModeFunctions.cpp
//  Reddown
//
//  Created by Zelle Mandez on 11/8/17.
//
//

#include "ofApp.h"

void ofApp::positionItems() {

}

void ofApp::reviewLoad() {
    picsBin = Manager->picsBin;
    keeperBin = Manager->keeperBin;
    trashBin = Manager->trashBin;
    colWidth = (ofGetWidth()-60) / cols;
    rowHeight = (ofGetHeight() - 60) / (rows);
    cout << colWidth << " " << rowHeight << endl;
    
    picsBin.allowExt("png");
    picsBin.allowExt("jpeg");
    picsBin.allowExt("jpg");
    picsBin.listDir();
    if (picsBin.size() > rows*cols)
        images.resize(rows*cols);
    else
        images.resize(picsBin.size());
    keepers.resize(images.size());
    imageOriginals.resize(images.size());
    if (picsBin.size() > rows*cols + images.size())
        images2.resize(rows*cols);
    else
        images2.resize(picsBin.size() - images.size());
    imageOriginals2.resize(images2.size());
    for(int i = 0; i < images.size(); ++i) {
        imageOriginals[i] = picsBin.getPath(i);
        loader.loadFromDisk(images[i], imageOriginals[i]);
    }
    for(int i = 0; i < images2.size(); ++i) {
        imageOriginals2[i] = picsBin.getPath(i+imageOriginals.size());
        loader.loadFromDisk(images2[i], imageOriginals2[i]);
    }
}

void ofApp::reviewDraw() {
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            int i = r*cols + c;
            if (images.size() == 0 || i >= images.size()) return;
            bool tooBig = false;
            double ratio = 0.0;
            
            if (((float)colWidth) / ((float)images[i].getWidth()) < colWidth) {
                ratio = ((float)colWidth) / ((float)images[i].getWidth());
            }
            if (images[i].getHeight() < rowHeight) {
                if (((float)rowHeight) / ((float)images[i].getHeight()) < ratio)
                    ratio = ((float)rowHeight) / ((float)images[i].getHeight());
            }
            if (images[i].getWidth() > colWidth) {
                if (((float)colWidth) / ((float)images[i].getWidth()) < ratio)
                    ratio = ((float)colWidth) / ((float)images[i].getWidth());
            }
            if (images[i].getHeight() > rowHeight) {
                if (((float)rowHeight) / ((float)images[i].getHeight()) < ratio)
                    ratio = ((float)rowHeight) / ((float)images[i].getHeight());
            }
            if (abs(ratio-1.0) > 0.25) {
                images[i].resize(images[i].getWidth()*ratio, images[i].getHeight()*ratio);
                cout << (abs(ratio-1.0)) << endl;
            }
            int x = 30 + c*colWidth;
            int y = 50 + rowHeight*(r);
            coord temp = getPfromC(c,r);
            //cout << "x " << x << " y " << y << " colWidth " << colWidth << endl;
            if (keepers[i]) {
                ofSetColor(50,255,50);
            }
            else
                ofSetColor(255);
            images[i].draw(temp.x,temp.y);
        }
    }
    
    ofDrawBitmapString(ofToString("Pics remaining: " + to_string(picsBin.size())) , ofGetWidth()/2-100, 10);
}

void ofApp::loadBatch() {
    imageOriginals = imageOriginals2;
    images = images2;
    keepers.resize(0);
    keepers.resize(images.size());
    picsBin.listDir();
    if (picsBin.size() > rows*cols + images.size())
        images2.resize(rows*cols);
    else
        images2.resize(picsBin.size() - images.size());
    imageOriginals2.resize(images2.size());
    for(int i = 0; i < images2.size(); ++i) {
        imageOriginals2[i] = picsBin.getPath(i+imageOriginals.size());
        loader.loadFromDisk(images2[i], imageOriginals2[i]);
    }
}

void ofApp::mousePressedReview(int x, int y, int button) {
    coord temp = getCfromP(x, y);
    if (temp.x < 0 || temp.y < 0)
        return;
    if (picsBin.size() == 0 || temp.x >= cols || temp.y >= rows)
        return;
    keepers[temp.y*cols+temp.x] = !keepers[temp.y*cols+temp.x];
}

void ofApp::CullButtonEvent(ofxDatGuiButtonEvent e) { //When the user presses the button to bring up a new batch of images
    ofFile temp;
    for (int i = 0; i < images.size(); ++i) {
        if (keepers[i]) { //For images we want to keep
            temp.open(imageOriginals[i]);
            temp.moveTo(keeperBin.getAbsolutePath());
        } else { //For images we don't like
            temp.open(imageOriginals[i]);
            //temp.moveTo(trashBin.getAbsolutePath());
            temp.remove();
        }
    }
    loadBatch();
}

coord ofApp::getCfromP(int x, int y) { //Get grid coordinates from pixels
    coord result{-1,-1};
    x -= 20;
    y -= 50;
    if (x < 0 || y < 0)
        return coord{-1,-1};
    x /= colWidth;
    y /= rowHeight;
    return coord{x,y};
}

coord ofApp::getPfromC(int x, int y) { //Get pixels from grid coordinates
    x *= colWidth;
    y *= rowHeight;
    x += 20;
    y += 50;
    return coord{x,y};
}
