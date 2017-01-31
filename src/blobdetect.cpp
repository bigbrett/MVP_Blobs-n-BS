#include "blobdetect.hpp"

const cv::Scalar SCALAR_BLACK = cv::Scalar(0.0, 0.0, 0.0);
const cv::Scalar SCALAR_WHITE = cv::Scalar(255.0, 255.0, 255.0);
const cv::Scalar SCALAR_YELLOW = cv::Scalar(0.0, 255.0, 255.0);
const cv::Scalar SCALAR_GREEN = cv::Scalar(0.0, 200.0, 0.0);
const cv::Scalar SCALAR_RED = cv::Scalar(0.0, 0.0, 255.0);

/*
 * detect blobs in the current image 
 */
int blobdetect(char* videoFilename) 
{

    cv::VideoCapture capVideo;

    cv::Mat imgFrame1;
    cv::Mat imgFrame2;

    std::vector<Blob> blobs;

    capVideo.open(videoFilename);

    if (!capVideo.isOpened()) {                                                 // if unable to open video file
        std::cout << "error reading video file" << std::endl << std::endl;      // show error message
        return(0);                                                              // and exit program
    }

    if (capVideo.get(CV_CAP_PROP_FRAME_COUNT) < 2) {
        std::cout << "error: video file must have at least two frames";
        return(0);
    }

    capVideo.read(imgFrame1);
    capVideo.read(imgFrame2);

    char chCheckForEscKey = 0;

    bool blnFirstFrame = true;

    int frameCount = 2;

    while (capVideo.isOpened() && chCheckForEscKey != 27) {

        // holds the blobs detected in the current frame 
        std::vector<Blob> currentFrameBlobs;

        // copies of the frames for processing
        cv::Mat imgFrame1Copy = imgFrame1.clone();
        cv::Mat imgFrame2Copy = imgFrame2.clone();

        cv::Mat imgDifference;
        cv::Mat imgThresh;

        // convert from BGR to grayscale 
        cv::cvtColor(imgFrame1Copy, imgFrame1Copy, CV_BGR2GRAY);
        cv::cvtColor(imgFrame2Copy, imgFrame2Copy, CV_BGR2GRAY);

        // apply a gaussian blur to reduce noise 
        cv::GaussianBlur(imgFrame1Copy, imgFrame1Copy, cv::Size(5, 5), 0);
        cv::GaussianBlur(imgFrame2Copy, imgFrame2Copy, cv::Size(5, 5), 0);

        // take the difference between the two frames and apply threshold 
        cv::absdiff(imgFrame1Copy, imgFrame2Copy, imgDifference);
        cv::threshold(imgDifference, imgThresh, 30, 255.0, CV_THRESH_BINARY);
        cv::imshow("imgThresh", imgThresh);

        cv::Mat structuringElement3x3 = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
        cv::Mat structuringElement5x5 = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));
        cv::Mat structuringElement7x7 = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(7, 7));
        cv::Mat structuringElement9x9 = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(9, 9));

        /*
        cv::dilate(imgThresh, imgThresh, structuringElement7x7);
        cv::erode(imgThresh, imgThresh, structuringElement3x3);
        */

        // dilate and erode the thresholded image 
        cv::dilate(imgThresh, imgThresh, structuringElement5x5);
        cv::dilate(imgThresh, imgThresh, structuringElement5x5);
        cv::erode(imgThresh, imgThresh, structuringElement5x5);

        cv::Mat imgThreshCopy = imgThresh.clone();

        // find the contour lines of the threshold image 
        std::vector<std::vector<cv::Point> > contours;
        cv::findContours(imgThreshCopy, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
        drawAndShowContours(imgThresh.size(), contours, "imgContours");

        // find the contour lines of the convex hull blobs 
        std::vector<std::vector<cv::Point> > convexHulls(contours.size());
        for (unsigned int i = 0; i < contours.size(); i++) 
            cv::convexHull(contours[i], convexHulls[i]);
        drawAndShowContours(imgThresh.size(), convexHulls, "imgConvexHulls");


        for (auto &convexHull : convexHulls) 
        {
            Blob possibleBlob(convexHull);

            if (possibleBlob.currentBoundingRect.area() > 100 &&
                possibleBlob.dblCurrentAspectRatio >= 0.2 &&
                possibleBlob.dblCurrentAspectRatio <= 1.25 &&
                possibleBlob.currentBoundingRect.width > 20 &&
                possibleBlob.currentBoundingRect.height > 20 &&
                possibleBlob.dblCurrentDiagonalSize > 30.0 &&
                (cv::contourArea(possibleBlob.currentContour) / (double)possibleBlob.currentBoundingRect.area()) > 0.40) {
                currentFrameBlobs.push_back(possibleBlob);
            }
        }
        drawAndShowContours(imgThresh.size(), currentFrameBlobs, "imgCurrentFrameBlobs");

        if (blnFirstFrame == true) 
        {
            for (auto &currentFrameBlob : currentFrameBlobs) 
                blobs.push_back(currentFrameBlob);
        }
        else 
            matchCurrentFrameBlobsToExistingBlobs(blobs, currentFrameBlobs);
        drawAndShowContours(imgThresh.size(), blobs, "imgBlobs");

        // get another copy of frame 2 since we changed the previous frame 2 copy in the processing above
        imgFrame2Copy = imgFrame2.clone();          

        // draw blob info 
        drawBlobInfoOnImage(blobs, imgFrame2Copy);
        cv::imshow("imgFrame2Copy", imgFrame2Copy);
        currentFrameBlobs.clear();

        // move frame 1 up to where frame 2 is
        imgFrame1 = imgFrame2.clone();           // move frame 1 up to where frame 2 is

        if ((capVideo.get(CV_CAP_PROP_POS_FRAMES) + 2) < capVideo.get(CV_CAP_PROP_FRAME_COUNT)) 
            capVideo.read(imgFrame2);
        else {
            std::cout << "end of video\n";
            break;
        }

        blnFirstFrame = false;
        frameCount++;
        chCheckForEscKey = cv::waitKey(1);
    }

    if (chCheckForEscKey != 27) {               // if the user did not press esc (i.e. we reached the end of the video)
        cv::waitKey(0);                         // hold the windows open to allow the "end of video" message to show
    }
    // note that if the user did press esc, we don't need to hold the windows open, we can simply let the program end which will close the windows

    return(0);
}

