/********************************************************************************
*																				*
*	CV Library - SpaceConversions.c												*
*																				*
*	Author:  Petar Nikolov														*
*																				*
*																				*
*	The algorithms included in this file are:									*
*																				*
*	- Convert to Grayscale	1 / 3 channels										*
*	- Convert to Binary															*
*	- RGB_to_HSL																*
*	- HSL_to_RGB																*
*	- RGB_to_XYZ																*
*	- XYZ_to_RGB																*
*	- RGB_to_LAB																*
*	- LAB_to_RGB																*
*	- Color Temperature															*
*																				*
*********************************************************************************/

#include <stdio.h>
#include "jpeglib.h"
#include <setjmp.h>
#include <math.h>
#include "CV_Library.h"
#include <string.h>
#include <stdlib.h>

#include <complex.h>

//#undef I
//#define J _Imaginary_I

/* 
	convert to    G R A Y S C A L E  - 3 channels RGB
*/
struct Image ConvertToGrayscale_3Channels(struct Image *Img_src, struct Image *Img_dst)
{
	int i, j;
	
	for (i = 0; i < Img_dst->Height; i++)
	{
		for (j = 0; j < Img_dst->Width; j++)
		{
			Img_dst->rgbpix[3 * (i*Img_src->Width + j)] = 0.3 * Img_src->rgbpix[3 * (i*Img_src->Width + j)] + 0.59 * Img_src->rgbpix[3 * (i*Img_src->Width + j) + 1] + 0.11 * Img_src->rgbpix[3 * (i*Img_src->Width + j) + 2];
			Img_dst->rgbpix[3 * (i*Img_src->Width + j) + 1] = Img_dst->rgbpix[3 * (i*Img_src->Width + j)];
			Img_dst->rgbpix[3 * (i*Img_src->Width + j) + 2] = Img_dst->rgbpix[3 * (i*Img_src->Width + j)];
		}
	}

	return *Img_dst;
}

/*
	convert to    G R A Y S C A L E  - 1 channel
*/
struct Image ConvertToGrayscale_1Channel(struct Image *Img_src, struct Image *Img_dst)
{
	int i, j;
	
	if (Img_dst->isLoaded != 1) return *Img_dst;
	
	for (i = 0; i < Img_dst->Height; i++)
	{
		for (j = 0; j < Img_dst->Width; j++)
		{
			Img_dst->rgbpix[(i*Img_src->Width + j)] = 0.3 * Img_src->rgbpix[3 * (i*Img_src->Width + j)] + 0.59 * Img_src->rgbpix[3 * (i*Img_src->Width + j) + 1] + 0.11 * Img_src->rgbpix[3 * (i*Img_src->Width + j) + 2];
		}
	}
}


/*
	B I N A R Y  image 
*/
struct Image ConvertToBinary(struct Image *Img_src, struct Image *Img_dst, int Threshold)
{
	int i, j, l;
	int Sum = 0;
	int AverageGray = 0;

	struct Image GrayImage = CreateNewImage(&GrayImage, Img_src->Width, Img_src->Height, 1, COLORSPACE_GRAYSCALE, 8);

	if (Img_src->Width * Img_src->Height != Img_dst->Width * Img_dst->Height)
	{
		SetDestination(Img_src, Img_dst);
	}
	if (Img_src->Num_channels != 1)
	{
		ConvertToGrayscale_1Channel(Img_src, &GrayImage);
	}
	if (Threshold == 0)
	{
		for (i = 0; i < Img_dst->Height; i++)
		{
			for (j = 0; j < Img_dst->Width; j++)
			{
				if (Img_src->Num_channels != 1)
					Sum += GrayImage.rgbpix[i * Img_dst->Width + j];
				else
					Sum += Img_src->rgbpix[i * Img_dst->Width + j];
			}
		}
		AverageGray = Sum / (float)(Img_dst->Width * Img_dst->Height);
	}
	else
		AverageGray = Threshold;
	for (i = 0; i < Img_dst->Height; i++)
	{
		for (j = 0; j < Img_dst->Width; j++)
		{
			if (Img_src->Num_channels == 1)
			{
				if (Img_src->rgbpix[i * Img_src->Width + j] > AverageGray)
					Img_dst->rgbpix[i * Img_src->Width + j] = 255;
				else
					Img_dst->rgbpix[i * Img_src->Width + j] = 0;
			}
			else
			{
				if (GrayImage.rgbpix[i * Img_src->Width + j] > AverageGray)
					Img_dst->rgbpix[i * Img_src->Width + j] = 255;
				else
					Img_dst->rgbpix[i * Img_src->Width + j] = 0;
			}
		}
	}

	DestroyImage(&GrayImage);
	return *Img_dst;
}

/*
	Chech Color values - Used in HSL - RGB conversion
*/
float CheckColorValue(float TempColor, float Temporary_1, float Temporary_2)
{
	float NewColor;

	if (6 * TempColor < 1)
	{
		NewColor = Temporary_2 + ((Temporary_1 - Temporary_2) * 6 * TempColor);
	}
	else
	{
		if (2 * TempColor < 1)
		{
			NewColor = Temporary_1;
		}
		else
		{
			if (3 * TempColor < 2)
			{
				NewColor = Temporary_2 + ((Temporary_1 - Temporary_2) * 6 * (0.6666 - TempColor));
			}
			else
				NewColor = Temporary_2;
		}
	}

	return NewColor;
}

/*
	C O N V E R T -  RGB to HSV
*/
void ConvertImage_RGB_to_HSL(struct Image *Img_src, struct Image *Img_dst)
{
	FILE *fdebug = NULL;
	float R_scaled;
	float G_scaled;
	float B_scaled;
	float C_max;
	float C_min;
	float Delta;
	float Hue;

	float del_R, del_B, del_G;
	float Saturation, Luma = 0;

	int i, j;

	if ((Img_src->Num_channels != 3) || (Img_dst->Num_channels != 3))
	{
		#ifdef DEBUG_FILE
				fdebug = fopen(DEBUG_FILE, "wt");
				fprintf(fdebug, "The input images are not with 3 channels\n");
				fclose(fdebug);
		#endif // DEBUG_FILE
		return;
	}

	if (Img_src->Width * Img_src->Height != Img_dst->Width * Img_dst->Height)
	{
		SetDestination(Img_src, Img_dst);
	}

	if (Img_src->ColorSpace != 2)
	{
		#ifdef DEBUG_FILE
				fdebug = fopen(DEBUG_FILE, "wt");
				fprintf(fdebug, "The input image is not in RGB format\n");
				fclose(fdebug);
		#endif // DEBUG_FILE
		return;
	}

	Img_dst->ColorSpace = 5;


	for (i = 0; i < Img_dst->Height; i++)
	{
		for (j = 0; j < Img_dst->Width; j++)
		{
			R_scaled = Img_src->rgbpix[3 * (i * Img_src->Width + j) + 0] / (float)255;
			G_scaled = Img_src->rgbpix[3 * (i * Img_src->Width + j) + 1] / (float)255;
			B_scaled = Img_src->rgbpix[3 * (i * Img_src->Width + j) + 2] / (float)255;
			C_max = MAX(R_scaled, MAX(G_scaled, B_scaled));
			C_min = MIN(R_scaled, MIN(G_scaled, B_scaled));
			Delta = C_max - C_min;

			// HUE
			if (C_max == R_scaled)
			{
				Hue = RoundValue_toX_SignificantBits((60 * (fmod(((G_scaled - B_scaled) / Delta), 6))), 2);
				if (Hue < 0) Hue = 360 + Hue;
				if (Hue > 360) Hue = fmod(Hue, 360);
			}
			else if (C_max == G_scaled)
			{
				Hue = RoundValue_toX_SignificantBits((60 * (((B_scaled - R_scaled) / Delta) + 2)),2);
				if (Hue < 0) Hue = 360 + Hue;
				if (Hue > 360) Hue = fmod(Hue, 360);
			}
			else if (C_max == B_scaled)
			{
				Hue = RoundValue_toX_SignificantBits((60 * (((R_scaled - G_scaled) / Delta) + 4)),2);
				if (Hue < 0) Hue = 360 + Hue;
				if (Hue > 360) Hue = fmod(Hue, 360);
			}
			

			// LUMA
			Luma = (C_max + C_min) / (float)2;

			// SATURATION
			if (Delta == 0) 
			{
				Saturation = 0; 
				Hue = 0;
			}
			else
			{ 
				Saturation = Luma > 0.5 ? Delta / (float)(2 - C_max - C_min) : Delta / (float)(C_max + C_min);
			}

			Img_dst->rgbpix[3 * (i * Img_src->Width + j) + 0] = RoundValue_toX_SignificantBits((Hue / (float)360) * 100, 2);
			Img_dst->rgbpix[3 * (i * Img_src->Width + j) + 1] = RoundValue_toX_SignificantBits(Saturation * 100, 2);
			Img_dst->rgbpix[3 * (i * Img_src->Width + j) + 2] = Luma * 100;//round(Luma * 100);

		}
	}
}

