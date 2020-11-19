#include<stdio.h>
#include<stdlib.h>
#include<windows.h>
#include<math.h>

#define B_size 8
#define nint(x)   ( (x)<0. ? (int)((x)-0.5) : (int)((x)+0.5) )
#define N 512
#define BYTE unsigned char

void dct8x8(int ix[B_size][B_size]);
void idct8x8(int ix[B_size][B_size]);

int ix[B_size][B_size];

unsigned char InImg[N][N];
unsigned char OutImg_DCT[N][N];
unsigned char OutImg_INVERSE[N][N];

void main()
{
	int i, j, ii, jj, k, m, s, x;
	double cal = 0.0;
	double MSE = 0.0;

	FILE *infile, *outfile1, *outfile2;

	do{

		MSE = 0.0;
		cal = 0.0;
		printf("=====================================================");
		printf("\n\n\t 1.Lena   2. Boat   3. exit   :   ");
		scanf_s("%d", &x, sizeof(x));

		if (x == 1){
			fopen_s(&infile, "lena_raw_512x512.raw", "rb");
			fread(InImg, sizeof(char), 512 * 512, infile);
			fclose(infile);
		}
		else if (x == 2){
			fopen_s(&infile, "BOAT512.raw.raw", "rb");
			fread(InImg, sizeof(char), 512 * 512, infile);
			fclose(infile);
		}
		else if (x == 3){ printf("\n"); printf("=====================================================\n\n"); break; }
		else{ printf("\n\t잘못 입력 하셨습니다.\n\n"); fflush(stdin); }

		for (i = 0; i<N; i = i + 8)
		{
			for (j = 0; j<N; j = j + 8)
			{
				for (ii = 0; ii<B_size; ii++){
					for (jj = 0; jj<B_size; jj++){
						ix[ii][jj] = InImg[i + ii][j + jj];
					}
				}

				dct8x8(ix);

				for (k = 0; k<B_size; k++){
					for (m = 0; m<B_size; m++){
						s = ix[k][m];
						OutImg_DCT[i + k][j + m] = s>255 ? 255 : s<0 ? 0 : s;
					}
				}

				idct8x8(ix);

				for (k = 0; k<B_size; k++){
					for (m = 0; m<B_size; m++){
						OutImg_INVERSE[i + k][j + m] = ix[k][m];
					}
				}
			}
		}

		if (x == 1){
			fopen_s(&outfile1, "OutImg_DCT_Lena.raw", "wb");
			fwrite(OutImg_DCT, sizeof(unsigned char), 512 * 512, outfile1);
			fclose(outfile1);
			printf("\n\t Lena DCT is created.\n");

			fopen_s(&outfile2, "OutImg_Inverse_Lena.raw", "wb");
			fwrite(OutImg_INVERSE, sizeof(unsigned char), 512 * 512, outfile2);
			fclose(outfile2);
			printf("\t Lena Inverse is created.\n");
		}
		if (x == 2){
			fopen_s(&outfile1, "OutImg_DCT_Boat.raw", "wb");
			fwrite(OutImg_DCT, sizeof(unsigned char), 512 * 512, outfile1);
			fclose(outfile1);
			printf("\n\t Boat DCT is created.\n");

			fopen_s(&outfile2, "OutImg_Inverse_Boat.raw", "wb");
			fwrite(OutImg_INVERSE, sizeof(unsigned char), 512 * 512, outfile2);
			fclose(outfile2);
			printf("\t Boat Inverse is created.\n");
		}

		for (i = 0; i<N; i++)
		{
			for (j = 0; j<N; j++)
			{
				cal += ((InImg[i][j] - OutImg_INVERSE[i][j])*(InImg[i][j] - OutImg_INVERSE[i][j]));
			}
		}
		MSE = cal / (N*N);
		if (x == 1){ printf("\n\t MSE of Lena is %lf \n\n", MSE); }
		else if (x == 2){ printf("\n\t MSE of Boat is %lf \n\n", MSE); }

	} while (1);

}


