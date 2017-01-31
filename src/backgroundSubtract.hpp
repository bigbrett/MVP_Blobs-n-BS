//opencv
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/video.hpp>
#include <opencv2/bgsegm.hpp>
//C++
#include <iostream>
#include <sstream>

using namespace cv;
using namespace std;

#define BS_METHOD 1

void backgroundSubtract(char* videoFilename);
