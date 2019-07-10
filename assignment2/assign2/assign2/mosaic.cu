#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>
#include <string.h>
#include "cuda_runtime.h"
#include "device_launch_parameters.h"

struct EveryPixel {
	unsigned char r, g, b;
} ;

struct WholeImage {
	int x, y;
	EveryPixel *d;
} ;

#define FAILURE 0
#define SUCCESS !FAILURE

#define USER_NAME "acp18gc"		//replace with your user name

void print_help();
char *filename, *modename, *outname, *option;

int process_command_line(int argc, char *argv[]);

typedef enum MODE { CPU, OPENMP, CUDA, ALL } MODE;

unsigned int c;
MODE execution_mode = CUDA;

WholeImage *image;
char buffer[16];

int readFile();

void checkCUDAError(const char *msg)
{
	cudaError_t err = cudaGetLastError();
	if (cudaSuccess != err)
	{
		fprintf(stderr, "CUDA ERROR: %s: %s.\n", msg, cudaGetErrorString(err));
		exit(EXIT_FAILURE);
	}
}

#define gpuErrchk(ans) { gpuAssert((ans), __FILE__, __LINE__); }
inline void gpuAssert(cudaError_t code, const char *file, int line, bool abort = true)
{
	if (code != cudaSuccess)
	{
		fprintf(stderr, "GPUassert: %s %s %d\n", cudaGetErrorString(code), file, line);
		if (abort) exit(code);
	}
}

//writeFile function
void writeFile(const char *filename, WholeImage *img, int target, const char *option) {
	int tp = 6;

	if (option == NULL)
	{
		tp = 6;
	}
	else if (!strcmp(option, "PPM_BINARY")) {
		tp = 6;
	}
	else {
		tp = 3;
	}
	FILE *fp;
	fp = fopen(filename, "wb");
	fprintf(fp, "P%d\n", tp);
	fprintf(fp, "# Created by %s\n", USER_NAME);
	fprintf(fp, "%d %d\n", img->x, img->y);
	fprintf(fp, "%d\n", 255);
	int i;
	if (tp == 3) {
		for (i = 0; i < (img->x*img->y); i++) {
			fprintf(fp, "%d %d %d   ", img->d[i].r, img->d[i].g, img->d[i].b);
			if ((i + 1) % img->x == 0) {
				fprintf(fp, "\n");
			}
		}
	}
	else {

		for (i = 0; i < (img->x*img->y); i++) {
			fwrite(&img->d[i].r, sizeof(unsigned char), 1, fp);
			fwrite(&img->d[i].g, sizeof(unsigned char), 1, fp);
			fwrite(&img->d[i].b, sizeof(unsigned char), 1, fp);
		}
	}

	

	fclose(fp);
}
void runCPU(int *ModeAllred, int *ModeAllgreen, int *ModeAllblue) {
	unsigned short row_quot = 0;
	unsigned short row_rema = 0;
	unsigned short col_quot = 0;
	unsigned short col_rema = 0;
	unsigned short row_limit = 0;
	unsigned short col_limit = 0;
	unsigned short cell_row_limit = 0;
	unsigned short cell_col_limit = 0;
	unsigned long sumr = 0;
	unsigned long sumg = 0;
	unsigned long sumb = 0;
	unsigned long allsumr = 0, allsumg = 0, allsumb = 0;
	signed short y_in_cell, y_in_col, x_in_row, x_in_cell;


	int aveRed = 0, aveGreen = 0, aveBlue = 0, allaver = 0, allaveg = 0, allaveb = 0;

	int i;

	row_quot = image->x / c;
	row_rema = image->y % c;
	col_quot = image->x / c;
	col_rema = image->y % c;

	int cells_per_row;
	int cells_per_col;

	if (row_rema)
		cells_per_row = row_quot + 1;
	else
		cells_per_row = row_quot;

	if (col_rema)
		cells_per_col = col_quot + 1;
	else
		cells_per_col = col_quot;


	for (y_in_col = 0; y_in_col < cells_per_col; y_in_col++)
	{
		if (y_in_col == col_quot)
			col_limit = col_rema;
		else
			col_limit = c;
		for (x_in_row = 0; x_in_row < cells_per_row; x_in_row++)
		{
			if (x_in_row == row_quot)
				row_limit = row_rema;
			else
				row_limit = c;

			sumr = 0;
			sumg = 0;
			sumb = 0;
			for (y_in_cell = 0; y_in_cell < col_limit; y_in_cell++)
			{
				for (x_in_cell = 0; x_in_cell < row_limit; x_in_cell++)
				{
					int temp;
					temp = y_in_col * (c*image->x) + y_in_cell * image->x + (x_in_row*c + x_in_cell);
					sumr += image->d[temp].r;
					sumg += image->d[temp].g;
					sumb += image->d[temp].b;
				}
			}

			for (y_in_cell = 0; y_in_cell < col_limit; y_in_cell++)
			{
				for (x_in_cell = 0; x_in_cell < row_limit; x_in_cell++)
				{
					int temp;
					temp = y_in_col * (c*image->x) + y_in_cell * image->x + (x_in_row*c + x_in_cell);
					image->d[temp].r = (int)sumr / (col_limit*row_limit);
					image->d[temp].g = (int)sumg / (col_limit*row_limit);
					image->d[temp].b = (int)sumb / (col_limit*row_limit);
				}
			}
			allsumr += sumr / (col_limit*row_limit);
			allsumg += sumg / (col_limit*row_limit);
			allsumb += sumb / (col_limit*row_limit);
		}
	}

	allaver = allsumr / (cells_per_row*cells_per_col);
	allaveg = allsumg / (cells_per_row*cells_per_col);
	allaveb = allsumb / (cells_per_row*cells_per_col);

	*ModeAllred = allaver;
	*ModeAllgreen = allaveg;
	*ModeAllblue = allaveb;

}

