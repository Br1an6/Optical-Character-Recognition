/*
 1.讀到src
 黑白反轉
 Get Contours

 2.讀比對用的abcd
 黑白反轉
 Get Contours

 3.cvMatchShapes()
 0.4 0 之間
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
	cvThreshold( src, src,120, 255, CV_THRESH_BINARY ); // 2值化

	cvSmooth(src,src,CV_MEDIAN,3,0,0,0); // 去雜訊
	Mat res(src);
	// reverse pixel(black and white)
	for( int i= 0; i< res.rows; i++)  {  
		for( int j=0; j< res.cols * res.channels(); j++)  {  
            res.at<uchar>(i, j)= 255- res.at<uchar>(i, j);   
        }  
    }  
	src = &res.operator IplImage(); 
	cvNamedWindow( "Source", 1 );
	cvShowImage( "Source", src );
	// Find Contours
	cvFindContours( src, storage, &contour, sizeof(CvContour), CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE );
	cvZero( dst );//clear
	CvSeq* _contour =contour; 
	int count=0;

	CvSeq* contourA = 0;
	IplImage* ua;
	CvMemStorage* storageA = cvCreateMemStorage(0);
	ua=cvLoadImage("U_A.bmp",CV_LOAD_IMAGE_GRAYSCALE);
	cvSmooth(ua,ua,CV_MEDIAN,3,0,0,0);
	cvThreshold( ua, ua,120, 255, CV_THRESH_BINARY );
	// reverse pixel(black and white)
	Mat resA(ua);
	for( int i= 0; i< resA.rows; i++)  {  
		for( int j=0; j< resA.cols * resA.channels(); j++)  {  
            resA.at<uchar>(i, j)= 255- resA.at<uchar>(i, j);   // reverse pixel(black and white)
        }  
    }  
	ua = &resA.operator IplImage(); 
	cvFindContours( ua, storageA, &contourA, sizeof(CvContour), CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE );
	cvNamedWindow( "a", 1 );
	cvShowImage( "a", ua );
	// Draw Contours
	for( ; contour != 0; contour = contour->h_next ){
		count++;
		CvScalar color = CV_RGB( 255, 255, 255);
		double answer = cvMatchShapes(contour, contourA, CV_CONTOURS_MATCH_I1, 0 );
		cout << answer << endl;
		if ( 0.4 > answer && answer >= 0)
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