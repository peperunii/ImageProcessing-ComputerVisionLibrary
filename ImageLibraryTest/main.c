#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>
#include "jpeglib.h"
#include "CV_Library.h"

int main()
{
	/*OPEN*/
	Image Img_src = ReadImage("C:\\Users\\Petar\\Downloads\\kostenurka.jpg");
	Image Img_dst = CreateNewImage(Img_src);
	struct point_xy CentralPoint;
	CentralPoint.X = Img_dst.Width / 2;
	CentralPoint.Y = Img_dst.Height / 2;

	/*BLUR*/
	//Img_dst = BlurImage(Img_src, CentralPoint, 15, 6, 1, 80 );
	/*ROTATE*/
	Img_dst = RotateImage(Img_src, 180, CentralPoint);
	/*BRIGHTNESS*/
	//Img_dst = BrightnessCorrection(Img_dst, 25);
	/*CONRAST*/
	//Img_dst = ContrastCorrection(Img_dst, 15);
	/*WRITE*/
	Img_dst =  NoiseCorrection(Img_dst,60,1);
    WriteImage("C:\\Users\\Petar\\Downloads\\pic.jpg", Img_dst, 100) ;
	return 0;
}