/*
	C O N V E R T -  HSL to RGB
*/
void ConvertImage_HSL_to_RGB(struct Image *Img_src, struct Image *Img_dst)
{
	FILE *fdebug = NULL;
	float R_temp;
	float G_temp;
	float B_temp;
	float C, X, m;
	float Hue;
	float Temporary_1, Temporary_2;
	float Saturation, Luma = 0;

	int i, j;

	if ((Img_src->Num_channels != 3) || (Img_dst->Num_channels != 3))
	{
		#ifdef DEBUG_FILE
				fdebug = fopen(DEBUG_FILE, "wt");
				fprintf(fdebug, "The input images are not with 3 channels\n");
				fclose(fdebug);
		#endif // DEBUG_FILE
		return;
	}

	if (Img_src->Width * Img_src->Height != Img_dst->Width * Img_dst->Height)
	{
		SetDestination(Img_src, Img_dst);
	}

	if (Img_src->ColorSpace != 5)
	{
		#ifdef DEBUG_FILE
				fdebug = fopen(DEBUG_FILE, "wt");
				fprintf(fdebug, "The input image is not in HSL format\n");
				fclose(fdebug);
		#endif // DEBUG_FILE
		return;
	}

	Img_dst->ColorSpace = 2;


	for (i = 0; i < Img_dst->Height; i++)
	{
		for (j = 0; j < Img_dst->Width; j++)
		{
			
			Hue = Img_src->rgbpix[3 * (i * Img_src->Width + j) + 0] / (float)100;
			
			Saturation = Img_src->rgbpix[3 * (i * Img_src->Width + j) + 1] / (float)100;

			Luma = Img_src->rgbpix[3 * (i * Img_src->Width + j) + 2] / (float)100;
			
			if (Saturation == 0)
			{
				R_temp = Luma * 255;
				if (R_temp > 255) 
				{
					R_temp = 255; B_temp = 255; G_temp = 255;
				}
				else
					B_temp = G_temp = R_temp;
			}
			else
			{
				if (Luma >= 0.5)
				{
					Temporary_1 = ((Luma + Saturation)  - (Luma * Saturation));
				}
				else
				{
					Temporary_1 = Luma * (1 + Saturation);
				}
				Temporary_2 = 2 * Luma - Temporary_1;
				
				R_temp = Hue + 0.33333;
				if (R_temp < 0) R_temp += 1;
				if (R_temp > 1) R_temp -= 1;
				
				G_temp = Hue;
				if (G_temp < 0) G_temp += 1;
				if (G_temp > 1) G_temp -= 1;

				B_temp = Hue - 0.33333;
				if (B_temp < 0) B_temp += 1;
				if (B_temp > 1) B_temp -= 1;

				// Check R
				R_temp = CheckColorValue(R_temp, Temporary_1, Temporary_2);

				// Check G
				G_temp = CheckColorValue(G_temp, Temporary_1, Temporary_2);

				// Check B
				B_temp = CheckColorValue(B_temp, Temporary_1, Temporary_2);

				
				R_temp *= 255;
				if (R_temp > 255) R_temp = 255;
				G_temp *= 255;
				if (G_temp > 255) G_temp = 255;
				B_temp *= 255;
				if (B_temp > 255) B_temp = 255;
			}
			
			Img_dst->rgbpix[3 * (i * Img_src->Width + j) + 0] = RoundValue_toX_SignificantBits(R_temp,2);
			Img_dst->rgbpix[3 * (i * Img_src->Width + j) + 1] = RoundValue_toX_SignificantBits(G_temp,2);
			Img_dst->rgbpix[3 * (i * Img_src->Width + j) + 2] = RoundValue_toX_SignificantBits(B_temp,2);

		}
	}
}


/*
	C O N V E R T  - RGB to XYZ 
*/
void Convert_RGB_to_XYZ(struct Image *Img_src, struct Image *Img_dst)
{
	int i, j;
	float X, Y, Z;
	float var_R, var_B, var_G;

	for (i = 0; i < Img_src->Height; i++)
	{
		for (j = 0; j < Img_src->Width; j++)
		{
			var_R = Img_src->rgbpix[3 * (i * Img_dst->Width + j) + 0];
			var_G = Img_src->rgbpix[3 * (i * Img_dst->Width + j) + 1];
			var_B = Img_src->rgbpix[3 * (i * Img_dst->Width + j) + 2];

			//if (i == 638 && j == 2372)
			//	_getch();
			var_R /= 255;
			var_G /= 255;
			var_B /= 255;

			if (var_R > 0.04045)
				var_R = pow(((var_R + 0.055) / 1.055), 2.4);
			else
				var_R = var_R / 12.92;
			if (var_G > 0.04045)
				var_G = pow(((var_G + 0.055) / 1.055), 2.4);
			else
				var_G = var_G / 12.92;
			if (var_B > 0.04045)
				var_B = pow(((var_B + 0.055) / 1.055), 2.4);
			else
				var_B = var_B / 12.92;

			var_R = var_R * 100;
			var_G = var_G * 100;
			var_B = var_B * 100;

			X = var_R * 0.4124 + var_G * 0.3576 + var_B * 0.1805;
			Y = var_R * 0.2126 + var_G * 0.7152 + var_B * 0.0722;
			Z = var_R * 0.0193 + var_G * 0.1192 + var_B * 0.9505;
			
			//X = var_R * 0.4887 + var_G * 0.3107 + var_B * 0.2006;
			//Y = var_R * 0.1762 + var_G * 0.8130 + var_B * 0.0108;
			//Z = var_R * 0 + var_G * 0.102 + var_B * 0.9898;

			//X = var_R * 0.4124 + var_G * 0.3576 + var_B * 0.1805;
			//Y = var_R * 0.2126 + var_G * 0.7152 + var_B * 0.0722;
			//Z = var_R * 0.0193 + var_G * 0.1192 + var_B * 0.9505;
			/*
				Calculate color temperature;
			*/

			if (X < 0) X = 0;
			if (Y < 0) Y = 0;
			if (Z < 0) Z = 0;

			X = RoundValue_toX_SignificantBits(X, 0);
			Y = RoundValue_toX_SignificantBits(Y, 0);
			Z = RoundValue_toX_SignificantBits(Z, 0);
			
			Img_dst->rgbpix[3 * (i * Img_dst->Width + j) + 0] = X;
			Img_dst->rgbpix[3 * (i * Img_dst->Width + j) + 1] = Y;
			Img_dst->rgbpix[3 * (i * Img_dst->Width + j) + 2] = Z;

		}
	}
}

