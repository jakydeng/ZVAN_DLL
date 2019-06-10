
#include "process.h"
#include "swavelet.h"
#include <string>
#include <thread>


using namespace std;
using namespace chrono;

#define WSIZE 2
#define CVCONTOUR_APPROX_LEVEL 2

/* debug level define */
const int g_dbg_level = 15;
FILE *g_log_fp;

/* debug level */
#define DBG_INFOR       0x01  // call information
#define DBG_WARNING     0x02  // paramters invalid,  
#define DBG_ERROR       0x04  // process error, leading to one call fails
#define DBG_CRITICAL    0x08  // process error, leading to voip process can't run exactly or exit

#if 0
/* debug macro */
#define DBG(level, fmt, para...)    do  \
{  \
	time_t t = time(NULL);  \
struct tm *tmm = localtime(&t); \
if(g_dbg_level & level)     \
	printf("[%d-%02d-%02d %02d:%02d:%02d][%s][%s:%d]" fmt "\n",tmm->tm_year+1900,tmm->tm_mon+1,tmm->tm_mday,tmm->tm_hour,tmm->tm_min,tmm->tm_sec,__FUNCTION__,__FILE__,__LINE__,##para); \
if((g_log_fp != NULL) && (level >= DBG_CRITICAL))   \
	fprintf(g_log_fp, "[%d-%02d-%02d %02d:%02d:%02d][%s][%s:%d]" fmt "\n",tmm->tm_year+1900,tmm->tm_mon+1,tmm->tm_mday,tmm->tm_hour,tmm->tm_min,tmm->tm_sec,__FUNCTION__,__FILE__,__LINE__,##para); \
} while (0)
#endif

#if 0
FILE * g_file_Handle = NULL;
#define LOG_OPEN(fname) \
do { \
if (g_file_Handle != NULL) { \
		printf("Test suite error @ file '%s' line %d: The log file can't be opened!\n", __FILE__, __LINE__); \
		return; \
}\
g_file_Handle = fopen(fname, "at+"); \
} while (0)
#define LOG_CLOSE() \
do { \
	if (g_file_Handle == NULL) { printf("Test suite error: Log file has already been closed!\n"); return; } \
		fclose(g_file_Handle); \
} while (0)
#define LOG_WRITE(format,...) \
do { \
SYSTEMTIME sys; \
GetLocalTime(&sys); \
fprintf((FILE*)g_file_Handle, "%4d/%02d/%02d %02d:%02d:%02d.%03d  ", sys.wYear, sys.wMonth, sys.wDay, sys.wHour, sys.wMinute, sys.wSecond, sys.wMilliseconds); \
fprintf((FILE*)g_file_Handle, "Func %s: ", __FUNCTION__); \
fprintf((FILE*)g_file_Handle, format, __VA_ARGS__); \
fprintf((FILE*)g_file_Handle, "\r\n"); \
fflush(g_file_Handle); \
} while (0)
#else
FILE * g_file_Handle = NULL;
#define LOG_OPEN(fname) 
#define LOG_CLOSE()
#define LOG_WRITE(format,...) 
#endif

/********************************************************
* @brief ??????idx
* @param  ?????
* @return ??
*********************************************************/
int getMaxIdx(int a[])
{
	int idx = 0;
	int max = 0;
	for (int i = 0; i < 100; i++)
	{
		if (a[i] > max)
		{
			max = a[i];
			idx = i;
		}
	}
	return idx;
}

/****************************************
* @brief ??????
* @param Zvan_Analysis_Params ?????????
* @return bool ??????????
* @remark ????????,?????????,
****************************************/
bool  Zvan_VIA_Analysis(Zvan_Analysis_Params* param){
		if (g_file_Handle == NULL)
		{
			fopen_s(&g_file_Handle, "log.txt", "at+");  //??????
		}
		LOG_WRITE("");

		Mat img[MAXIMAGENUM];
		Mat imgROI[MAXIMAGENUM];
		int nEmptyImgNum = 0;
		for (int i = 0; i < MAXIMAGENUM; i++)
		{
			img[i] = imread(string(param->images[i]), 2 | 4);


			LOG_WRITE("################################################################################################################################\n");
			LOG_WRITE("image%d width  is %d,height is %d\n", i, img[i].cols, img[i].rows);
			if (!img[i].data)
			{
				printf("%d th image is null\n", i);
				LOG_WRITE("%d th image is null\n", i);
				nEmptyImgNum++;

				return false;
			}

			if (MAXIMAGENUM == nEmptyImgNum)
			{
				printf("Input all empty!\n");
				LOG_WRITE("Input all empty!\n");
				return false;
			}
		}

		int nImgWidth = img[0].cols;
		int nImgHeight = img[0].rows;
		CvRect roiRect;

		int nType; //????
		int nSource; // smog or liguid 
		for (int i = 0; i < MAXANALYSISINFONUM; i++)
		{
			param->infos[i].param.work = false;
			param->infos[i].param.color = 0;
			for (int j = 0; j< MAXANALYSISLINENUM; j++)
			{
				param->infos[i].param.line[j].cover = false;
			}
			param->infos[i].param.object.color = 0;
			param->infos[i].param.object.froth = false;
			param->infos[i].param.activity = false;
			param->infos[i].param.ratio = 0.0f;


			nSource = param->infos[i].source;
			nType = param->infos[i].type;

			//???

#if 1
			roiRect.x = (int)(param->infos[i].rect.x * nImgWidth / 100.0);
			roiRect.y = (int)(param->infos[i].rect.y * nImgHeight / 100.0);
			roiRect.height = (int)(param->infos[i].rect.h * nImgHeight / 100.0);
			roiRect.width = (int)(param->infos[i].rect.w * nImgWidth / 100.0);
#else
			roiRect.x = param->infos[i].rect.x;
			roiRect.y = param->infos[i].rect.y;
			roiRect.height = param->infos[i].rect.h;
			roiRect.width = param->infos[i].rect.w;
#endif

			LOG_WRITE("################################################################################################################################\n");
			LOG_WRITE("The %d Analysys:[Type:%d,Source:%d,ROI.x:%d,ROI.y:%d,ROI.w:%d,ROI.h:%d]\n", i, param->infos[i].type, param->infos[i].source, param->infos[i].rect.x, param->infos[i].rect.y, param->infos[i].rect.w, param->infos[i].rect.h);
			if (roiRect.height % 2 != 0)
			{
				if (roiRect.height > nImgHeight - 3)
				{
					roiRect.height = roiRect.height - 1;
				}
				else
				{
					roiRect.height = roiRect.height + 1;
				}

			}
			if (roiRect.width % 2 != 0)
			{
				if (roiRect.width > nImgWidth - 3)
				{
					roiRect.width = roiRect.width - 1;
				}
				else
				{
					roiRect.width = roiRect.width + 1;
				}
			}

			if (roiRect.height <= 0 || roiRect.width <= 0)
			{
				printf("%d infos has no ROI rect!\n", i);
				LOG_WRITE("%d infos has no ROI rect!\n", i);
				break;
			}
			//??ROI


			for (int k = 0; k < MAXIMAGENUM; k++)
			{
				imgROI[k] = img[k](roiRect);

			}


			if (SourceType_Water == nSource)
			{
				switch (nType)
				{
				case AnalysisType_None:   //??
					break;
				case AnalysisType_Color:  //??????
					zvanLiquidColorDetect(imgROI, MAXIMAGENUM, i, param);
					break;

				case AnalysisType_Height:  //??????
					zvanLiquidHeightDetect(imgROI, MAXIMAGENUM, i, param);
					break;

				case AnalysisType_Object:  //?????? ????
					zvanFoamDetect(imgROI, MAXIMAGENUM, i, param);
					break;

				case AnalysisType_Activity:  //????
					zvanObjectMoveDetect(imgROI, MAXIMAGENUM, i, param);
					break;

				case AnalysisType_Work:  //????????
					zvanWaterMoveDetect(imgROI, MAXIMAGENUM, i, param);
					break;

				case AnalysisType_Turbidity:  //????????
					zvanLiquidTurbidityDetect(imgROI, MAXIMAGENUM, i, param);
					break;

				default:
					break;
				}
			}
			else if (SourceType_Air == nSource)
			{
				switch (nType)
				{
				case AnalysisType_None:   //??
					break;

				case AnalysisType_Color:  //??????
					zvanSmogColorDetect(imgROI, MAXIMAGENUM, i, param);
					break;

				case AnalysisType_Activity:  //????
					zvanObjectMoveDetect(imgROI, MAXIMAGENUM, i, param);
					break;

				case AnalysisType_Work:  //???????
					zvanSmokeMoveDetect(imgROI, MAXIMAGENUM, i, param);
					break;

				default:
					break;
				}
			}
		}
		//LOG_CLOSE();  //??????
		return true;
	}




/****************************************
* @brief ??????
* @param Zvan_Analysis_Params ?????????
* @return bool ??????????
* @remark ????????,?????????,
****************************************/

void zvanLiquidColorDetect(Mat img[], int imgNum, int infoId, Zvan_Analysis_Params* param)
{
	LOG_WRITE("################################################################################################################################\n");
	LOG_WRITE("[ENTER] %s\n", __FUNCTION__);

	param->infos[infoId].param.color = 0;

	//detector wator
	Mat lastFrame;
	cv::cvtColor(img[imgNum - 1], lastFrame, CV_BGR2GRAY);
	Mat firstFrame;
	cv::cvtColor(img[0], firstFrame, CV_BGR2GRAY);

	Mat diff = abs(lastFrame - firstFrame);
	int lumaThreshold = 10;
	diff = diff > lumaThreshold;

	Scalar s = cv::sum(diff);

	int total = diff.rows * diff.cols;
	int thres = total * 0.02;
	int diffCnt = s.val[0] / 255;
	if (diffCnt < thres) {
		LOG_WRITE("No water %s\n", __FUNCTION__);
		return;
	}

	Mat gray;
	Mat src = img[imgNum - 1];
	cv::cvtColor(src, gray, CV_BGR2GRAY);
	Mat edge;
	Canny(gray, edge, 30, 60, 3);

	Mat edgeErode;
	Mat element = getStructuringElement(MORPH_RECT, Size(3, 3));
	dilate(edge, edgeErode, element);

	Mat channels[3];
	cv::split(src, channels);

	uint r = 0;
	uint g = 0;
	uint b = 0;
	uint cnt = 0;
	int width = edgeErode.cols;
	int height = edgeErode.rows;


	for (int j = 0; j < height; j++)
	{
		uchar* pEdge = edgeErode.ptr<uchar>(j);
		uchar* pB = channels[0].ptr<uchar>(j);
		uchar* pG = channels[1].ptr<uchar>(j);
		uchar* pR = channels[2].ptr<uchar>(j);

		for (int i = 0; i < width; i++)
		{
			if (pEdge[i] == 0)
			{
				b += pB[i];
				g += pG[i];
				r += pR[i];
				cnt++;
			}
		}
	}
	LOG_WRITE("[EXIT] %s %d\n", __FUNCTION__, __LINE__);
	b = b / cnt;
	g = g / cnt;
	r = r / cnt;

	param->infos[infoId].param.color = r << 16 | g << 8 | b;;
	LOG_WRITE("[EXIT] %s\n", __FUNCTION__);
}

/****************************************
* @brief ??????
* @param Zvan_Analysis_Params ?????????
* @return bool ??????????
* @remark ????????,?????????,
****************************************/
int getHeight(Mat diff, Mat srcImg)
{
	const int nGrayThre = 2;

	int nImgHeight = diff.rows;
	int nImgWidth = diff.cols;
	int diffCnt = 0;

	for (int h = 2; h < nImgHeight - 2; h++)
	{
		Mat diffNumMat = diff.rowRange(h - 2, h + 2).clone();
		Scalar diffNum = cv::sum(diffNumMat);
		diffCnt = (int)diffNum.val[0];

		int nThr1 = (int)(nImgWidth * 0.9);
		//int nThr1 = nImgWidth * 0.4;
		int nThr2 = (int)(nImgWidth * 0.8);

		LOG_WRITE("[Param]nCnt=%d thr1=%d thr2=%d h=%d height=%d\n", diffCnt, nThr1, nThr2, h, nImgHeight);
		if (diffCnt > nThr1  && h > 2 && h < nImgHeight - 2)
		{
			Mat row1 = srcImg.rowRange(h - 2, h).clone();
			Mat row2 = srcImg.rowRange(h, h + 2).clone();

			Scalar tempVal1 = cv::mean(row1);
			Scalar tempVal2 = cv::mean(row2);

			LOG_WRITE("[Param]gray1=%.2f  gray1=%.2f\n", (float)tempVal1.val[0], (float)tempVal2.val[0]);

			if (abs((float)tempVal1.val[0] - (float)tempVal2.val[0]) > nGrayThre)
			{
				LOG_WRITE("[Param] ***** H=%d\n", h);
				return h;
			}

		}
	}

	return 0;
}

float liquidHeightRatio(Mat &firstFrame, Mat &secondFrame) {

	Mat element = getStructuringElement(MORPH_RECT, Size(3, 3));
	int roiWidth = firstFrame.cols;
	int roiHeight = firstFrame.rows;

	Mat firstFrameGray, secondFrameGray;
	cvtColor(firstFrame, firstFrameGray, COLOR_BGR2GRAY);
	cvtColor(secondFrame, secondFrameGray, COLOR_BGR2GRAY);


	Mat firstFrameDenoise, secondFrameDenoise;
	bilateralFilter(firstFrameGray, firstFrameDenoise, 5, 5, 5);
	bilateralFilter(secondFrameGray, secondFrameDenoise, 5, 5, 5);


	Mat firstFramX, firstFramY;
	Sobel(firstFrameDenoise, firstFramX, CV_16S, 0, 1, 3);
	Sobel(firstFrameDenoise, firstFramY, CV_16S, 1, 0, 3);
	Mat firstFramX_abs, firstFramY_abs;
	convertScaleAbs(firstFramX, firstFramX_abs);
	convertScaleAbs(firstFramY, firstFramY_abs);

	Mat secondFramX, secondFramY;
	Sobel(secondFrameDenoise, secondFramX, CV_16S, 0, 1, 3);
	Sobel(secondFrameDenoise, secondFramY, CV_16S, 1, 0, 3);
	Mat secondFramX_abs, secondFramY_abs;
	convertScaleAbs(secondFramX, secondFramX_abs);
	convertScaleAbs(secondFramY, secondFramY_abs);

	Mat FirstsobelMat;
	addWeighted(firstFramX_abs, 0.5, firstFramY_abs, 0.5, 0, FirstsobelMat);
	Mat SecondsobelMat;
	addWeighted(secondFramX_abs, 0.5, secondFramY_abs, 0.5, 0, SecondsobelMat);

	int sobelThr = 40;
	FirstsobelMat = FirstsobelMat > sobelThr;
	SecondsobelMat = SecondsobelMat > sobelThr;



	GaussianBlur(firstFrameDenoise, firstFrameDenoise, Size(3, 3), -1);
	GaussianBlur(secondFrameDenoise, secondFrameDenoise, Size(3, 3), -1);


	Mat grayDiff = abs(firstFrameDenoise - secondFrameDenoise) > 2;
	erode(grayDiff, grayDiff, element);


	//运动检测，无运动则不是液面
	FirstsobelMat /= 255;
	SecondsobelMat /= 255;
	grayDiff /= 255;

	Mat firstMat = FirstsobelMat.mul(grayDiff);
	Mat secondtMat = SecondsobelMat.mul(grayDiff);

	Scalar firstSum = sum(firstMat);
	Scalar secondSum = sum(secondtMat);

	int startP = -1, endP = -1;

	for (int i = 0; i < roiHeight; i++)
	{
		if (startP < 0) {
			Mat row = SecondsobelMat.rowRange(i, i + 1).clone();
			if (sum(row)[0] > 2)
				startP = i;

		}

		if (endP < 0)
		{
			Mat row = SecondsobelMat.rowRange(roiHeight - 2, roiHeight - 1).clone();
			if (sum(row)[0] > 2)
				endP = roiHeight - 1;
		}
	}

	int sumThr = endP - startP;
	if (firstSum[0] > sumThr || secondSum[0] > sumThr) {
		int h = (endP + startP) / 2;
		return ((float)h / (float)roiHeight);
	}
	return 0;
}

bool getMoveStatus(const Mat& _first, const Mat& _second, int diffThre, float ratio) {
	
	int width = _first.cols;
	int moveThre = width * ratio;

	Mat diffMat = abs(_first - _second);
	diffMat = (diffMat > diffThre) / 255;

	double sum = cv::sum(diffMat)[0];

	if (sum > moveThre) {
		return true;
	}
	else {
		return false;
	}	
}

float getLiquidRatio(const Mat& _input) {
	Mat hsv[3];

	Mat hsvMat;
	GaussianBlur(_input, hsvMat, Size(5, 5), 0);
	split(hsvMat, hsv);

	Mat sMat = hsv[1];

	double meanValue = cv::mean(sMat)[0];

	Mat threMat;
	threshold(sMat, threMat, meanValue, 1, THRESH_BINARY);

	int totalPixel = _input.rows * _input.cols;
	double totalLiquidPixel = cv::sum(threMat)[0];

	//判断液面位置
	double bottomMean = cv::mean(sMat.rowRange(_input.rows - 1,_input.rows))[0];

	float ratio;
	if (meanValue > bottomMean)
	{
		ratio = 1.0f - totalLiquidPixel / (totalPixel);
	}
	else {
		ratio = totalLiquidPixel / (totalPixel);
	}

	return ratio;
}

float getLiquidRatio(const Mat& _first, const Mat& _second) {

	int diffThre = 10;
	float ratio = 0.8;
	if (getMoveStatus( _first, _second, diffThre, ratio)) {
		return getLiquidRatio(_second);
	}

	return 0.0f;
}

void zvanLiquidHeightDetect(Mat img[], int imgNum, int infoId, Zvan_Analysis_Params* param)
{
	LOG_WRITE("################################################################################################################################\n");
	printf("[ENTER] %s  img size=%d\n", __FUNCTION__, sizeof(img));
	LOG_WRITE("[ENTER] %s\n", __FUNCTION__);

	
	param->infos[infoId].param.ratio = getLiquidRatio(img[0], img[4]);

	
	LOG_WRITE("[EXIT] %s\n", __FUNCTION__);
}

/****************************************
* @brief ??????
* @param Zvan_Analysis_Params ?????????
* @return bool ??????????
* @remark ????????,?????????,
****************************************/
void zvanLiquidTurbidityDetect(Mat img[], int imgNum, int infoId, Zvan_Analysis_Params* param)
{
	LOG_WRITE("################################################################################################################################\n");
	LOG_WRITE("[ENTER] %s\n", __FUNCTION__);

	const float turbidityThe = 85.0f;

	for (int idx = 0; idx < imgNum; idx++)
	{
		GaussianBlur(img[idx], img[idx], Size(3, 3), 0);

		Mat hsv;
		cvtColor(img[idx], hsv, CV_BGR2HSV);
		Scalar hsvVal = mean(hsv);

		if (hsvVal.val[0] < turbidityThe)
		{
			param->infos[infoId].param.turbidity = 1;
			LOG_WRITE("[update] %s H=%.2f Tre=%.2f\n", __FUNCTION__, hsvVal.val[0], turbidityThe);
		}
		else
		{
			param->infos[infoId].param.turbidity = 0;
			LOG_WRITE("[update] %s H=%.2f Tre=%.2f\n", __FUNCTION__, hsvVal.val[0], turbidityThe);
		}
	}


	LOG_WRITE("[EXIT] %s\n", __FUNCTION__);
}
/****************************************
* @brief ??????
* @param Zvan_Analysis_Params ?????????
* @return bool ??????????
* @remark ????????,?????????,
****************************************/
void zvanSmogColorDetect(Mat img[], int imgNum, int infoId, Zvan_Analysis_Params* param)
{
	LOG_WRITE("################################################################################################################################\n");
	LOG_WRITE("[ENTER] %s\n", __FUNCTION__);
	const int nWindowSize = 10;  //ROI????nWindowSize X nWindowSize ????
	int nImgWidth = img[0].cols;
	int nImgHeight = img[0].rows;
	int hOffset = nImgHeight / nWindowSize;
	int wOffset = nImgWidth / nWindowSize;

	double r = 0;
	double g = 0;
	double b = 0;
	int nCnt = 0;
	int nFrameCnt = 0;

	int roiIdx = 0;
	int grayValue[nWindowSize*nWindowSize] = { 0 };

	Mat frame;    //??????
	Mat grayImg;  //?????

	Scalar  means, stddev;
	for (int imgId = 0; imgId < imgNum; imgId++)
	{
		int rowStart = 0, rowEnd = 0, colStart = 0, colEnd = 0;

		roiIdx = 0;
		memset(grayValue, 0, sizeof(int)*nWindowSize*nWindowSize);
		cvtColor(img[imgId], grayImg, COLOR_BGR2GRAY);

		rowStart = 0;
		rowEnd = hOffset;
		for (int j = 0; j < nWindowSize; j++)
		{
			colStart = 0;
			colEnd = wOffset;

			for (int i = 0; i < nWindowSize; i++)
			{
				Mat imgROI = grayImg(Range(rowStart, rowEnd), Range(colStart, colEnd));
				meanStdDev(imgROI, means, stddev);
				grayValue[roiIdx] = (int)means.val[0];
				roiIdx++;

				colStart = colEnd;
				colEnd += wOffset;
			}

			rowStart = rowEnd;
			rowEnd += hOffset;
		}

		int nIdx = getMaxIdx(grayValue);
		int row = nIdx / nWindowSize;
		int col = nIdx % nWindowSize;

		Mat aChannels[3];
		split(img[imgId](Range(row*hOffset, (row + 1)*hOffset), Range(col*wOffset, (col + 1)*wOffset)), aChannels);
		b += mean(aChannels[0]).val[0];
		g += mean(aChannels[1]).val[0];
		r += mean(aChannels[2]).val[0];
		nCnt++;
	}

	unsigned char bAve = (unsigned char)(b / nCnt + 0.0001);
	unsigned char gAve = (unsigned char)(g / nCnt + 0.0001);
	unsigned char rAve = (unsigned char)(r / nCnt + 0.0001);

	LOG_WRITE("Smog Color(R:%d G:%d B:%d)\n", rAve, gAve, bAve);
	int color = rAve << 16 | gAve << 8 | bAve;

	param->infos[infoId].param.color = color;
	LOG_WRITE("[EXIT] %s\n", __FUNCTION__);
}

/****************************************
* @brief ????
* @param Zvan_Analysis_Params ?????????
* @return bool ??????????
* @remark ????????,?????????,
****************************************/
void zvanSmokeMoveDetect(Mat img[], int imgNum, int infoId, Zvan_Analysis_Params* param)
{
	LOG_WRITE("################################################################################################################################\n");
	LOG_WRITE("[ENTER] %s\n", __FUNCTION__);
	IplImage* pFrame = NULL;
	IplImage* pFrImg = NULL;
	IplImage* pBkImg = NULL;
	IplImage* pResult = NULL;
	IplImage* pImg = NULL;
	CvMat* pFrameMat = NULL;
	CvMat* pFrMat = NULL;
	CvMat* pBkMat = NULL;
	CvMat* pMatChannels[4] = { NULL, NULL, NULL, NULL };
	CvMat* pFrameMatPre1 = NULL;
	CvMat* pFrameMatPre2 = NULL;
	CvMat* pFrameMatCur = NULL;
	CvMat* framediff1 = NULL;
	CvMat* framediff2 = NULL;
	CvMat* pWaveletCur = NULL;
	CvMat* pWaveletBk = NULL;
	CvMat* thresholdMat = NULL;
	int threshold_wavelet = 15;
	bool isMove = false;
	int MoveCount = 0;
	for (int nFrmNum = 0; nFrmNum < imgNum; nFrmNum++)
	{
		int frame_height;
		int frame_width;
		int frame_channel;
		int DetectedCount = 0;
		GaussianBlur(img[nFrmNum], img[nFrmNum], Size(3, 3), 0, 0); //
		pFrame = &IplImage(img[nFrmNum]);
		frame_height = pFrame->height;
		frame_width = pFrame->width;
		frame_channel = pFrame->nChannels;
		if (nFrmNum == 0)
		{
			pBkImg = cvCreateImage(cvSize(frame_width, frame_height), IPL_DEPTH_8U, 1);
			pFrImg = cvCreateImage(cvSize(frame_width, frame_height), IPL_DEPTH_8U, 1);
			pResult = cvCreateImage(cvSize(frame_width, frame_height), IPL_DEPTH_8U, 3);
			pBkMat = cvCreateMat(frame_height, frame_width, CV_32FC1);
			pFrMat = cvCreateMat(frame_height, frame_width, CV_32FC1);
			pFrameMat = cvCreateMat(frame_height, frame_width, CV_32FC3);
			pFrameMatPre1 = cvCreateMat(frame_height, frame_width, CV_32FC1);
			pFrameMatPre2 = cvCreateMat(frame_height, frame_width, CV_32FC1);
			pFrameMatCur = cvCreateMat(frame_height, frame_width, CV_32FC1);
			framediff1 = cvCreateMat(frame_height, frame_width, CV_32FC1);
			framediff2 = cvCreateMat(frame_height, frame_width, CV_32FC1);
			pWaveletCur = cvCreateMat(frame_height, frame_width, CV_32FC1);
			pWaveletBk = cvCreateMat(frame_height, frame_width, CV_32FC1);
			thresholdMat = cvCreateMat(frame_height, frame_width, CV_32FC1);
			pImg = cvCreateImage(cvSize(frame_width, frame_height), IPL_DEPTH_8U, 1);

			for (int k = 0; k < frame_channel; k++)
			{
				pMatChannels[k] = cvCreateMat(frame_height, frame_width, CV_32FC1);
			}

			//???????????
			cvConvertScale(pFrame, pFrameMat, 1, 0);
			cvSplit(pFrameMat, pMatChannels[0], pMatChannels[1], pMatChannels[2], pMatChannels[3]);
			cvAddWeighted(pMatChannels[0], 0.114, pMatChannels[1], 0.587, 0, pFrameMatPre1);
			cvAddWeighted(pMatChannels[2], 0.299, pFrameMatPre1, 1, 0, pFrameMatPre1);
			cvConvert(pFrameMatPre1, pBkMat);
			cvConvert(pFrameMatPre1, pFrameMatPre2);
			cvZero(pFrameMatCur);
			cvZero(pFrameMatPre2);
			cvSet(thresholdMat, cvScalar(10), NULL);
		}
		else
		{
			const int threshold_detection = 20;
			const double alpha = 0.02;
			const int threshold_color = 7;
			int i, j;

			cvConvertScale(pFrame, pFrameMat, 1, 0);
			cvConvertScale(pFrame, pResult, 1, 0);
			cvSplit(pFrameMat, pMatChannels[0], pMatChannels[1], pMatChannels[2], pMatChannels[3]);
			cvAddWeighted(pMatChannels[0], 0.114, pMatChannels[1], 0.587, 0, pFrameMatCur);
			cvAddWeighted(pMatChannels[2], 0.299, pFrameMatCur, 1, 0, pFrameMatCur);

			//?????????	
			cvAbsDiff(pFrameMatCur, pFrameMatPre1, framediff1);
			cvAbsDiff(pFrameMatCur, pFrameMatPre2, framediff2);
			//??????	 
			for (i = 0; i<frame_height; i++)
				for (j = 0; j<frame_width; j++)
				{
					if ((cvmGet(framediff1, i, j) > cvmGet(thresholdMat, i, j)) && (cvmGet(framediff2, i, j) > cvmGet(thresholdMat, i, j)))
						cvmSet(pFrMat, i, j, 255.0);
					else
						cvmSet(pFrMat, i, j, 0.0);
				}
			cvDilate(pFrMat, pFrMat, 0, 1);

			// ??????
			WaveletAnalysis(pFrameMatCur, pWaveletCur, 1);
			WaveletAnalysis(pBkMat, pWaveletBk, 1);
			float* energycur = new float[frame_width*frame_height / WSIZE / WSIZE];
			float* energypre = new float[frame_width*frame_height / WSIZE / WSIZE];
			int num = 0;
			for (i = 0; i < frame_height - 1; i += WSIZE)
				for (j = 0; j < frame_width - 1; j += WSIZE)
				{
					int ii, jj;
					energycur[num] = 0;
					energypre[num] = 0;
					for (ii = i; ii < i + WSIZE; ii++)
						for (jj = j; jj < j + WSIZE; jj++)
						{
							energycur[num] += cvmGet(pWaveletCur, ii, jj);
							energypre[num] += cvmGet(pWaveletBk, ii, jj);
						}
					if (abs(energypre[num] - energycur[num]) >= threshold_wavelet)
					{
						for (ii = i; ii < i + WSIZE; ii++)
							for (jj = j; jj < j + WSIZE; jj++)
							{
								cvmSet(pWaveletCur, ii, jj, 255);
							}
					}
					else
					{
						for (ii = i; ii < i + WSIZE; ii++)
							for (jj = j; jj < j + WSIZE; jj++)
							{
								cvmSet(pWaveletCur, ii, jj, 0);
							}
					}
					num++;
				}
			delete[]energycur;
			delete[]energypre;

			for (i = 0; i < frame_height; i++)
				for (j = 0; j < frame_width; j++)
				{
					if (cvmGet(pWaveletCur, i, j)) //cvmGet(pFrMat, i, j))
					{
						cvSet2D(pResult, i, j, CV_RGB(255, 0, 0));
						DetectedCount++;
					}
				}

			//???????
			cvConvert(pFrameMatPre1, pFrameMatPre2);
			cvConvert(pFrameMatCur, pFrameMatPre1);
			cvConvert(pFrameMatPre1, pBkMat);

			//???????,????
			cvConvert(pBkMat, pBkImg);
			cvConvert(pFrMat, pFrImg);

			//??????                 
			pFrImg->origin = pFrame->origin;
			pImg->origin = pFrame->origin;
			pResult->origin = pFrame->origin;
		}
		if (DetectedCount >= 1500)
		{
			MoveCount++;
		}
	}
	if (MoveCount >= imgNum*0.7)
	{
		param->infos[infoId].param.work = true;
	}
	else
	{
		param->infos[infoId].param.work = false;
	}
	LOG_WRITE("Work:%d\n", param->infos[infoId].param.work);
	LOG_WRITE("[EXIT] %s\n", __FUNCTION__);
}

/****************************************
* @brief ????
* @param Zvan_Analysis_Params ?????????
* @return bool ??????????
* @remark ????????,?????????,
****************************************/
void zvanWaterMoveDetect(Mat img[], int imgNum, int infoId, Zvan_Analysis_Params* param)
{
	LOG_WRITE("################################################################################################################################\n");
	LOG_WRITE("[ENTER] %s\n", __FUNCTION__);
	IplImage* pFrame = NULL;
	IplImage* pFrImg = NULL;
	IplImage* pBkImg = NULL;
	IplImage* pResult = NULL;
	IplImage* pImg = NULL;
	CvMat* pFrameMat = NULL;
	CvMat* pFrMat = NULL;
	CvMat* pBkMat = NULL;
	CvMat* pMatChannels[4] = { NULL, NULL, NULL, NULL };
	CvMat* pFrameMatPre1 = NULL;
	CvMat* pFrameMatPre2 = NULL;
	CvMat* pFrameMatCur = NULL;
	CvMat* framediff1 = NULL;
	CvMat* framediff2 = NULL;
	CvMat* pWaveletCur = NULL;
	CvMat* pWaveletBk = NULL;
	CvMat* thresholdMat = NULL;
	int threshold_wavelet = 15;
	bool isMove = false;
	int MoveCount = 0;
	for (int nFrmNum = 0; nFrmNum < imgNum; nFrmNum++)
	{
		int frame_height;
		int frame_width;
		int frame_channel;
		int DetectedCount = 0;
		pFrame = &IplImage(img[nFrmNum]);
		frame_height = pFrame->height;
		frame_width = pFrame->width;
		frame_channel = pFrame->nChannels;
		if (nFrmNum == 0)
		{
			pBkImg = cvCreateImage(cvSize(frame_width, frame_height), IPL_DEPTH_8U, 1);
			pFrImg = cvCreateImage(cvSize(frame_width, frame_height), IPL_DEPTH_8U, 1);
			pResult = cvCreateImage(cvSize(frame_width, frame_height), IPL_DEPTH_8U, 3);
			pBkMat = cvCreateMat(frame_height, frame_width, CV_32FC1);
			pFrMat = cvCreateMat(frame_height, frame_width, CV_32FC1);
			pFrameMat = cvCreateMat(frame_height, frame_width, CV_32FC3);
			pFrameMatPre1 = cvCreateMat(frame_height, frame_width, CV_32FC1);
			pFrameMatPre2 = cvCreateMat(frame_height, frame_width, CV_32FC1);
			pFrameMatCur = cvCreateMat(frame_height, frame_width, CV_32FC1);
			framediff1 = cvCreateMat(frame_height, frame_width, CV_32FC1);
			framediff2 = cvCreateMat(frame_height, frame_width, CV_32FC1);
			pWaveletCur = cvCreateMat(frame_height, frame_width, CV_32FC1);
			pWaveletBk = cvCreateMat(frame_height, frame_width, CV_32FC1);
			thresholdMat = cvCreateMat(frame_height, frame_width, CV_32FC1);
			pImg = cvCreateImage(cvSize(frame_width, frame_height), IPL_DEPTH_8U, 1);

			for (int k = 0; k < frame_channel; k++)
			{
				pMatChannels[k] = cvCreateMat(frame_height, frame_width, CV_32FC1);
			}

			//???????????
			cvConvertScale(pFrame, pFrameMat, 1, 0);
			cvSplit(pFrameMat, pMatChannels[0], pMatChannels[1], pMatChannels[2], pMatChannels[3]);
			cvAddWeighted(pMatChannels[0], 0.114, pMatChannels[1], 0.587, 0, pFrameMatPre1);
			cvAddWeighted(pMatChannels[2], 0.299, pFrameMatPre1, 1, 0, pFrameMatPre1);
			cvConvert(pFrameMatPre1, pBkMat);
			cvConvert(pFrameMatPre1, pFrameMatPre2);
			cvZero(pFrameMatCur);
			cvZero(pFrameMatPre2);
			cvSet(thresholdMat, cvScalar(10), NULL);
		}
		else
		{
			const int threshold_detection = 20;
			const double alpha = 0.02;
			const int threshold_color = 7;
			int i, j;

			cvConvertScale(pFrame, pFrameMat, 1, 0);
			cvConvertScale(pFrame, pResult, 1, 0);
			cvSplit(pFrameMat, pMatChannels[0], pMatChannels[1], pMatChannels[2], pMatChannels[3]);
			cvAddWeighted(pMatChannels[0], 0.114, pMatChannels[1], 0.587, 0, pFrameMatCur);
			cvAddWeighted(pMatChannels[2], 0.299, pFrameMatCur, 1, 0, pFrameMatCur);

			//?????????	
			cvAbsDiff(pFrameMatCur, pFrameMatPre1, framediff1);
			cvAbsDiff(pFrameMatCur, pFrameMatPre2, framediff2);
			//??????	 
			for (i = 0; i<frame_height; i++)
				for (j = 0; j<frame_width; j++)
				{
					if ((cvmGet(framediff1, i, j) > cvmGet(thresholdMat, i, j)) && (cvmGet(framediff2, i, j) > cvmGet(thresholdMat, i, j)))
						cvmSet(pFrMat, i, j, 255.0);
					else
						cvmSet(pFrMat, i, j, 0.0);
				}
			cvDilate(pFrMat, pFrMat, 0, 1);

			// ??????
			WaveletAnalysis(pFrameMatCur, pWaveletCur, 1);
			WaveletAnalysis(pBkMat, pWaveletBk, 1);
			float* energycur = new float[frame_width*frame_height / WSIZE / WSIZE];
			float* energypre = new float[frame_width*frame_height / WSIZE / WSIZE];
			int num = 0;
			for (i = 0; i<frame_height - 1; i += WSIZE)
				for (j = 0; j<frame_width - 1; j += WSIZE)
				{
					int ii, jj;
					energycur[num] = 0;
					energypre[num] = 0;
					for (ii = i; ii<i + WSIZE; ii++)
						for (jj = j; jj<j + WSIZE; jj++)
						{
							energycur[num] += cvmGet(pWaveletCur, ii, jj);
							energypre[num] += cvmGet(pWaveletBk, ii, jj);
						}
					if (abs(energypre[num] - energycur[num]) >= threshold_wavelet)
					{
						for (ii = i; ii<i + WSIZE; ii++)
							for (jj = j; jj<j + WSIZE; jj++)
							{
								cvmSet(pWaveletCur, ii, jj, 255);
							}
					}
					else
					{
						for (ii = i; ii<i + WSIZE; ii++)
							for (jj = j; jj<j + WSIZE; jj++)
							{
								cvmSet(pWaveletCur, ii, jj, 0);
							}
					}
					num++;
				}
			delete[]energycur;
			delete[]energypre;

			for (i = 0; i < frame_height; i++)
				for (j = 0; j < frame_width; j++)
				{
					if (cvmGet(pWaveletCur, i, j)) //cvmGet(pFrMat, i, j))
					{
						cvSet2D(pResult, i, j, CV_RGB(255, 0, 0));
						DetectedCount++;
					}
				}

			//???????
			cvConvert(pFrameMatPre1, pFrameMatPre2);
			cvConvert(pFrameMatCur, pFrameMatPre1);
			cvConvert(pFrameMatPre1, pBkMat);

			//???????,????
			cvConvert(pBkMat, pBkImg);
			cvConvert(pFrMat, pFrImg);

			//??????                 
			pFrImg->origin = pFrame->origin;
			pImg->origin = pFrame->origin;
			pResult->origin = pFrame->origin;
		}
		if (DetectedCount >= 1500)
		{
			MoveCount++;
		}
	}
	if (MoveCount >= imgNum*0.5)
	{
		param->infos[infoId].param.work = true;
	}
	else
	{
		param->infos[infoId].param.work = false;
	}

	//if (pFrame != NULL)
		//cvReleaseImage(&pFrame);
	if (pFrImg != NULL)
		cvReleaseImage(&pFrImg);
	if (pBkImg != NULL)
		cvReleaseImage(&pBkImg);
	if (pResult != NULL)
		cvReleaseImage(&pResult);
	if (pImg != NULL)
		cvReleaseImage(&pImg);

	if (pFrameMat != NULL)
		cvReleaseMat(&pFrameMat);
	if (pFrMat != NULL)
		cvReleaseMat(&pFrMat);
	if (pBkMat != NULL)
		cvReleaseMat(&pBkMat);
	if (pMatChannels[0] != NULL)
		cvReleaseMat(&pMatChannels[0]);
	if (pMatChannels[1] != NULL)
		cvReleaseMat(&pMatChannels[1]);
	if (pMatChannels[2] != NULL)
		cvReleaseMat(&pMatChannels[2]);
	if (pMatChannels[3] != NULL)
		cvReleaseMat(&pMatChannels[3]);
	if (pFrameMatPre1 != NULL)
		cvReleaseMat(&pFrameMatPre1);
	if (pFrameMatPre2 != NULL)
		cvReleaseMat(&pFrameMatPre2);
	if (pFrameMatCur != NULL)
		cvReleaseMat(&pFrameMatCur);
	if (framediff1 != NULL)
		cvReleaseMat(&framediff1);
	if (framediff2 != NULL)
		cvReleaseMat(&framediff2);
	if (pWaveletCur != NULL)
		cvReleaseMat(&pWaveletCur);
	if (pWaveletBk != NULL)
		cvReleaseMat(&pWaveletBk);
	if (thresholdMat != NULL)
		cvReleaseMat(&thresholdMat);

	LOG_WRITE("Work:%d\n", param->infos[infoId].param.work);
	LOG_WRITE("[EXIT] %s\n", __FUNCTION__);
}

/****************************************
* @brief ????
* @param Zvan_Analysis_Params ?????????
* @return bool ??????????
* @remark ????????,?????????,
****************************************/
//void zvanFoamDetect(Mat img[], int imgNum, int infoId, Zvan_Analysis_Params* param)
//{
//	LOG_WRITE("################################################################################################################################\n");
//	LOG_WRITE("[ENTER] %s\n", __FUNCTION__);
//	Mat frame1, frame2;
//	float *B = new float[imgNum - 1];
//	float *G = new float[imgNum - 1];
//	float *R = new float[imgNum - 1];
//	for (int i = 0; i < imgNum - 1; i++)
//	{
//		B[i] = 0;
//		G[i] = 0;
//		R[i] = 0;
//	}
//	for (int nFrmNum = 0; nFrmNum < imgNum; nFrmNum++)
//	{
//		if (nFrmNum == 0)
//		{
//			frame2 = img[nFrmNum].clone();
//			GaussianBlur(frame2, frame2, Size(3, 3), 0, 0); //
//		}
//		else
//		{
//			frame1 = img[nFrmNum];
//			GaussianBlur(frame1, frame1, Size(3, 3), 0, 0); //
//			Mat gray1, gray2;
//			cvtColor(frame1, gray1, CV_BGR2GRAY);
//			cvtColor(frame2, gray2, CV_BGR2GRAY);
//			Mat diff;
//			absdiff(gray1, gray2, diff);
//			threshold(diff, diff, 50, 255, CV_THRESH_BINARY);
//			threshold(gray1, gray1, 130, 255, CV_THRESH_BINARY);
//			for (int row = 0; row < gray1.rows; row++)
//			{
//				uchar *pDiff = diff.ptr<uchar>(row);
//				uchar *pGray1 = gray1.ptr<uchar>(row);
//				for (int col = 0; col < gray1.cols; col++)
//				{
//					if (pDiff[col] >0 && pGray1[col]>0)
//					{
//						pGray1[col] = pGray1[col];
//					}
//					else
//					{
//						pGray1[col] = 0;
//					}
//				}
//			}
//			int ValidCount = 0;
//			Mat Foam = frame1.clone();
//			for (int row = 0; row < gray1.rows; row++)
//			{
//				uchar *pGray1 = gray1.ptr<uchar>(row);
//				for (int col = 0; col < gray1.cols; col++)
//				{
//					if (pGray1[col] >0)//frame1?????130,???????
//					{
//						ValidCount++;
//					}
//					else//???????130,???????
//					{
//						Foam.at<Vec3b>(row, col)[0] = 0;
//						Foam.at<Vec3b>(row, col)[1] = 0;
//						Foam.at<Vec3b>(row, col)[2] = 0;
//
//					}
//				}
//			}
//			Mat FoamGray;
//			cvtColor(Foam, FoamGray, CV_BGR2GRAY);
//			//?????
//			int channels = 0;
//			MatND dstHist;
//			int histSize[] = { 256 };       //????int histSize = 256;????????????,??????&histSize  
//			float midRanges[] = { 0, 256 };
//			const float *ranges[] = { midRanges };
//			calcHist(&FoamGray, 1, &channels, Mat(), dstHist, 1, histSize, ranges, true, false);
//			double maxV, minV;
//			Point maxP, minP;
//			minMaxLoc(FoamGray, &minV, &maxV, &minP, &maxP);
//			int i = maxV;
//			int ThresholdFoam = 0;
//			int sum = 0;
//			while (i > 0)
//			{
//				sum = sum + dstHist.at<float>(i);
//				if (sum > ValidCount*0.99)//?????130????????????30%????????
//				{
//					ThresholdFoam = i;
//					break;
//				}
//				i = i - 1;
//			}
//			int HighLum = 0;
//			for (int row = 0; row < FoamGray.rows; row++)
//			{
//				uchar *pFoamGray = FoamGray.ptr<uchar>(row);
//				for (int col = 0; col < FoamGray.cols; col++)
//				{
//					if (pFoamGray[col] > ThresholdFoam)
//					{
//						B[nFrmNum - 1] += Foam.at<Vec3b>(row, col)[0];
//						G[nFrmNum - 1] += Foam.at<Vec3b>(row, col)[1];
//						R[nFrmNum - 1] += Foam.at<Vec3b>(row, col)[2];
//						HighLum++;
//					}
//				}
//			}
//			param->infos[infoId].param.object.pixel = ValidCount;
//			param->infos[infoId].param.object.pixelpct = ((float)(ValidCount) / (float)(FoamGray.rows*FoamGray.cols));
//			if (ValidCount > param->infos[infoId].param.object.threshold)//??50????????
//			{
//				B[nFrmNum - 1] = B[nFrmNum - 1] / (HighLum + 0.000001);
//				G[nFrmNum - 1] = G[nFrmNum - 1] / (HighLum + 0.000001);
//				R[nFrmNum - 1] = R[nFrmNum - 1] / (HighLum + 0.000001);
//
//			}
//			else
//			{
//				B[nFrmNum - 1] = 0;
//				G[nFrmNum - 1] = 0;
//				R[nFrmNum - 1] = 0;
//			}
//			frame2 = frame1.clone();
//		}
//	}
//	float bAve = 0;
//	float gAve = 0;
//	float rAve = 0;
//	int num = 0;
//	for (int i = 0; i < imgNum - 1; i++)
//	{
//		if (B[i] >0 || G[i] > 0 || R[i] > 0)
//		{
//			bAve += B[i];
//			gAve += G[i];
//			rAve += R[i];
//			num++;
//
//		}
//	}
//	bAve = bAve / (num + 0.000001);
//	gAve = gAve / (num + 0.000001);
//	rAve = rAve / (num + 0.000001);
//	if ((int)bAve > 0 || (int)gAve > 0 || (int)rAve > 0)
//	{
//		param->infos[infoId].param.object.froth = true;
//		param->infos[infoId].param.object.color = ((int)(rAve + 0.5)) << 16;
//		param->infos[infoId].param.object.color += ((int)(gAve + 0.5)) << 8;
//		param->infos[infoId].param.object.color += (int)(bAve + 0.5);
//	}
//	else
//	{
//		param->infos[infoId].param.object.froth = false;
//		param->infos[infoId].param.object.color = 0;
//	}
//
//	LOG_WRITE("Foam:%d\n", param->infos[infoId].param.object.froth);
//	LOG_WRITE("Foam Color:(%d,%d,%d)\n", param->infos[infoId].param.object.color >> 16, param->infos[infoId].param.object.color >> 8 & 0xff, param->infos[infoId].param.object.color & 0xff);
//	LOG_WRITE("[EXIT] %s\n", __FUNCTION__);
//}
void zvanFoamDetect(Mat img[], int imgNum, int infoId, Zvan_Analysis_Params* param)
{
	int ValidCountPre = 0;
	LOG_WRITE("################################################################################################################################\n");
	LOG_WRITE("[ENTER] %s\n", __FUNCTION__);
	Mat frame1, frame2;
	float *B = new float[imgNum - 1];
	float *G = new float[imgNum - 1];
	float *R = new float[imgNum - 1];
	for (int i = 0; i < imgNum - 1; i++)
	{
		B[i] = 0;
		G[i] = 0;
		R[i] = 0;
	}
	param->infos[infoId].param.object.pixel = 0;
	param->infos[infoId].param.object.pixelpct = 0;
	for (int nFrmNum = 0; nFrmNum < imgNum; nFrmNum++)
	{
		if (nFrmNum == 0)
		{
			frame2 = img[nFrmNum].clone();
			GaussianBlur(frame2, frame2, Size(3, 3), 0, 0); //
		}
		else
		{
			frame1 = img[nFrmNum];
			GaussianBlur(frame1, frame1, Size(3, 3), 0, 0); //
			Mat gray1, gray2;
			cvtColor(frame1, gray1, CV_BGR2GRAY);
			cvtColor(frame2, gray2, CV_BGR2GRAY);
			Mat diff;
			absdiff(gray1, gray2, diff);
			threshold(diff, diff, 50, 255, CV_THRESH_BINARY);
			Mat lowLumaForm = Mat::zeros(frame1.size(), CV_8UC3);
			threshold(gray1, gray1, 150, 255, CV_THRESH_BINARY);
			threshold(gray2, gray2, 150, 255, CV_THRESH_BINARY);
			Mat Foam = Mat::zeros(frame1.size(), CV_8UC3);
			for (int row = 0; row < gray1.rows; row++)
			{
				uchar *pDiff = diff.ptr<uchar>(row);
				uchar *pGray1 = gray1.ptr<uchar>(row);
				uchar *pGray2 = gray2.ptr<uchar>(row);
				for (int col = 0; col < gray1.cols; col++)
				{
					if ((pDiff[col] >0 && pGray1[col]>0))
					{
						pGray1[col] = pGray1[col];
						Foam.at<Vec3b>(row, col)[0] = frame1.at<Vec3b>(row, col)[0];
						Foam.at<Vec3b>(row, col)[1] = frame1.at<Vec3b>(row, col)[1];
						Foam.at<Vec3b>(row, col)[2] = frame1.at<Vec3b>(row, col)[2];

					}
					else if ((pDiff[col] > 0 && pGray2[col] > 0))
					{
						pGray1[col] = pGray1[col];
						Foam.at<Vec3b>(row, col)[0] = frame2.at<Vec3b>(row, col)[0];
						Foam.at<Vec3b>(row, col)[1] = frame2.at<Vec3b>(row, col)[1];
						Foam.at<Vec3b>(row, col)[2] = frame2.at<Vec3b>(row, col)[2];
					}
					else
					{
						pGray1[col] = 0;
					}
				}
			}

			for (int row = 0; row < lowLumaForm.rows; row++)
			{
				uchar *pDiff = diff.ptr<uchar>(row);
				for (int col = 0; col < lowLumaForm.cols; col++)
				{
					float B_G = (float)frame1.at<Vec3b>(row, col)[0] / (float)frame1.at<Vec3b>(row, col)[1];
					float R_G = (float)frame1.at<Vec3b>(row, col)[2] / (float)frame1.at<Vec3b>(row, col)[1];
					float B_G2 = (float)frame2.at<Vec3b>(row, col)[0] / (float)frame2.at<Vec3b>(row, col)[1];
					float R_G2 = (float)frame2.at<Vec3b>(row, col)[2] / (float)frame2.at<Vec3b>(row, col)[1];
					if ((B_G > 0.95  && B_G < 1.05) && (R_G > 0.95  && R_G < 1.05) && pDiff[col] >0)
					{
						lowLumaForm.at<Vec3b>(row, col)[0] = frame1.at<Vec3b>(row, col)[0];
						lowLumaForm.at<Vec3b>(row, col)[1] = frame1.at<Vec3b>(row, col)[1];
						lowLumaForm.at<Vec3b>(row, col)[2] = frame1.at<Vec3b>(row, col)[2];
						Foam.at<Vec3b>(row, col)[0] = frame1.at<Vec3b>(row, col)[0];
						Foam.at<Vec3b>(row, col)[1] = frame1.at<Vec3b>(row, col)[1];
						Foam.at<Vec3b>(row, col)[2] = frame1.at<Vec3b>(row, col)[2];
					}
					else if (((B_G2 > 0.95 && B_G2 < 1.05) && (R_G2 > 0.95  && R_G2 < 1.05) && pDiff[col] >0))
					{
						lowLumaForm.at<Vec3b>(row, col)[0] = frame2.at<Vec3b>(row, col)[0];
						lowLumaForm.at<Vec3b>(row, col)[1] = frame2.at<Vec3b>(row, col)[1];
						lowLumaForm.at<Vec3b>(row, col)[2] = frame2.at<Vec3b>(row, col)[2];
						Foam.at<Vec3b>(row, col)[0] = frame2.at<Vec3b>(row, col)[0];
						Foam.at<Vec3b>(row, col)[1] = frame2.at<Vec3b>(row, col)[1];
						Foam.at<Vec3b>(row, col)[2] = frame2.at<Vec3b>(row, col)[2];
					}
					else
					{
						lowLumaForm.at<Vec3b>(row, col)[0] = 0;
						lowLumaForm.at<Vec3b>(row, col)[1] = 0;
						lowLumaForm.at<Vec3b>(row, col)[2] = 0;
					}

				}
			}
			int ValidCount = 0;

			for (int row = 0; row < gray1.rows; row++)
			{
				uchar *pGray1 = gray1.ptr<uchar>(row);
				for (int col = 0; col < gray1.cols; col++)
				{
					if (pGray1[col] >0 || (lowLumaForm.at<Vec3b>(row, col)[0] != 0 && lowLumaForm.at<Vec3b>(row, col)[1] != 0 && lowLumaForm.at<Vec3b>(row, col)[2] != 0))//frame1?????130,???????
					{
						ValidCount++;
					}
					else//???????130,???????
					{
						Foam.at<Vec3b>(row, col)[0] = 0;
						Foam.at<Vec3b>(row, col)[1] = 0;
						Foam.at<Vec3b>(row, col)[2] = 0;

					}
				}
			}
			Mat FoamGray;
			cvtColor(Foam, FoamGray, CV_BGR2GRAY);
			//?????
			int channels = 0;
			MatND dstHist;
			int histSize[] = { 256 };       //????int histSize = 256;????????????,??????&histSize  
			float midRanges[] = { 0, 256 };
			const float *ranges[] = { midRanges };
			calcHist(&FoamGray, 1, &channels, Mat(), dstHist, 1, histSize, ranges, true, false);
			double maxV, minV;
			Point maxP, minP;
			minMaxLoc(FoamGray, &minV, &maxV, &minP, &maxP);
			int i = maxV;
			int ThresholdFoam = 0;
			int sum = 0;
			while (i > 0)
			{
				sum = sum + dstHist.at<float>(i);
				if (sum > ValidCount*0.99)//?????130????????????30%????????
				{
					ThresholdFoam = i;
					break;
				}
				i = i - 1;
			}
			int HighLum = 0;
			for (int row = 0; row < FoamGray.rows; row++)
			{
				uchar *pFoamGray = FoamGray.ptr<uchar>(row);
				for (int col = 0; col < FoamGray.cols; col++)
				{
					if (pFoamGray[col] > ThresholdFoam)
					{
						B[nFrmNum - 1] += Foam.at<Vec3b>(row, col)[0];
						G[nFrmNum - 1] += Foam.at<Vec3b>(row, col)[1];
						R[nFrmNum - 1] += Foam.at<Vec3b>(row, col)[2];
						HighLum++;
					}
				}
			}

			if (ValidCountPre < ValidCount)
			{
				//param->infos[infoId].param.object.pixel = ValidCount;
				//param->infos[infoId].param.object.pixelpct = ((float)(ValidCount) / (float)(FoamGray.rows*FoamGray.cols));
				ValidCountPre = ValidCount;
			}

			if (ValidCount > 300)//??50????????
			{
				B[nFrmNum - 1] = B[nFrmNum - 1] / (HighLum + 0.000001);
				G[nFrmNum - 1] = G[nFrmNum - 1] / (HighLum + 0.000001);
				R[nFrmNum - 1] = R[nFrmNum - 1] / (HighLum + 0.000001);

			}
			else
			{
				B[nFrmNum - 1] = 0;
				G[nFrmNum - 1] = 0;
				R[nFrmNum - 1] = 0;
			}
			frame2 = frame1.clone();
		}
	}
	float bAve = 0;
	float gAve = 0;
	float rAve = 0;
	int num = 0;
	for (int i = 0; i < imgNum - 1; i++)
	{
		if (B[i] >0 || G[i] > 0 || R[i] > 0)
		{
			bAve += B[i];
			gAve += G[i];
			rAve += R[i];
			num++;

		}
	}
	bAve = bAve / (num + 0.000001);
	gAve = gAve / (num + 0.000001);
	rAve = rAve / (num + 0.000001);
	int trueFormCount = 0;
	int trueFormCountPre = 0;
	float formPct = 0;

	for (int i = 1; i < imgNum - 1; i++)
	{
		Mat trueForm = img[i].clone();
		Mat grayTrueForm;
		cvtColor(trueForm, grayTrueForm, CV_BGR2GRAY);
		for (int row = 0; row < trueForm.rows; row++)
		{
			for (int col = 0; col < trueForm.cols; col++)
			{

				if ((abs(trueForm.at<Vec3b>(row, col)[0] - bAve) < trueForm.at<Vec3b>(row, col)[0] * 0.15) && (abs(trueForm.at<Vec3b>(row, col)[1] - gAve) < trueForm.at<Vec3b>(row, col)[1] * 0.15) && (abs(trueForm.at<Vec3b>(row, col)[2] - rAve) < trueForm.at<Vec3b>(row, col)[2] * 0.15) && grayTrueForm.at<uchar>(row, col) > 145)//frame1?????130,???????
				{

					//trueFormCount++;

				}
				else//???????130,???????
				{
					trueForm.at<Vec3b>(row, col)[0] = 0;
					trueForm.at<Vec3b>(row, col)[1] = 0;
					trueForm.at<Vec3b>(row, col)[2] = 0;

				}
			}
		}
		Mat tmp;
		Mat trueFormgray;
		cvtColor(trueForm, trueFormgray, CV_BGR2GRAY);
		Mat structure_element = getStructuringElement(MORPH_RECT, Size(5, 5)); //ÉèÖÃÅòÕÍ/¸¯Ê´µÄºËÎª¾ØÐÎ£¬´óÐ¡Îª3*3
		erode(trueFormgray, trueFormgray, structure_element); //¸¯Ê´
		dilate(trueFormgray, trueFormgray, structure_element); //ÅòÕÍ

		cv::filterSpeckles(trueFormgray, 0, 30, 130, tmp);
		for (int row = 0; row < trueForm.rows; row++)
		{
			for (int col = 0; col < trueForm.cols; col++)
			{
				if (trueFormgray.at<uchar>(row, col) == 0)
				{
					trueForm.at<Vec3b>(row, col)[0] = 0;
					trueForm.at<Vec3b>(row, col)[1] = 0;
					trueForm.at<Vec3b>(row, col)[2] = 0;

				}
				else
				{
					trueFormCount++;
				}
			}
		}

		if (trueFormCount > trueFormCountPre)
		{
			param->infos[infoId].param.object.pixel = trueFormCount;
			param->infos[infoId].param.object.pixelpct = ((float)(trueFormCount) / (float)(trueForm.rows*trueForm.cols));
			trueFormCountPre = trueFormCount;

		}
		trueFormCount = 0;
	}
	formPct = ((float)(trueFormCountPre) / (float)(frame1.rows*frame1.cols));
	if (((int)bAve > 0 || (int)gAve > 0 || (int)rAve > 0) && (formPct >= param->infos[infoId].param.object.threshold))
	{
		param->infos[infoId].param.object.froth = true;
		param->infos[infoId].param.object.color = ((int)(rAve + 0.5)) << 16;
		param->infos[infoId].param.object.color += ((int)(gAve + 0.5)) << 8;
		param->infos[infoId].param.object.color += (int)(bAve + 0.5);
	}
	else
	{
		param->infos[infoId].param.object.froth = false;
		param->infos[infoId].param.object.color = 0;
	}
	delete[] B;
	delete[] G;
	delete[] R;
	LOG_WRITE("Foam:%d\n", param->infos[infoId].param.object.froth);
	LOG_WRITE("Foam Color:(%d,%d,%d)\n", param->infos[infoId].param.object.color >> 16, param->infos[infoId].param.object.color >> 8 & 0xff, param->infos[infoId].param.object.color & 0xff);
	LOG_WRITE("[EXIT] %s\n", __FUNCTION__);
}

/****************************************
* @brief ??????
* @param Zvan_Analysis_Params ?????????
* @return bool ??????????
* @remark ????????,?????????,
****************************************/
vector<Point2f> getConer(const Mat& grayImg) {
	vector<Point2f> corners;
	double qualityLevel = 0.01;//角点检测可接受的最小特征�?
	double minDistance = 10;//角点之间的最小距�?
	int blockSize = 3;//计算导数自相关矩阵时指定的邻域范�?
	double k = 0.04;//权重系数
	

	//�?】进行Shi-Tomasi角点检�?
	goodFeaturesToTrack(grayImg,//输入图像
		corners,//检测到的角点的输出向量
		1,//角点的最大数�?
		qualityLevel,//角点检测可接受的最小特征�?
		minDistance,//角点之间的最小距�?
		Mat(),//感兴趣区�?
		blockSize,//计算导数自相关矩阵时指定的邻域范�?
		false,//不使用Harris角点检�?
		k);//权重系数

	return corners;
}

float getDistance(const Mat& _firstGray, const Mat& _secondGray) {
	vector<Point2f> coner1, coner2;

	coner1 = getConer(_firstGray);
	coner2 = getConer(_secondGray);

	return abs(coner1[0].x - coner2[0].x) + abs(coner1[0].y - coner2[0].y);
}

void zvanObjectMoveDetect(Mat img[], int imgNum, int infoId, Zvan_Analysis_Params* param)
{
	LOG_WRITE("################################################################################################################################\n");
	LOG_WRITE("[ENTER] %s\n", __FUNCTION__);
	
	float moveThreshold = 10.0f;

	Mat firstImg, secondImg;
	cvtColor(img[0], firstImg, CV_BGR2GRAY);
	cvtColor(img[4], secondImg, CV_BGR2GRAY);

	GaussianBlur(firstImg, firstImg, Size(5, 5), 0);
	GaussianBlur(secondImg, secondImg, Size(5, 5), 0);

	if (getDistance(firstImg, secondImg) >= moveThreshold) {
		param->infos[infoId].param.activity = true;
	}
	else {
		param->infos[infoId].param.activity = false;
	}

	LOG_WRITE("Activity:%d\n", param->infos[infoId].param.activity);
	LOG_WRITE("[EXIT] %s\n", __FUNCTION__);
}