/*------------
8x8 block DCT
------------*/
void dct8x8(int ix[B_size][B_size])
{
	static double pi = 3.141592653589793238;
	float x[B_size][B_size], z[B_size][B_size], y[B_size], c[40], s[40],
		ft[4], fxy[4], yy[B_size], zz;
	int i, j, ii, jj;

	for (i = 0; i<40; i++) {
		zz = pi * (float)(i + 1) / 64.0;
		c[i] = cos(zz);
		s[i] = sin(zz);
	}

	for (ii = 0; ii<B_size; ii++)
		for (jj = 0; jj<B_size; jj++)
			x[ii][jj] = (float)ix[ii][jj];

	for (ii = 0; ii<B_size; ii++) {
		for (jj = 0; jj<B_size; jj++)
			y[jj] = x[ii][jj];

		for (jj = 0; jj<4; jj++)
			ft[jj] = y[jj] + y[7 - jj];

		fxy[0] = ft[0] + ft[3];
		fxy[1] = ft[1] + ft[2];
		fxy[2] = ft[1] - ft[2];
		fxy[3] = ft[0] - ft[3];

		ft[0] = c[15] * (fxy[0] + fxy[1]);
		ft[2] = c[15] * (fxy[0] - fxy[1]);
		ft[1] = s[7] * fxy[2] + c[7] * fxy[3];
		ft[3] = -s[23] * fxy[2] + c[23] * fxy[3];

		for (jj = 4; jj<8; jj++)
			yy[jj] = y[7 - jj] - y[jj];

		y[4] = yy[4];
		y[7] = yy[7];
		y[5] = c[15] * (-yy[5] + yy[6]);
		y[6] = c[15] * (yy[5] + yy[6]);

		yy[4] = y[4] + y[5];
		yy[5] = y[4] - y[5];
		yy[6] = -y[6] + y[7];
		yy[7] = y[6] + y[7];

		y[0] = ft[0];
		y[4] = ft[2];
		y[2] = ft[1];
		y[6] = ft[3];
		y[1] = s[3] * yy[4] + c[3] * yy[7];
		y[5] = s[19] * yy[5] + c[19] * yy[6];
		y[3] = -s[11] * yy[5] + c[11] * yy[6];
		y[7] = -s[27] * yy[4] + c[27] * yy[7];

		for (jj = 0; jj<B_size; jj++)
			z[ii][jj] = y[jj];
	}


	for (ii = 0; ii<B_size; ii++) {
		for (jj = 0; jj<B_size; jj++)
			y[jj] = z[jj][ii];

		for (jj = 0; jj<4; jj++)
			ft[jj] = y[jj] + y[7 - jj];

		fxy[0] = ft[0] + ft[3];
		fxy[1] = ft[1] + ft[2];
		fxy[2] = ft[1] - ft[2];
		fxy[3] = ft[0] - ft[3];

		ft[0] = c[15] * (fxy[0] + fxy[1]);
		ft[2] = c[15] * (fxy[0] - fxy[1]);
		ft[1] = s[7] * fxy[2] + c[7] * fxy[3];
		ft[3] = -s[23] * fxy[2] + c[23] * fxy[3];

		for (jj = 4; jj<8; jj++)
			yy[jj] = y[7 - jj] - y[jj];

		y[4] = yy[4];
		y[7] = yy[7];
		y[5] = c[15] * (-yy[5] + yy[6]);
		y[6] = c[15] * (yy[5] + yy[6]);

		yy[4] = y[4] + y[5];
		yy[5] = y[4] - y[5];
		yy[6] = -y[6] + y[7];
		yy[7] = y[6] + y[7];

		y[0] = ft[0];
		y[4] = ft[2];
		y[2] = ft[1];
		y[6] = ft[3];
		y[1] = s[3] * yy[4] + c[3] * yy[7];
		y[5] = s[19] * yy[5] + c[19] * yy[6];
		y[3] = -s[11] * yy[5] + c[11] * yy[6];
		y[7] = -s[27] * yy[4] + c[27] * yy[7];

		for (jj = 0; jj<B_size; jj++)
			y[jj] = y[jj] / 4.0;

		for (jj = 0; jj<B_size; jj++)
			z[jj][ii] = y[jj];
	}


	for (ii = 0; ii<B_size; ii++)
		for (jj = 0; jj<B_size; jj++)
			ix[ii][jj] = nint(z[ii][jj]);


}