/*
C O N V E R T  - XYZ to RGB
*/
void Convert_XYZ_to_RGB(struct Image *Img_src, struct Image *Img_dst)
{
	int i, j;
	float var_X, var_Y, var_Z;
	float R, B, G;

	for (i = 0; i < Img_dst->Height; i++)
	{
		for (j = 0; j < Img_src->Width; j++)
		{
			var_X = Img_src->rgbpix[3 * (i * Img_dst->Width + j) + 0];// +Img_src->rgbpix[6 * (i * Img_dst->Width + j) + 3] / 100.0;
			var_Y = Img_src->rgbpix[3 * (i * Img_dst->Width + j) + 1];// +Img_src->rgbpix[6 * (i * Img_dst->Width + j) + 4] / 100.0;
			var_Z = Img_src->rgbpix[3 * (i * Img_dst->Width + j) + 2];// +Img_src->rgbpix[6 * (i * Img_dst->Width + j) + 5] / 100.0;
			
			var_X /= 100.0;
			var_Y /= 100.0;
			var_Z /= 100.0;

			R = var_X *  3.2406 + var_Y * -1.5372 + var_Z * -0.4986;
			G = var_X * -0.9689 + var_Y *  1.8758 + var_Z *  0.0415;
			B = var_X *  0.0557 + var_Y * -0.2040 + var_Z *  1.0570;

			//R = var_X *  2.3707 + var_Y * -0.9001 + var_Z * -0.4706;
			//G = var_X * -0.5139 + var_Y *  1.4253 + var_Z *  0.0886;
			//B = var_X *  0.0053 + var_Y * -0.0147 + var_Z *  1.0094;

			//if (i == 638 && j == 2372)
			//	printf("ds");
			if (R < 0) 
				R = 0;
			if (G < 0) 
				G = 0;
			if (B < 0) 
				B = 0;

			if (R > 0.0031308)
				R = 1.055 * pow(R, (1 / 2.4)) - 0.055;
			else
				R = 12.92 * R;
			if (G > 0.0031308)
				G = 1.055 * pow(G, (1 / 2.4)) - 0.055;
			else
				G = 12.92 * G;
			if (B > 0.0031308) 
				B = 1.055 * pow(B , (1 / 2.4)) - 0.055;
			else
				B = 12.92 * B;
			
			R = R * 255;
			if (R > 255) R = 255;
			if (R < 0) R = 0;
			G = G * 255;
			if (G > 255) G = 255;
			if (G < 0) G = 0;
			B = B * 255;
			if (B > 255) B = 255;
			if (B < 0) B = 0;

			R = RoundValue_toX_SignificantBits(R, 0);
			G = RoundValue_toX_SignificantBits(G, 0);
			B = RoundValue_toX_SignificantBits(B, 0);

			Img_dst->rgbpix[3 * (i * Img_dst->Width + j) + 0] = R;
			Img_dst->rgbpix[3 * (i * Img_dst->Width + j) + 1] = G;
			Img_dst->rgbpix[3 * (i * Img_dst->Width + j) + 2] = B;
		}
	}
}

/*
	C O N V E R T - RGB to L*ab
*/
void ConvertImage_RGB_to_LAB(struct Image *Img_src, struct Image *Img_dst, struct WhitePoint WhitePoint_XYZ)
{
	FILE *fdebug = NULL;
	struct Image Img_XYZ = CreateNewImage(&Img_XYZ, Img_src->Width, Img_src->Height, 3, 2, 8);

	int i, j;
	float L, a, b;
	float X, Y, Z;
	float F_x, F_y, F_z;
	float R_t, G_t, B_t;
	float RatioY, RatioX, RatioZ;
	float e = 0.008856;
	float k = 903.3;
	float dult;
	if ((Img_src->Num_channels != 3) || (Img_dst->Num_channels != 3))
	{
		#ifdef DEBUG_FILE
				fdebug = fopen(DEBUG_FILE, "wt");
				fprintf(fdebug, "The input images are not with 3 channels\n");
				fclose(fdebug);
		#endif // DEBUG_FILE
		return;
	}

	if (Img_src->Width * Img_src->Height != Img_dst->Width * Img_dst->Height)
	{
		SetDestination(Img_src, Img_dst);
	}
	Img_dst->rgbpix = (unsigned char *)realloc(Img_dst->rgbpix, Img_dst->Height * Img_dst->Width * Img_dst->Num_channels* sizeof(unsigned char));

	if (Img_src->ColorSpace != 2)
	{
		#ifdef DEBUG_FILE
				fdebug = fopen(DEBUG_FILE, "wt");
				fprintf(fdebug, "The input image is not in RGB format\n");
				fclose(fdebug);
		#endif // DEBUG_FILE
		return;
	}

	Img_dst->ColorSpace = 4;

	// 1st step: Convert to XYZ color space
	Convert_RGB_to_XYZ(Img_src, &Img_XYZ);

	for (i = 0; i < Img_src->Height; i++)
	{
		for (j = 0; j < Img_src->Width; j++)
		{
			X = Img_XYZ.rgbpix[3 * (i * Img_dst->Width + j) + 0];// +Img_XYZ.rgbpix[6 * (i * Img_dst->Width + j) + 3] / 100.0;
			Y = Img_XYZ.rgbpix[3 * (i * Img_dst->Width + j) + 1];// +Img_XYZ.rgbpix[6 * (i * Img_dst->Width + j) + 4] / 100.0;
			Z = Img_XYZ.rgbpix[3 * (i * Img_dst->Width + j) + 2];// +Img_XYZ.rgbpix[6 * (i * Img_dst->Width + j) + 5] / 100.0;

			X /= 100.0;
			Y /= 100.0;
			Z /= 100.0;

			RatioX = X / 1;// (100 * WhitePoint_XYZ.X);
			RatioY = Y / 1;// (100 * WhitePoint_XYZ.Y);
			RatioZ = Z / 1;// (100 * WhitePoint_XYZ.Z);

			RatioX = RoundValue_toX_SignificantBits(RatioX, 4);
			RatioY = RoundValue_toX_SignificantBits(RatioY, 4);
			RatioZ = RoundValue_toX_SignificantBits(RatioZ, 4);

			if (RatioX > e)
			{
				F_x = pow(RatioX, pow(3, -1));
			}
			else
			{
				F_x = (k * RatioX + 16) / 116;
			}

			if (RatioY > e)
			{
				F_y = pow(RatioY, pow(3, -1));
			}
			else
			{
				F_y = (k * RatioY + 16) / 116;

			}

			if (RatioZ > e)
			{
				F_z = pow(RatioZ, pow(3, -1));
			}
			else
			{
				F_z = (k * RatioZ + 16) / 116;
			}

			
			//L = 116 * F_y - 16;
			R_t = Img_src->rgbpix[3 * (i * Img_dst->Width + j) + 0] / 255.0;
			G_t = Img_src->rgbpix[3 * (i * Img_dst->Width + j) + 1] / 255.0;
			B_t = Img_src->rgbpix[3 * (i * Img_dst->Width + j) + 2] / 255.0;
			
			F_x = RoundValue_toX_SignificantBits(F_x, 3);
			F_y = RoundValue_toX_SignificantBits(F_y, 3);
			F_z = RoundValue_toX_SignificantBits(F_z, 3);

			dult = MIN(R_t, MIN(G_t, B_t)) + MAX(R_t, MAX(G_t, B_t));
			L = ((float)dult / 2) * 100;

			a = 500 * (F_x - F_y);
			b = 200 * (F_y - F_z);

			L = RoundValue_toX_SignificantBits(L, 0);
			a = RoundValue_toX_SignificantBits(a, 0);
			b = RoundValue_toX_SignificantBits(b, 0);
			a += 128;
			b += 128;
			if (a > 255) 
				a = 255;
			if (a < 0) 
				a = 0;

			if (b > 255) 
				b = 255;
			if (b < 0) 
				b = 0;

			if (L < 0) L = 0;
			if (L > 255) L = 255;

			Img_dst->rgbpix[3 * (i * Img_dst->Width + j) + 0] = L;
			if (L - Img_dst->rgbpix[3 * (i * Img_dst->Width + j) + 0] > 0.5) Img_dst->rgbpix[3 * (i * Img_dst->Width + j) + 0] += 1;
			Img_dst->rgbpix[3 * (i * Img_dst->Width + j) + 1] = a;
			if (a - Img_dst->rgbpix[3 * (i * Img_dst->Width + j) + 1] > 0.5) Img_dst->rgbpix[3 * (i * Img_dst->Width + j) + 1] += 1;
			Img_dst->rgbpix[3 * (i * Img_dst->Width + j) + 2] = b;
			if (b - Img_dst->rgbpix[3 * (i * Img_dst->Width + j) + 2] > 0.5) Img_dst->rgbpix[3 * (i * Img_dst->Width + j) + 2] += 1;

		}
	}

	DestroyImage(&Img_XYZ);
}