/*
 * Matches the existing blobs to those detected in the current frame
 */
void matchCurrentFrameBlobsToExistingBlobs(std::vector<Blob> &existingBlobs, std::vector<Blob> &currentFrameBlobs) {

    // initialize each existing blob to the unmatched state, and 
    // predict its next position 
    for (auto &existingBlob : existingBlobs) 
    {
        existingBlob.blnCurrentMatchFoundOrNewBlob = false;
        existingBlob.predictNextPosition();
    }

    // for each blob in the current frame...
    for (auto &currentFrameBlob : currentFrameBlobs) 
    {
        int intIndexOfLeastDistance = 0;
        double dblLeastDistance = 100000.0;

        for (unsigned int i = 0; i < existingBlobs.size(); i++) 
        {
            // find the closest blob to it
            if (existingBlobs[i].blnStillBeingTracked == true) 
            {
                double dblDistance = distanceBetweenPoints(currentFrameBlob.centerPositions.back(), existingBlobs[i].predictedNextPosition);

                if (dblDistance < dblLeastDistance) 
                {
                    dblLeastDistance = dblDistance;
                    intIndexOfLeastDistance = i;
                }
            }
        }

        // if the distance to the blob is sufficiently close, it is the same blob. If it is farther,
        // register it as a new blob 
        if (dblLeastDistance < currentFrameBlob.dblCurrentDiagonalSize * 1.15) 
            addBlobToExistingBlobs(currentFrameBlob, existingBlobs, intIndexOfLeastDistance);
        else {
            addNewBlob(currentFrameBlob, existingBlobs);
        }
    }

    // toss out blobs that have not been matched in the last 5 frames
    for (auto &existingBlob : existingBlobs) 
    {
        if (existingBlob.blnCurrentMatchFoundOrNewBlob == false) 
            existingBlob.intNumOfConsecutiveFramesWithoutAMatch++;

        if (existingBlob.intNumOfConsecutiveFramesWithoutAMatch >= 5) 
            existingBlob.blnStillBeingTracked = false;
    }

}


