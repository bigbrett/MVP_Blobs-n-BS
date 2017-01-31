#include "backgroundSubtract.hpp"
#include "blobdetect.hpp"
#include <iostream>

using namespace std; 
using namespace cv;

int main(void)
{
    blobdetect("/Users/bretttt/iCloud_drive/School/17W/CS189/project/sandbox/MVPvideo/data/MVPdrills.mov");
    // backgroundSubtract("/Users/bretttt/iCloud_drive/School/17W/CS189/project/sandbox/MVPvideo/data/MVPdrills.mov");
    return 0;
}