/*
	C O N V E R T - L*ab to RGB
*/
void ConvertImage_LAB_to_RGB(struct Image *Img_src, struct Image *Img_dst, struct WhitePoint WhitePoint_XYZ)
{
	FILE *fdebug = NULL;
	struct Image Img_XYZ = CreateNewImage(&Img_XYZ, Img_src->Width, Img_src->Height, 3, 2, 8);

	int i, j;
	float L, a, b;
	float X, Y, Z;
	float P;
	float Number;
	float RatioY, RatioX, RatioZ;
	if ((Img_src->Num_channels != 3) || (Img_dst->Num_channels != 3))
	{
		#ifdef DEBUG_FILE
				fdebug = fopen(DEBUG_FILE, "wt");
				fprintf(fdebug, "The input images are not with 3 channels\n");
				fclose(fdebug);
		#endif // DEBUG_FILE
		return;
	}

	if (Img_src->Width * Img_src->Height != Img_dst->Width * Img_dst->Height)
	{
		SetDestination(Img_src, Img_dst);
	}

	if (Img_src->ColorSpace != 4)
	{
		#ifdef DEBUG_FILE
				fdebug = fopen(DEBUG_FILE, "wt");
				fprintf(fdebug, "The input image is not in L*ab format\n");
				fclose(fdebug);
		#endif // DEBUG_FILE
		return;
	}

	Img_dst->ColorSpace = 2;

	// 1st step: Convert to XYZ color space

	for (i = 0; i < Img_dst->Height; i++)
	{
		for (j = 0; j < Img_dst->Width; j++)
		{
			L = Img_src->rgbpix[3 * (i * Img_dst->Width + j) + 0];
			a = Img_src->rgbpix[3 * (i * Img_dst->Width + j) + 1];
			b = Img_src->rgbpix[3 * (i * Img_dst->Width + j) + 2];

			a -= 128;
			b -= 128;

			Y = L * (1.0 / 116.0) + 16.0 / 116.0;
			X = a * (1.0 / 500.0) + Y;
			Z = b * (-1.0 / 200.0) + Y;

			X = X > 6.0 / 29.0 ? X * X * X : X * (108.0 / 841.0) - 432.0 / 24389.0;
			Y = L > 8.0 ? Y * Y * Y : L * (27.0 / 24389.0);
			Z = Z > 6.0 / 29.0 ? Z * Z * Z : Z * (108.0 / 841.0) - 432.0 / 24389.0;
			
			X *= 100;
			Y *= 100;
			Z *= 100;

			X = RoundValue_toX_SignificantBits(X, 0);
			Y = RoundValue_toX_SignificantBits(Y, 0);
			Z = RoundValue_toX_SignificantBits(Z, 0);
			Img_XYZ.rgbpix[3 * (i * Img_dst->Width + j) + 0] = X;
			Img_XYZ.rgbpix[3 * (i * Img_dst->Width + j) + 1] = Y;
			Img_XYZ.rgbpix[3 * (i * Img_dst->Width + j) + 2] = Z;
			
		}
	}
	Convert_XYZ_to_RGB(&Img_XYZ, Img_dst);

	DestroyImage(&Img_XYZ);
}



float xyz_to_lab(float c)
{
	return c > 216.0 / 24389.0 ? pow(c, 1.0 / 3.0) : c * (841.0 / 108.0) + (4.0 / 29.0);
}

