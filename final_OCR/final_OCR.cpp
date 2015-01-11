/*
 1.Get Contours
*/
#include <iostream>
# include "opencv/cv.h"
# include "opencv/cxcore.h"
# include "opencv/highgui.h"
#include <vector>
using namespace std ;
using namespace cv ;

int main( int argc, char** argv )
{
	string filename ;
	cout << "Enter File name:" << endl;
	cin >> filename ;
	IplImage* src;
	src=cvLoadImage(filename.c_str(),CV_LOAD_IMAGE_GRAYSCALE);
	IplImage* dst = cvCreateImage( cvGetSize(src), 8, 3 );
	CvMemStorage* storage = cvCreateMemStorage(0);
	CvSeq* contour = 0;
	cvThreshold( src, src,120, 255, CV_THRESH_BINARY );
	cvNamedWindow( "Source", 1 );
	cvShowImage( "Source", src );
	// Find Contours
	cvFindContours( src, storage, &contour, sizeof(CvContour), CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE );
	cvZero( dst );//clear
	CvSeq* _contour =contour; 
	int count=0;
	// Draw Contours
	for( ; contour != 0; contour = contour->h_next ){
		count++;
		double tmparea=fabs(cvContourArea(contour));

		CvScalar color = CV_RGB( 255, 255, 255);
		cvDrawContours( dst, contour, color, color, -1, CV_FILLED, 8 );
	}
	printf("The total number of contours is:%d",count);
	cvNamedWindow( "Components", 1 );
	cvShowImage( "Components", dst );
	cvWaitKey(0);
	cvDestroyWindow( "Source" );
	cvReleaseImage(&src);
	cvDestroyWindow( "Components" );
	cvReleaseImage(&dst);
	return 0;
}