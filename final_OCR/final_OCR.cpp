# include "opencv/cxcore.h"
# include "opencv/highgui.h"
# include "opencv/cv.h"
# include <iostream>
# include <vector>
# include <algorithm>

using namespace std;

typedef struct LetterLocation {
	int height ;
	int width ;
}LetterLocation;

LetterLocation letterLocation[52] = {
	{ 120, 115 },
	{ 120, 164 },
	{ 120, 217 },
	{ 120, 270 },
	{ 120, 323 },
	{ 120, 370 },
	{ 120, 420 },
	{ 120, 475 },
	{ 210, 103 },
	{ 210, 130 },
	{ 210, 175 },
	{ 210, 224 },
	{ 210, 270 },
	{ 210, 327 },
	{ 210, 380 },
	{ 210, 433 },
	{ 300, 124 },
	{ 300, 175 },
	{ 300, 227 },
	{ 300, 274 },
	{ 300, 323 },
	{ 300, 374 },
	{ 300, 433 },
	{ 392, 115 },
	{ 392, 163 },
	{ 392, 210 },
	{ 486, 112 },
	{ 486, 155 },
	{ 486, 195 },
	{ 486, 232 },
	{ 486, 273 },
	{ 486, 306 },
	{ 486, 335 },
	{ 486, 376 },
	{ 486, 403 },
	{ 486, 419 },
	{ 486, 443 },
	{ 486, 472 },
	{ 585, 123 },
	{ 585, 175 },
	{ 585, 213 },
	{ 585, 256 },
	{ 585, 296 },
	{ 585, 331 },
	{ 585, 358 },
	{ 585, 389 },
	{ 585, 417 },
	{ 585, 457 },
	{ 673, 119 },
	{ 673, 163 },
	{ 673, 197 },
	{ 673, 233 }
} ;


void inverseImage( IplImage * img ) {
	for ( int i = 0 ; i < img->height ; i++ ) {
		for ( int j = 0 ; j < img->width ; j++ ) {
			cvSet2D( img, i, j, cvScalar( 255 - cvGet2D( img, i, j ).val[0], 255 - cvGet2D( img, i, j ).val[1], 255 - cvGet2D( img, i, j ).val[2] ) ) ;
		}
	}
}

bool compareX( CvSeq* s1, CvSeq* s2 ) {
	return cvBoundingRect( s1 ).x < cvBoundingRect( s2 ).x ;
}

float countAngle( vector<CvSeq*> contourVec ) {

	float * diff = new float[contourVec.size( ) - 1] ;

	for ( int i = 0 ; i < contourVec.size( ) - 1 ; i++ ) {

		float a = cvBoundingRect( contourVec.at( i ) ).y + cvBoundingRect( contourVec.at( i ) ).height ;
		float b = cvBoundingRect( contourVec.at( i + 1 ) ).y + cvBoundingRect( contourVec.at( i + 1 ) ).height ;

		diff[i] = b - a ;

	}

	float x = 0 ;
	float y = 0 ;
	for ( int i = 0 ; i < contourVec.size( ) - 1 ; i++ ) {
		if ( diff[i] > 5 || diff[i] < -5 ) continue ;

		x++ ;
		y += diff[i] ;
	}

	return y / x - 0.2 ;
}

vector<CvSeq*> doFindContour( IplImage *src ) {

	/// find Contours src
	IplImage* temp = cvCreateImage( cvGetSize( src ), src->depth, src->nChannels );
	cvCopy( src, temp ) ;
	CvMemStorage* storage = cvCreateMemStorage( 0 );
	CvSeq* contour = 0;
	cvFindContours( temp, storage, &contour, sizeof( CvContour ), CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE );
	//cvZero( dst ); //clear

	/// get Contours and sort by horizontal
	vector<CvSeq*> contourVec ;
	// int count=0;
	for ( ; contour != 0; contour = contour->h_next ) {
		// 把介於 1500 ~ 50 面積的放到 vector // Brian Modify
		if ( 1200 > cvContourArea( contour, CV_WHOLE_SEQ, 0 ) && cvContourArea( contour, CV_WHOLE_SEQ, 0 ) > 50 )
			contourVec.push_back( contour ) ;

	}

	// 因為不是從右到左，所以先排序一次
	sort( contourVec.begin( ), contourVec.end( ), compareX ) ;


	return contourVec ;
}