void WhiteBalanceGREENY(struct Image *src, struct Image *dst, struct WhitePoint WhitePoint_XYZ)
{
	float LuminanceAverage = 0;
	struct ColorPoint_UV UV;
	struct WhitePoint WhitePointXYZ_new;
	struct ColorPoint_RGB RGB;
	struct ColorPoint_XYZ XYZ;
	struct ColorPoint_XYZ XYZ_D;
	float RatioX, RatioY, RatioZ;
	float e = 0.008856;
	float u,v;
	float k = 903.3;
	float F_x, F_z, F_y;
	
	//XYZ
	//float Matrix_M_a[9] = { 1, 0, 0, 0, 1, 0, 0, 0, 1 };
	//float matrix_M_min1[9] = { 1, 0, 0, 0, 1, 0, 0, 0, 1 };
	//BRADFORD
	//float Matrix_M_a[9] = { 0.8951000, 0.2664000, -0.1614000, -0.7502000, 1.7135000, 0.0367000, 0.0389000, -0.0685000, 1.0296000 };
	//float matrix_M_min1[9] = { 0.9869929, -0.1470543, 0.1599627, 0.4323053, 0.5183603, 0.0492912, -0.0085287, 0.0400428, 0.9684867 };
	//CAT97s
	//float Matrix_M_a[9] = { 0.08562, 0.3372, -0.1934, -0.836, 1.8327, 0.0033, 0.0357, -0.0469, 1.0112 };
	//float matrix_M_min1[9] = { 0.9869929, -0.1470543, 0.1599627, 0.4323053, 0.5183603, 0.0492912, -0.0085287, 0.0400428, 0.9684867 };
	//CAT02s
	float Matrix_M_a[9] = { 0.7328, 0.4296, -0.1624, -0.7036, 1.6975, 0.0061, 0.0030, 0.0136, 0.9834 };
	float matrix_M_min1[9] = { 0.9869929, -0.1470543, 0.1599627, 0.4323053, 0.5183603, 0.0492912, -0.0085287, 0.0400428, 0.9684867 };
	//VON Kries
	//float Matrix_M_a[9] = { 0.40024, 0.7076, -0.08081, -0.2263, 1.16532, 0.0457, 0, 0, 0.91822 };
	//float matrix_M_min1[9] = { 1.8599364, -1.1293816, 0.2198974, 0.3611914, 0.6388125, -0.0000064, 0, 0, 1.08906 };
	
	float S_params[3];
	float D_params[3];
	float S_D_ParamsMatrix[9];
	float MatrixMultiplication_1[9];
	float matrix_M_final[9];
	float maxv;
	float P, Number;
	int i, j, z;
	long double R_Global = 0;
	long double G_Global = 0;
	long double B_Global = 0;
	float MAX_VALUE = 0; 
	int Razlika_R, Razlika_G, Razlika_B;
	int tmpKelvin = 0;

	float R, G, B, dult;
	float X, Y, Z, L, a, b, bbb, intensity;



	// FULL

	R_Global = 0;
	G_Global = 0;
	B_Global = 0;

	for (i = 0; i < src->Height; i++)
	{
		for (j = 0; j < src->Width; j++)
		{
			R = (float)src->rgbpix[(i*dst->Width + j) * 3 + 0];
			G = (float)src->rgbpix[(i*dst->Width + j) * 3 + 1];
			B = (float)src->rgbpix[(i*dst->Width + j) * 3 + 2];

			/* if there is a perfect white pixel in the image */
			//if (R > 252 && G > 252 && B > 252)
			//	return;

			R_Global += R;
			G_Global += G;
			B_Global += B;
		}
	}

	R_Global /= (float)(src->Height * src->Width);
	G_Global /= (float)(src->Height * src->Width);
	B_Global /= (float)(src->Height * src->Width);

	LuminanceAverage = R_Global + G_Global + B_Global;
	R = R_Global;
	G = G_Global;
	B = B_Global;
	MAX_VALUE = MAX(R, MAX(G, B));

	if (MAX_VALUE != G)
	{
		//WhitebalanceCorrectionBLUEorRED(src, dst, WhitePoint_XYZ);
		//return;
	}

	Razlika_B = 255 - MAX_VALUE;
//	if (R == MAX_VALUE)
//	{
	//R += Razlika_B;
	//G += Razlika_B;
	//B += Razlika_B;
/*	}
	else if (G == MAX_VALUE)
	{
		R += Razlika;
		B += Razlika;
	}
	else if (B == MAX_VALUE)
	{
		G += Razlika;
		R += Razlika;
	}
*/
	RGB.R = RoundValue_toX_SignificantBits(R, 0);
	RGB.G = RoundValue_toX_SignificantBits(G, 0);
	RGB.B = RoundValue_toX_SignificantBits(B, 0);

	XYZ = POINT_Convert_RGB_to_XYZ(&RGB);
	UV = POINT_Convert_XYZ_to_UV(&XYZ);

	WhitePointXYZ_new.u = UV.u;
	WhitePointXYZ_new.v = UV.v;

	WhitePointXYZ_new.X = XYZ.X;
	WhitePointXYZ_new.Y = XYZ.Y;
	WhitePointXYZ_new.Z = XYZ.Z;

	ColorTemperature(&WhitePointXYZ_new, 0);// EXP_HIGH_T);

	//printf("Zone_ALL\nu: %f v: %f, K:%d\n\n", UV.u, UV.v, WhitePointXYZ_new.Temperature);


	tmpKelvin = WhitePoint_XYZ.Temperature / 100;

	if (tmpKelvin <= 66){

		R = 255;

		G = tmpKelvin;
		G = 99.4708025861 * log(G) - 161.1195681661;


		if (tmpKelvin <= 19)
		{
			B = 0;
		}
		else {

			B = tmpKelvin - 10;
			B = 138.5177312231 * log(B) - 305.0447927307;

		}

	}
	else {

		R = tmpKelvin - 60;
		R = 329.698727446 * pow(R, -0.1332047592);

		G = tmpKelvin - 60;
		G = 288.1221695283 * pow(G, -0.0755148492);

		B = 255;

	}

	tmpKelvin = WhitePointXYZ_new.Temperature / 100;

	if (tmpKelvin <= 66){

		Razlika_R = 255;

		Razlika_G = tmpKelvin;
		Razlika_G = 99.4708025861 * log(Razlika_G) - 161.1195681661;


		if (tmpKelvin <= 19)
		{
			Razlika_B = 0;
		}
		else {

			Razlika_B = tmpKelvin - 10;
			Razlika_B = 138.5177312231 * log(Razlika_B) - 305.0447927307;

		}

	}
	else {

		Razlika_R = tmpKelvin - 60;
		Razlika_R = 329.698727446 * pow(Razlika_R, -0.1332047592);

		Razlika_G = tmpKelvin - 60;
		Razlika_G = 288.1221695283 * pow(Razlika_G, -0.0755148492);

		Razlika_B = 255;
	}

	//Razlika_R = R - Razlika_R;
	//Razlika_G = G - Razlika_G;
	//Razlika_B = B - Razlika_B;
	/*Luminance between 0 and 1*/
 	LuminanceAverage /=(3.0 * 255);
	/*change Matrix_M_a to match the luminance*/
	for(i = 0; i < 9; i++)
	{
		//if(i == 2)Matrix_M_a[i] *= (0 *LuminanceAverage); // cherveno
		//if(i == 8)Matrix_M_a[i] *= (0 *LuminanceAverage); // Zeleno
		//else
			Matrix_M_a[i] *= (1.4 * LuminanceAverage);
			//matrix_M_min1[i] *= (0.4 * LuminanceAverage);
		//Matrix_M_a[i] += LuminanceAverage - 0.8; // Epic fail
		//Matrix_M_a[i] += LuminanceAverage - 0.9;
		//if (i == 1)  Matrix_M_a[i] *= (0.1 * LuminanceAverage);
		//else Matrix_M_a[i] *= (0.7 * LuminanceAverage);
	}

	WhitePointXYZ_new.X /= 100.0;
	WhitePointXYZ_new.Y /= 100.0;
	WhitePointXYZ_new.Z /= 100.0;

	//#pragma omp parallel for private(j, col, R, G, B, intensity,L,a,bbb,X,Y,Z)
	for (i = 0; i<src->Height; i++)
	{
		for (j = 0; j<src->Width; j++)
		{
			maxv = 255;

			R = (float)src->rgbpix[(i*dst->Width + j) * 3 + 0];
			G = (float)src->rgbpix[(i*dst->Width + j) * 3 + 1];
			B = (float)src->rgbpix[(i*dst->Width + j) * 3 + 2];

			RGB.R = R;
			RGB.G = G;
			RGB.B = B;
			// Convert RGB point to XYZ
			XYZ = POINT_Convert_RGB_to_XYZ(&RGB);
			XYZ.X /= 100.0;
			XYZ.Y /= 100.0;
			XYZ.Z /= 100.0;
			
			/*
			| X_d |           | X_s |
			| Y_d |   = |M| * | Y_s |
			| Z_s |           | Z_s |
			http://www.brucelindbloom.com/index.html?Eqn_RGB_XYZ_Matrix.html
			*/
			S_params[0] = WhitePointXYZ_new.X * Matrix_M_a[0] + WhitePointXYZ_new.Y * Matrix_M_a[1] + WhitePointXYZ_new.Z * Matrix_M_a[2];
			S_params[1] = WhitePointXYZ_new.X * Matrix_M_a[3] + WhitePointXYZ_new.Y * Matrix_M_a[4] + WhitePointXYZ_new.Z * Matrix_M_a[5];
			S_params[2] = WhitePointXYZ_new.X * Matrix_M_a[6] + WhitePointXYZ_new.Y * Matrix_M_a[7] + WhitePointXYZ_new.Z * Matrix_M_a[8];

			D_params[0] = WhitePoint_XYZ.X * Matrix_M_a[0] + WhitePoint_XYZ.Y * Matrix_M_a[1] + WhitePoint_XYZ.Z * Matrix_M_a[2];
			D_params[1] = WhitePoint_XYZ.X * Matrix_M_a[3] + WhitePoint_XYZ.Y * Matrix_M_a[4] + WhitePoint_XYZ.Z * Matrix_M_a[5];
			D_params[2] = WhitePoint_XYZ.X * Matrix_M_a[6] + WhitePoint_XYZ.Y * Matrix_M_a[7] + WhitePoint_XYZ.Z * Matrix_M_a[8];

			/* Compute M_min1 matrix * S/D */
			S_D_ParamsMatrix[0] = D_params[0] / S_params[0];
			S_D_ParamsMatrix[1] = 0;
			S_D_ParamsMatrix[2] = 0;
			S_D_ParamsMatrix[3] = 0;
			S_D_ParamsMatrix[4] = D_params[1] / S_params[1];
			S_D_ParamsMatrix[5] = 0;
			S_D_ParamsMatrix[6] = 0;
			S_D_ParamsMatrix[7] = 0;
			S_D_ParamsMatrix[8] = D_params[2] / S_params[2];

			MatrixMultiplication_1[0] = matrix_M_min1[0] * S_D_ParamsMatrix[0];
			MatrixMultiplication_1[1] = matrix_M_min1[1] * S_D_ParamsMatrix[4];
			MatrixMultiplication_1[2] = matrix_M_min1[2] * S_D_ParamsMatrix[8];
			MatrixMultiplication_1[3] = matrix_M_min1[3] * S_D_ParamsMatrix[0];
			MatrixMultiplication_1[4] = matrix_M_min1[4] * S_D_ParamsMatrix[4];
			MatrixMultiplication_1[5] = matrix_M_min1[5] * S_D_ParamsMatrix[8];
			MatrixMultiplication_1[6] = matrix_M_min1[6] * S_D_ParamsMatrix[0];
			MatrixMultiplication_1[7] = matrix_M_min1[7] * S_D_ParamsMatrix[4];
			MatrixMultiplication_1[8] = matrix_M_min1[8] * S_D_ParamsMatrix[8];
			
			/* Compute MatrixMultiplication_1 * matrix_M */

			matrix_M_final[0] = MatrixMultiplication_1[0] * Matrix_M_a[0] + MatrixMultiplication_1[1] * Matrix_M_a[3] + MatrixMultiplication_1[2] * Matrix_M_a[6];
			matrix_M_final[1] = MatrixMultiplication_1[0] * Matrix_M_a[1] + MatrixMultiplication_1[1] * Matrix_M_a[4] + MatrixMultiplication_1[2] * Matrix_M_a[7];
			matrix_M_final[2] = MatrixMultiplication_1[0] * Matrix_M_a[2] + MatrixMultiplication_1[1] * Matrix_M_a[5] + MatrixMultiplication_1[2] * Matrix_M_a[8];
			matrix_M_final[3] = MatrixMultiplication_1[3] * Matrix_M_a[0] + MatrixMultiplication_1[4] * Matrix_M_a[3] + MatrixMultiplication_1[5] * Matrix_M_a[6];
			matrix_M_final[4] = MatrixMultiplication_1[3] * Matrix_M_a[1] + MatrixMultiplication_1[4] * Matrix_M_a[4] + MatrixMultiplication_1[5] * Matrix_M_a[7];
			matrix_M_final[5] = MatrixMultiplication_1[3] * Matrix_M_a[2] + MatrixMultiplication_1[4] * Matrix_M_a[5] + MatrixMultiplication_1[5] * Matrix_M_a[8];
			matrix_M_final[6] = MatrixMultiplication_1[6] * Matrix_M_a[0] + MatrixMultiplication_1[7] * Matrix_M_a[3] + MatrixMultiplication_1[8] * Matrix_M_a[6];
			matrix_M_final[7] = MatrixMultiplication_1[6] * Matrix_M_a[1] + MatrixMultiplication_1[7] * Matrix_M_a[4] + MatrixMultiplication_1[8] * Matrix_M_a[7];
			matrix_M_final[8] = MatrixMultiplication_1[6] * Matrix_M_a[2] + MatrixMultiplication_1[7] * Matrix_M_a[5] + MatrixMultiplication_1[8] * Matrix_M_a[8];

			XYZ_D.X = matrix_M_final[0] * XYZ.X + matrix_M_final[1] * XYZ.Y + matrix_M_final[2] * XYZ.Z;
			XYZ_D.Y = matrix_M_final[3] * XYZ.X + matrix_M_final[4] * XYZ.Y + matrix_M_final[5] * XYZ.Z;
			XYZ_D.Z = matrix_M_final[6] * XYZ.X + matrix_M_final[7] * XYZ.Y + matrix_M_final[8] * XYZ.Z;
			//RGB: 150,55,7
			//XYZ: 0.14, 0.09, 0.01
			RGB = POINT_Convert_XYZ_to_RGB(&XYZ_D);

			dst->rgbpix[(i*dst->Width + j) * 3 + 0] = RGB.R;
			dst->rgbpix[(i*dst->Width + j) * 3 + 1] = RGB.G;
			dst->rgbpix[(i*dst->Width + j) * 3 + 2] = RGB.B;
		}
	}
	//WhitebalanceCorrectionBLUEorRED(src, dst, WhitePoint_XYZ);
}

