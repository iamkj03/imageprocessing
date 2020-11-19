#include<stdio.h>
#include<windows.h>
#include<math.h>
#include<stdlib.h>

#define N 512
#define height 512
#define width 512
#define BYTE unsigned char

int SNR = 0;

float get_image_power(unsigned char InImg[][N]);
float Gaussian(float sd);
void AddGaussianNoise(BYTE InImg[N][N], BYTE noise_img[N][N], double sigma);

void Roberts(BYTE InImg[N][N], BYTE noise_img[N][N], BITMAPFILEHEADER hf, BITMAPINFOHEADER hInfo, RGBQUAD *hRGB);
void Prewitt(BYTE InImg[N][N], BYTE noise_img[N][N], BITMAPFILEHEADER hf, BITMAPINFOHEADER hInfo, RGBQUAD *hRGB);
void Sobel(BYTE InImg[N][N], BYTE noise_img[N][N], BITMAPFILEHEADER hf, BITMAPINFOHEADER hInfo, RGBQUAD *hRGB);
void Stochastic(BYTE InImg[N][N], BYTE noise_img[N][N], BITMAPFILEHEADER hf, BITMAPINFOHEADER hInfo, RGBQUAD* hRGB);
void Lowpass(BYTE boat_img[N][N], BYTE noise_img2[N][N], BITMAPFILEHEADER hf, BITMAPINFOHEADER hInfo, RGBQUAD* hRGB);
void Median(BYTE boat_img[N][N], BYTE noise_img2[N][N], BITMAPFILEHEADER hf, BITMAPINFOHEADER hInfo, RGBQUAD* hRGB);
void sort(BYTE temp[3][3], BYTE temp_n[3][3]);

unsigned char m_OutImg[N][N];
unsigned char m_OutImg_n[N][N];
BITMAPFILEHEADER hf;
BITMAPINFOHEADER hInfo;
RGBQUAD hRGB[256];
BYTE InImg[N][N];
BYTE boat_img[N][N];
BYTE noise_img[N][N];
BYTE noise_img2[N][N];
BYTE temp[3][3];
BYTE temp_n[3][3];
unsigned char buffer;

void main(){

	FILE *infile;
	fopen_s(&infile, "lena_bmp_512x512_new.bmp", "rb");
	if (infile == NULL)
	{
		printf("파일이 없습니다");
		exit(1);
	}

	fread(&hf, sizeof(BITMAPFILEHEADER), 1, infile);
	fread(&hInfo, sizeof(BITMAPINFOHEADER), 1, infile);
	fread(hRGB, sizeof(RGBQUAD), 256, infile);
	fread(InImg, sizeof(BYTE), N*N, infile);

	FILE *boatfile; 
	fopen_s(&boatfile, "BOAT512.raw", "rb");
	if (boatfile == NULL) { printf("File open error!"); return; }
	fread(boat_img, sizeof(BYTE), N*N, boatfile);

	double variance;
	double stddev_noise;
	SNR = 8;
	variance = get_image_power(InImg);
	stddev_noise = sqrt(variance / pow(10.0, ((double)SNR / 10)));
	AddGaussianNoise(InImg, noise_img, stddev_noise);

	FILE *noise_file;
	fopen_s(&noise_file, "Noise_img_8dB.bmp", "wb");
	fwrite(&hf, sizeof(BITMAPFILEHEADER), 1, noise_file);
	fwrite(&hInfo, sizeof(BITMAPINFOHEADER), 1, noise_file);
	fwrite(&hRGB, sizeof(RGBQUAD), 256, noise_file);
	fwrite(noise_img, sizeof(char), hInfo.biSizeImage, noise_file);

	Roberts(InImg, noise_img, hf, hInfo, hRGB);
	Prewitt(InImg, noise_img, hf, hInfo, hRGB);
	Sobel(InImg, noise_img, hf, hInfo, hRGB);
	Stochastic(InImg, noise_img, hf, hInfo, hRGB);	

	SNR = 8;
	variance = get_image_power(boat_img);
	stddev_noise = sqrt(variance / pow(10.0, ((double)SNR / 10)));
	AddGaussianNoise(boat_img, noise_img2, stddev_noise);

	FILE *noise_file2;
	fopen_s(&noise_file2, "Boat_img_8dB.raw", "wb");
	fwrite(noise_img2, sizeof(char), 512 * 512, noise_file2);

	Lowpass(boat_img, noise_img2, hf, hInfo, hRGB);
	Median(boat_img, noise_img2, hf, hInfo, hRGB);

	fclose(infile);
	fclose(boatfile);

}

