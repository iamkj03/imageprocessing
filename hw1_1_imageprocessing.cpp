#include<stdio.h>
#include<Windows.h>

#define N 512

void main()
{
	unsigned char image[N][N];			// 512*512 배열

	FILE *fp1, *fp2;

	BITMAPFILEHEADER h_File;			// 파일헤드
	BITMAPINFOHEADER h_Info;			// 영상헤드
	RGBQUAD pallete[256];					// 팔레트정보

	fopen_s(&fp1, "lena_512x512.bmp", "rb");

	fread(&h_File, sizeof(BITMAPFILEHEADER), 1, fp1);		// 'lena_512x512.bmp' 파일헤드 읽기
	fread(&h_Info, sizeof(BITMAPINFOHEADER), 1, fp1);		// 'lena_512x512.bmp' 영상헤드 읽기
	fread(pallete, sizeof(RGBQUAD), 256, fp1);					// 'lena_512x512.bmp' 팔레트정보 읽기

	fopen_s(&fp2, "output1_1.raw", "wb");

	for (int i = 0; i<512; i++)							// y 축
	{
		for (int j = 0; j<512; j++)						// x 축
		{
			if (j<100)									// 0~100
				image[i][j] = 120;
			else if (j<200)								// 100~200
				image[i][j] = (15 * j) / 100 + 105;
			else if (j<280)								// 200~280
				image[i][j] = (90 * j) / 80 - 90;
			else if (j<300)								// 280~300
				image[i][j] = (15 * j) / 20 + 15;
			else if (j<512)								// 300~512								
				image[i][j] = 240;
		}
	}

	fwrite(&h_File, sizeof(BITMAPFILEHEADER), 1, fp2);		// 파일헤드 쓰기
	fwrite(&h_Info, sizeof(BITMAPINFOHEADER), 1, fp2);		// 영상헤드 쓰기
	fwrite(pallete, sizeof(RGBQUAD), 256, fp2);					// 팔레트 쓰기
	fwrite(image, sizeof(unsigned char), N*N, fp2);			// 영상데이터 쓰기

	fclose(fp1);
	fclose(fp2);
}