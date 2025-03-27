#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    
    ofBackground(0);
    isStartScreen = resetCounter % 3 == 0; // Start screen every 3 resets
    
    //EXTERNAL CAMERA
    int cameraIndex = 0;  // Change to '1' for external camera, '0' for default webcam
    webcam.setDeviceID(cameraIndex);  // Set the device ID for the webcam
    webcam.setup(640, 480);  // The setup method automatically calls initGrabber
    ofSetFrameRate(60);
    
    //HAAR FINDER SETUP
    color.allocate(640, 480);
    gray.allocate(640, 480);

    haar.setup("haarcascade_frontalface_default.xml");
    haarGray.allocate(640, 480); // face finder gray allocate

    // Allocate and clear the FBO once
    fbo.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA); // Allocate FBO to camera size
    fbo.begin();
        ofClear(0, 0, 0, 0); // Clear FBO to transparent black
    fbo.end();
    
    // GUI PANEL SETUP / DESIGN
    gui.setup("Manipulate");
    gui.setPosition(10, 10); // Initial position of the GUI
    //gui.setPosition(ofGetWidth() - gui.getWidth() - 20, 20); // Initial position of the GUI; top right
    gui.setDefaultTextColor(ofColor(255)); // white text
    gui.setDefaultBackgroundColor(ofColor(20, 20, 20, 200)); // Dark, semi-transparent background
    //gui.setDefaultFillColor(ofColor(0, 255, 255, 80)); // Cyan sliders
    gui.setDefaultFillColor(ofColor(204, 0, 0, 80)); // red sliders
    gui.setDefaultWidth(230);
    gui.setDefaultHeight(25);
   
    //PANEL TOGGLES
    gui.add(thresholdSlider.setup("Find your form", 80, 0, 255));
    //gui.add(faceCountSlider.setup("Surveillance", 1, 1, 10)); // Range from 1 to 10
    gui.add(scaleSlider.setup("Ego Distortion", 1.0, 0.1, 3.0));                                                       // POSITION ON SCREEN
    gui.add(vec2Slider.setup("Displacement", ofVec2f(0, 0), ofVec2f(0, 0), ofVec2f(ofGetWidth(), ofGetHeight()))); // SIZE
    gui.add(vec3Slider.setup("Synthetic Vibrancy", ofVec3f(255, 255, 255), ofVec3f(0, 0, 0), ofVec3f(255, 255, 255)));      // COLOR CHANGE
    gui.add(driftToggle.setup("Illusory Agency", false));   // DRIFT  -- Suspended in the endless void; opt: Feigned Freedom; False Autonomy
    gui.add(surrenderToggle.setup("Hectic Escapism", false)); // RANDOMIZER; title opts: Hectic Avoidance; Frantic Distraction
    gui.add(countdownLabel.setup("Soul Decay", "0", 0, 20));      // TIMER COUNT DOWN
   //gui.add(becomeOne.setup("Fleeting Identity", false));  // lines of fleeting identity;"Tracing the shape of perpetual longing"; "Echoing the outline of artificial fulfillment"
    gui.add(becomeOne.setup("The Void", false)); // Toggle for afterimage mode

    //PHOTOS
    screenshotTaken = false;    // Reset the screenshot state
    showPhotoMessage = false; // PHOTO TIMER
    
    //SOUL DECAY TIMER
    float minTime = 10.0f;                      // Minimum timer value in seconds
    float maxTime = 40.0f;                      // Maximum timer value in seconds
    timeLeftR = ofRandom(minTime, maxTime);     // Randomize the timer (if needed for other purposes)
    
    //MAIN TIMERS
    timeLeft = 10;                              // Tracks the remaining time, updated every frame.
    timerDuration = ofRandom(minTime, maxTime); // Randomized duration of the countdown
    startTime = ofGetElapsedTimef();            // Marks the starting time of the countdow
    timerActive = true;                         // timer is active bool
    
    ofSeedRandom();  //Ensures different random values each time

    // Message management
    showCensoredMessage       = false;
    endMessageStartTime       = 0.0f;
    showInfiniteScrollMessage = false;
    infiniteScrollStartTime   = 0.0f;
    screenshotMessageTime     = 0.0f;
    screenshotMessageVisible  = false;
    
    //Load Fonts
    emphasisFont.load("Roboto_Condensed-Black.ttf", 25); // loaded in data bin
    titleFont.load("Roboto_SemiCondensed-Regular.ttf", 20); // loaded in data bin
    bodyFont.load("CourierPrime-Regular.ttf", 13); // loaded in data bin
    // TIMER for when text shows up
    textStartTime = ofGetElapsedTimef(); // Start timing
    // Define when each text appears (in seconds)
    revealTimes = {2, 4, 6, 9}; // Delays for each line (adjust as needed)
    
    // Initialize a larger starfield
    numStars = 1000;  // Increase the number of stars
    for (int i = 0; i < numStars; i++) {
        float x = ofRandom(-ofGetWidth(), ofGetWidth());  // Random x-coordinate
        float y = ofRandom(-ofGetHeight(), ofGetHeight()); // Random y-coordinate
        float z = ofRandom(0, ofGetWidth());  // Random depth
        stars.push_back(ofVec3f(x, y, z));  // Add star to the list
    }
}

