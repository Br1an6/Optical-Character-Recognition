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
 --> 改為全部比對完找最接近的字母

*/
# include <iostream>
# include "opencv/cv.h"
# include "opencv/cxcore.h"
# include "opencv/highgui.h"
# include <vector>

using namespace std ;
using namespace cv ;

IplImage * letterArray[52] ;
class MatchResults{
	public:
	int id;
	double matchResult;
	void Init(int sId, double sMatchResult){
		id = sId, matchResult = sMatchResult;}
};
/*
float similarityValue[52] = // 相似度
{0.4, 0.1, 1, 0.1, 2.3, 1, 3.5, 2.0, 1, 1, 1, 1, 1, 
1, 1, 1, 1, 1, 1, 1, 1, 1.0, 1, 1.6, 1, 3.0,

0.1, 0.1, 1, 1, 1, 1, 0.4, 1, 1, 1, 1, 1, 1, 
1, 0.1, 0.4, 1, 1, 1, 5, 1.5, 1, 1, 1, 1, 1 } ;
*/
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
double MatchingMethod(CvSeq* input, CvSeq* contourTest, int i){
	IplImage* inputImg = cvCreateImage( cvSize(200, 200), 8, 3 ); // not sure about the size
	IplImage* testImg = cvCreateImage( cvGetSize(letterArray[i]), 8, 3 );
	cvZero( inputImg ) ;
	cvZero( testImg ) ;
	CvScalar color = CV_RGB( 255, 255, 255);
	cvDrawContours( inputImg, input, color, color, -1, CV_FILLED, 8 );
	cvDrawContours( testImg, contourTest, color, color, -1, CV_FILLED, 8 );

	int result_cols =  inputImg->width - testImg->width + 1;
	int result_rows = inputImg->height - testImg->height + 1;
	Mat resultImg;
	resultImg.create( result_cols, result_rows, CV_32FC1 );
	matchTemplate( Mat(inputImg, false), Mat(testImg, false), resultImg, CV_TM_CCOEFF_NORMED );
	normalize( resultImg, resultImg, 0, 1, NORM_MINMAX, -1, Mat() );

	// 使用minMaxLoc找出最佳匹配
	double minVal, maxVal;
	Point minLoc, maxLoc, matchLoc;
	minMaxLoc( resultImg, &minVal, &maxVal, &minLoc, &maxLoc, Mat() );
	if (i == 0){
		cvNamedWindow( "i",WINDOW_AUTOSIZE ) ;
		cvShowImage( "i", inputImg ) ;
		cvNamedWindow( "t",WINDOW_AUTOSIZE ) ;
		cvShowImage( "t", testImg ) ;
		cout << "Max:" << maxVal << endl;
	}
	return maxVal;
}
bool CompareOperation(const MatchResults lit ,const MatchResults rit){
	bool ret;
    lit.matchResult>rit.matchResult? ret=true:ret=false;
	return ret;
}
char matchLetter( CvSeq* input ) {
	// 會輸入一個字
	// fnction 輸出 比對到的字
	// Brian: 改成把所有自母比對一次 再找出作接近的
	int i = 0;
	vector<MatchResults> matchResults; // 0:id 1:answer
	for ( i = 0 ; i < 52 ; i++ ) {
		// 取得要比對的 contour
		CvSeq* contourTest = getTestContour( letterArray[i] ) ;
		double answer = MatchingMethod(input, contourTest, i); // cvMatchShapes(input, contourTest, CV_CONTOURS_MATCH_I1, 0 );
		MatchResults tempMatRes ;
		tempMatRes.Init(i, answer);
		matchResults.push_back(tempMatRes);
	}
	sort(matchResults.begin(),matchResults.end(),CompareOperation); // 排序
	cout << "Detected ID:" << matchResults[0].id << "  res:" << matchResults[0].matchResult << endl ;
	return ( matchResults[0].id < 26 ) ? ( 'A' + matchResults[0].id ) : ( 'a' + matchResults[0].id - 26 ) ;
	// 沒偵測到 Brian:not for now
	// return '＠' ;
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
		// 把介於 1500 ~ 100 面積的放到 vector // Brian Modify
		if ( 1500 > cvContourArea(contour ,CV_WHOLE_SEQ, 0) && cvContourArea(contour ,CV_WHOLE_SEQ, 0) > 100 )
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

	// cvNamedWindow( "Components", WINDOW_AUTOSIZE );
	// cvShowImage( "Components", dst );
 
    cvWaitKey(0);
 
    cvReleaseImage(&src);
    cvDestroyWindow("src");

    //cvReleaseImage(&ua);
    //cvDestroyWindow("ua");

	cvReleaseImage(&dst);
    cvDestroyWindow("Components");

    return 0 ;
}