IplImage * rotateImage( IplImage *src ) {

	vector<CvSeq*> contourVec = doFindContour( src ) ;

	cvZero( src ) ;

	for ( int i = 0 ; i < contourVec.size( ) ; i++ ) {
		cvDrawContours( src, contourVec.at( i ), CV_RGB( 255, 255, 255 ), CV_RGB( 255, 255, 255 ), -1, CV_FILLED, 8 );
	}

	float angle = countAngle( contourVec ) ;

	float m[6];

	m[0] = ( float ) ( cos( -angle * 2 * CV_PI / 180. ) );
	m[1] = ( float ) ( sin( -angle * 2 * CV_PI / 180. ) );
	m[3] = -m[1];
	m[4] = m[0];

	//將旋轉中心移至圖像中間
	m[2] = src->width * 0.5f;
	m[5] = src->height * 0.5f;


	CvMat M = cvMat( 2, 3, CV_32F, m );


	IplImage *pImgDst = cvCreateImage( cvGetSize( src ), src->depth, src->nChannels ) ;
	cvZero( pImgDst ) ;
	cvGetQuadrangleSubPix( src, pImgDst, &M );

	cvCopy( pImgDst, src ) ;

	return src ;

}

bool isInRect( IplImage * testImg, CvPoint min_loc, int index ) {

	if ( ( min_loc.x < letterLocation[index].width && letterLocation[index].width < min_loc.x + testImg->width ) &&
		 ( min_loc.y < letterLocation[index].height && letterLocation[index].height < min_loc.y + testImg->height ) )
		 return true ;

	return false ;
}

char doTemplateMatch( IplImage * templateImg, IplImage * testImg ) {

	/// template match
	int result_width = templateImg->width - testImg->width + 1;
	int result_height = templateImg->height - testImg->height + 1;

	IplImage * resultImg = cvCreateImage( cvSize( result_width, result_height ), 32, 1 );
	cvMatchTemplate( templateImg, testImg, resultImg, CV_TM_SQDIFF ) ;

	double min_val;
	double max_val;
	CvPoint min_loc;
	CvPoint max_loc;
	cvMinMaxLoc( resultImg, &min_val, &max_val, &min_loc, &max_loc, NULL );

	cvRectangle( templateImg, cvPoint( min_loc.x, min_loc.y ), cvPoint( ( min_loc.x + testImg->width ), ( min_loc.y + testImg->height ) ), CV_RGB( 0, 255, 0 ), 1 );
	cvRectangle( templateImg, cvPoint( max_loc.x, max_loc.y ), cvPoint( ( max_loc.x + testImg->width ), ( max_loc.y + testImg->height ) ), CV_RGB( 255, 0, 0 ), 1 );

	//cout << "min_loc.x " << min_loc.x << endl ;
	//cout << "min_loc.y " << min_loc.y << endl ;
	//cout << "width " << min_loc.x + testImg->width << endl ;
	//cout << "height " << min_loc.y + testImg->height << endl ;


	for ( int i = 0 ; i < 52 ; i++ ) {
		if ( isInRect( testImg, min_loc, i ) )
			return ( i < 26 ) ? ( 'A' + i ) : ( 'a' + i - 26 ) ;
	}

	return '@' ;
}