void Roberts(BYTE InImg[N][N], BYTE noise_img[N][N], BITMAPFILEHEADER hf, BITMAPINFOHEADER hInfo, RGBQUAD* hRGB) {
	int i, j, r, c;

	int MaskRobertsX[3][3] = { { 0, 0, -1 },
	{ 0, 1, 0 },
	{ 0, 0, 0 } };
	int MaskRobertsY[3][3] = { { -1, 0, 0 },
	{ 0, 1, 0 },
	{ 0, 0, 0 } };
	int heightm1 = height - 1;
	int widthm1 = width - 1;
	int mr, mc;
	int newValue;
	int newValue_n;
	int *pImgRobertsX, *pImgRobertsY;
	int *pImgRobertsX_n, *pImgRobertsY_n;
	int min, max, where;
	float constVal1, constVal2;

	//정수값을 갖는 이미지 동적 메모리 할당
	pImgRobertsX = new int[height*width];
	pImgRobertsY = new int[height*width];

	pImgRobertsX_n = new int[height*width];
	pImgRobertsY_n = new int[height*width];

	//결과 이미지 0으로 초기화
	for (i = 0; i<height; i++)
		for (j = 0; j<width; j++)
		{
			m_OutImg[i][j] = 0;
			m_OutImg_n[i][j] = 0;
			where = i*width + j;
			pImgRobertsX[where] = 0;
			pImgRobertsY[where] = 0;
			pImgRobertsX_n[where] = 0;
			pImgRobertsY_n[where] = 0;
		}

	//X 방향 에지 강도 계산 
	for (i = 1; i<heightm1; i++)
	{
		for (j = 1; j<widthm1; j++)
		{
			newValue = 0; //0으로 초기화
			newValue_n = 0;
			for (mr = 0; mr<3; mr++)
			{
				for (mc = 0; mc<3; mc++)
				{
					newValue += (MaskRobertsX[mr][mc] * InImg[i + mr - 1][j + mc - 1]);
					newValue_n += (MaskRobertsX[mr][mc] * noise_img[i + mr - 1][j + mc - 1]);
				}
				pImgRobertsX[i*width + j] = newValue;
				pImgRobertsX_n[i*width + j] = newValue_n;
			}
		}
	}
	//Y 방향 에지 강도 계산 

	for (i = 1; i<heightm1; i++)
	{
		for (j = 1; j<widthm1; j++)
		{
			newValue = 0; //0으로 초기화
			newValue_n = 0;
			for (mr = 0; mr<3; mr++)
			{
				for (mc = 0; mc<3; mc++)
				{
					newValue += (MaskRobertsY[mr][mc] * InImg[i + mr - 1][j + mc - 1]);
					newValue_n += (MaskRobertsY[mr][mc] * noise_img[i + mr - 1][j + mc - 1]);
				}
				pImgRobertsY[i*width + j] = newValue;
				pImgRobertsY_n[i*width + j] = newValue_n;
			}
		}
	}

	float value;
	float value_n;
	for (i = 0; i<height; i++)
	{
		for (j = 0; j<width; j++)
		{
			value = sqrt(pow(pImgRobertsX[i*width + j], 2.0) + pow(pImgRobertsY[i*width + j], 2.0));
			if (value>150){
				pImgRobertsX[i*width + j] = 255;
			}
			else
				pImgRobertsX[i*width + j] = value;
		}
	}

	for (i = 0; i<height; i++)
	{
		for (j = 0; j<width; j++)
		{
			newValue = pImgRobertsX[i*width + j];
			m_OutImg[i][j] = (BYTE)newValue;
		}
	}

	for (i = 0; i<height; i++)
	{
		for (j = 0; j<width; j++)
		{
			value_n = sqrt(pow(pImgRobertsX_n[i*width + j], 2.0) + pow(pImgRobertsY_n[i*width + j], 2.0));
			if (value_n>150){
				pImgRobertsX_n[i*width + j] = 255;
			}
			else
				pImgRobertsX_n[i*width + j] = value_n;
		}
	}

	for (i = 0; i<height; i++)
	{
		for (j = 0; j<width; j++)
		{
			newValue_n = pImgRobertsX_n[i*width + j];
			m_OutImg_n[i][j] = (BYTE)newValue_n;
		}
	}

	FILE *roberts_file;
	fopen_s(&roberts_file, "Roberts.bmp", "wb");
	fwrite(&hf, sizeof(char), sizeof(BITMAPFILEHEADER), roberts_file);
	fwrite(&hInfo, sizeof(char), sizeof(BITMAPINFOHEADER), roberts_file);
	fwrite(hRGB, sizeof(RGBQUAD), 256, roberts_file);
	fwrite(m_OutImg, sizeof(unsigned char), 512 * 512, roberts_file);

	FILE *roberts_file_n;
	fopen_s(&roberts_file_n, "Roberts_noise.bmp", "wb");
	fwrite(&hf, sizeof(char), sizeof(BITMAPFILEHEADER), roberts_file_n);
	fwrite(&hInfo, sizeof(char), sizeof(BITMAPINFOHEADER), roberts_file_n);
	fwrite(hRGB, sizeof(RGBQUAD), 256, roberts_file_n);
	fwrite(m_OutImg_n, sizeof(unsigned char), 512 * 512, roberts_file_n);

	//동적 할당 메모리 해제
	delete[] pImgRobertsX;
	delete[] pImgRobertsY;
	delete[] pImgRobertsX_n;
	delete[] pImgRobertsY_n;

	int cnt_1 = 0, cnt_0 = 0;
	float P_e;
	printf("\n\n=================================================================\n");
	printf("                          [Roberts P_e]               \n\n");
	for (i = 0; i<N; i++){
		for (j = 0; j<N; j++){
			if (m_OutImg[i][j] == 255 && m_OutImg_n[i][j] != 255) { cnt_1++; }
		}
	}
	for (i = 0; i<N; i++){
		for (j = 0; j<N; j++){
			if (m_OutImg[i][j] != 255 && m_OutImg_n[i][j] == 255) { cnt_1++; }
		}
	}
	for (i = 0; i<N; i++){
		for (j = 0; j<N; j++){
			if (m_OutImg[i][j] == 255){ cnt_0++; }
		}
	}
	printf("                     [N_1] = %d, [N_0] = %d          \n\n", cnt_1, cnt_0);
	P_e = (float)cnt_1 / cnt_0;
	printf("                       [P_e] = %f                    \n", P_e);
	printf("=================================================================\n");

}
void Prewitt(BYTE InImg[N][N], BYTE noise_img[N][N], BITMAPFILEHEADER hf, BITMAPINFOHEADER hInfo, RGBQUAD* hRGB) {
	int i, j, r, c;

	int MaskPrewittX[3][3] = { { -1, 0, 1 },
	{ -1, 0, 1 },
	{ -1, 0, 1 } };
	int MaskPrewittY[3][3] = { { 1, 1, 1 },
	{ 0, 0, 0 },
	{ -1, -1, -1 } };
	int heightm1 = height - 1;
	int widthm1 = width - 1;
	int mr, mc;
	int newValue;
	int newValue_n;
	int *pImgPrewittX, *pImgPrewittY;
	int *pImgPrewittX_n, *pImgPrewittY_n;
	int min, max, where;
	float constVal1, constVal2;

	//정수값을 갖는 이미지 동적 메모리 할당
	pImgPrewittX = new int[height*width];
	pImgPrewittY = new int[height*width];

	pImgPrewittX_n = new int[height*width];
	pImgPrewittY_n = new int[height*width];

	//결과 이미지 0으로 초기화
	for (i = 0; i<height; i++)
		for (j = 0; j<width; j++)
		{
			m_OutImg[i][j] = 0;
			m_OutImg_n[i][j] = 0;
			where = i*width + j;
			pImgPrewittX[where] = 0;
			pImgPrewittY[where] = 0;
			pImgPrewittX_n[where] = 0;
			pImgPrewittY_n[where] = 0;
		}

	//X 방향 에지 강도 계산 
	for (i = 1; i<heightm1; i++)
	{
		for (j = 1; j<widthm1; j++)
		{
			newValue = 0; //0으로 초기화
			newValue_n = 0;
			for (mr = 0; mr<3; mr++)
			{
				for (mc = 0; mc<3; mc++)
				{
					newValue += (MaskPrewittX[mr][mc] * InImg[i + mr - 1][j + mc - 1]);
					newValue_n += (MaskPrewittX[mr][mc] * noise_img[i + mr - 1][j + mc - 1]);
				}
				pImgPrewittX[i*width + j] = newValue;
				pImgPrewittX_n[i*width + j] = newValue_n;
			}
		}
	}
	//Y 방향 에지 강도 계산 

	for (i = 1; i<heightm1; i++)
	{
		for (j = 1; j<widthm1; j++)
		{
			newValue = 0; //0으로 초기화
			newValue_n = 0;
			for (mr = 0; mr<3; mr++)
			{
				for (mc = 0; mc<3; mc++)
				{
					newValue += (MaskPrewittY[mr][mc] * InImg[i + mr - 1][j + mc - 1]);
					newValue_n += (MaskPrewittY[mr][mc] * noise_img[i + mr - 1][j + mc - 1]);
				}
				pImgPrewittY[i*width + j] = newValue;
				pImgPrewittY_n[i*width + j] = newValue_n;
			}
		}
	}

	float value;
	float value_n;
	for (i = 0; i<height; i++)
	{
		for (j = 0; j<width; j++)
		{
			value = sqrt(pow(pImgPrewittX[i*width + j], 2.0) + pow(pImgPrewittY[i*width + j], 2.0));
			if (value>150){
				pImgPrewittX[i*width + j] = 255;
			}
			else
				pImgPrewittX[i*width + j] = value;
		}
	}

	for (i = 0; i<height; i++)
	{
		for (j = 0; j<width; j++)
		{
			newValue = pImgPrewittX[i*width + j];
			m_OutImg[i][j] = (BYTE)newValue;
		}
	}

	for (i = 0; i<height; i++)
	{
		for (j = 0; j<width; j++)
		{
			value_n = sqrt(pow(pImgPrewittX_n[i*width + j], 2.0) + pow(pImgPrewittY_n[i*width + j], 2.0));
			if (value_n>150){
				pImgPrewittX_n[i*width + j] = 255;
			}
			else
				pImgPrewittX_n[i*width + j] = value_n;
		}
	}

	for (i = 0; i<height; i++)
	{
		for (j = 0; j<width; j++)
		{
			newValue_n = pImgPrewittX_n[i*width + j];
			m_OutImg_n[i][j] = (BYTE)newValue_n;
		}
	}

	FILE *Prewitt_file;
	fopen_s(&Prewitt_file, "Prewitt.bmp", "wb");
	fwrite(&hf, sizeof(char), sizeof(BITMAPFILEHEADER), Prewitt_file);
	fwrite(&hInfo, sizeof(char), sizeof(BITMAPINFOHEADER), Prewitt_file);
	fwrite(hRGB, sizeof(RGBQUAD), 256, Prewitt_file);
	fwrite(m_OutImg, sizeof(unsigned char), 512 * 512, Prewitt_file);

	FILE *Prewitt_file_n;
	fopen_s(&Prewitt_file_n, "Prewitt_noise.bmp", "wb");
	fwrite(&hf, sizeof(char), sizeof(BITMAPFILEHEADER), Prewitt_file_n);
	fwrite(&hInfo, sizeof(char), sizeof(BITMAPINFOHEADER), Prewitt_file_n);
	fwrite(hRGB, sizeof(RGBQUAD), 256, Prewitt_file_n);
	fwrite(m_OutImg_n, sizeof(unsigned char), 512 * 512, Prewitt_file_n);

	//동적 할당 메모리 해제
	delete[] pImgPrewittX;
	delete[] pImgPrewittY;
	delete[] pImgPrewittX_n;
	delete[] pImgPrewittY_n;

	int cnt_1 = 0, cnt_0 = 0;
	float P_e;
	printf("=================================================================\n");
	printf("                          [Prewitt P_e]               \n\n");
	for (i = 0; i<N; i++){
		for (j = 0; j<N; j++){
			if (m_OutImg[i][j] == 255 && m_OutImg_n[i][j] != 255) { cnt_1++; }
		}
	}
	for (i = 0; i<N; i++){
		for (j = 0; j<N; j++){
			if (m_OutImg[i][j] != 255 && m_OutImg_n[i][j] == 255) { cnt_1++; }
		}
	}
	for (i = 0; i<N; i++){
		for (j = 0; j<N; j++){
			if (m_OutImg[i][j] == 255){ cnt_0++; }
		}
	}
	printf("                   [N_1] = %d, [N_0] = %d          \n\n", cnt_1, cnt_0);
	P_e = (float)cnt_1 / cnt_0;
	printf("                         [P_e] = %f                    \n", P_e);
	printf("=================================================================\n");
}
void Sobel(BYTE InImg[N][N], BYTE noise_img[N][N], BITMAPFILEHEADER hf, BITMAPINFOHEADER hInfo, RGBQUAD* hRGB) {
	int i, j, r, c;

	int MaskSobelX[3][3] = { { -1, 0, 1 },
	{ -2, 0, 2 },
	{ -1, 0, 1 } };
	int MaskSobelY[3][3] = { { 1, 2, 1 },
	{ 0, 0, 0 },
	{ -1, -2, -1 } };
	int heightm1 = height - 1;
	int widthm1 = width - 1;
	int mr, mc;
	int newValue;
	int newValue_n;
	int *pImgSobelX, *pImgSobelY;
	int *pImgSobelX_n, *pImgSobelY_n;
	int min, max, where;
	float constVal1, constVal2;

	//정수값을 갖는 이미지 동적 메모리 할당
	pImgSobelX = new int[height*width];
	pImgSobelY = new int[height*width];

	pImgSobelX_n = new int[height*width];
	pImgSobelY_n = new int[height*width];

	//결과 이미지 0으로 초기화
	for (i = 0; i<height; i++)
		for (j = 0; j<width; j++)
		{
			m_OutImg[i][j] = 0;
			m_OutImg_n[i][j] = 0;
			where = i*width + j;
			pImgSobelX[where] = 0;
			pImgSobelY[where] = 0;
			pImgSobelX_n[where] = 0;
			pImgSobelY_n[where] = 0;
		}

	//X 방향 에지 강도 계산 
	for (i = 1; i<heightm1; i++)
	{
		for (j = 1; j<widthm1; j++)
		{
			newValue = 0; //0으로 초기화
			newValue_n = 0;
			for (mr = 0; mr<3; mr++)
			{
				for (mc = 0; mc<3; mc++)
				{
					newValue += (MaskSobelX[mr][mc] * InImg[i + mr - 1][j + mc - 1]);
					newValue_n += (MaskSobelX[mr][mc] * noise_img[i + mr - 1][j + mc - 1]);
				}
				pImgSobelX[i*width + j] = newValue;
				pImgSobelX_n[i*width + j] = newValue_n;
			}
		}
	}
	//Y 방향 에지 강도 계산 

	for (i = 1; i<heightm1; i++)
	{
		for (j = 1; j<widthm1; j++)
		{
			newValue = 0; //0으로 초기화
			newValue_n = 0;
			for (mr = 0; mr<3; mr++)
			{
				for (mc = 0; mc<3; mc++)
				{
					newValue += (MaskSobelY[mr][mc] * InImg[i + mr - 1][j + mc - 1]);
					newValue_n += (MaskSobelY[mr][mc] * noise_img[i + mr - 1][j + mc - 1]);
				}
				pImgSobelY[i*width + j] = newValue;
				pImgSobelY_n[i*width + j] = newValue_n;
			}
		}
	}

	float value;
	float value_n;
	for (i = 0; i<height; i++)
	{
		for (j = 0; j<width; j++)
		{
			value = sqrt(pow(pImgSobelX[i*width + j], 2.0) + pow(pImgSobelY[i*width + j], 2.0));
			if (value>150){
				pImgSobelX[i*width + j] = 255;
			}
			else
				pImgSobelX[i*width + j] = value;
		}
	}

	for (i = 0; i<height; i++)
	{
		for (j = 0; j<width; j++)
		{
			newValue = pImgSobelX[i*width + j];
			m_OutImg[i][j] = (BYTE)newValue;
		}
	}

	for (i = 0; i<height; i++)
	{
		for (j = 0; j<width; j++)
		{
			value_n = sqrt(pow(pImgSobelX_n[i*width + j], 2.0) + pow(pImgSobelY_n[i*width + j], 2.0));
			if (value_n>150){
				pImgSobelX_n[i*width + j] = 255;
			}
			else
				pImgSobelX_n[i*width + j] = value_n;
		}
	}

	for (i = 0; i<height; i++)
	{
		for (j = 0; j<width; j++)
		{
			newValue_n = pImgSobelX_n[i*width + j];
			m_OutImg_n[i][j] = (BYTE)newValue_n;
		}
	}

	FILE *Sobel_file;
	fopen_s(&Sobel_file, "Sobel.bmp", "wb");
	fwrite(&hf, sizeof(char), sizeof(BITMAPFILEHEADER), Sobel_file);
	fwrite(&hInfo, sizeof(char), sizeof(BITMAPINFOHEADER), Sobel_file);
	fwrite(hRGB, sizeof(RGBQUAD), 256, Sobel_file);
	fwrite(m_OutImg, sizeof(unsigned char), 512 * 512, Sobel_file);

	FILE *Sobel_file_n;
	fopen_s(&Sobel_file_n, "Sobel_noise.bmp", "wb");
	fwrite(&hf, sizeof(char), sizeof(BITMAPFILEHEADER), Sobel_file_n);
	fwrite(&hInfo, sizeof(char), sizeof(BITMAPINFOHEADER), Sobel_file_n);
	fwrite(hRGB, sizeof(RGBQUAD), 256, Sobel_file_n);
	fwrite(m_OutImg_n, sizeof(unsigned char), 512 * 512, Sobel_file_n);

	//동적 할당 메모리 해제
	delete[] pImgSobelX;
	delete[] pImgSobelY;
	delete[] pImgSobelX_n;
	delete[] pImgSobelY_n;

	int cnt_1 = 0, cnt_0 = 0;
	float P_e;
	printf("=================================================================\n");
	printf("                          [Sobel P_e]               \n\n");
	for (i = 0; i<N; i++){
		for (j = 0; j<N; j++){
			if (m_OutImg[i][j] == 255 && m_OutImg_n[i][j] != 255) { cnt_1++; }
		}
	}
	for (i = 0; i<N; i++){
		for (j = 0; j<N; j++){
			if (m_OutImg[i][j] != 255 && m_OutImg_n[i][j] == 255) { cnt_1++; }
		}
	}
	for (i = 0; i<N; i++){
		for (j = 0; j<N; j++){
			if (m_OutImg[i][j] == 255){ cnt_0++; }
		}
	}
	printf("                   [N_1] = %d, [N_0] = %d          \n\n", cnt_1, cnt_0);
	P_e = (float)cnt_1 / cnt_0;
	printf("                         [P_e] = %f                    \n", P_e);
	printf("=================================================================\n");
}