void runOPENMP(int *ModeAllred, int *ModeAllgreen, int *ModeAllblue) {
	unsigned short row_quot = 0;
	unsigned short row_rema = 0;
	unsigned short col_quot = 0;
	unsigned short col_rema = 0;
	unsigned short row_limit = 0;
	unsigned short col_limit = 0;
	unsigned short cell_row_limit = 0;
	unsigned short cell_col_limit = 0;
	unsigned long sumr = 0;
	unsigned long sumg = 0;
	unsigned long sumb = 0;
	unsigned long allsumr = 0, allsumg = 0, allsumb = 0;
	signed short y_in_cell, y_in_col, x_in_row, x_in_cell;


	int aveRed = 0, aveGreen = 0, aveBlue = 0, allaver = 0, allaveg = 0, allaveb = 0;

	int i;

	row_quot = image->x / c;
	row_rema = image->y % c;
	col_quot = image->x / c;
	col_rema = image->y % c;

	int cells_per_row;
	int cells_per_col;

	if (row_rema)
		cells_per_row = row_quot + 1;
	else
		cells_per_row = row_quot;

	if (col_rema)
		cells_per_col = col_quot + 1;
	else
		cells_per_col = col_quot;


#pragma omp parallel private(y_in_cell, y_in_col, x_in_row, x_in_cell, sumr, sumg, sumb) reduction(+: allsumr, allsumg, allsumb)
	{
#pragma omp for
		for (y_in_col = 0; y_in_col < cells_per_col; y_in_col++)
		{
			if (y_in_col == col_quot)
				col_limit = col_rema;
			else
				col_limit = c;

			for (x_in_row = 0; x_in_row < cells_per_row; x_in_row++)
			{
				if (x_in_row == row_quot)
					row_limit = row_rema;
				else
					row_limit = c;

				sumr = 0;
				sumg = 0;
				sumb = 0;

				for (y_in_cell = 0; y_in_cell < col_limit; y_in_cell++)
				{
					//#pragma omp parallel for reduction(+: sumr,sumg,sumb)
					for (x_in_cell = 0; x_in_cell < row_limit; x_in_cell++)
					{
						int temp;
						temp = y_in_col * (c*image->x) + y_in_cell * image->x + (x_in_row*c + x_in_cell);
						//#pragma omp critical
												//{
#pragma omp atomic 
						sumr += image->d[temp].r;
#pragma omp atomic 
						sumg += image->d[temp].g;
#pragma omp atomic 
						sumb += image->d[temp].b;
						//}
					}
				}

				for (y_in_cell = 0; y_in_cell < col_limit; y_in_cell++)
				{
					//#pragma omp parallel for
					for (x_in_cell = 0; x_in_cell < row_limit; x_in_cell++)
					{
						int temp;
						temp = y_in_col * (c*image->x) + y_in_cell * image->x + (x_in_row*c + x_in_cell);
						//#pragma omp critical
						//						{

						image->d[temp].r = (int)sumr / (col_limit*row_limit);

						image->d[temp].g = (int)sumg / (col_limit*row_limit);

						image->d[temp].b = (int)sumb / (col_limit*row_limit);
						//}
					}
				}
				//#pragma omp atomic 
				allsumr += sumr / (col_limit*row_limit);
				allsumg += sumg / (col_limit*row_limit);
				allsumb += sumb / (col_limit*row_limit);
			}
		}
	}
	//calculate the whole image average value
	allaver = allsumr / (cells_per_row*cells_per_col);
	allaveg = allsumg / (cells_per_row*cells_per_col);
	allaveb = allsumb / (cells_per_row*cells_per_col);

	*ModeAllred = allaver;
	*ModeAllgreen = allaveg;
	*ModeAllblue = allaveb;
}