//--------------------------------------------------------------
void ofApp::update(){
    
    webcam.update();
    
    // --------------------- HAAR + CONTOURS  ---------------------
    if (webcam.isInitialized() && webcam.isFrameNew()) {
        
        // Convert webcam to color and grayscale
        color.setFromPixels(webcam.getPixels());
        gray = color;

        // Apply threshold for contours
        gray.threshold(thresholdSlider);

        // Find contours
        contourFinder.findContours(
            gray,             // The image to analyze
            20,                  // Min area of a blob
            (640*480)/2,       // Max area of a blob
            10,                  // Max number of blobs
            false                // Find holes?
        );

        // Sync the internal flag with GUI toggle
        afterimageEnabled = becomeOne;
        
        // --------------------- Haar Face Tracking ---------------------
        haarGray = color; // Clean grayscale image for Haar detection
        haar.findHaarObjects(haarGray, 50, 50); // Adjust face size as needed
       
        // ---------- IMAGE CAPTURE WITH NEW FRAMES -------------
        screenshotImage.setFromPixels(webcam.getPixels());  // Capture the pixels only when the frame is new
    }
    
    // --------------------- TIMERS + MESSAGES  ---------------------
    if (timerActive) {
        
        //  Calculate the elapsed time since the timer started
        float elapsedTime = ofGetElapsedTimef() - startTime;
        timeLeft = timerDuration - elapsedTime; // Update the remaining time by the timerDuration random timer and countdown
       
        // When the main countdown hits 0:
        if (timeLeft <= 0) {
            timeLeft = 0;                              // Ensure timeLeft is not negative
            timerActive = false;                       // Stop the timer when it reaches 0
            webcam.close();                            // Stops the webcam feed
            showGui = false;                           // Hides the GUI
            showCensoredMessage = true;                // Trigger "CENSORED" message to show
            endMessageStartTime = ofGetElapsedTimef(); // Record the start time of the "CENSORED" message
        }
        
//        // Show the SALVATION R/ibleach 5 seconds before the program ends
//         if (elapsedTime >= timerDuration - 5.0f) {
//             showSalvationButton = true;
//             programEndingSoon = true;
//
//            salvationButtonStartTime = ofGetElapsedTimef();
//
//            salvationButtonRect.set(ofGetWidth() / 2 - 50, ofGetHeight() / 2 + 50, 120, 40);  // Define button size and position
//         }
//
//         // If the timer reaches 0 and no button was pressed, stop the program
//         if (elapsedTime >= timerDuration) {
//             timerActive = false;
//             showSalvationButton = false;
//             programEndingSoon = false;
//         }
    }
    
    // “Soul Decay” random timer (if used after the main timer ends)
    if (!timerActive && !showCensoredMessage && !showInfiniteScrollMessage) {
        
        // When the "soul decay" timer reaches zero, start a new random timer
        if (timeLeftR <= 0) {
            timeLeftR = ofRandom(10.0f, 30.0f);           // Randomized countdown duration (in seconds)
            showCensoredMessage = true;                   // Trigger "CENSORED" message
            endMessageStartTime = ofGetElapsedTimef();    // Record the start time of the message
        } else {
            // Decrease timeLeftR by the frame’s elapsed time
            float frameDelta = ofGetElapsedTimef() - startTime;
            timeLeftR -= frameDelta;
            startTime = ofGetElapsedTimef();
        }
    }
    
    // CHECK IF CENSORED MESSAGE SHOULD GO AWAY
       if (showCensoredMessage && ofGetElapsedTimef() - endMessageStartTime > 3) {
           showCensoredMessage       = false; // Hide the "CENSORED" message
           censoredMessageCount++;            // Increment the counter for "Censored" messages
           // After 3 "Censored" messages, restart the program
            if (censoredMessageCount >= 3) {
                restartSketch(); // Restart the sketch and return to the start screen
                }
           
           showInfiniteScrollMessage = true; //  Show the "INFINITE SCROLL" message
           screenshotTaken           = false; // Reset screenshot flag
           infiniteScrollStartTime = ofGetElapsedTimef();  // Record the start time for "INFINITE SCROLL"
       }

    // If “INFINITE SCROLL” is up, hide after 5 seconds => restart the sketch
       if (showInfiniteScrollMessage && ofGetElapsedTimef() - infiniteScrollStartTime > 5) {
         
           showInfiniteScrollMessage = false; // Hide the "INFINITE SCROLL" message
           screenshotTaken           = false; // Reset screenshot flag
           isRestarting              = true; // Set to true when restarting
           restartSketch(); // Trigger the sketch restart
       }
    
    // SHOW THE PHOTO MESSAGE 10 SECONDS IN; KEEP IT UP FOR 10 SECONDS AND BLINK
    if (timerActive && !showCensoredMessage && !showInfiniteScrollMessage) {
        float elapsedTime = ofGetElapsedTimef() - startTime;
       
        if (elapsedTime >= 10.0f && elapsedTime < 20.0f) {
            // Blink every 0.5 seconds
            if (int(elapsedTime * 2) % 2 == 0) {
                showPhotoMessage = true;
            } else {
                showPhotoMessage = false;
            }
        }
        //Restart timer every 20 seconds if NO toggles are active
        if (elapsedTime >= 30.0f && !becomeOne && !driftToggle) {
            startTime = ofGetElapsedTimef(); // Reset timer
            showPhotoMessage = false;  // Hide the message upon reset
        }
    }
    
    // countdown timer
    countdownLabel = ofToString(timeLeft, 1) + " seconds";
    
    // --------------------- SURRENDER ( RANDOMIZER )   ---------------------
    if (surrenderToggle) {
        randomizeOptions();
    }
    
    // --------------------- TOGGLES  ---------------------
    if (becomeOne) {
        if (driftToggle || surrenderToggle) {
            becomeOne = false;  // Disable becomeOne if another toggle is activated
        }
    }
    
    if (driftToggle) {
        if (becomeOne || surrenderToggle) {
            driftToggle = false;  // Disable becomeOne if another toggle is activated
        }
    }
    
    
    // --------------------- GUI BECOME ONE + DRIFT ACTIVE  ---------------------
    static bool previousDriftToggle = false; // Track the previous state of driftToggle
   
    if (driftToggle && !previousDriftToggle) {
        guiHideStartTime = 0; // Reset the timer when driftToggle is turned on
    }
    previousDriftToggle = driftToggle; // Update the previous state

    // Check if becomeOne was just activated
    static bool previousBecomeOne = false; // Track the previous state of becomeOne
    
    if (becomeOne && !previousBecomeOne) {
        guiHideStartTime = 0; // Reset the timer when becomeOne is turned on
    }
    previousBecomeOne = becomeOne; // Update the previous state

    // --------------------- ILLUSORY CONTROL SETTINGS RESET ---------------------

    if (driftToggle && !previousDriftToggle) {
        // When Illusory Control is toggled ON, reset denial score
        denialScore = 0;
        
        // Reset camera position when toggled ON
        cameraScale = 1.0f;
        driftOffsetX = 0.0f;
        driftOffsetY = 0.0f;
        cameraZ = 0.0f;
        
        cout << "Illusory Control Activated: Denial Score = " << denialScore << ", Camera Reset." << endl;
    }
    
        // Reset drift offset (stop drifting effect)
        driftOffsetX = 0.0f;
        driftOffsetY = 0.0f;
        cameraZ = 0.0f;
        
    // When Illusory Control is toggled OFF, reset everything immediately
      if (!driftToggle && previousDriftToggle) {
          cameraScale = 1.0f;
          driftOffsetX = 0.0f;
          driftOffsetY = 0.0f;
          cameraZ = 0.0f;
          
          denialScore = 0; // Ensure the denial score resets completely

          cout << "Illusory Control Deactivated: Camera and Denial Score Fully Reset." << endl;
      }

      previousDriftToggle = driftToggle; // ✅ Track previous state

}

