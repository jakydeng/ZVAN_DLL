#ifndef _SWAVELET_H_
#define _SWAVELET_H_

#include "cv.h"
#include <stdlib.h>
#include <math.h>

void WaveletAnalysis(CvMat *pImage, CvMat* pMatenergy,int nLayer);

#endif