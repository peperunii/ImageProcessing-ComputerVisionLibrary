#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>
#include "jpeglib.h"

#include "CV_Library.h"

int main()
{
	/*OPEN*/
	Image Img_src = ReadImage("flower.jpg");
	
	/*CREATE*/
	Image Img_srDst = CreateNewImage_BasedOnPrototype(&Img_src, &Img_srDst);
	Image Img_srDst2 = CreateNewImage_BasedOnPrototype(&Img_src, &Img_srDst2);
	Image Img_dst = CreateNewImage(&Img_dst, Img_src.Width, Img_src.Height, 1, 1);
	Image Img_dst2 = CreateNewImage(&Img_dst2, Img_src.Width, Img_src.Height, 1, 1);

	struct point_xy CentralPoint;
	struct ColorPoint_RGB ColorPoint;
	struct WhitePoint WhitePoint_lab1;
	struct WhitePoint WhitePoint_lab2;

	CentralPoint.X = 2000;// Img_dst.Width / 2 - 200;
	CentralPoint.Y = 920;// Img_dst.Height / 2 + 200;
	
	ColorPoint.R = 230;
	ColorPoint.G = 150;
	ColorPoint.B = 140;
	
	SetWhiteBalanceValues(&WhitePoint_lab1, D65_6504K);
	SetWhiteBalanceValues(&WhitePoint_lab2, D55_5500K);

	/*SET destination*/
	//SetDestination(&Img_src, &Img_srDst);

	/*BLUR*/
	//BlurImageAroundPoint(&Img_src, &Img_dst, CentralPoint, 17, 3, BLUR_AROUND_CENTER, 100 );
	
	/*BLUR - gaussian*/
	//BlurImageGussian(&Img_src, &Img_dst, 15, 0.6);

	/*ROTATE*/
	//RotateImage(&Img_src, &Img_dst, 180, CentralPoint);
	
	/*BRIGHTNESS*/
	//BrightnessCorrection(&Img_src, &Img_srDst, 15, BRIGHTNESS_PERCENTAGE_ALGO);
	
	/*NOISE*/
	//NoiseCorrection(&Img_src, &Img_srDst, 60, 1);
	
	/*GAMMA*/
	//GammaCorrection(&Img_src, &Img_srDst, 0.9, 0.9, 0.9);
	
	/*CONRAST*/
	//ContrastCorrection(&Img_srDst, &Img_srDst2, 5);

	/*WHITE Balance*/
	//WhiteBalanceCorrectionRGB(&Img_src, &Img_srDst, WB_GREEN_WORLD);

	/*GrayScale - result in 3 channels*/
	//ConvertToGrayscale_3Channels(&Img_src, &Img_dst);
	//ConvertToGrayscale_1Channel(&Img_src, &Img_dst);
	
	/*ZOOM - in_or_out +-Percentage (SCALE)*/
	//ScaleImage(&Img_src, &Img_srDst, -50);

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
	//MorphDilate(&Img_dst, &Img_dst2, 3 , 7);
	//MorphErode(&Img_dst, &Img_dst2, 3, 1);

	/*SHARP*/
	//SharpImageContours(&Img_src, &Img_srDst2, 60);

	/*COLOR  from gray*/
	//ColorFromGray(&Img_dst, &Img_srDst2, ColorPoint);

	/*BINARY  image*/
	//ConvertToBinary(&Img_src, &Img_dst, 0);
	
	/*RGB to HSL convert*/
	//ConvertImage_RGB_to_HSL(&Img_src, &Img_srDst);

	/*HSL to RGB convert*/
	//ConvertImage_HSL_to_RGB(&Img_srDst, &Img_srDst2);

	/*SATURATION*/
	//Saturation(&Img_srDst, &Img_srDst2, -50);
	
	/* RGB, XYZ, LAB convert */
	//ConvertImage_RGB_to_LAB(&Img_src, &Img_srDst2, WhitePoint_lab1);

	//ConvertImage_LAB_to_RGB(&Img_srDst2, &Img_srDst, WhitePoint_lab2);

	/*WRITE*/
	WriteImage("Result.jpg", Img_src, QUALITY_MAX);

	/* DESTROY images*/
	DestroyImage(&Img_src);
	DestroyImage(&Img_dst);
	DestroyImage(&Img_dst2);
	DestroyImage(&Img_srDst);
	DestroyImage(&Img_srDst2);

	return 0;
}