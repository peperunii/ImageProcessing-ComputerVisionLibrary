#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>
#include "jpeglib.h"
#include "CV_Library.h"

int main()
{
	/*http://rosettacode.org/wiki/Canny_edge_detector*/

	/*OPEN*/
	Image Img_src = ReadImage("test.jpg");
	Image Img_srDst = CreateNewImage(&Img_src, &Img_srDst, 3);
	Image Img_srDst2 = CreateNewImage(&Img_src, &Img_srDst2, 3);
	Image Img_dst = CreateNewImage(&Img_src, &Img_dst, 1);
	Image Img_dst2 = CreateNewImage(&Img_src, &Img_dst2, 1);

	struct point_xy CentralPoint;
	CentralPoint.X = Img_dst.Width / 2 - 200;
	CentralPoint.Y = Img_dst.Height / 2 + 200;

	/*BLUR*/
	//BlurImageAroundPoint(&Img_src, &Img_dst, CentralPoint, 17, 3, 1, 100 );
	
	/*BLUR - gaussian*/
	//BlurImageGussian(&Img_src, &Img_dst, 15, 0.6);

	/*ROTATE*/
	//RotateImage(&Img_src, &Img_dst, 180, CentralPoint);
	
	/*BRIGHTNESS*/
	//BrightnessCorrection(&Img_src, &Img_srDst, 25);
	
	/*CONRAST*/
	//ContrastCorrection(&Img_srDst, &Img_srDst2, -5);
	
	/*NOISE*/
	//NoiseCorrection(&Img_src, &Img_srDst, 60, 1);
	
	/*WHITE Balance*/
	WhiteBalanceCorrection(&Img_src, &Img_srDst, 4);
	
	/*GAMMA*/
	//GammaCorrection(&Img_src, &Img_srDst2, 0.6, 0.65, 0.45);

	/*GrayScale - result in 3 channels*/
	//ConvertToGrayscale_3Channels(&Img_src, &Img_dst);
	//ConvertToGrayscale_1Channel(&Img_src, &Img_dst);
	
	/*ZOOM - in_or_out +-Percentage (SCALE)*/
	//ScaleImage(&Img_srDst, &Img_srDst2, 30);

	/*TRANSLATION*/
	//TranslateImage(&Img_src, &Img_dst, CentralPoint);
	
	/*EDGE - Contour*/
	//EdgeExtraction(&Img_dst, &Img_dst2, 3, 1, 0.9);
	
	/*WRITE*/
	WriteImage("Result.jpg", Img_srDst, 100);
	
	/* DESTROY images*/
	DestroyImage(&Img_src);
	DestroyImage(&Img_dst);
	DestroyImage(&Img_dst2);
	DestroyImage(&Img_srDst);
	DestroyImage(&Img_srDst2);

	return 0;
}