/*
	P O I N T  - convert RGB to XYZ
*/
struct ColorPoint_XYZ POINT_Convert_RGB_to_XYZ(struct ColorPoint_RGB *RGB_Point)
{
	float R, G, B;
	struct ColorPoint_XYZ XYZ;
	R = RGB_Point->R;
	G = RGB_Point->G;
	B = RGB_Point->B;

	R /= 255;
	G /= 255;
	B /= 255;

	if (R > 0.04045)
		R = pow(((R + 0.055) / 1.055), 2.4);
	else
		R = R / 12.92;
	if (G > 0.04045)
		G = pow(((G + 0.055) / 1.055), 2.4);
	else
		G = G / 12.92;
	if (B > 0.04045)
		B = pow(((B + 0.055) / 1.055), 2.4);
	else
		B = B / 12.92;
	
	R = R * 100;
	G = G * 100;
	B = B * 100;
	
	XYZ.X = R * 0.4124 + G * 0.3576 + B * 0.1805;
	XYZ.Y = R * 0.2126 + G * 0.7152 + B * 0.0722;
	XYZ.Z = R * 0.0193 + G * 0.1192 + B * 0.9505;

	return XYZ;
}

/*
	P O I N T  - convert XYZ to RGB
*/
struct ColorPoint_RGB POINT_Convert_XYZ_to_RGB(struct  ColorPoint_XYZ *XYZ)
{
	float R, G, B;
	struct ColorPoint_RGB RGB;
	float X, Y, Z;

	X = XYZ->X;
	Y = XYZ->Y;
	Z = XYZ->Z;
	
	R = X *  3.2406 + Y * -1.5372 + Z * -0.4986;
	G = X * -0.9689 + Y *  1.8758 + Z *  0.0415;
	B = X *  0.0557 + Y * -0.2040 + Z *  1.0570;
	
	if (R < 0) 
		R = 0;
	if (G < 0) 
		G = 0;
	if (B < 0) 
		B = 0;
	
	if (R > 0.0031308)
		R = 1.055 * pow(R, (1 / 2.4)) - 0.055;
	else
		R = 12.92 * R;
	if (G > 0.0031308)
		G = 1.055 * pow(G, (1 / 2.4)) - 0.055;
	else
		G = 12.92 * G;
	if (B > 0.0031308) 
		B = 1.055 * pow(B , (1 / 2.4)) - 0.055;
	else
		B = 12.92 * B;
	
	R = R * 255;
	if (R > 255) R = 255;
	if (R < 0) R = 0;
	G = G * 255;
	if (G > 255) G = 255;
	if (G < 0) G = 0;
	B = B * 255;
	if (B > 255) B = 255;
	if (B < 0) B = 0;
	
	RGB.R = RoundValue_toX_SignificantBits(R, 0);
	RGB.G = RoundValue_toX_SignificantBits(G, 0);
	RGB.B = RoundValue_toX_SignificantBits(B, 0);

	return RGB;
}

/*
	P O I N T  - convert  XYZ to UV  coordinates
*/
struct ColorPoint_UV POINT_Convert_XYZ_to_UV(struct ColorPoint_XYZ *XYZ)
{
	struct ColorPoint_UV UV;
	UV.u = 4 * XYZ->X / (float)(XYZ->X + 15 * XYZ->Y + 3 * XYZ->Z);
	UV.v = 6 * XYZ->Y / (float)(XYZ->X + 15 * XYZ->Y + 3 * XYZ->Z);

	return UV;
}

/*
	P O I N T  - convert  UV to XYZ  coordinates
*/
struct point_xy POINT_Convert_UV_to_XY(struct ColorPoint_UV *UV)
{
	struct point_xy XY;
	
	XY.X = 3 * UV->u / (2 * UV->u - 8 * UV->v + 4);
	XY.Y = 2 * UV->v / (2 * UV->u - 8 * UV->v + 4);

	return XY;
}

/*
	C O L O R - Temperature
*/
void ColorTemperature(struct WhitePoint *WhitePoint_lab, int AlgoType )
{
	float X_e = 0.3366;
	float Y_e = 0.1735;
	float A_0 = -949.86315;
	float A_1 = 6253.803;
	float t_1 = 0.92159;
	float A_2 = 28.706;
	float t_2 = 0.20039;
	float A_3 = 0.00004;
	float t_3 = 0.07125;
	//
	float n;
	float CCT;
	struct point_xy XY;
	struct ColorPoint_UV UV;
	if (AlgoType == 0)
	{
		// Calculate color temperature and XYZ coordinates from UV coordinates
		if (WhitePoint_lab->u != 0)
		{
			UV.u = WhitePoint_lab->u;
			UV.v = WhitePoint_lab->v;

			//Caclulate XY from UV
			XY = POINT_Convert_UV_to_XY(&UV);
		}
		else
		{
			//Calculate XY from XYZ;
			XY.X = WhitePoint_lab->X / (WhitePoint_lab->X + WhitePoint_lab->Y + WhitePoint_lab->Z);
			XY.Y = WhitePoint_lab->Y / (WhitePoint_lab->X + WhitePoint_lab->Y + WhitePoint_lab->Z);
		}

		n = (XY.X - 0.332) / (0.1858 - XY.Y);
		CCT = 449 * pow(n, 3) + 3525 * pow(n, 2) + 6823.3 * n + 5520.33;
	}
	else
	{
		//EXP_HIGH_T
		XY.X = WhitePoint_lab->X / (WhitePoint_lab->X + WhitePoint_lab->Y + WhitePoint_lab->Z);
		XY.Y = WhitePoint_lab->Y / (WhitePoint_lab->X + WhitePoint_lab->Y + WhitePoint_lab->Z);
		n = (XY.X - X_e) / (XY.Y - Y_e);
		CCT = A_0 + A_1 * exp(-n / t_1) + A_2 * exp(-n / t_2) + A_3*exp(-n/t_3);
	}

	//Differences in values for both algorithms
	// 6347 - pic1     4287 - pic 2; //Algo_type = 0
	// 6344 - pic1     4293 - pic 2; // Algo_type = 1;
	WhitePoint_lab->Temperature = CCT;
}

