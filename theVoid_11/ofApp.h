#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include "ofxOpenCv.h"
#include "ofxCv.h"
#include <vector>

class ofApp : public ofBaseApp {
public:
    void setup() override;
    void update() override;
    void draw() override;
    void keyPressed(int key) override;
    void mousePressed(int x, int y, int button) override;
    void mouseDragged(int x, int y, int button) override;

    // Helper functions
    void restartSketch();
    void randomizeOptions();
    void resetScreenshots();
    void drawDriftMode(float x, float y, float width, float height);
    void drawBecomeOneMode();
    //void salvation();

    // Webcam
    ofVideoGrabber webcam;
    bool webcamActive = false; // Initially false, webcam won't start until RETURN is pressed

    // OpenCV
    ofxCvColorImage color;
    ofxCvGrayscaleImage gray;
    ofxCvContourFinder contourFinder;
    ofxCvGrayscaleImage haarGray;
    ofxCvHaarFinder haar;

    // GUI
    ofxPanel gui;
    bool showGui = true;
    float guiHideStartTime = 0;

    // GUI Sliders & Toggles
    ofxFloatSlider scaleSlider;
    ofxVec2Slider vec2Slider;
    ofxVec3Slider vec3Slider;
    ofxToggle surrenderToggle;
    ofxToggle driftToggle;
    ofxToggle becomeOne;
    ofxIntSlider thresholdSlider;
    ofxSlider<int> faceCountSlider;
    ofxLabel countdownLabel;
    
    // Afterimage
    bool afterimageEnabled = false;
    ofFbo fbo;
    
    // Structure to store outlines with their creation time
    struct TimedOutline {
        ofPolyline outline;
        float creationTime; // Stores when the outline was drawn
    };
    
    // Screenshot Handling
    struct ScreenshotInstance {
        ofImage image;
        ofVec2f position;
    };
    std::vector<ScreenshotInstance> screenshots;
    ofImage screenshotImage;
    bool screenshotTaken = false;
    float screenshotMessageTime = 0.0f;
    int screenshotCount = 0;

    // Timer Handling
    bool timerActive = false;
    float startTime = 0.0f;
    float timeLeft = 0.0f;
    float timeLeftR;
    float timerDuration;
    bool isRestarting = false;

    // Messages & UI States
    bool showCensoredMessage = false;
    float endMessageStartTime = 0.0f;
    bool showInfiniteScrollMessage = false;
    float infiniteScrollStartTime = 0.0f;
    bool showPhotoMessage = false;
    bool screenshotMessageVisible = false;
    
    // Start & End Screens
    bool isStartScreen = true;
    int resetCounter = 0;
    int censoredMessageCount = 0;

    // Starfield for Drift Mode
    vector<ofVec3f> stars;
    int numStars = 1000;
    float starSpeed = 2.0;

    // Camera Drift Variables
    float cameraZ = 0.0f;
    float driftSpeed = 0.5f;
    float driftOffsetX = 0.0f;
    float driftOffsetY = 0.0f;
    float cameraScale = 0.9; // Camera starts at larger size
    
    // denial score keeping
    int denialScore = 0;       // Tracks number of clicks
    float lastClickTime = 0;   // Tracks last click time for smooth popups
    bool showDenialPopup = false;  // Controls when the popup is visible
    
    // TEXT AND FONTS
    ofTrueTypeFont titleFont, bodyFont, emphasisFont; // Declare multiple fonts
    float textStartTime; // When text sequence begins
    vector<float> revealTimes; // Store reveal times for each text

    
//    //Salvation - iBleach - Button
//    bool showSalvationButton = false;  // Controls button visibility
//    float salvationButtonStartTime = 0; // Time when button appears
//    bool programEndingSoon = false;  // Tracks if the program is about to end
//    ofRectangle salvationButtonRect; // Button hitbox

};
