#include "backgroundSubtract.hpp"

// Global variables
Mat frame; //current frame
Mat fgMask; //fg mask generated by method of choise
Ptr<BackgroundSubtractor> pBS; // Background subtractor
int  keyboard; //input from keyboard

void backgroundSubtract(char* videoFilename)
{
    int  keyboard = 0; //input from keyboard

    //create GUI windows
    namedWindow("Frame");
    namedWindow("FG Mask");

    //create Background Subtractor objects
#if (BS_METHOD == 0)
    pBS = bsegm::createBackgroundSubtractorMOG(); //MOG approach
#elif (BS_METHOD == 1)
    pBS = createBackgroundSubtractorMOG2(); //MOG2 approach
#else
    pBS = createBackgroundSubtractorKNN(); //KNN approach
#endif

    //create the capture object
    VideoCapture capture(videoFilename);

    if(!capture.isOpened()){
        //error in opening the video input
        cerr << "Unable to open video file: " << videoFilename << endl;
        exit(EXIT_FAILURE);
    }

    //read input data. ESC or 'q' for quitting
    while( (char)keyboard != 'q' && (char)keyboard != 27 )
    {
        //read the current frame
        if(!capture.read(frame)) {
            cerr << "Unable to read next frame." << endl;
            cerr << "Exiting..." << endl;
            exit(EXIT_FAILURE);
        }

        //update the background model
        pBS->apply(frame, fgMask);

        //get the frame number and write it on the current frame
        stringstream ss;
        rectangle(frame, cv::Point(10, 2), cv::Point(100,20), cv::Scalar(255,255,255), -1);
        ss << capture.get(CAP_PROP_POS_FRAMES);
        string frameNumberString = ss.str();
        putText(frame, frameNumberString.c_str(), cv::Point(15, 15),
                FONT_HERSHEY_SIMPLEX, 0.5 , cv::Scalar(0,0,0));
        
        //show the current frame and the fg masks
        imshow("Frame", frame);
        imshow("FG Mask", fgMask);

        // get keyboard input
        keyboard = waitKey( 30 );
    }
    //delete capture object
    capture.release();
}