//--------------------------------------------------------------
void ofApp::draw() {
    
    // --------------------- START SCREEN---------------------
//    if (isStartScreen) {
//
//        ofSetColor(255);
//        ofDrawBitmapString ("RETURN to The Void", ofGetWidth() / 2 - 100, ofGetHeight() / 2);
//        ofDrawBitmapString("(you are in the void)", ofGetWidth() / 2 - 100, ofGetHeight() / 2 + 20);
//        ofSetColor(255,0,0);
//        ofDrawBitmapString("(( Find your form First ))", ofGetWidth() / 2 - 100, ofGetHeight() / 2 + 30);
//        
//        ofDrawBitmapString("This project aims to highlights how we surrender our authenticity to technology, letting it consume us and remaining trapped in a self-perpetuating cycle we willingly embrace.", ofGetWidth() / 2 - 100, ofGetHeight() / 2 + 30);
//
//        return; // Don't draw anything else if we're on the start screen
//    }
    
    // --------------------- NEW START SCREEN---------------------
    if (isStartScreen) {
        
        ofSetColor(255);
        float elapsedTime = ofGetElapsedTimef() - textStartTime; // Time since start
        
        struct TextLine {
               string text;
               ofTrueTypeFont* font;
               ofColor color;
               float yOffset;
               float revealTime;
           };
        
        // Calculate center position
         float centerX = ofGetWidth() / 2;
         float centerY = ofGetHeight() / 2;

        vector<TextLine> textLines = {
              {"You are in the void.", &bodyFont, ofColor(255), 0, revealTimes[0]},
              {"We surrender. Technology consumes. A cycle of hollow salvation we willingly embrace.", &bodyFont, ofColor(248, 255, 0), 40, revealTimes[1]},
              {"And still, it continues.", &bodyFont, ofColor(248, 255, 0), 70, revealTimes[1]},
            // TITLE OPTIONS:
                    // "We surrender. Technology consumes. Trapped in a cycle we willingly embrace. The cycle continues."
                    //"We surrender. Technology consumes. A cycle of hollow salvation we willingly embrace. And still, it continues."
                    // "We surrender. Technology consumes. A cycle of illusion we willingly embrace."
                    //"We dissolve. The machine absorbs. Trapped in a cycle of hollow salvation."
              {"Find your form First", &bodyFont, ofColor(255, 0, 0, 95), 110, revealTimes[2]},

              {"RETURN to The Void.", &bodyFont, ofColor(255), 135, revealTimes[3]}
          };

        for (const auto& line : textLines) {
               if (elapsedTime >= line.revealTime) { // Show only if time has passed
                   float textWidth = line.font->stringWidth(line.text);
                   float x = centerX - textWidth / 2;
                   float y = centerY + line.yOffset;
                   
                   ofSetColor(line.color);
                   line.font->drawString(line.text, x, y);
               }
           }
        
        return;
    }
    
    // --------------------- IF WEBCAM IS RUNNING ...   ---------------------
    if (webcamActive && webcam.isInitialized()) {
        
        // --------------------- Webcam Feed ---------------------
        float scale = scaleSlider; // Get the current scale value from scaleSlider

        // Calculate the width and height of the webcam feed based on the scale
        float width = webcam.getWidth() * scale;
        float height = webcam.getHeight() * scale;

        // Get the current x and y positions from the vec2Slider
        float x = vec2Slider->x;
        float y = vec2Slider->y;

        // Draw the webcam feed
        ofSetColor(vec3Slider->x, vec3Slider->y, vec3Slider->z);
        webcam.draw(x, y, width, height);
        
        // --------------------- Haar Face Detection ---------------------
        ofSetColor(0, 255, 0); // Green for face rectangles
        ofNoFill();
        
        int maxFaces = static_cast<int>(faceCountSlider); // Ensure  integer
        int facesDrawn = 0;                  // Track how many faces have been drawn
     
        for (int i = 0; i < haar.blobs.size(); i++) {
               ofRectangle face = haar.blobs[i].boundingRect;

               // Scale the face rectangle to match the webcam feed scale
               float scaledX = x + face.x * scale;
               float scaledY = y + face.y * scale;
               float scaledWidth = face.width * scale;
               float scaledHeight = face.height * scale;

               // Draw the face rectangle
               ofDrawRectangle(scaledX, scaledY, scaledWidth, scaledHeight);

               facesDrawn++;
               if (facesDrawn >= maxFaces) break; // Stop drawing once maxFaces is reached
           }
        
    
        // ------------------- Contour Drawing MANUALLY (red) -------------------
        if (!becomeOne) { // Only draw contours if becomeOne is OFF
            ofSetColor(255, 0, 0); // Red for contours
            for (int i = 0; i < contourFinder.nBlobs; i++) {
                ofPolyline outline;
                for (auto point : contourFinder.blobs[i].pts) {
                    outline.addVertex(point); // Add each point in the blob to the polyline
                }
                outline.close(); // Close the shape
                outline.draw(); // Draw the contour
            }
        }
        
        // --------------------- BECOME ONE - DRAW  ---------------------
        if (becomeOne) {
            drawBecomeOneMode();
            return;
        }
        
        // --------------------- DRIFT - DRAW  ---------------------
        if (driftToggle) {
            drawDriftMode(0, 0, ofGetWidth(), ofGetHeight());
            return;
        }
    }

    // --------------------- SCREENSHOT HANDLING ---------------------
    if (screenshotTaken) {
        
        //ofSetColor(255);  //Reset color to white before drawing screenshots
        ofSetColor(255,0,0,70);

        for (const auto& screenshotInstance : screenshots) {
            screenshotInstance.image.draw(screenshotInstance.position);
        }

        ofSetColor(255, 255, 51);
        ofDrawBitmapString("Hollow Salvation", 10, ofGetHeight() - 20);
        ofDrawBitmapString("Soul Capture: " + ofToString(screenshotCount), 10, ofGetHeight() - 40);
        //bodyFont.drawString("Hollow Salvation", 10, ofGetHeight() - 20);
        //bodyFont.drawString("Soul Capture: " + ofToString(screenshotCount), 10, ofGetHeight() - 40);


        if (ofGetElapsedTimef() - screenshotMessageTime > 10) { // amount screenshots stay on screen
            screenshotMessageVisible = false;
            screenshotTaken = false;
        }
    }

// --------------------- MESSAGES HANDLING ---------------------
    // "CENSORED" overlay
    if (showCensoredMessage) {
        ofBackground(0);
        ofSetColor(255, 0, 0);
        ofDrawBitmapStringHighlight("CENSORED", ofGetWidth() / 2, ofGetHeight() / 2, ofColor(0, 0, 0, 200), ofColor(255, 0, 0));
    }

    // "INFINITE SCROLL" overlay
    if (showInfiniteScrollMessage) {
        float remainingTime = 5 - (ofGetElapsedTimef() - infiniteScrollStartTime);
        ofSetColor(255);
        ofDrawBitmapStringHighlight("The infinite scroll continues\nCountdown: " + ofToString(remainingTime, 1) + " seconds",
                                ofGetWidth() / 2 - 250, ofGetHeight() / 2 + 50, ofColor(0, 0, 0, 200), ofColor(255, 255, 255));
    }

    // Show photo message
    if (showPhotoMessage && !showCensoredMessage && !showInfiniteScrollMessage) {
        ofSetColor(248, 255, 0);
        ofDrawBitmapString("press control for hollow salvation", 800, 500);
        //bodyFont.drawString("press control for hollow salvation", 600, 500);

    }

    // Show GUI if visible
    if (showGui) {
        gui.draw();
    }
    
//    //SHOW SALVATION BUTTON
//    if (showSalvationButton) {
//        ofSetColor(255, 0, 0); // Red button
//        ofDrawRectangle(salvationButtonRect);
//
//        ofSetColor(255);
//        ofDrawBitmapString("SALVATION", salvationButtonRect.getX() + 15, salvationButtonRect.getY() + 25);
//    }
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
   
    //----------------- PROGRAM STARTS -------------------
    // Handle "Return" key to hide the start screen and begin the program
    if (key == OF_KEY_RETURN && isStartScreen) {
        isStartScreen = false;  // Hide the start screen and begin the sketch
        webcamActive = true;  // Enable webcam when RETURN is pressed

    }

    //----------------- SCREENSHOTS -------------------
    // Only take a screenshot if not on start screen and no messages are visible
    if (!isStartScreen && !showCensoredMessage && !showInfiniteScrollMessage) {
        if (key == OF_KEY_CONTROL) {

            // Ensure randomness is fresh each time
            ofSetRandomSeed(ofGetElapsedTimeMillis() + ofRandom(2000));

            // Capture pixels from the webcam
            ofImage newScreenshot;
            newScreenshot.setFromPixels(webcam.getPixels());
            newScreenshot.resize(webcam.getWidth() / 4, webcam.getHeight() / 4);

            // Create a new ScreenshotInstance
            ScreenshotInstance instance;
            instance.image = newScreenshot;

            // Ensure truly random positions within valid bounds
            float imgW = instance.image.getWidth();
            float imgH = instance.image.getHeight();

            instance.position.x = ofClamp(ofRandom(0, ofGetWidth() - imgW), 0, ofGetWidth() - imgW);
            instance.position.y = ofClamp(ofRandom(0, ofGetHeight() - imgH), 0, ofGetHeight() - imgH);

            // Add to the list
            screenshots.push_back(instance);
            
            if (screenshots.size() > 20) {
                 screenshots.erase(screenshots.begin()); // Limit storage to x images
             }

            // Update screenshot state
            screenshotTaken = true;
            screenshotMessageVisible = true;
            screenshotMessageTime = ofGetElapsedTimef(); // Record the current time
            screenshotCount++; // Increment the screenshot counter
            
        }
    }
}