void Stochastic(BYTE InImg[N][N], BYTE noise_img[N][N], BITMAPFILEHEADER hf, BITMAPINFOHEADER hInfo, RGBQUAD* hRGB) {
	int i, j, r, c;

	double MaskStochasticX[5][5] = { { 0.267, 0.364, 0, -0.364, -0.267 },
	{ 0.373, 0.562, 0, -0.562, -0.373 },
	{ 0.463, 1.000, 0, -1.000, -0.463 },
	{ 0.373, 0.562, 0, -0.562, -0.373 },
	{ 0.267, 0.364, 0, -0.364, -0.267 } };

	double MaskStochasticY[5][5] = { { 0.267, 0.373, 0.463, 0.373, 0.267 },
	{ 0.364, 0.562, 1.000, 0.562, 0.364 },
	{ 0.000, 0.000, 0.000, 0.000, 0.000 },
	{ -0.364, -0.562, -1.000, -0.562, -0.364 },
	{ -0.267, -0.373, -0.463, -0.373, -0.267 } };

	int heightm1 = height - 1;
	int widthm1 = width - 1;
	int mr, mc;
	int newValue;
	int newValue_n;
	int *pImgStochasticX, *pImgStochasticY;
	int *pImgStochasticX_n, *pImgStochasticY_n;
	int min, max, where;
	float constVal1, constVal2;

	//정수값을 갖는 이미지 동적 메모리 할당
	pImgStochasticX = new int[height*width];
	pImgStochasticY = new int[height*width];

	pImgStochasticX_n = new int[height*width];
	pImgStochasticY_n = new int[height*width];

	//결과 이미지 0으로 초기화
	for (i = 0; i<height; i++)
		for (j = 0; j<width; j++)
		{
			m_OutImg[i][j] = 0;
			m_OutImg_n[i][j] = 0;
			where = i*width + j;
			pImgStochasticX[where] = 0;
			pImgStochasticY[where] = 0;
			pImgStochasticX_n[where] = 0;
			pImgStochasticY_n[where] = 0;
		}

	//X 방향 에지 강도 계산 
	for (i = 1; i<heightm1; i++)
	{
		for (j = 1; j<widthm1; j++)
		{
			newValue = 0; //0으로 초기화
			newValue_n = 0;
			for (mr = 0; mr<5; mr++)
			{
				for (mc = 0; mc<5; mc++)
				{
					newValue += (MaskStochasticX[mr][mc] * InImg[i + mr - 1][j + mc - 1]);
					newValue_n += (MaskStochasticX[mr][mc] * noise_img[i + mr - 1][j + mc - 1]);
				}
				pImgStochasticX[i*width + j] = newValue;
				pImgStochasticX_n[i*width + j] = newValue_n;
			}
		}
	}
	//Y 방향 에지 강도 계산 

	for (i = 1; i<heightm1; i++)
	{
		for (j = 1; j<widthm1; j++)
		{
			newValue = 0; //0으로 초기화
			newValue_n = 0;
			for (mr = 0; mr<5; mr++)
			{
				for (mc = 0; mc<5; mc++)
				{
					newValue += (MaskStochasticY[mr][mc] * InImg[i + mr - 1][j + mc - 1]);
					newValue_n += (MaskStochasticY[mr][mc] * noise_img[i + mr - 1][j + mc - 1]);
				}
				pImgStochasticY[i*width + j] = newValue;
				pImgStochasticY_n[i*width + j] = newValue_n;
			}
		}
	}

	float value;
	float value_n;
	for (i = 0; i<height; i++)
	{
		for (j = 0; j<width; j++)
		{
			value = sqrt(pow(pImgStochasticX[i*width + j], 2.0) + pow(pImgStochasticY[i*width + j], 2.0));
			if (value>150){
				pImgStochasticX[i*width + j] = 255;
			}
			else
				pImgStochasticX[i*width + j] = value;
		}
	}

	for (i = 0; i<height; i++)
	{
		for (j = 0; j<width; j++)
		{
			newValue = pImgStochasticX[i*width + j];
			m_OutImg[i][j] = (BYTE)newValue;
		}
	}

	for (i = 0; i<height; i++)
	{
		for (j = 0; j<width; j++)
		{
			value_n = sqrt(pow(pImgStochasticX_n[i*width + j], 2.0) + pow(pImgStochasticY_n[i*width + j], 2.0));
			if (value_n>150){
				pImgStochasticX_n[i*width + j] = 255;
			}
			else
				pImgStochasticX_n[i*width + j] = value_n;
		}
	}

	for (i = 0; i<height; i++)
	{
		for (j = 0; j<width; j++)
		{
			newValue_n = pImgStochasticX_n[i*width + j];
			m_OutImg_n[i][j] = (BYTE)newValue_n;
		}
	}

	FILE *Stochastic_file;
	fopen_s(&Stochastic_file, "Stochastic.bmp", "wb");
	fwrite(&hf, sizeof(char), sizeof(BITMAPFILEHEADER), Stochastic_file);
	fwrite(&hInfo, sizeof(char), sizeof(BITMAPINFOHEADER), Stochastic_file);
	fwrite(hRGB, sizeof(RGBQUAD), 256, Stochastic_file);
	fwrite(m_OutImg, sizeof(unsigned char), 512 * 512, Stochastic_file);

	FILE *Stochastic_file_n;
	fopen_s(&Stochastic_file_n, "Stochastic_noise.bmp", "wb");
	fwrite(&hf, sizeof(char), sizeof(BITMAPFILEHEADER), Stochastic_file_n);
	fwrite(&hInfo, sizeof(char), sizeof(BITMAPINFOHEADER), Stochastic_file_n);
	fwrite(hRGB, sizeof(RGBQUAD), 256, Stochastic_file_n);
	fwrite(m_OutImg_n, sizeof(unsigned char), 512 * 512, Stochastic_file_n);

	//동적 할당 메모리 해제
	delete[] pImgStochasticX;
	delete[] pImgStochasticY;
	delete[] pImgStochasticX_n;
	delete[] pImgStochasticY_n;

	int cnt_1 = 0, cnt_0 = 0;
	float P_e;
	printf("=================================================================\n");
	printf("                        [Stochastic P_e]               \n\n");
	for (i = 0; i<N; i++){
		for (j = 0; j<N; j++){
			if (m_OutImg[i][j] == 255 && m_OutImg_n[i][j] != 255) { cnt_1++; }
		}
	}
	for (i = 0; i<N; i++){
		for (j = 0; j<N; j++){
			if (m_OutImg[i][j] != 255 && m_OutImg_n[i][j] == 255) { cnt_1++; }
		}
	}
	for (i = 0; i<N; i++){
		for (j = 0; j<N; j++){
			if (m_OutImg[i][j] == 255){ cnt_0++; }
		}
	}
	printf("                   [N_1] = %d, [N_0] = %d          \n\n", cnt_1, cnt_0);
	P_e = (float)cnt_1 / cnt_0;
	printf("                         [P_e] = %f                    \n", P_e);
	printf("=================================================================\n");
}