/*--------------------
8x8 block inverse DCT
--------------------*/
void idct8x8(int ix[B_size][B_size])
{
	static double pi = 3.141592653589793238;
	float x[B_size][B_size], z[B_size][B_size], y[B_size], c[40], s[40],
		ait[4], aixy[4], yy[B_size], zz;
	int i, ii, jj;

	for (i = 0; i<40; i++) {
		zz = pi * (float)(i + 1) / 64.0;
		c[i] = cos(zz);
		s[i] = sin(zz);
	}

	for (ii = 0; ii<B_size; ii++)
		for (jj = 0; jj<B_size; jj++)
			x[ii][jj] = (float)ix[ii][jj];

	for (ii = 0; ii<B_size; ii++) {
		for (jj = 0; jj<B_size; jj++)
			y[jj] = x[jj][ii];

		ait[0] = y[0];
		ait[1] = y[2];
		ait[2] = y[4];
		ait[3] = y[6];

		aixy[0] = c[15] * (ait[0] + ait[2]);
		aixy[1] = c[15] * (ait[0] - ait[2]);
		aixy[2] = s[7] * ait[1] - s[23] * ait[3];
		aixy[3] = c[7] * ait[1] + c[23] * ait[3];

		ait[0] = aixy[0] + aixy[3];
		ait[1] = aixy[1] + aixy[2];
		ait[2] = aixy[1] - aixy[2];
		ait[3] = aixy[0] - aixy[3];

		yy[4] = s[3] * y[1] - s[27] * y[7];
		yy[5] = s[19] * y[5] - s[11] * y[3];
		yy[6] = c[19] * y[5] + c[11] * y[3];
		yy[7] = c[3] * y[1] + c[27] * y[7];

		y[4] = yy[4] + yy[5];
		y[5] = yy[4] - yy[5];
		y[6] = -yy[6] + yy[7];
		y[7] = yy[6] + yy[7];

		yy[4] = y[4];
		yy[7] = y[7];
		yy[5] = c[15] * (-y[5] + y[6]);
		yy[6] = c[15] * (y[5] + y[6]);

		for (jj = 0; jj<4; jj++)
			y[jj] = ait[jj] + yy[7 - jj];

		for (jj = 4; jj<8; jj++)
			y[jj] = ait[7 - jj] - yy[jj];

		for (jj = 0; jj<B_size; jj++)
			z[jj][ii] = y[jj];

	}

	for (ii = 0; ii<B_size; ii++) {
		for (jj = 0; jj<B_size; jj++)
			y[jj] = z[ii][jj];

		ait[0] = y[0];
		ait[1] = y[2];
		ait[2] = y[4];
		ait[3] = y[6];

		aixy[0] = c[15] * (ait[0] + ait[2]);
		aixy[1] = c[15] * (ait[0] - ait[2]);
		aixy[2] = s[7] * ait[1] - s[23] * ait[3];
		aixy[3] = c[7] * ait[1] + c[23] * ait[3];

		ait[0] = aixy[0] + aixy[3];
		ait[1] = aixy[1] + aixy[2];
		ait[2] = aixy[1] - aixy[2];
		ait[3] = aixy[0] - aixy[3];

		yy[4] = s[3] * y[1] - s[27] * y[7];
		yy[5] = s[19] * y[5] - s[11] * y[3];
		yy[6] = c[19] * y[5] + c[11] * y[3];
		yy[7] = c[3] * y[1] + c[27] * y[7];

		y[4] = yy[4] + yy[5];
		y[5] = yy[4] - yy[5];
		y[6] = -yy[6] + yy[7];
		y[7] = yy[6] + yy[7];

		yy[4] = y[4];
		yy[7] = y[7];
		yy[5] = c[15] * (-y[5] + y[6]);
		yy[6] = c[15] * (y[5] + y[6]);

		for (jj = 0; jj<4; jj++)
			y[jj] = ait[jj] + yy[7 - jj];

		for (jj = 4; jj<8; jj++)
			y[jj] = ait[7 - jj] - yy[jj];

		for (jj = 0; jj<B_size; jj++)
			z[ii][jj] = y[jj] / 4.0;

	}

	for (ii = 0; ii<B_size; ii++)
		for (jj = 0; jj<B_size; jj++)
			ix[ii][jj] = nint(z[ii][jj]);
}