__device__ unsigned long long int d_sumr, d_sumg, d_sumb;

__device__ void cal_cell_sum(EveryPixel *input,EveryPixel *output, int i, int j,unsigned int *dc, unsigned int *width) {

	int temp;
	unsigned long sumr = 0;
	unsigned long sumg = 0;
	unsigned long sumb = 0;

	unsigned long allsumr = 0;

	unsigned long aver = 0;
	unsigned long aveg = 0;
	unsigned long aveb = 0;

	for (int k = 0; k < *dc; k++) {
		for (int l = 0; l < *dc; l++) {
			temp = j * *dc * *width + k * *width + (i* *dc + l);
			sumr += input[temp].r;
			sumg += input[temp].g;
			sumb += input[temp].b;


		}
	}
	aver = sumr / (*dc* *dc);
	aveg = sumg / (*dc* *dc);
	aveb = sumb / (*dc* *dc);
	//d_sumr[j*(*width / *dc) +i] = aver;
	//d_sumg[j*(*width / *dc) + i] = aveg;
	//d_sumb[j*(*width / *dc) + i] = aveb;


	for (int k = 0; k < *dc; k++) {
		for (int l = 0; l < *dc; l++) {
			temp = j * *dc * *width + k * *width + (i* *dc + l);
			output[temp].r = aver;
			output[temp].g = aveg;
			output[temp].b = aveb;
		}
	}
	__syncthreads();

	atomicAdd(&d_sumr, sumr);
	atomicAdd(&d_sumg, sumg);
	atomicAdd(&d_sumb, sumb);

}

__global__ void runCuda(EveryPixel *input, EveryPixel *output, unsigned int *dc, unsigned int *width) {
	

	cal_cell_sum( input, output, blockIdx.x , threadIdx.x, dc, width);
	

}

