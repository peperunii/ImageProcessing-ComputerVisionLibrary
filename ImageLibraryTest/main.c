#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>
#include "jpeglib.h"
#include "CV_Library.h"

int main()
{
	/*OPEN*/
	Image Img_src = ReadImage("C:\\Users\\petar.nikolov\\Downloads\\attachments\\2 et.JPG");
	Image Img_dst = CreateNewImage(Img_src);
	struct point_xy CentralPoint;
	CentralPoint.X = Img_dst.Width/2;
	CentralPoint.Y = Img_dst.Height / 2;



	/*BLUR*/
	//Img_dst = BlurImage(Img_src, CentralPoint, 7, 6, 0, 50 );
	/*ROTATE*/
	//Img_dst = RotateImage(Img_dst, 90, CentralPoint);
	/*BRIGHTNESS*/
	//Img_dst = BrightnessCorrection(Img_src, 25);
	/*CONRAST*/
	//Img_dst = ContrastCorrection(Img_src, 50);
	/*WRITE*/
    WriteImage("D:\\output_brightness.jpg", Img_dst, 100);
	return 0;
}