//--------------------------------------------------------------
void ofApp::randomizeOptions() {
    scaleSlider = ofRandom(0.1, 3.0);
    vec2Slider = ofVec2f(ofRandom(0, ofGetWidth()), ofRandom(0, ofGetHeight())); // Randomize Position
    vec3Slider = ofVec3f(ofRandom(0, 255), ofRandom(0, 255), ofRandom(0, 255));  // Randomize Color
    
    // Randomize the position of the the entire GUI so it is hard to click
    gui.setPosition(
        ofRandom(0, ofGetWidth() - gui.getWidth()),
        ofRandom(0, ofGetHeight() - gui.getHeight())
    );
   
    // /GUI MOVEMENT STOPS IF CLICK NEAR GUI panel
    // Detect if the user clicks anywhere within the GUI's area
    if (surrenderToggle && ofGetMousePressed()) {
        // Check if the mouse is within the bounds of the GUI
        if (gui.getPosition().x < ofGetMouseX() && ofGetMouseX() < gui.getPosition().x + gui.getWidth() &&
            gui.getPosition().y < ofGetMouseY() && ofGetMouseY() < gui.getPosition().y + gui.getHeight()) {
            
            // If the mouse clicks the GUI, turn off 'Become One' and stop randomizing
            surrenderToggle = false;
        }
    }
}