void runGPU(int *ModeAllred, int *ModeAllgreen, int *ModeAllblue) {
	unsigned short row_quot = 0;
	unsigned short row_rema = 0;
	unsigned short col_quot = 0;
	unsigned short col_rema = 0;
	unsigned short row_limit = 0;
	unsigned short col_limit = 0;
	unsigned short cell_row_limit = 0;
	unsigned short cell_col_limit = 0;
	unsigned long *sumr;
	unsigned long *sumg;
	unsigned long *sumb;
	unsigned long allsumr = 0, allsumg = 0, allsumb = 0;
	signed short y_in_cell, y_in_col, x_in_row, x_in_cell;
	signed short *dy_in_cell, *dy_in_col, *dx_in_row, *dx_in_cell;
	//unsigned long *d_sumr;
	//unsigned long *d_sumg;
	//unsigned long *d_sumb;

	unsigned long long int sumr2 = 0;
	unsigned long long int sumg2 = 0;
	unsigned long long int sumb2 = 0;

	EveryPixel *d_image, *d_image_out;
	unsigned int *dc, *length, *width;




	int aveRed = 0, aveGreen = 0, aveBlue = 0, allaver = 0, allaveg = 0, allaveb = 0;

	int i;

	row_quot = image->x / c;
	row_rema = image->y % c;
	col_quot = image->x / c;
	col_rema = image->y % c;

	int cells_per_row;
	int cells_per_col;

	if (row_rema)
		cells_per_row = row_quot + 1;
	else
		cells_per_row = row_quot;

	if (col_rema)
		cells_per_col = col_quot + 1;
	else
		cells_per_col = col_quot;

	float time;
	cudaEvent_t dstart, stop;
	cudaEventCreate(&dstart);
	cudaEventCreate(&stop);


	sumr = (unsigned long *)malloc(sizeof(unsigned long *)*cells_per_col*cells_per_row);
	sumg = (unsigned long *)malloc(sizeof(unsigned long *)*cells_per_col*cells_per_row);
	sumb = (unsigned long *)malloc(sizeof(unsigned long *)*cells_per_col*cells_per_row);

	cudaMalloc((void **)&dc, sizeof(unsigned int));
	cudaMalloc((void **)&length, sizeof(unsigned int));
	cudaMalloc((void **)&width, sizeof(unsigned int));

	cudaMalloc((void **)&d_image, image->x*image->y * sizeof(EveryPixel));
	cudaMalloc((void **)&d_image_out, image->x*image->y * sizeof(EveryPixel));
	cudaMalloc((void **)&dy_in_cell, sizeof(signed short));

	cudaMalloc((void **)&dx_in_cell, sizeof(signed short));

	//cudaMalloc((void **)&d_sumr, sizeof(unsigned long*)*cells_per_col*cells_per_row);
	//cudaMalloc((void **)&d_sumg, sizeof(unsigned long*)*cells_per_col*cells_per_row);
	//cudaMalloc((void **)&d_sumb, sizeof(unsigned long*)*cells_per_col*cells_per_row);





	checkCUDAError("Memory allocation");

	gpuErrchk(cudaMemcpyToSymbol(d_sumr, &sumr2, sizeof(unsigned long long int)));
	gpuErrchk(cudaMemcpyToSymbol(d_sumg, &sumg2, sizeof(unsigned long long int)));
	cudaMemcpyToSymbol(d_sumb, &sumb2, sizeof(unsigned long long int));

	cudaMemcpy(dc, &c, sizeof(int), cudaMemcpyHostToDevice);
	cudaMemcpy(length, &image->y, sizeof(int), cudaMemcpyHostToDevice);
	cudaMemcpy(width, &image->x, sizeof(int), cudaMemcpyHostToDevice);

	cudaMemcpy(dy_in_cell, &y_in_cell, 1, cudaMemcpyHostToDevice);
	cudaMemcpy(dx_in_cell, &x_in_cell, 1, cudaMemcpyHostToDevice);
	cudaMemcpy(d_image, image->d, image->x*image->y * sizeof(EveryPixel), cudaMemcpyHostToDevice);



	checkCUDAError("Input transfer to device");



	dim3 blocksPerGrid(cells_per_col, 1, 1);
	dim3 threadsPerBlock(cells_per_row, 1, 1);

	cudaEventRecord(dstart, 0);

	runCuda << <blocksPerGrid, threadsPerBlock >> > (d_image, d_image_out, dc, width);

	cudaEventRecord(stop, 0);
	cudaEventSynchronize(dstart);
	cudaEventSynchronize(stop);
	cudaEventElapsedTime(&time, dstart, stop);
	printf("CUDA mode execution time took:\t%f(ms)\n", time);
	cudaEventDestroy(dstart);
	cudaEventDestroy(stop);

	cudaThreadSynchronize();
	checkCUDAError("Kernel execution");

	cudaMemcpy(image->d, d_image_out, image->x*image->y * sizeof(EveryPixel), cudaMemcpyDeviceToHost);
	//cudaMemcpy(sumr, d_sumr, sizeof(unsigned long *)*cells_per_col*cells_per_row, cudaMemcpyDeviceToHost);
	//cudaMemcpy(sumg, d_sumg, sizeof(unsigned long *)*cells_per_col*cells_per_row, cudaMemcpyDeviceToHost);
	//cudaMemcpy(sumb, d_sumb, sizeof(unsigned long *)*cells_per_col*cells_per_row, cudaMemcpyDeviceToHost);

	cudaMemcpyFromSymbol(&sumr2, d_sumr, sizeof(unsigned long long int));
	cudaMemcpyFromSymbol(&sumg2, d_sumg, sizeof(unsigned long long int));
	cudaMemcpyFromSymbol(&sumb2, d_sumb, sizeof(unsigned long long int));

	long long int showaver = 0, showaveg = 0, showaveb = 0;
	//for (int i = 0; i < cells_per_col*cells_per_row; i++) {
	//	showaver += sumr[i];
	//	showaveg += sumg[i];
	//	showaveb += sumb[i];
	//}
	showaver = sumr2;
	showaveg = sumg2;
	showaveb = sumb2;

	allaver = showaver / (image->x*image->y);
	allaveg = showaveg / (image->x*image->y);
	allaveb = showaveb / (image->x*image->y);
	*ModeAllred = allaver;
	*ModeAllgreen = allaveg;
	*ModeAllblue = allaveb;

	cudaFree(d_image);
	cudaFree(d_image_out);
	//cudaFree(d_sumr);
	//cudaFree(d_sumg);
	//cudaFree(d_sumb);
	cudaFree(dc);
	cudaFree(length);
	cudaFree(width);
	cudaFree(dy_in_cell);
	cudaFree(dx_in_cell);
	checkCUDAError("Free memory");
}
int main(int argc, char *argv[]) {
	int ModeAllRed = 0;
	int ModeAllGreen = 0;
	int ModeAllBlue = 0;

	if (process_command_line(argc, argv) == FAILURE)
		return 1;
	//if c is the power of 2.
	if (!((c > 0) && ((c & (c - 1)) == 0)))
	{
		printf("Entry power of 2 number");
		return 1;
	}

	if (!strcmp(modename, "CPU")) {
		execution_mode = CPU;
	}
	else
	{
		if (!strcmp(modename, "OPENMP")) {
			execution_mode = OPENMP;
		}
		else {
			if (!strcmp(modename, "CUDA")) {
				execution_mode = CUDA;
			}
			else {
				if (!strcmp(modename, "ALL")) {
					execution_mode = ALL;
				}
				else {
					printf("Enter a right mode");
					exit(1);
				}
			}
		}
	}

	//TODO: read input image file (either binary or plain text PPM) 

	image = (WholeImage *)malloc(sizeof(WholeImage));
	readFile();

	//TODO: execute the mosaic filter based on the mode
	switch (execution_mode) {
	case (CPU): {

		//TODO: starting timing here
		clock_t start, end;
		start = clock();
		double total;
		
		unsigned short row_quot = 0;
		unsigned short row_rema = 0;
		unsigned short col_quot = 0;
		unsigned short col_rema = 0;
		unsigned short row_limit = 0;
		unsigned short col_limit = 0;
		unsigned short cell_row_limit = 0;
		unsigned short cell_col_limit = 0;
		unsigned long sumr = 0;
		unsigned long sumg = 0;
		unsigned long sumb = 0;
		unsigned long allsumr = 0, allsumg = 0, allsumb = 0;
		signed short y_in_cell, y_in_col, x_in_row, x_in_cell;

	
		int aveRed = 0, aveGreen = 0, aveBlue = 0, allaver = 0, allaveg = 0, allaveb = 0;

		int i;

		row_quot = image->x / c;
		row_rema = image->y % c;
		col_quot = image->x / c;
		col_rema = image->y % c;

		int cells_per_row;
		int cells_per_col;

		if (row_rema)
			cells_per_row = row_quot + 1;
		else
			cells_per_row = row_quot;

		if (col_rema)
			cells_per_col = col_quot + 1;
		else
			cells_per_col = col_quot;
		
		
		for ( y_in_col = 0; y_in_col < cells_per_col; y_in_col++)
		{
			if (y_in_col == col_quot)
				col_limit = col_rema;
			else
				col_limit = c;
			for (x_in_row = 0; x_in_row < cells_per_row; x_in_row++)
			{
				if (x_in_row == row_quot)
					row_limit = row_rema;
				else
					row_limit = c;

				sumr = 0;
				sumg = 0;
				sumb = 0;
				for (y_in_cell = 0; y_in_cell < col_limit; y_in_cell++)
				{
					for ( x_in_cell = 0; x_in_cell < row_limit; x_in_cell++)
					{
						int temp;
						temp = y_in_col * (c*image->x) + y_in_cell*image->x + (x_in_row*c + x_in_cell);
						sumr += image->d[temp].r;
						sumg += image->d[temp].g;
						sumb += image->d[temp].b;
					}
				}

				for (y_in_cell = 0; y_in_cell < col_limit; y_in_cell++)
				{
					for (x_in_cell = 0; x_in_cell < row_limit; x_in_cell++)
					{
						int temp;
						temp = y_in_col * (c*image->x) + y_in_cell * image->x + (x_in_row*c + x_in_cell);
						image->d[temp].r = (int)sumr / (col_limit*row_limit);
						image->d[temp].g = (int)sumg / (col_limit*row_limit);
						image->d[temp].b = (int)sumb / (col_limit*row_limit);
					}
				}
				allsumr += sumr / (col_limit*row_limit);
				allsumg += sumg / (col_limit*row_limit);
				allsumb += sumb / (col_limit*row_limit);
			}
		}

		allaver = allsumr / (cells_per_row*cells_per_col);
		allaveg = allsumg / (cells_per_row*cells_per_col);
		allaveb = allsumb / (cells_per_row*cells_per_col);
		printf("CPU Average image colour red = %d, green = %d, blue = %d \n", allaver, allaveg, allaveb);

		//TODO: end timing here
		end = clock();
		total = (double)(end - start) / CLOCKS_PER_SEC;
		int a = total;
		double ms = (double)(total - a) * 1000;
		printf("CPU mode execution time took %d s and %f ms\n", a, ms);
		break;
	}
	case (OPENMP): {
		//TODO: starting timing here
		double start = omp_get_wtime();
		double total;

		unsigned short row_quot = 0;
		unsigned short row_rema = 0;
		unsigned short col_quot = 0;
		unsigned short col_rema = 0;
		unsigned short row_limit = 0;
		unsigned short col_limit = 0;
		unsigned short cell_row_limit = 0;
		unsigned short cell_col_limit = 0;
		unsigned long sumr = 0;
		unsigned long sumg = 0;
		unsigned long sumb = 0;
		unsigned long allsumr = 0, allsumg = 0, allsumb = 0;
		signed short y_in_cell, y_in_col, x_in_row, x_in_cell;


		int aveRed = 0, aveGreen = 0, aveBlue = 0, allaver = 0, allaveg = 0, allaveb = 0;

		int i;

		row_quot = image->x / c;
		row_rema = image->y % c;
		col_quot = image->x / c;
		col_rema = image->y % c;

		int cells_per_row;
		int cells_per_col;

		if (row_rema)
		cells_per_row = row_quot + 1;
		else
		cells_per_row = row_quot;

		if (col_rema)
		cells_per_col = col_quot + 1;
		else
		cells_per_col = col_quot;


#pragma omp parallel private(y_in_cell, y_in_col, x_in_row, x_in_cell, sumr, sumg, sumb) reduction(+: allsumr, allsumg, allsumb)
		{
#pragma omp for
			for (y_in_col = 0; y_in_col < cells_per_col; y_in_col++)
			{
				if (y_in_col == col_quot)
					col_limit = col_rema;
				else
					col_limit = c;

				for (x_in_row = 0; x_in_row < cells_per_row; x_in_row++)
				{
					if (x_in_row == row_quot)
						row_limit = row_rema;
					else
						row_limit = c;

					sumr = 0;
					sumg = 0;
					sumb = 0;

					for (y_in_cell = 0; y_in_cell < col_limit; y_in_cell++)
					{
						//#pragma omp parallel for reduction(+: sumr,sumg,sumb)
						for (x_in_cell = 0; x_in_cell < row_limit; x_in_cell++)
						{
							int temp;
							temp = y_in_col * (c*image->x) + y_in_cell * image->x + (x_in_row*c + x_in_cell);
							//#pragma omp critical
													//{
#pragma omp atomic 
							sumr += image->d[temp].r;
#pragma omp atomic 
							sumg += image->d[temp].g;
#pragma omp atomic 
							sumb += image->d[temp].b;
							//}
						}
					}

					for (y_in_cell = 0; y_in_cell < col_limit; y_in_cell++)
					{
						//#pragma omp parallel for
						for (x_in_cell = 0; x_in_cell < row_limit; x_in_cell++)
						{
							int temp;
							temp = y_in_col * (c*image->x) + y_in_cell * image->x + (x_in_row*c + x_in_cell);
							//#pragma omp critical
							//						{

							image->d[temp].r = (int)sumr / (col_limit*row_limit);

							image->d[temp].g = (int)sumg / (col_limit*row_limit);

							image->d[temp].b = (int)sumb / (col_limit*row_limit);
							//}
						}
					}
//#pragma omp atomic 
					allsumr += sumr / (col_limit*row_limit);
					allsumg += sumg / (col_limit*row_limit);
					allsumb += sumb / (col_limit*row_limit);
				}
			}
		}
		//calculate the whole image average value
		allaver = allsumr / (cells_per_row*cells_per_col);
		allaveg = allsumg / (cells_per_row*cells_per_col);
		allaveb = allsumb / (cells_per_row*cells_per_col);


//		// Output the average colour value for the image
		printf("OPENMP Average image colour red = %d, green = %d, blue = %d \n", allaver, allaveg, allaveb);
//
//		//TODO: end timing here
		double end = omp_get_wtime();
		total = end - start;
		int a = total;
		double ms = (double)(total - a) * 1000;
		printf("OPENMP mode execution time took %d s and %f ms\n", a, ms);
		break;
	}
	case (CUDA): {

		runGPU(&ModeAllRed, &ModeAllGreen, &ModeAllBlue);
		printf("CUDA Average image colour red = %d, green = %d, blue = %d \n", ModeAllRed, ModeAllGreen, ModeAllBlue);
		break;
	}
	case (ALL): {

		clock_t start, end;
		start = clock();
		double total;

		runCPU(&ModeAllRed, &ModeAllGreen, &ModeAllBlue);
		printf("CPU Average image colour red = %d, green = %d, blue = %d \n", ModeAllRed, ModeAllGreen, ModeAllBlue);

		end = clock();
		total = (double)(end - start) / CLOCKS_PER_SEC;
		int a = total;
		double ms = (double)(total - a) * 1000;
		printf("CPU mode execution time took %d s and %f ms\n", a, ms);

		readFile();
		double start2 = omp_get_wtime();
		double total2;

		runOPENMP(&ModeAllRed, &ModeAllGreen, &ModeAllBlue);
		printf("OPENMP Average image colour red = %d, green = %d, blue = %d \n", ModeAllRed, ModeAllGreen, ModeAllBlue);

		double end2 = omp_get_wtime();
		total2 = end2 - start2;
		int a2 = total2;
		double ms2 = (double)(total2 - a2) * 1000;
		printf("OPENMP mode execution time took %d s and %f ms\n", a2, ms2);

		readFile();


		
		runGPU(&ModeAllRed, &ModeAllGreen, &ModeAllBlue);
		printf("CUDA Average image colour red = %d, green = %d, blue = %d \n", ModeAllRed, ModeAllGreen, ModeAllBlue);
		/*cudaFree(d_image);
		cudaFree(d_image_out);
		cudaFree(d_sumr);
		cudaFree(d_sumg);
		cudaFree(d_sumb);
		cudaFree(dc);
		cudaFree(length);
		cudaFree(width);
		cudaFree(dy_in_cell);
		cudaFree(dx_in_cell);
		checkCUDAError("Free memory");*/

		break;
	}
	}

	writeFile(outname, image, c, option);
	free(image);
	//save the output image file (from last executed mode) 
	return 0;
}

