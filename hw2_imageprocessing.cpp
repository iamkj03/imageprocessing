#include<stdio.h>
#include<windows.h>
#include<math.h>
#define N 512
#define BYTE unsigned char

void hw2_1();
void hw2_2();
void hw2_3();
void hw2_4();

void main(){
	hw2_1();
	hw2_2();
	hw2_3();
	hw2_4();
}
void hw2_1(){

	BITMAPFILEHEADER hf;
	BITMAPINFOHEADER hInfo;
	RGBQUAD hRGB[256];
	BYTE image[N][N];

	int cnt[256] = { 0, };
	int i, j;

	FILE *infile;
	fopen_s(&infile, "lena_bmp_512x512_new.bmp", "rb");

	fread(&hf, sizeof(BITMAPFILEHEADER), 1, infile);	// 파일헤더 읽기
	fread(&hInfo, sizeof(BITMAPINFOHEADER), 1, infile);	// 영상헤더 읽기
	fread(&hRGB, sizeof(RGBQUAD), 256, infile);		// 팔레트 정보 읽기
	fread(image, sizeof(BYTE), N*N, infile);

	// 픽셀값 존재하면 카운트
	for (i = 0; i<512; i++){
		for (j = 0; j<512; j++){
			cnt[image[i][j]]++;
		}
	}

	FILE *outfile;
	fopen_s(&outfile, "Histogram.txt", "w");

	for (i = 0; i<256; i++){
		fprintf(outfile, "%d\n", cnt[i]);
	}
	fclose(infile);
	fclose(outfile);
}
void hw2_2(){

	BITMAPFILEHEADER hf;
	BITMAPINFOHEADER hInfo;
	RGBQUAD hRGB[256];
	BYTE image[N][N];
	BYTE temp[N][N];

	int cnt[256] = { 0, };
	int i, j;

	FILE *infile;
	fopen_s(&infile, "lena_bmp_512x512_new.bmp", "rb");

	fread(&hf, sizeof(BITMAPFILEHEADER), 1, infile);	// 파일헤더 읽기
	fread(&hInfo, sizeof(BITMAPINFOHEADER), 1, infile);	// 영상헤더 읽기
	fread(&hRGB, sizeof(RGBQUAD), 256, infile);		// 팔레트 정보 읽기
	fread(image, sizeof(BYTE), N*N, infile);
	fclose(infile);

	// 픽셀값 존재하면 카운트
	for (i = 0; i<512; i++){
		for (j = 0; j<512; j++){
			cnt[image[i][j]]++;
		}
	}

	int sum[256];
	float Nsum[256];  // Normalized sum

	for (i = 0; i<256; i++){
		if (i == 0)
			sum[i] = cnt[i];
		else{
			sum[i] = sum[i - 1] + cnt[i];
			Nsum[i] = floor((float)((sum[i] * 256) / (N*N) + 0.5));
		}
	}

	for (i = 0; i<512; i++){
		for (j = 0; j<512; j++){
			temp[i][j] = (BYTE)(Nsum[image[i][j]]);
		}
	}

	// histogram equalize를 위해 초기화
	for (i = 0; i<256; i++)
		cnt[i] = 0;
	for (i = 0; i<512; i++){
		for (j = 0; j<512; j++){
			cnt[temp[i][j]]++;
		}
	}

	FILE *outfile;
	fopen_s(&outfile, "Lena_Hist_Equalization.bmp", "wb");
	fwrite(&hf, sizeof(BITMAPFILEHEADER), 1, outfile);	// 파일헤더 읽기
	fwrite(&hInfo, sizeof(BITMAPINFOHEADER), 1, outfile);	// 영상헤더 읽기
	fwrite(&hRGB, sizeof(RGBQUAD), 256, outfile);		// 팔레트 정보 읽기
	fwrite(temp, sizeof(BYTE), N*N, outfile);
	fclose(outfile);

	FILE *fp;
	fopen_s(&fp, "Hist_Equalization.txt", "w");
	for (i = 0; i<256; i++){
		fprintf(outfile, "%d\n", cnt[i]);
	}
	fclose(fp);
}
void hw2_3(){

	BITMAPFILEHEADER hf;
	BITMAPINFOHEADER hInfo;
	RGBQUAD hRGB[256];
	BYTE image[N][N];
	BYTE temp[N][N];

	int cnt[256] = { 0, };
	int i, j;

	FILE *infile;
	fopen_s(&infile, "lena_bmp_512x512_new.bmp", "rb");

	fread(&hf, sizeof(BITMAPFILEHEADER), 1, infile);	// 파일헤더 읽기
	fread(&hInfo, sizeof(BITMAPINFOHEADER), 1, infile);	// 영상헤더 읽기
	fread(&hRGB, sizeof(RGBQUAD), 256, infile);		// 팔레트 정보 읽기
	fread(image, sizeof(BYTE), N*N, infile);
	fclose(infile);

	for (i = 0; i<512; i++){
		for (j = 0; j<512; j++){
			cnt[image[i][j]]++;
		}
	}
	int low = 0;
	int high = 0;

	for (i = 0; i<256; i++){
		if (cnt[i] != 0){
			low = i;
			break;
		}
	}
	for (i = 255; i>0; i--){
		if (cnt[i] != 0){
			high = i;
			break;
		}
	}
	for (i = 0; i<512; i++){
		for (j = 0; j<512; j++){
			temp[i][j] = (BYTE)floor((float)(image[i][j] - low) / (high - low) * 255 + 0.5);
		}
	}
	for (i = 0; i<256; i++)
		cnt[i] = 0;

	for (i = 0; i<512; i++){
		for (j = 0; j<512; j++){
			cnt[temp[i][j]]++;
		}
	}

	FILE *outfile;
	fopen_s(&outfile, "Lena_Hist_Basic.bmp", "wb");
	fwrite(&hf, sizeof(BITMAPFILEHEADER), 1, outfile);	// 파일헤더 읽기
	fwrite(&hInfo, sizeof(BITMAPINFOHEADER), 1, outfile);	// 영상헤더 읽기
	fwrite(&hRGB, sizeof(RGBQUAD), 256, outfile);		// 팔레트 정보 읽기
	fwrite(temp, sizeof(BYTE), N*N, outfile);
	fclose(outfile);

	FILE *fp;
	fopen_s(&fp, "Hist_Basic.txt", "w");
	for (i = 0; i<256; i++){
		fprintf(outfile, "%d\n", cnt[i]);
	}
	fclose(fp);
}
void hw2_4(){

	BITMAPFILEHEADER hf;
	BITMAPINFOHEADER hInfo;
	RGBQUAD hRGB[256];
	BYTE image[N][N];
	BYTE temp[N][N];

	int cnt[256] = { 0, };
	int i, j;

	FILE *infile;
	fopen_s(&infile, "lena_bmp_512x512_new.bmp", "rb");

	fread(&hf, sizeof(BITMAPFILEHEADER), 1, infile);	// 파일헤더 읽기
	fread(&hInfo, sizeof(BITMAPINFOHEADER), 1, infile);	// 영상헤더 읽기
	fread(&hRGB, sizeof(RGBQUAD), 256, infile);		// 팔레트 정보 읽기
	fread(image, sizeof(BYTE), N*N, infile);
	fclose(infile);

	for (i = 0; i<512; i++){
		for (j = 0; j<512; j++){
			cnt[image[i][j]]++;
		}
	}
	int low = 50;
	int high = 190;

	for (i = 0; i<512; i++){
		for (j = 0; j<512; j++){
			if (image[i][j]<50)
				temp[i][j] = 0;
			else if (image[i][j]>190)
				temp[i][j] = 255;
			else
				temp[i][j] = (BYTE)floor((float)(image[i][j] - low) / (high - low) * 255 + 0.5);
		}
	}

	for (i = 0; i<256; i++)
		cnt[i] = 0;

	for (i = 0; i<512; i++){
		for (j = 0; j<512; j++){
			cnt[temp[i][j]]++;
		}
	}

	FILE *outfile;
	fopen_s(&outfile, "Lena_Hist_Endin.bmp", "wb");
	fwrite(&hf, sizeof(BITMAPFILEHEADER), 1, outfile);	// 파일헤더 읽기
	fwrite(&hInfo, sizeof(BITMAPINFOHEADER), 1, outfile);	// 영상헤더 읽기
	fwrite(&hRGB, sizeof(RGBQUAD), 256, outfile);		// 팔레트 정보 읽기
	fwrite(temp, sizeof(BYTE), N*N, outfile);
	fclose(outfile);

	FILE *fp;
	fopen_s(&fp, "Hist_Endin.txt", "w");
	for (i = 0; i<256; i++){
		fprintf(outfile, "%d\n", cnt[i]);
	}
	fclose(fp);


}