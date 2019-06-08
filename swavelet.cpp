
#include "swavelet.h"
#include "windows.h"
// 二维静态离散小波变换（单通道浮点图像）Haar提升小波
void WaveletAnalysis(CvMat *pImage, CvMat* pMatenergy,int nLayer)
{
   // 执行条件
   if (pImage)
   {
      if (((pImage->width >> nLayer) << nLayer) == pImage->width &&
         ((pImage->height >> nLayer) << nLayer) == pImage->height)
      {
         int     i, j,x, y, n;
         int     nWidth   = pImage->width;
         int     nHeight  = pImage->height;
         int     nHalfW   = nWidth / 2;
         int     nHalfH   = nHeight / 2;
         float **pData    = new float*[pImage->height];
         float  *pRow     = new float[pImage->width];
         float  *pColumn  = new float[pImage->height];
		 int step = pImage->step/sizeof(float);
	     double temp;
		 CvMat* pMat = cvCloneMat(pImage);
		 CvMat* pWeight = cvCreateMat(nHalfH, nHalfW, CV_32FC1);


         for (i = 0; i < pImage->height; i++)
         {
            pData[i] = (float*) (pMat->data.fl + step * i);
         }
         // 多层小波变换
         for (n = 0; n < nLayer; n++, nWidth /= 2, nHeight /= 2, nHalfW /= 2, nHalfH /= 2)
         {
            // 水平变换
            for (y = 0; y < nHeight; y++)
            {
               // 
               memcpy(pRow, pData[y], sizeof(float) * nWidth);
               for (i = 0; i < nHalfW; i++)
               {
                  x = i * 2;
                  pData[y][i] = pRow[x];
                  pData[y][nHalfW + i] = pRow[x + 1];
               }
               // 提升小波变换
               for (i = 0; i < nHalfW ; i++)
               {
                  pData[y][nHalfW + i] =pData[y][nHalfW + i]-pData[y][i] ;
               }
               for (i = 0; i < nHalfW; i++)
               {
                  pData[y][i] = pData[y][i]+pData[y][nHalfW + i]/2;
               }
            }
            // 垂直变换
            for (x = 0; x < nWidth; x++)
            {
               // 奇偶分离
               for (i = 0; i < nHalfH; i++)
               {
                  y = i * 2;
                  pColumn[i] = pData[y][x];
                  pColumn[nHalfH + i] = pData[y + 1][x];
               }
               for (i = 0; i < nHeight; i++)
               {
                  pData[i][x] = pColumn[i];
               }
               // 提升小波变换
               for (i = 0; i < nHalfH; i++)
               {
       
                  pData[nHalfH + i][x] = pData[nHalfH + i][x]-pData[i][x];
               }
               for (i = 0; i < nHalfH; i++)
               {
                  pData[i][x] = pData[i][x]+pData[nHalfH + i][x]/2;
               }
            }
         }

		 nWidth   = pImage->width;
         nHeight  = pImage->height;
         nHalfW   = nWidth / 2;
         nHalfH   = nHeight / 2;
		 cvZero(pMatenergy);
		 for(i=0;i<nHalfH;i++)
			 for(j=0;j<nHalfW;j++)
			 {
				temp = pData[i+nHalfH][j]*pData[i+nHalfH][j]+pData[i][j+nHalfW]*pData[i][j+nHalfW]
					   +pData[i+nHalfH][j+nHalfW]*pData[i+nHalfH][j+nHalfW];
				temp = sqrt(temp);
				cvSetReal2D(pWeight,i,j,temp);
			 }
		 	
		 cvResize(pWeight,pMatenergy,CV_INTER_LINEAR);
		 
         delete[] pData;
         delete[] pRow;
         delete[] pColumn;
		 cvReleaseMat(&pMat);
		 cvReleaseMat(&pWeight);
      }
   }
}