void print_help() {
	printf("mosaic_%s C M -i input_file -o output_file [options]\n", USER_NAME);

	printf("where:\n");
	printf("\tC              Is the mosaic cell size which should be any positive\n"
		"\t               power of 2 number \n");
	printf("\tM              Is the mode with a value of either CPU, OPENMP, CUDA or\n"
		"\t               ALL. The mode specifies which version of the simulation\n"
		"\t               code should execute. ALL should execute each mode in\n"
		"\t               turn.\n");
	printf("\t-i input_file  Specifies an input image file\n");
	printf("\t-o output_file Specifies an output image file which will be used\n"
		"\t               to write the mosaic image\n");
	printf("[options]:\n");
	printf("\t-f ppm_format  PPM image output format either PPM_BINARY (default) or \n"
		"\t               PPM_PLAIN_TEXT\n ");
}

int process_command_line(int argc, char *argv[]) {
	if (argc < 7) {
		fprintf(stderr, "Error: Missing program arguments. Correct usage is...\n");
		print_help();
		return FAILURE;
	}

	//first argument is always the executable name

	//read in the non optional command line arguments
	c = (unsigned int)atoi(argv[1]);

	//TODO: read in the mode
	modename = argv[2];
	//TODO: read in the input image name
	filename = argv[4];
	//TODO: read in the output image name
	outname = argv[6];
	//TODO: read in any optional part 3 arguments
	if (argc == 7) {
		option = NULL;
	}
	else
		option = argv[7];
	
	return SUCCESS;
}