int main( int argc, char** argv ) {

	char * fileName = ( argc >= 2 ) ? argv[1] : "Train01.bmp" ;

	IplImage * inputImg = cvLoadImage( fileName, CV_LOAD_IMAGE_GRAYSCALE ) ;

	if ( inputImg == NULL ) {
		cout << "image not found" << endl ;
		system( "pause" ) ;
		return 1 ;
	}

	/// cut image rect I want 
	cvSetImageROI( inputImg, cvRect( inputImg->width / 6, inputImg->height / 4, inputImg->width / 6 * 4, inputImg->height / 3 ) ) ; // 設定我要圖片的範圍
	IplImage * temp = cvCreateImage( cvGetSize( inputImg ), inputImg->depth, inputImg->nChannels ) ;
	cvCopy( inputImg, temp, NULL );
	//cvReleaseImage( &src );
	inputImg = temp ;

	/// pro-processing src
	cvSmooth( inputImg, inputImg, CV_MEDIAN, 3, 3, 0, 0 ); // 去雜訊
	cvThreshold( inputImg, inputImg, 80, 255, CV_THRESH_BINARY ); // 2值化, 80以上設為255, 以下設為0，設小是因為可以把兩個單字黏住的部分分開
	inverseImage( inputImg ) ; // 黑白反轉

	// rotate image
	rotateImage( inputImg ) ;

	//cvNamedWindow( "inputImg", CV_WINDOW_AUTOSIZE ) ;
	//cvShowImage( "inputImg", inputImg ) ;

	vector<CvSeq*> contourVec = doFindContour( inputImg ) ;

	//----------------------------------------------------------------------------------------------------------------------


	/// get template image
	IplImage * templateImg = cvLoadImage( "Template.bmp", CV_LOAD_IMAGE_COLOR ) ;
	for ( int i = 0 ; i < templateImg->height ; i++ ) {
		for ( int j = 0 ; j < templateImg->width ; j++ ) {
			cvSet2D( templateImg, i, j, cvScalar( 255 - cvGet2D( templateImg, i, j ).val[0], 255 - cvGet2D( templateImg, i, j ).val[1], 255 - cvGet2D( templateImg, i, j ).val[2] ) ) ;
			// cvSet2D( templateImg, i, j, cvScalar( 255 - cvGet2D( templateImg, i, j ).val[0] ) ) ;
		}
	}


	//for ( int i = 0 ; i < contourVec.size( ) ; i++ ) {
	//	cvDrawContours( inputImg, contourVec.at( i ), CV_RGB( 255, 255, 255 ), CV_RGB( 255, 255, 255 ), -1, CV_FILLED, 8 );

	//	CvRect rect = cvBoundingRect( contourVec.at( i ) ) ;
	//	cvRectangle( inputImg, cvPoint( rect.x, rect.y ), cvPoint( rect.x + rect.width, rect.y + rect.height ), CV_RGB( 255, 255, 255 ) ) ;

	//}

	//cvNamedWindow( "inputImg", CV_WINDOW_AUTOSIZE ) ;
	//cvShowImage( "inputImg", inputImg ) ;

	float scaleOfTemplate = 0.0 ;

	for ( int i = 0 ; i < contourVec.size( ) /*contourVec.size()*/ ; i++ ) {
		/// get test image
		IplImage * tempInputImg = cvCreateImage( cvGetSize( inputImg ), inputImg->depth, 3 ) ;
		cvCvtColor( inputImg, tempInputImg, CV_GRAY2BGR ) ;

		CvRect rect = cvBoundingRect( contourVec.at( i ) ) ;
		cvSetImageROI( tempInputImg, rect ) ; // 設定我要圖片的範圍

		
		scaleOfTemplate = ( i == 0 ) ? ( 53.0f / ( float ) rect.height ) : scaleOfTemplate ;
		int tempWidth = ( int ) ( ( float ) rect.width * scaleOfTemplate );
		int tempHeight = ( int ) ( ( float ) rect.height * scaleOfTemplate );
		if ( tempHeight < 53 ) tempWidth++, tempHeight++;
		IplImage * testImg = cvCreateImage( cvSize( tempWidth, tempHeight ), tempInputImg->depth, 3 ) ;

		//cvNamedWindow( "testImg", CV_WINDOW_AUTOSIZE ) ;
		//cvShowImage( "testImg", testImg ) ;

		// IplImage * testImg = cvCreateImage( cvSize( rect.width, rect.height ), inputImg->depth, inputImg->nChannels ) ;
		cvResize( tempInputImg, testImg, CV_INTER_LINEAR );

		cout << doTemplateMatch( templateImg, testImg ) ;

		cvResetImageROI( tempInputImg );
		cvReleaseImage( &tempInputImg );
	}

	cout << endl ;

	// cvWaitKey( 0 );

	//cvReleaseImage( &inputImg );
	//cvDestroyWindow( "inputImg" );


	system( "pause" ) ;

	return 0 ;
}