void Lowpass(BYTE boat_img[N][N], BYTE noise_img2[N][N], BITMAPFILEHEADER hf, BITMAPINFOHEADER hInfo, RGBQUAD* hRGB){
	int i, j, r, c;

	double MaskLowpassX[3][3] = { { 1, 1, 1 }, { 1, 1, 1 }, { 1, 1, 1 } };

	int heightm1 = height - 1;
	int widthm1 = width - 1;
	int mr, mc;
	int newValue;
	int newValue_n;
	int *pImgLowpassX, *pImgLowpassY;
	int *pImgLowpassX_n, *pImgLowpassY_n;
	int min, max, where;
	float constVal1, constVal2;

	//정수값을 갖는 이미지 동적 메모리 할당
	pImgLowpassX = new int[height*width];
	pImgLowpassY = new int[height*width];

	pImgLowpassX_n = new int[height*width];
	pImgLowpassY_n = new int[height*width];

	//결과 이미지 0으로 초기화
	for (i = 0; i<height; i++)
		for (j = 0; j<width; j++)
		{
			m_OutImg[i][j] = 0;
			m_OutImg_n[i][j] = 0;
			where = i*width + j;
			pImgLowpassX[where] = 0;
			pImgLowpassY[where] = 0;
			pImgLowpassX_n[where] = 0;
			pImgLowpassY_n[where] = 0;
		}

	//X 방향 에지 강도 계산 
	for (i = 1; i<heightm1; i++)
	{
		for (j = 1; j<widthm1; j++)
		{
			newValue = 0; //0으로 초기화
			newValue_n = 0;
			for (mr = 0; mr<3; mr++)
			{
				for (mc = 0; mc<3; mc++)
				{
					newValue += (MaskLowpassX[mr][mc] * boat_img[i + mr - 1][j + mc - 1]) / 9;
					newValue_n += (MaskLowpassX[mr][mc] * noise_img2[i + mr - 1][j + mc - 1]) / 9;
				}
				pImgLowpassX[i*width + j] = newValue;
				pImgLowpassX_n[i*width + j] = newValue_n;
			}
		}
	}

	for (i = 0; i<height; i++)
	{
		for (j = 0; j<width; j++)
		{
			newValue = pImgLowpassX[i*width + j];
			m_OutImg[i][j] = (BYTE)newValue;
		}
	}

	for (i = 0; i<height; i++)
	{
		for (j = 0; j<width; j++)
		{
			newValue_n = pImgLowpassX_n[i*width + j];
			m_OutImg_n[i][j] = (BYTE)newValue_n;
		}
	}

	FILE *Lowpass_file;
	fopen_s(&Lowpass_file, "Lowpass.raw", "wb");
	fwrite(m_OutImg, sizeof(unsigned char), 512 * 512, Lowpass_file);

	FILE *Lowpass_file_n;
	fopen_s(&Lowpass_file_n, "Lowpass_noise.raw", "wb");
	fwrite(m_OutImg_n, sizeof(unsigned char), 512 * 512, Lowpass_file_n);

	//동적 할당 메모리 해제
	delete[] pImgLowpassX;
	delete[] pImgLowpassY;
	delete[] pImgLowpassX_n;
	delete[] pImgLowpassY_n;

	float MSE = 0.0;
	for (i = 0; i<N; i++){
		for (j = 0; j<N; j++){
			MSE += ((boat_img[i][j] - m_OutImg_n[i][j])*(boat_img[i][j] - m_OutImg_n[i][j]));
		}
	}
	MSE = MSE / (N*N);

	printf("=================================================================\n");
	printf("                         [Low-Pass MSE]               \n");
	printf("                       [LP MSE] = %.3f                  \n", MSE);
	printf("=================================================================\n");
}