/*
 *
 */
void addBlobToExistingBlobs(Blob &currentFrameBlob, std::vector<Blob> &existingBlobs, int &intIndex) 
{
    existingBlobs[intIndex].currentContour = currentFrameBlob.currentContour;
    existingBlobs[intIndex].currentBoundingRect = currentFrameBlob.currentBoundingRect;

    existingBlobs[intIndex].centerPositions.push_back(currentFrameBlob.centerPositions.back());

    existingBlobs[intIndex].dblCurrentDiagonalSize = currentFrameBlob.dblCurrentDiagonalSize;
    existingBlobs[intIndex].dblCurrentAspectRatio = currentFrameBlob.dblCurrentAspectRatio;

    existingBlobs[intIndex].blnStillBeingTracked = true;
    existingBlobs[intIndex].blnCurrentMatchFoundOrNewBlob = true;
}

/*
 *
 */
void addNewBlob(Blob &currentFrameBlob, std::vector<Blob> &existingBlobs) 
{
    currentFrameBlob.blnCurrentMatchFoundOrNewBlob = true;
    existingBlobs.push_back(currentFrameBlob);
}


/*
 * 
 */
double distanceBetweenPoints(cv::Point point1, cv::Point point2) 
{
    int intX = abs(point1.x - point2.x);
    int intY = abs(point1.y - point2.y);
    return(sqrt(pow(intX, 2) + pow(intY, 2)));
}


/*
 *
 */
void drawAndShowContours(cv::Size imageSize, std::vector<std::vector<cv::Point> > contours, std::string strImageName) 
{
    cv::Mat image(imageSize, CV_8UC3, SCALAR_BLACK);
    cv::drawContours(image, contours, -1, SCALAR_WHITE, -1);
    cv::imshow(strImageName, image);
}


/*
 *
 */
void drawAndShowContours(cv::Size imageSize, std::vector<Blob> blobs, std::string strImageName) 
{
    cv::Mat image(imageSize, CV_8UC3, SCALAR_BLACK);
    std::vector<std::vector<cv::Point> > contours;

    for (auto &blob : blobs) 
    {
        if (blob.blnStillBeingTracked == true) 
            contours.push_back(blob.currentContour);
    }

    cv::drawContours(image, contours, -1, SCALAR_WHITE, -1);
    cv::imshow(strImageName, image);
}


/*
 *
 */
void drawBlobInfoOnImage(std::vector<Blob> &blobs, cv::Mat &imgFrame2Copy) 
{
    for (unsigned int i = 0; i < blobs.size(); i++) 
    {
        if (blobs[i].blnStillBeingTracked == true) 
        {
            cv::rectangle(imgFrame2Copy, blobs[i].currentBoundingRect, SCALAR_RED, 2);
            // int intFontFace = CV_FONT_HERSHEY_SIMPLEX;
            // double dblFontScale = blobs[i].dblCurrentDiagonalSize / 60.0;
            // int intFontThickness = (int)std::round(dblFontScale * 1.0);
            // cv::putText(imgFrame2Copy, std::to_string(i), blobs[i].centerPositions.back(), intFontFace, dblFontScale, SCALAR_GREEN, intFontThickness);
        }
    }
}