//--------------------------------------------------------------
void ofApp::resetScreenshots() {
    screenshots.clear();     // Clear all stored screenshots
    screenshotImage.clear(); // Clear the temporary screenshot image
    screenshotCount = 0;     // Reset screenshot counter
    screenshotTaken = false; // Reset the screenshot state
    screenshotMessageVisible = false; // Hide any visible messages
    webcam.close();          // Close the webcam
    webcam.setup(640, 480);  // Reinitialize the webcam
    //setup(); // Re-call the setup function
}

//--------------------------------------------------------------
void ofApp::drawDriftMode(float x, float y, float width, float height) {
    // Clear the screen to black for a deep space effect
    ofBackground(0);

    // ------------------ STARFIELD------------------
    // Update the starfield positions
    ofSetColor(255); // White stars
    for (auto& star : stars) {
        // Move stars forward along the Z-axis
        star.z -= 10; // Speed of the movement forward (adjust as needed)
        
        // Add slight random movement in x and y for more organic motion
        star.x += ofRandom(-2, 2); // Random drift on x-axis
        star.y += ofRandom(-2, 2); // Random drift on y-axis

        // Reset stars that have "moved past" the camera
        if (star.z < 1) {
            star.x = ofRandom(-ofGetWidth(), ofGetWidth());  // Random x-coordinate
            star.y = ofRandom(-ofGetHeight(), ofGetHeight()); // Random y-coordinate
            star.z = ofGetWidth(); // Reset z-coordinate to the far edge
        }
        // Calculate the screen position and size based on depth
        float screenX = (star.x / star.z) * ofGetWidth() / 2 + ofGetWidth() / 2;
        float screenY = (star.y / star.z) * ofGetHeight() / 2 + ofGetHeight() / 2;
        float size = ofMap(star.z, 0, ofGetWidth(), 2, 0); // Smaller stars for distant ones
        
        // Draw the star as a dot
        ofDrawCircle(screenX, screenY, size);
    }
    
    // ------------------ DISPLAY "DENIAL" SCORE POPUP ------------------
       float popupDuration = 3.0f; // Show popup for 3 seconds
       if (showDenialPopup && (ofGetElapsedTimef() - lastClickTime < popupDuration)) {
           ofSetColor(255, 0, 0, 200); // Red semi-transparent text
           ofDrawBitmapString("Denial: " + ofToString(denialScore), ofGetWidth() / 2 - 50, ofGetHeight() / 2);
       } else {
           showDenialPopup = false; // Hide popup after duration
       }
   
    // ------------------ CAMERA MOVEMENT ------------------
    ofPushMatrix();

     // Make sure camera scale and position reset correctly
     float feedScale = 0.4 - (cameraZ / 2000.0);
     feedScale = feedScale * cameraScale; // Keep updated reset value
     feedScale = ofClamp(feedScale, 0.2, 1.0); // Prevent it from disappearing

     ofTranslate(ofGetWidth() / 2 + driftOffsetX, ofGetHeight() / 2 + driftOffsetY);
     ofScale(feedScale, feedScale);
     ofRotateDeg(ofGetElapsedTimef() * 0.05, 0, 0, 1);

     ofSetColor(255, 80);
     webcam.draw(-webcam.getWidth() / 2, -webcam.getHeight() / 2);
     
     ofPopMatrix();
    
    
    // ------------------  HANDLE GUI ------------------
    if (guiHideStartTime == 0) {
        guiHideStartTime = ofGetElapsedTimef(); // Start the timer when the mode begins
    }

    if (ofGetElapsedTimef() - guiHideStartTime < 5) {
        showGui = false; // Hide the GUI for the first 8 seconds
    } else {
        showGui = true; // Show the GUI after 8 seconds
    }

    // Draw GUI if visible
    if (showGui) {
        gui.draw();
        
    }
}

