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
# include <iostream>
# include "opencv/cv.h"
# include "opencv/cxcore.h"
# include "opencv/highgui.h"
# include <vector>

using namespace std ;
using namespace cv ;

void inverseImage( IplImage * img ) {
	for ( int i = 0 ; i < img->height ; i++	) {
		for ( int j = 0 ; j < img->width ; j++ ) {
			cvSet2D( img, i, j, cvScalar( 255 - cvGet2D(img, i, j ).val[0] ) ) ;
		} 
	}
} 

int main( int argc, char** argv )
{

	IplImage *src = cvLoadImage( "TrainTest.bmp", CV_LOAD_IMAGE_GRAYSCALE ) ;

	if ( src == NULL ) {
		cout << "image not found" << endl ;
		system( "pause" ) ;
		return 1 ;
	}

	/// cut image rect I want 
    cvSetImageROI( src, cvRect( src->width/6, src->height/4, src->width/6*4 , src->height/4*2 ) ) ; // 設定我要圖片的範圍
	IplImage * temp = cvCreateImage(cvGetSize(src), src->depth, src->nChannels) ;
	cvCopy(src, temp, NULL);
	cvReleaseImage(&src);
	src = temp ;

	/// pro-processing src
	cvSmooth(src,src,CV_MEDIAN,3,0,0,0); // 去雜訊
	cvThreshold( src, src,120, 255, CV_THRESH_BINARY ); // 2值化, 120以上設為255, 以下設為0
	inverseImage( src ) ; // 黑白反轉
	
	/// find Contours src
	IplImage* dst = cvCreateImage( cvGetSize(src), 8, 3 );
	CvMemStorage* storage = cvCreateMemStorage(0);
	CvSeq* contour = 0;
	cvFindContours( src, storage, &contour, sizeof(CvContour), CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE );
	cvZero( dst ); //clear

	//----------------------------------------------------------------------------------------------------------

	/// compare image
	IplImage* ua;
	ua=cvLoadImage("g.bmp",CV_LOAD_IMAGE_GRAYSCALE);

	/// pro-processing us
	cvSmooth(ua,ua,CV_MEDIAN,3,0,0,0); // 去雜訊
	cvThreshold( ua, ua,120, 255, CV_THRESH_BINARY );// 2值化, 120以上設為255, 以下設為0
	inverseImage( ua ) ; // 黑白反轉
	
	/// find Contours ua
	CvMemStorage* storageA = cvCreateMemStorage(0);
	CvSeq* contourA = 0;
	cvFindContours( ua, storageA, &contourA, sizeof(CvContour), CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE );
	

	/// Draw Contours
	int count=0;
	for( ; contour != 0; contour = contour->h_next ){
		count++;
		CvScalar color = CV_RGB( 255, 255, 255);
		double answer = cvMatchShapes(contour, contourA, CV_CONTOURS_MATCH_I1, 0 );
		cout << answer << endl;
		if ( 0.4 > answer && answer >= 0 && cvContourArea(contour ,CV_WHOLE_SEQ, 0) > 10 )
		 	cvDrawContours( dst, contour, color, color, -1, CV_FILLED, 8 );

		//if ( count == 1 ) 
		//	cvDrawContours( dst, contour, color, color, -1, CV_FILLED, 8 );
	}


    cvNamedWindow( "src",WINDOW_AUTOSIZE ) ;
    cvShowImage( "src", src ) ;

	cvNamedWindow( "ua",WINDOW_AUTOSIZE ) ;
    cvShowImage( "ua", ua ) ;

	cvNamedWindow( "Components", WINDOW_AUTOSIZE );
	cvShowImage( "Components", dst );
 
    cvWaitKey(0);
 
    cvReleaseImage(&src);
    cvDestroyWindow("src");

    cvReleaseImage(&ua);
    cvDestroyWindow("ua");

	cvReleaseImage(&dst);
    cvDestroyWindow("Components");

    return 0 ;
}
