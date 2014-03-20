#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>
#include "jpeglib.h"
#include "CV_Library.h"

int main()
{
	/*OPEN*/
	Image Img_src = ReadImage("test.jpg");
	Image Img_srDst = CreateNewImage(&Img_src, &Img_srDst, 3);
	Image Img_srDst2 = CreateNewImage(&Img_src, &Img_srDst2, 3);
	Image Img_dst = CreateNewImage(&Img_src, &Img_dst, 1);
	
	/*CREATE*/
	Image Img_dst2 = CreateNewImage(&Img_src, &Img_dst2, 1);

	struct point_xy CentralPoint;
	CentralPoint.X = 2000;// Img_dst.Width / 2 - 200;
	CentralPoint.Y = 920;// Img_dst.Height / 2 + 200;
	
	/*SET destination*/
	//SetDestination(&Img_src, &Img_srDst);

	/*BLUR*/
	//BlurImageAroundPoint(&Img_src, &Img_dst, CentralPoint, 17, 3, BLUR_AROUND_CENTER, 100 );
	
	/*BLUR - gaussian*/
	//BlurImageGussian(&Img_src, &Img_dst, 15, 0.6);

	/*ROTATE*/
	//RotateImage(&Img_src, &Img_dst, 180, CentralPoint);
	
	/*BRIGHTNESS*/
	//BrightnessCorrection(&Img_src, &Img_srDst, 3, PERCENTAGE_ALGO);
	
	/*CONRAST*/
	//ContrastCorrection(&Img_srDst, &Img_srDst2, 3);
	
	/*NOISE*/
	//NoiseCorrection(&Img_src, &Img_srDst, 60, 1);
	
	/*WHITE Balance*/
	//WhiteBalanceCorrection(&Img_src, &Img_srDst, WB_GREEN_WORLD);
	
	/*GAMMA*/
	//GammaCorrection(&Img_srDst2, &Img_srDst, 1.5, 1.5, 1.5);

	/*GrayScale - result in 3 channels*/
	//ConvertToGrayscale_3Channels(&Img_src, &Img_dst);
	//ConvertToGrayscale_1Channel(&Img_src, &Img_dst);
	
	/*ZOOM - in_or_out +-Percentage (SCALE)*/
	//ScaleImage(&Img_src, &Img_srDst, -70);

	/*TRANSLATION*/
	//TranslateImage(&Img_src, &Img_dst, CentralPoint);
	
	/*EDGE - Contour*/
	//EdgeExtraction(&Img_dst, &Img_dst2, EDGES_PREWITT , 1, 0.9);
	
	/*MIRROR*/
	//MirrorImageHorizontal(&Img_src, &Img_srDst);
	//MirrorImageVertical(&Img_src, &Img_srDst);

	/*CROP*/
	//CropImage(&Img_src, &Img_srDst, CentralPoint,500,500 );

	/*MORPH*/
	//MorphDilate(&Img_dst2, &Img_dst, 3 , 7);
	//MorphErode(&Img_dst2, &Img_dst, 3, 25);

	/*SHARP*/
	SharpImageContours(&Img_src, &Img_srDst2, 100);

	/*WRITE*/
	WriteImage("Result.jpg", Img_srDst2, QUALITY_MAX);

	/* DESTROY images*/
	DestroyImage(&Img_src);
	DestroyImage(&Img_dst);
	DestroyImage(&Img_dst2);
	DestroyImage(&Img_srDst);
	DestroyImage(&Img_srDst2);

	return 0;
}