void Median(BYTE boat_img[N][N], BYTE noise_img2[N][N], BITMAPFILEHEADER hf, BITMAPINFOHEADER hInfo, RGBQUAD* hRGB){
	int i, j, r, c, k, h;

	int heightm1 = height - 1;
	int widthm1 = width - 1;
	int mr, mc;
	int newValue;
	int newValue_n;
	int *pImgLowpassX, *pImgLowpassY;
	int *pImgLowpassX_n, *pImgLowpassY_n;
	int min, max, where;
	float constVal1, constVal2;

	//정수값을 갖는 이미지 동적 메모리 할당
	pImgLowpassX = new int[height*width];
	pImgLowpassY = new int[height*width];

	pImgLowpassX_n = new int[height*width];
	pImgLowpassY_n = new int[height*width];

	//결과 이미지 0으로 초기화
	for (i = 0; i<height; i++){
		for (j = 0; j<width; j++)
		{
			m_OutImg[i][j] = 0;
			m_OutImg_n[i][j] = 0;
			where = i*width + j;
			pImgLowpassX[where] = 0;
			pImgLowpassY[where] = 0;
			pImgLowpassX_n[where] = 0;
			pImgLowpassY_n[where] = 0;
		}
	}

	for (i = 1; i<N - 1; i++)
	{
		for (j = 1; j<N - 1; j++)
		{
			for (mr = 0; mr<3; mr++)
			{
				for (mc = 0; mc<3; mc++)
				{
					temp[mr][mc] = boat_img[i + mr - 1][j + mc - 1];
					temp_n[mr][mc] = noise_img2[i + mr - 1][j + mc - 1];
				}
			}
			sort(temp, temp_n);
			m_OutImg[i][j] = temp[1][1];
			m_OutImg_n[i][j] = temp_n[1][1];
		}
	}

	FILE *Median_file;
	fopen_s(&Median_file, "Median.raw", "wb");
	fwrite(m_OutImg, sizeof(unsigned char), 512 * 512, Median_file);

	FILE *Median_file_n;
	fopen_s(&Median_file_n, "Median_noise.raw", "wb");
	fwrite(m_OutImg_n, sizeof(unsigned char), 512 * 512, Median_file_n);

	float MSE = 0.0;
	for (i = 0; i<N; i++){
		for (j = 0; j<N; j++){
			MSE += ((boat_img[i][j] - m_OutImg_n[i][j])*(boat_img[i][j] - m_OutImg_n[i][j]));
		}
	}
	MSE = MSE / (N*N);
	printf("=================================================================\n");
	printf("                         [Median MSE]               \n");
	printf("                       [Md MSE] = %.3f                  \n", MSE);
	printf("=================================================================\n");
}

