/*
 1.讀到src
 黑白反轉
 Get Contours

 2.讀比對用的abcd
 黑白反轉
 Get Contours

 3.cvMatchShapes()
 0.4 0 之間


 --> 改變想法，我會先根據一個range，找出是單字的部分
 因為他的順序並不是由右而左，所以我用cvBoundingRect來找

*/
# include <iostream>
# include "opencv/cv.h"
# include "opencv/cxcore.h"
# include "opencv/highgui.h"
# include <vector>

using namespace std ;
using namespace cv ;

IplImage * letterArray[52] ;

float similarityValue[52] = // 相似度
{0.4, 0.1, -1, 0.1, 2.3, -1, 3.5, 2.0, -1, -1, -1, -1, -1, 
-1, -1, -1, -1, -1, -1, -1, -1, 1.0, -1, 1.6, -1, 3.0,

0.1, 0.1, -1, -1, -1, -1, 0.4, -1, -1, -1, -1, -1, -1, 
-1, 0.1, 0.4, -1, -1, 1, 5, 1.5, -1, -1, -1, -1, -1 } ;

void readLetterArray() {

	char letterName[32] ;
	for ( int i = 0 ; i < 26 ; i++ ) { // upper case
		sprintf( letterName, "LetterSet/U_%c.bmp", ( 'A' + i ) ) ;
		letterArray[i] = cvLoadImage( letterName, CV_LOAD_IMAGE_GRAYSCALE ) ;
	}
	for ( int i = 0 ; i < 26 ; i++ ) { // lower case
		sprintf( letterName, "LetterSet/U_%c.bmp", ( 'a' + i ) ) ;
		letterArray[i+26] = cvLoadImage( letterName, CV_LOAD_IMAGE_GRAYSCALE ) ;
	}

}

void inverseImage( IplImage * img ) {
	for ( int i = 0 ; i < img->height ; i++	) {
		for ( int j = 0 ; j < img->width ; j++ ) {
			cvSet2D( img, i, j, cvScalar( 255 - cvGet2D(img, i, j ).val[0] ) ) ;
		} 
	}
} 

bool compareX( CvSeq* s1, CvSeq* s2 ) {
	return cvBoundingRect( s1 ).x < cvBoundingRect( s2 ).x ;
} 


CvSeq* getTestContour(IplImage* test) {

	cvThreshold( test, test, 120, 255, CV_THRESH_BINARY );// 2值化, 120以上設為255, 以下設為0
	cvSmooth(test,test,CV_MEDIAN,3,0,0,0); // 去雜訊
	inverseImage( test ) ; // 黑白反轉

	/// find Contours test
	CvMemStorage* storageA = cvCreateMemStorage(0);
	CvSeq* contourTest = 0;
	cvFindContours( test, storageA, &contourTest, sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE );

	return contourTest ;
}

char matchLetter( CvSeq* input ) {
	// 會輸入一個字
	// fnction 輸出 比對到的字
	
	for ( int i = 0 ; i < 52 ; i++ ) {
		// 取得要比對的 contour
		CvSeq* contourTest = getTestContour( letterArray[i] ) ;
		double answer = cvMatchShapes(input, contourTest, CV_CONTOURS_MATCH_I1, 0 );


		if ( answer < similarityValue[i] ) {
			cout << i << "\t" ;
			return ( i < 26 ) ? ( 'A' + i ) : ( 'a' + i - 26 ) ;
		}
	}

	// 沒偵測到
	return '＠' ;
}


int main( int argc, char** argv ){

	// 一開始會先把圖片讀進來
	readLetterArray() ;

	char * fileName = ( argc >= 2 ) ? argv[1] : "Train02.bmp" ;

	IplImage *src = cvLoadImage( fileName, CV_LOAD_IMAGE_GRAYSCALE ) ;

	if ( src == NULL ) {
		cout << "image not found" << endl ;
		system( "pause" ) ;
		return 1 ;
	}

	/// cut image rect I want 
    cvSetImageROI( src, cvRect( src->width/6, src->height/4, src->width/6*4 , src->height/4 ) ) ; // 設定我要圖片的範圍
	IplImage * temp = cvCreateImage(cvGetSize(src), src->depth, src->nChannels) ;
	cvCopy(src, temp, NULL);
	cvReleaseImage(&src);
	src = temp ;

	/// pro-processing src
	cvSmooth(src,src,CV_MEDIAN,3,3,0,0); // 去雜訊
	cvThreshold( src, src, 80, 255, CV_THRESH_BINARY ); // 2值化, 80以上設為255, 以下設為0，設小是因為可以把兩個單字黏住的部分分開
	inverseImage( src ) ; // 黑白反轉
	
	/// find Contours src
	IplImage* dst = cvCreateImage( cvGetSize(src), 8, 3 );
	CvMemStorage* storage = cvCreateMemStorage(0);
	CvSeq* contour = 0;
	cvFindContours( src, storage, &contour, sizeof(CvContour), CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE );
	cvZero( dst ); //clear

	//----------------------------------------------------------------------------------------------------------

	/// get Contours and sort by horizontal

	vector<CvSeq*> contourVec ;

	// int count=0;
	for( ; contour != 0; contour = contour->h_next ){
		// 把介於 1000 ~ 50 面積的放到 vector
		if ( 1000 > cvContourArea(contour ,CV_WHOLE_SEQ, 0) && cvContourArea(contour ,CV_WHOLE_SEQ, 0) > 50 )
			contourVec.push_back( contour ) ;

	}

	// 因為不是從右到左，所以先排序一次
	sort( contourVec, compareX ) ;

	// 依照排序左到右做 match
	for ( int i = 0 ; i < contourVec.size() ; i++ ){
		cout << matchLetter( contourVec.at(i) ) << endl ;
	}



    cvNamedWindow( "src",WINDOW_AUTOSIZE ) ;
    cvShowImage( "src", src ) ;

	//cvNamedWindow( "ua",WINDOW_AUTOSIZE ) ;
	//cvShowImage( "ua", ua ) ;

	cvNamedWindow( "Components", WINDOW_AUTOSIZE );
	cvShowImage( "Components", dst );
 
    cvWaitKey(0);
 
    cvReleaseImage(&src);
    cvDestroyWindow("src");

    //cvReleaseImage(&ua);
    //cvDestroyWindow("ua");

	cvReleaseImage(&dst);
    cvDestroyWindow("Components");

    return 0 ;
}