//
/*

//R_Global += R;
//G_Global += G;
//B_Global += B;

//dult = MIN(R, MIN(G, B)) + MAX(R, MAX(G, B));
//intensity = (float)dult / 2.0;

R /= (float)maxv;
G /= (float)maxv;
B /= (float)maxv;

//CONVERT RGB - XYZ
if (R > 0.04045)
R = pow(((R + 0.055) / 1.055), 2.4);
else
R = R / 12.92;
if (G > 0.04045)
G = pow(((G + 0.055) / 1.055), 2.4);
else
G = G / 12.92;
if (B > 0.04045)
B = pow(((B + 0.055) / 1.055), 2.4);
else
B = B / 12.92;


X = R * 0.4124 + G * 0.3576 + B * 0.1805;
Y = R * 0.2126 + G * 0.7152 + B * 0.0722;
Z = R * 0.0193 + G * 0.1192 + B * 0.9505;


if (X < 0) X = 0;
if (Y < 0) Y = 0;
if (Z < 0) Z = 0;

// XYZ - Lab
RatioX = X / (WhitePointXYZ_new.X / 100.0);
RatioY = Y / (WhitePointXYZ_new.Y / 100.0);
RatioZ = Z / (WhitePointXYZ_new.Z / 100.0);

//RatioX /= 100;
//RatioY /= 100;
//RatioZ /= 100;
if (RatioX > e)
{
	F_x = pow(RatioX, pow(3, -1));
}
else
{
	F_x = (k * RatioX + 16) / 116;
}

if (RatioY > e)
{
	F_y = pow(RatioY, pow(3, -1));
}
else
{
	F_y = (k * RatioY + 16) / 116;
}

if (RatioZ > e)
{
	F_z = pow(RatioZ, pow(3, -1));
}
else
{
	F_z = (k * RatioZ + 16) / 116;
}

if (RatioY > 0.008856)
{
	L = 116 * pow(RatioY, (0.333)) - 16;
}
else
L = 903.3 *  RatioY;

a = 500 * (F_x - F_y);

b = 200 * (F_y - F_z);

// Lab to XYZ 
P = (L + 16) / 116.0;

Number = P;
if (Number > 6 / 29.0)
{
	Number = pow(Number, 3);
}
else
{
	Number = 3 * pow(6 / 29.0, 2) * (Number - 4 / 29.0);
}

Y = WhitePoint_XYZ.Y *
Number;

Number = P + a / 500.0;
if (Number > 6 / 29.0)
{
	Number = pow(Number, 3);
}
else
{
	Number = 3 * pow(6 / 29.0, 2) * (Number - 4 / 29.0);
}
X = WhitePoint_XYZ.X *
Number;

Number = P - b / 200;
if (Number > 6 / 29.0)
{
	Number = pow(Number, 3);
}
else
{
	Number = 3 * pow(6 / 29.0, 2) * (Number - 4 / 29.0);
}
Z = WhitePoint_XYZ.Z *
Number;

if (Z > 0.35585 && WhitePoint_XYZ.Temperature < 3700)
	Z = 0.3585;

if (Z > 0.82521 && WhitePoint_XYZ.Temperature < 5400)
	Z = 0.82521;
// XYZ to RGB 

R = X *  3.2406 + Y * -1.5372 + Z * -0.4986;
G = X * -0.9689 + Y *  1.8758 + Z *  0.0415;
B = X *  0.0557 + Y * -0.2040 + Z *  1.0570;

if (R < 0)
	R = 0;
if (G < 0)
	G = 0;
if (B < 0)
	B = 0;

if (R > 0.0031308)
R = 1.055 * pow(R, (1 / 2.4)) - 0.055;
else
R = 12.92 * R;
if (G > 0.0031308)
G = 1.055 * pow(G, (1 / 2.4)) - 0.055;
else
G = 12.92 * G;
if (B > 0.0031308)
B = 1.055 * pow(B, (1 / 2.4)) - 0.055;
else
B = 12.92 * B;

R = R * 255;
if (R > 255) R = 255;
if (R < 0) R = 0;
G = G * 255;
if (G > 255) G = 255;
if (G < 0) G = 0;
B = B * 255;
if (B > 255) B = 255;
if (B < 0) B = 0;

*/

float pow_func(float Number, float Stepen, int precision)
{
	int i = 0;
	int StepenInteger = 0;
	float NewNumber = 1;
	int flag_Reverse = 0;
	float Chislitel = 1, Dopylnenie = 0;

	if (Stepen > 0 && Stepen < 1)
	{
		Stepen = 1 / Stepen;
		flag_Reverse = 1;
	}

	StepenInteger = Stepen; // ako sme imali za stepen 2.3 - zapisvame samo 2

	for (i = 0; i < StepenInteger; i++)
	{
		NewNumber *= Number;
	}

	for (i = 0; i < (Stepen - StepenInteger) * precision; i++)
	{
		Chislitel *= Number;
	}
	Dopylnenie = Chislitel / (precision * precision);

	NewNumber += Dopylnenie;

	if (flag_Reverse == 1) NewNumber = 1 / NewNumber;

	return NewNumber;
}

/*
	W H I T E   B A L A N C E  algorithm using RGB and Temperature of src and dest
*/
void WhitebalanceCorrectionBLUEorRED(struct Image *src, struct Image *dst, struct WhitePoint WhitePoint_lab)
{
	float LuminanceAverage = 0;
	struct ColorPoint_UV UV;
	struct WhitePoint WhitePointXYZ_new;
	struct ColorPoint_RGB RGB;
	struct ColorPoint_XYZ XYZ;
	struct ColorPoint_XYZ XYZ_D;

	float maxv;
	float P, Number;
	int i, j, z;
	long double R_Global = 0;
	long double G_Global = 0;
	long double B_Global = 0;
	float MAX_VALUE = 0;
	int Razlika_R, Razlika_G, Razlika_B;
	int tmpKelvin = 0;

	float R, G, B, dult;
	float X, Y, Z, L, a, b, bbb, intensity;

	R_Global = 0;
	G_Global = 0;
	B_Global = 0;

	for (i = 0; i < src->Height; i++)
	{
		for (j = 0; j < src->Width; j++)
		{
			R = (float)src->rgbpix[(i*dst->Width + j) * 3 + 0];
			G = (float)src->rgbpix[(i*dst->Width + j) * 3 + 1];
			B = (float)src->rgbpix[(i*dst->Width + j) * 3 + 2];

			R_Global += R;
			G_Global += G;
			B_Global += B;
		}
	}

	R_Global /= (float)(src->Height * src->Width);
	G_Global /= (float)(src->Height * src->Width);
	B_Global /= (float)(src->Height * src->Width);

	if (MAX(R_Global, MAX(G_Global, B_Global)) == G_Global)
	{
	//	memcpy(src->rgbpix, dst->rgbpix, 3 * src->Height * src->Width * sizeof(unsigned char));
	//	return;
	}

	RGB.R = RoundValue_toX_SignificantBits(R_Global, 0);
	RGB.G = RoundValue_toX_SignificantBits(G_Global, 0);
	RGB.B = RoundValue_toX_SignificantBits(B_Global, 0);

	XYZ = POINT_Convert_RGB_to_XYZ(&RGB);
	UV = POINT_Convert_XYZ_to_UV(&XYZ);

	WhitePointXYZ_new.u = UV.u;
	WhitePointXYZ_new.v = UV.v;

	WhitePointXYZ_new.X = XYZ.X;
	WhitePointXYZ_new.Y = XYZ.Y;
	WhitePointXYZ_new.Z = XYZ.Z;

	ColorTemperature(&WhitePointXYZ_new, 0);// EXP_HIGH_T);

	tmpKelvin = WhitePointXYZ_new.Temperature / 100;

	if (tmpKelvin <= 66){

		Razlika_R = 255;

		Razlika_G = tmpKelvin;
		Razlika_G = 99.4708025861 * log(Razlika_G) - 161.1195681661;


		if (tmpKelvin <= 19)
		{
			Razlika_B = 0;
		}
		else {

			Razlika_B = tmpKelvin - 10;
			Razlika_B = 138.5177312231 * log(Razlika_B) - 305.0447927307;

		}

	}
	else 
	{

		Razlika_R = tmpKelvin - 60;
		Razlika_R = 329.698727446 * pow(Razlika_R, -0.1332047592);

		Razlika_G = tmpKelvin - 60;
		Razlika_G = 288.1221695283 * pow(Razlika_G, -0.0755148492);

		Razlika_B = 255;

	}
	//printf("Zone_ALL\nu: %f v: %f, K:%d\n\n", UV.u, UV.v, WhitePointXYZ_new.Temperature);


	for (i = 0; i < src->Height; i++)
	{
		for (j = 0; j < src->Width; j++)
		{
			maxv = 255;

			R = (float)src->rgbpix[(i*dst->Width + j) * 3 + 0];
			G = (float)src->rgbpix[(i*dst->Width + j) * 3 + 1];
			B = (float)src->rgbpix[(i*dst->Width + j) * 3 + 2];

			RGB.R = RoundValue_toX_SignificantBits(R * 255 / (float)Razlika_R, 0);
			RGB.G = RoundValue_toX_SignificantBits(G * 255 / (float)Razlika_G, 0);
			RGB.B = RoundValue_toX_SignificantBits(B * 255 / (float)Razlika_B, 0);
			
			if (RGB.R > 255) RGB.R = 255;
			if (RGB.G > 255) RGB.G = 255;
			if (RGB.B > 255) RGB.B = 255;
			if (RGB.R < 0) RGB.R = 0;
			if (RGB.G < 0) RGB.G = 0;
			if (RGB.B < 0) RGB.B = 0;

			dst->rgbpix[(i*dst->Width + j) * 3 + 0] = RGB.R;
			dst->rgbpix[(i*dst->Width + j) * 3 + 1] = RGB.G;
			dst->rgbpix[(i*dst->Width + j) * 3 + 2] = RGB.B;
		}
	}
}