int readFile() {
	FILE * f = fopen(filename, "rb");
	if (f == NULL)
	{
		printf("cannot open");
		return 1;
	}


	image = (WholeImage *)malloc(sizeof(WholeImage));
	if (!fgets(buffer, sizeof(buffer), f)) {
		printf("error");
		exit(1);
	}
	char type = buffer[1];
	int comments, rgbcolor;
	switch (type)
	{
	case('3'): {
		comments = getc(f);
		while (comments == '#')
		{
			while (getc(f) != '\n');
			comments = getc(f);
		}
		ungetc(comments, f);
		fscanf(f, "%d %d", &image->x, &image->y);
		fscanf(f, "%d", &rgbcolor);
		while (fgetc(f) != '\n');
		image->d = (EveryPixel *)malloc(image->x*image->y * sizeof(EveryPixel));
		for (int i = 0; i < (image->x*image->y); i++) {
			fscanf(f, "%d%d%d", &image->d[i].r, &image->d[i].g, &image->d[i].b);
		}
		fclose(f);
		break;
	}
	case('6'): {
		comments = getc(f);
		while (comments == '#')
		{
			while (getc(f) != '\n');
			comments = getc(f);
		}
		ungetc(comments, f);
		fscanf(f, "%d %d", &image->x, &image->y);
		fscanf(f, "%d", &rgbcolor);
		while (fgetc(f) != '\n');
		image->d = (EveryPixel *)malloc(image->x*image->y * sizeof(EveryPixel));
		fread(image->d, 3 * image->x, image->y, f);
		fclose(f);
		break;
	}
	default:
		break;
	}
	return 0;
}


