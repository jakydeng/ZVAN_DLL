#ifndef __PROCESS_H__
#define __PROCESS_H__

#include "Zvan_VIA_SDK.h"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/ml/ml.hpp>
using namespace cv;


void zvanLiquidColorDetect(Mat img[], int imgNum, int infoId, Zvan_Analysis_Params* param);
void zvanLiquidHeightDetect(Mat img[], int imgNum, int infoId, Zvan_Analysis_Params* param);
void zvanLiquidTurbidityDetect(Mat img[], int imgNum, int infoId, Zvan_Analysis_Params* param);
void zvanSmogColorDetect(Mat img[], int imgNum, int infoId, Zvan_Analysis_Params* param);
void zvanSmokeMoveDetect(Mat img[], int imgNum, int infoId, Zvan_Analysis_Params* param);
void zvanWaterMoveDetect(Mat img[], int imgNum, int infoId, Zvan_Analysis_Params* param);
void zvanFoamDetect(Mat img[], int imgNum, int infoId, Zvan_Analysis_Params* param);
void zvanObjectMoveDetect(Mat img[], int imgNum, int infoId, Zvan_Analysis_Params* param);
bool  Zvan_VIA_Analysis(Zvan_Analysis_Params* param);

#endif
