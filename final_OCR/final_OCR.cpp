# include <iostream>
# include "opencv/cv.h"
# include "opencv/cxcore.h"
# include "opencv/highgui.h"

using namespace std ;
using namespace cv ;

int main(){

	string filename;
	cout << "Enter File name:" << endl;
	cin >> filename ;
    Mat src = imread(filename);
    if (src.empty())
        return -1;

    imshow("src", src);

    waitKey(0);

    return 0;
}