int dft(long int length, int length2, long double real_sample[], long double imag_sample[], long double temp_real[], long double temp_imag[])
{
	long int i, j, k, l;
	long double arg;
	long double CurrentValCalcs;
	long double cosarg, sinarg;
	double radius;
	double Pitagor;
	//long double *temp_real = NULL, *temp_imag = NULL;

	//temp_real = calloc(length * length2, sizeof(long double));
	//temp_imag = calloc(length * length2, sizeof(long double));
	//if (temp_real == NULL || temp_imag == NULL)
	//{
	//	return(FALSE);
	//}

	radius = length2 / 2 - (0.35 * length2 / 2);

	for (k = 0; k < length; k += 1)
	{
		for (l = 0; l < length2; l += 1)
		{

			Pitagor = sqrt(pow((length2 / 2 - l), 2) + pow((length / 2 - k), 2));
			if (Pitagor > (MIN(length, length2) - 3) )
			{
				temp_real[k * length2 + l] = 0;
				temp_imag[k * length2 + l] = 0;
				continue;
			}
		
			temp_real[k * length2 + l] = 0;
			temp_imag[k * length2 + l] = 0;
			for (i = 0; i < length; i += 1)
			{
				for (j = 0; j < length2; j += 1)
				{
					arg = -1.0 * 2.0 * 3.141592654 * ((long double)(i * k) / (long double)length + (long double)(j * l) / (long double)length2);

					cosarg = cos(arg);
					sinarg = sin(arg);
					temp_real[k * length2 + l] += (real_sample[i * length2 + j] * cosarg - imag_sample[i * length2 + j] * sinarg);
					temp_imag[k * length2 + l] += (real_sample[i * length2 + j] * sinarg + imag_sample[i * length2 + j] * cosarg);
				}
			}
		}
	}
	

	for (i = 0; i<length * length2; i += 1)
	{
		real_sample[i] = temp_real[i];
		imag_sample[i] = temp_imag[i];
	}

	//free(temp_real);
	//free(temp_imag);
	return(TRUE);
}



//Inverse Discrete Fourier Transform


int inverse_dft(long int length, int length2, long double real_sample[], long double imag_sample[], long double temp_real[], long double temp_imag[])
{
	long int i, j, k, l;
	long double arg;
	long double cosarg, sinarg;
	//long double *temp_real = NULL, *temp_imag = NULL;

	//temp_real = calloc(length * length2, sizeof(long double));
	//temp_imag = calloc(length * length2, sizeof(long double));
	//if (temp_real == NULL || temp_imag == NULL)
	//{
	//	return(FALSE);
	//}

	for (k = 0; k < length; k += 1)
	{
		for (l = 0; l < length2; l += 1)
		{
			temp_real[k * length2 + l] = 0;
			temp_imag[k * length2 + l] = 0;
			for (i = 0; i < length; i += 1)
			{
				//arg = 2.0 * 3.141592654 * (long double)i / (long double)length;
				for (j = 0; j < length2; j += 1)
				{
					arg = 2.0 * 3.141592654 * ((long double)(i * k) / (long double)length + (long double)(j * l) / (long double)length2);

					cosarg = cos(arg);
					sinarg = sin(arg);
					temp_real[k * length2 + l] += (real_sample[i * length2 + j] * cosarg - imag_sample[i * length2 + j] * sinarg);
					temp_imag[k * length2 + l] += (real_sample[i * length2 + j] * sinarg + imag_sample[i * length2 + j] * cosarg);
				}
			}
		}
	}


	for (i = 0; i<length * length2; i += 1)
	{
		real_sample[i] = temp_real[i] / (long double)(length * length2);
		imag_sample[i] = temp_imag[i] / (long double)(length * length2);
	}

	//free(temp_real);
	//free(temp_imag);
	return(TRUE);
}


void SpatialToFrequencyDomain(struct Image *img_src, struct Image *img_dst)
{
	// perform DFT
	// The number of frequencies corresponds to the number of pixels in the spatial domain image, i.e. the image in the spatial and Fourier domain are of the same size.
	// Formula is published here: http://homepages.inf.ed.ac.uk/rbf/HIPR2/fourier.htm

	long double* dr = NULL;
	long double* di = NULL;
	long double* fr = NULL;
	long double* fi = NULL;

	int i, j, k, l, z;
	float MATH_PI = 3.14;
	long double CurrentValCalcs = 0;
	long double CurrentValCalcs2 = 0;
	//_Complex double ui = 0;
	int Min = 655035, Max = 0;
	int Width = img_src->Width;
	int Height = img_src->Height;
	long double *Arr_calc = NULL;
	long double *Arr_calc2 = NULL;
	long double *Arr_calc3 = NULL;
	long double *Arr_calc4 = NULL;
	int broi = 0;
	//struct Image Mapping = CreateNewImage(&Mapping, img_src->Width, img_src->Height, 1, 1);
	struct Image grayscaledImage = CreateNewImage(&grayscaledImage, img_src->Width, img_src->Height, 1, 1, 8);

	Arr_calc = (long double *)malloc(img_src->Width * img_src->Height * sizeof(long double));
	Arr_calc2 = (long double *)malloc(img_src->Width * img_src->Height * sizeof(long double));
	Arr_calc3 = (long double *)malloc(img_src->Width * img_src->Height * sizeof(long double));
	Arr_calc4 = (long double *)malloc(img_src->Width * img_src->Height * sizeof(long double));

	if (img_src->Num_channels != 1) ConvertToGrayscale_1Channel(img_src, &grayscaledImage);
	else memcpy(grayscaledImage.rgbpix, img_src->rgbpix, img_src->Width * img_src->Height * sizeof(unsigned char));

	if (grayscaledImage.Width * grayscaledImage.Height != img_dst->Width * img_dst->Height)
	{
		SetDestination(&grayscaledImage, img_dst);
	}

	dr = (long double *)malloc(grayscaledImage.Width * grayscaledImage.Height * sizeof(long double));
	di = (long double *)malloc(grayscaledImage.Width * grayscaledImage.Height * sizeof(long double));
	fr = (long double *)malloc(grayscaledImage.Width * grayscaledImage.Height * sizeof(long double));
	fi = (long double *)malloc(grayscaledImage.Width * grayscaledImage.Height * sizeof(long double));

	for (i = 0; i < grayscaledImage.Height * grayscaledImage.Width; i++)
	{
		di[i] = 0;
		dr[i] = grayscaledImage.rgbpix[i];
	}

	dft(grayscaledImage.Height , grayscaledImage.Width, dr, di, fr, fi);
	inverse_dft(grayscaledImage.Height , grayscaledImage.Width, dr, di, fr, fi);

	for (i = 0; i < grayscaledImage.Height * grayscaledImage.Width; i++)
	{
		if (dr[i] < 0) dr[i] = 0;
		if (dr[i] > 255) dr[i] = 255;
		img_dst->rgbpix[i] = RoundValue_toX_SignificantBits(dr[i], 0);
	}

	free(fr);
	free(fi);
	free(dr);
	free(di);
}

