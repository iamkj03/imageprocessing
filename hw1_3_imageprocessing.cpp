#include<stdio.h>
#include<Windows.h>

#define N 512

void main()
{
	unsigned char image[N][N];
	unsigned char image_rotate[N][N];

	FILE *fp1, *fp2;

	BITMAPFILEHEADER h_File;			// �������
	BITMAPINFOHEADER h_Info;			// �������
	RGBQUAD pallete[256];					// �ȷ�Ʈ����

	fopen_s(&fp1, "lena_512x512.bmp", "rb");

	fread(&h_File, sizeof(BITMAPFILEHEADER), 1, fp1);		// 'lena_512x512.bmp' ������� �б�
	fread(&h_Info, sizeof(BITMAPINFOHEADER), 1, fp1);		// 'lena_512x512.bmp' ������� �б�
	fread(pallete, sizeof(RGBQUAD), 256, fp1);					// 'lena_512x512.bmp' �ȷ�Ʈ���� �б�

	fopen_s(&fp2, "output1_3.bmp", "wb");

	// x ��� y ���� �ٲ۴�.
	for (int i = 0; i<512; i++)							// y ��
	{
		for (int j = 0; j<512; j++)						// x ��
		{
			if (i<100)									// 0~100
				image[i][j] = 120;
			else if (i<200)								// 100~200
				image[i][j] = (i * 15) / 100 + 105;
			else if (i<280)								// 200~280
				image[i][j] = (i * 90) / 80 - 90;
			else if (i<300)								// 280~300
				image[i][j] = (i * 15) / 20 + 15;
			else if (i<512)								// 300~512
				image[i][j] = 240;
		}
	}

	for (int i = 0; i<512; i++)							// y ��
		for (int j = 0; j<512; j++)						// x ��
			image_rotate[i][j] = image[N - 1 - i][j];	// x, y ���� �ٲ� ������ ���� ������ �ٲ۴�

	fwrite(&h_File, sizeof(BITMAPFILEHEADER), 1, fp2);		// ������� ����
	fwrite(&h_Info, sizeof(BITMAPINFOHEADER), 1, fp2);		// ������� ����
	fwrite(pallete, sizeof(RGBQUAD), 256, fp2);					// �ȷ�Ʈ ����
	fwrite(image_rotate, sizeof(unsigned char), N*N, fp2);			// �������� ����

	fclose(fp1);
	fclose(fp2);
}