//--------------------------------------------------------------
void ofApp::drawBecomeOneMode() {
    ofBackground(0); // Clear the screen to black

    fbo.begin();

    // Only clear the FBO once when we initialize/reset, not every frame
    if (ofGetFrameNum() == 1) {
        ofClear(0, 0, 0, 0); // Clear once to fully transparent
    }

    ofEnableAlphaBlending();

    // Semi-transparent fade effect
    ofSetColor(0, 0, 0, 8); // Lower alpha for a slow fade
    ofDrawRectangle(0, 0, fbo.getWidth(), fbo.getHeight());
   
    //Scale factor for full-screen effect
    float scaleX = (float)ofGetWidth() / (float)webcam.getWidth();
    float scaleY = (float)ofGetHeight() / (float)webcam.getHeight();

    // ------------- SMOOTH CYCLES BETWEEN COLORS-------------
    // Define 3 specific glitch colors
    ofColor glitchColors[3] = {
        ofColor(153,204,204),  // blueish-green
        ofColor(153,204,153),  // greenish-blue
        ofColor(153,204,102)   // green
    };

    // Get current time-based cycle index
    float timeCycle = fmod(ofGetElapsedTimef() / 6.0, 1.0); // Normalized cycle (0 to 1)

    // Determine the two closest colors to blend
    int baseIndex = (static_cast<int>(ofGetElapsedTimef()) / 6) % 3;
    int nextIndex = (baseIndex + 1) % 3; // Loop back after last color

    // Smoothly interpolate between colors
    ofColor glitchColor = glitchColors[baseIndex].getLerped(glitchColors[nextIndex], timeCycle);
    
    // ------------- HARD SWAP BETWEEN COLORS -------------
//    // Define 3 specific glitch colors
//    ofColor glitchColors[3] = {
//        ofColor(153,204,204),  // blueish - green
//        ofColor(153,204,153),  // greenish - blue
//        ofColor(153,204,102)   // green
//    };
//
//    // Cycle through colors every 6 seconds
//    int colorIndex = (static_cast<int>(ofGetElapsedTimef()) / 6) % 3;
//    ofColor glitchColor = glitchColors[colorIndex];

    // Track time for each outline
    static vector<TimedOutline> activeOutlines; // Store outlines with timestamps
    float currentTime = ofGetElapsedTimef();    // Get current time

    //Capture new outlines
    for (int i = 0; i < contourFinder.nBlobs; i++) {
        ofPolyline outline;
        for (auto point : contourFinder.blobs[i].pts) {

            // new Proper scaling: Map webcam points to full-screen FBO
            float glitchX = ofMap(point.x, 0, webcam.getWidth(), 0, fbo.getWidth()) + ofRandom(-5, 5);  // Add minor glitch movement
            float glitchY = ofMap(point.y, 0, webcam.getHeight(), 0, fbo.getHeight()) + ofRandom(-5, 5);
            
            outline.addVertex(glitchX, glitchY);
        }
        outline.close();

        // Store outline with timestamp
        activeOutlines.push_back({outline, currentTime});
    }

    // Draw & fade outlines
    for (int i = 0; i < activeOutlines.size(); i++) {
        float elapsed = currentTime - activeOutlines[i].creationTime;

        // **Smooth fade: Use noise for organic transparency transition**
        float fadeAmount = ofMap(ofNoise(elapsed * 0.1), 0, 1, 255, 0);

        if (elapsed >= 3.0f) { // If outline is older than 5 seconds, fade out
            ofSetColor(0, 0, 0, fadeAmount); // Fade to black
        } else {
            ofSetColor(glitchColor, fadeAmount); // Use glitch color while active
        }

        activeOutlines[i].outline.draw();
    }

    // Remove fully faded outlines
    activeOutlines.erase(
        remove_if(activeOutlines.begin(), activeOutlines.end(), [&](const TimedOutline& o) {
            return (currentTime - o.creationTime) > 6.0f; // Remove after full fade-out
        }),
        activeOutlines.end()
    );

    ofDisableAlphaBlending();
    fbo.end();

    // Draw the FBO Fullscreen
    ofSetColor(255);
    fbo.draw(0, 0, ofGetWidth(), ofGetHeight());

//    // Add label/text for the mode
//    ofSetColor(255);
//    ofDrawBitmapString("There is nothing for you here", ofGetWidth() / 2, ofGetHeight() - 20);

    string message = "There is nothing for you here";
    int textWidth = message.length() * 8; // Approximate width (each char ~8px in default font)
    int centerX = (ofGetWidth() / 2) - (textWidth / 2);
    int bottomY = ofGetHeight() - 20; // Adjust Y position as needed

    ofSetColor(255);
    //ofDrawBitmapString(message, centerX, bottomY);
    titleFont.drawString(message, centerX, bottomY);

    
    // Handle GUI visibility
    if (guiHideStartTime == 0) {
        guiHideStartTime = ofGetElapsedTimef();
    }

    if (ofGetElapsedTimef() - guiHideStartTime < 8) {
        showGui = false;  // Hide GUI for the first 8 seconds
    } else {
        showGui = true;  // Show GUI after 8 seconds
    }

    //Draw GUI if visibl
    if (showGui) {
        gui.draw();
    }
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {
    if (driftToggle) { // Only count clicks if in Drift Mode
        denialScore++; // Increase score
        lastClickTime = ofGetElapsedTimef(); // Store time for popup visibility
        showDenialPopup = true; // Activate popup
        
        // Shrink camera image slightly (limit to 0.2 size minimum)
        cameraScale = std::max(cameraScale * 0.95f, 0.2f); // Ensure it doesn't shrink below 0.2
    }
    
//    //SALVATION BUTTON
//    if (showSalvationButton && salvationButtonRect.inside(x, y)) {
//        salvation();  // Call the function when button is clicked
//        showSalvationButton = false;  // Hide the button after clicking
//       }

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {
    if (driftToggle) { // Only count drags if in Drift Mode
        denialScore++; // Increase score
        lastClickTime = ofGetElapsedTimef(); // Store time for popup visibility
        showDenialPopup = true; // Activate popup
    }
}

////--------------------------------------------------------------
//void ofApp::salvation() {
//    //timerDuration += 10.0f; // Add 10 more seconds to the timer
//    //startTime = ofGetElapsedTimef(); // Reset timer so it doesn't immediately end
//    showSalvationButton = false;  // Hide button after clicking
//    programEndingSoon = false;
//
//    ofLaunchBrowser("https://www.reddit.com/r/ibleach/"); // Open live page
//}

//--------------------------------------------------------------
void ofApp::restartSketch() {
    
    //START SCREEN RESTART
    resetCounter++; // Increment the reset counter
    isStartScreen = resetCounter % 3 == 0;
    
    // Else do a normal “reboot”
    timerActive   = true;
    timeLeft      = 10.0f;
    startTime     = ofGetElapsedTimef(); // Reset start time
    
    //SCREENSHOTS /MESSAGES
    screenshotTaken           = false;   // no photos taken
    screenshotCount           = 0;      // Reset count ot 0
    screenshots.clear();                // Clear all stored screenshots
    screenshotImage.clear();        // Clear the single screenshot image
    screenshotMessageVisible  = false;  // Hide screenshot message

    //MESSAGES
    censoredMessageCount = 0;  // Reset "CENSORED" message counter
    showCensoredMessage       = false;  // Hide end message
    showInfiniteScrollMessage = false;  // Hide infinite scroll message
    driftToggle               = false;        // reset drift toggle off
    surrenderToggle           = false;        // reset surrender toggle off
    showGui                   = true;         // Re-enable the GUI upon restart
    
    // Reset mode toggles
    becomeOne = false;
    driftToggle = false;
    surrenderToggle = false;
    denialScore = 0;  //Reset denial score
    cameraScale = 1.0f;
    driftOffsetX = 0.0f;
    driftOffsetY = 0.0f;
    cameraZ = 0.0f;
        
//    //salvation
//    showSalvationButton = false;
//    programEndingSoon = false;
//    salvationButtonStartTime = 0;    // Prevent web page from opening again after reset

    webcam.close();                   // close webcam
    webcam.setup(640, 480);           // Restart webcam
    setup();
    }