void sort(BYTE temp[3][3], BYTE temp_n[3][3]){
	int k, h, i, j;
	for (i = 0; i<3; i++)
	{
		for (j = 0; j<3; j++)
		{
			for (k = 0; k<3; k++)
			{
				for (h = 0; h<3; h++)
				{
					if (temp[i][j]>temp[k][h]){
						buffer = temp[i][j];
						temp[i][j] = temp[k][h];
						temp[k][h] = buffer;
					}
				}
			}
		}
	}
}


/// 분산 구하기 ///
float get_image_power(unsigned char InImg[][N])
{
	int i, j;
	float mean_square, second_order_moment;
	float var;

	mean_square = -0.f;
	second_order_moment = 0;

	for (i = 0; i<N; i++){
		for (j = 0; j<N; j++){
			second_order_moment += (InImg[i][j] * InImg[i][j]);
			mean_square += InImg[i][j];
		}
	}
	second_order_moment = second_order_moment / (N*N);				// 제곱의 평균
	mean_square = (mean_square / (N*N) * mean_square / (N*N));	// 평균의 제곱
	var = second_order_moment - mean_square;
	return var;
}



//////////////////////////////////////////////////////////////////////////
/// 가우시안 노이즈 더함///
void AddGaussianNoise(BYTE input_img[N][N], BYTE noise_img[N][N], double sigma)				 /*Add Gaussian Noise to the input image */
{
	int i, j;
	int s;
	for (i = 0; i<N; i++)
		for (j = 0; j<N; j++)
		{
			s = input_img[i][j] + (int)Gaussian((float)sigma);
			noise_img[i][j] = s>255 ? 255 : s<0 ? 0 : s;
		}
}

//////////////////////////////////////////////////////////////////////////
/// 가우시안 생성 ///
float Gaussian(float sd)
{
	static int ready = 0;
	static float gstore;
	float v1, v2, r, fac, gaus;
	int r1, r2;

	if (ready == 0) {
		do {
			r1 = rand();
			r2 = rand();
			v1 = (float)(2.*((float)r1 / RAND_MAX - 0.5));
			v2 = (float)(2.*((float)r2 / (float)RAND_MAX - 0.5));
			r = v1*v1 + v2*v2;
		} while (r>1.0);
		fac = (float)sqrt((double)(-2 * log(r) / r));
		gstore = v1*fac;
		gaus = v2*fac;
		ready = 1;
	}
	else {
		ready = 0;
		gaus = gstore;
	}
	return (gaus*sd);
}
//////////////////////////////////////////////////////////////////////////