/********************************************************************************
*																				*
*	CV Library - Main file														*
*																				*
*	Author:  Petar Nikolov														*
*																				*
*																				*
*	This library provide image processing and computer vision algorithms.		*
*	It can be used in any projects for free.									*
*   If you find some errors, or if you want to contact me for any reason - 		*
*	please use my e-mail:														*
*						p.bijev@gmail.com										*
*																				*
*																				*
*																				*
*																				*
*********************************************************************************/

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>
#include <string.h>
#include "jpeglib.h"

#include "CV_Library.h"

int main()
{
	int i,j;
	float hui;
	FILE *LUT = NULL;
	struct Image *Layers = NULL;
	struct Image *ptrToImage = NULL;
	/*OPEN*/
	Image Img_src = ReadImage("Dimo2.jpg");
	
	/*CREATE*/
	Image Img_srDst = CreateNewImage_BasedOnPrototype(&Img_src, &Img_srDst);
	Image Img_srDst2 = CreateNewImage_BasedOnPrototype(&Img_src, &Img_srDst2);
	Image Img_srDst3 = CreateNewImage_BasedOnPrototype(&Img_src, &Img_srDst3);
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
	
	fopen_s(&LUT, "Kelvins-x-z.txt","rt");
	if(LUT == NULL) return 0;
	for(i = 0; i < 391; i++)
	{
		fscanf(LUT,"%d %f %f\n", &Kelvin_LUT[i], &X_LUT[i], &Y_LUT[i]);
	}
	/*LAYERS*/
	//Layers = CreateImageLayersBasedOnPrototype(&Img_src, 100);

	// The color temperature of the imput image
	SetWhiteBalanceValues(&WhitePoint_lab1, 7);
	// The color temperature of the output image
	//SetWhiteBalanceValues(&WhitePoint_lab2, WHITE_5000K_D50_DAYLIGHT_RENDER);

	/*SET destination*/
	//SetDestination(&Img_src, &Img_srDst);

	/*BLUR*/
	//BlurImageAroundPoint(&Img_src, &Layers[0], CentralPoint, 17, 3, BLUR_AROUND_CENTER, 100 );

	/*BLUR - gaussian*/
	//BlurImageGussian(&Img_srDst, &Img_srDst2, 15, 0.7);

	/*ROTATE*/
	//RotateImage(&Img_src, &Img_dst, 180, CentralPoint);
	
	/*BRIGHTNESS*/
	BrightnessCorrection(&Img_srDst, &Img_srDst2, 230, BRIGHTNESS_PERCENTAGE_ALGO);
	
	/*NOISE*/
	//NoiseCorrection(&Img_src, &Img_srDst, 60, 1);
	
	/*GAMMA*/
	//GammaCorrection(&Img_src, &Img_srDst3, 1.2, 0.6, 1.2);
	
	/*CONRAST*/
	ContrastCorrection(&Img_srDst2, &Img_srDst, 50);

	/* COLOR TEMPERATURE */
	//ColorTemperature()

	/*WHITE BALANCE*/
	//WhiteBalanceCorrectionRGB(&Img_src, &Layers[0], 1);
	
	/* WHITE BALANCE - convert to XYZ. Set WhitePoints first */
	//WhiteBalanceGREENY(&Img_src, &Layers[1], WhitePoint_lab1);

	/*WHITE BALANCE - using T and RGB*/
	//WhitebalanceCorrectionBLUEorRED(&Img_srDst2, &Img_srDst, WhitePoint_lab1);

/*	j = 0;
	for (i = -50; i < 50; i++)
	{
		BrightnessCorrection(&Img_src, &Layers[j++], 2 * (i), BRIGHTNESS_PERCENTAGE_ALGO);
	}
*/	
	/*MASK create*/
	//Img_dst = CreateMaskForLayers(&Layers[0], 2, 100);

	/*MERGE Layers*/
	//CombineLayers(Layers, &Img_srDst, Img_dst);

	/*GrayScale - result in 3 channels*/
	//ConvertToGrayscale_3Channels(&Img_src, &Img_dst);
	//ConvertToGrayscale_1Channel(&Img_src, &Img_dst);
	
	/*ZOOM - in_or_out +-Percentage (SCALE)*/
	//ScaleImage(&Img_src, &Img_srDst, 100);

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
	Saturation(&Img_srDst, &Img_srDst2, 15);

	/* RGB, XYZ, LAB convert */
	//ConvertImage_RGB_to_LAB(&Img_src, &Img_srDst2, WhitePoint_lab1);

	//ConvertImage_LAB_to_RGB(&Img_srDst2, &Img_srDst, WhitePoint_lab2);
	//hui = pow_func(2, 2.5, 2);
	
	/*WRITE*/
	WriteImage("Result_Brightness.jpg", Img_srDst2, QUALITY_MAX);

	//WriteImage("minimacbeth\\Result_Greeny.jpg", Layers[1], QUALITY_MAX);

	//WriteImage("minimacbeth\\Result_BlueOrRed.jpg", Layers[2], QUALITY_MAX);

	/* DESTROY images*/

	if(Layers != NULL)DestroyImage(Layers);
	DestroyImage(&Img_src);
	DestroyImage(&Img_dst);
	DestroyImage(&Img_dst2);
	DestroyImage(&Img_srDst);
	DestroyImage(&Img_srDst2);
	DestroyImage(&Img_srDst3);

	fclose(LUT);
	return 0;
}