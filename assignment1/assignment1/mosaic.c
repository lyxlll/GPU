#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>
#include <string.h>

typedef struct {
	unsigned char r, g, b;
} EveryPixel;

typedef struct {
	int x, y;
	EveryPixel *d;
} WholeImage;


#define FAILURE 0
#define SUCCESS !FAILURE

#define USER_NAME "acp18gc"		//replace with your user name

void print_help();
char *filename, *modename, *outname, *option;

int process_command_line(int argc, char *argv[]);

typedef enum MODE { CPU, OPENMP, CUDA, ALL } MODE;

unsigned int c ;
MODE execution_mode = OPENMP;

WholeImage *image;
char buffer[16];

int readFile();


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

int main(int argc, char *argv[]) {

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
	//FILE * f = fopen(filename, "rb");
	//if (f == NULL)
	//{
	//	printf("cannot open");
	//	return;
	//}


	
	image = (WholeImage *)malloc(sizeof(WholeImage));


	readFile();

	int target = c;
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

	
		int aveRed = 0, aveGreen = 0, aveBlue = 0, allaver = 0, allaveg = 0, allaveb = 0;
		int t1 = target;
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
		
		
		for (int y_in_col = 0; y_in_col < cells_per_col; y_in_col++)
		{
			if (y_in_col == col_quot)
				col_limit = col_rema;
			else
				col_limit = c;
			for (int x_in_row = 0; x_in_row < cells_per_row; x_in_row++)
			{
				if (x_in_row == row_quot)
					row_limit = row_rema;
				else
					row_limit = c;

				sumr = 0;
				sumg = 0;
				sumb = 0;
				for (int y_in_cell = 0; y_in_cell < col_limit; y_in_cell++)
				{
					for (int x_in_cell = 0; x_in_cell < row_limit; x_in_cell++)
					{
						int temp;
						temp = y_in_col * (c*image->x) + y_in_cell*image->x + (x_in_row*c + x_in_cell);
						sumr += image->d[temp].r;
						sumg += image->d[temp].g;
						sumb += image->d[temp].b;
					}
				}

				for (int y_in_cell = 0; y_in_cell < col_limit; y_in_cell++)
				{
					for (int x_in_cell = 0; x_in_cell < row_limit; x_in_cell++)
					{
						int temp;
						temp = y_in_col * (c*image->x) + y_in_cell * image->x + (x_in_row*c + x_in_cell);
						image->d[temp].r = (int)sumr / (col_limit*row_limit);
						image->d[temp].g = (int)sumg / (col_limit*row_limit);
						image->d[temp].b = (int)sumb / (col_limit*row_limit);
					}
				}
				allsumr += sumr;
				allsumg += sumg;
				allsumb += sumb;
			}
		}

		allaver = allsumr / (image->x*image->y);
		allaveg = allsumg / (image->x*image->y);
		allaveb = allsumb / (image->x*image->y);
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
		int t1 = target;
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
					allsumr += sumr;
					allsumg += sumg;
					allsumb += sumb;
				}
			}
		}
		//calculate the whole image average value
		allaver = allsumr / (image->x*image->y);
		allaveg = allsumg / (image->x*image->y);
		allaveb = allsumb / (image->x*image->y);


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
		printf("CUDA Implementation not required for assignment part 1\n");
		break;
	}
	case (ALL): {
//		//TODO
//		clock_t startcp, endcp;
//		startcp = clock();
//		double totalcp;
//		char buffer[16];
//
//		if (!fgets(buffer, sizeof(buffer), f)) {
//			printf("error1");
//			exit(1);
//		}
//		char type = buffer[1];
//		int comments, rgbcolor;
//		switch (type)
//		{
//		case('3'): {
//			comments = getc(f);
//			while (comments == '#')
//			{
//				while (getc(f) != '\n');
//				comments = getc(f);
//			}
//			ungetc(comments, f);
//			fscanf(f, "%d %d", &image->x, &image->y);
//			fscanf(f, "%d", &rgbcolor);
//			while (fgetc(f) != '\n');
//			image->d = (EveryPixel *)malloc(image->x*image->y * sizeof(EveryPixel));
//			for (int i = 0; i < (image->x*image->y); i++) {
//				fscanf(f, "%d%d%d", &image->d[i].r, &image->d[i].g, &image->d[i].b);
//			}
//			fclose(f);
//			break;
//		}
//		case('6'): {
//			comments = getc(f);
//			while (comments == '#')
//			{
//				while (getc(f) != '\n');
//				comments = getc(f);
//			}
//			ungetc(comments, f);
//			fscanf(f, "%d %d", &image->x, &image->y);
//			fscanf(f, "%d", &rgbcolor);
//			while (fgetc(f) != '\n');
//			image->d = (EveryPixel *)malloc(image->x*image->y * sizeof(EveryPixel));
//			fread(image->d, 3 * image->x, image->y, f);
//			fclose(f);
//			break;
//		}
//		default:
//			break;
//		}
//		int aveRed = 0, aveGreen = 0, aveBlue = 0, allaver = 0, allaveg = 0, allaveb = 0, allsumr = 0, allsumg = 0, allsumb = 0;
//		int t1 = target;
//		int i;
//
//		for (i = 0; i < image->x*image->y; i += target) {
//			int outi = i;
//			int count = target - 1;
//			int sumr = 0;
//			int sumg = 0;
//			int sumb = 0;
//			t1 = i + target;
//			int j;
//			for (j = i; j < t1&&j < image->x*image->y; j++) {
//				sumr += image->d[j].r;
//				sumg += image->d[j].g;
//				sumb += image->d[j].b;
//				allsumr += image->d[j].r;
//				allsumg += image->d[j].g;
//				allsumb += image->d[j].b;
//				if (count != 0 && j == t1 - 1) {
//					t1 += image->x;
//					j = t1 - target - 1;
//					count--;
//				}
//
//			}
//			//calculate every cell.
//			aveRed = sumr / (target*target);
//			aveGreen = sumg / (target*target);
//			aveBlue = sumb / (target*target);
//
//			int t3 = target;
//			int count2 = target - 1;
//			t3 = i + target;
//			int k;
//			for (k = i; k < t3&&k < image->x*image->y; k++) {
//				image->d[k].r = aveRed;
//				image->d[k].g = aveGreen;
//				image->d[k].b = aveBlue;
//				if (count2 != 0 && k == t3 - 1) {
//					t3 += image->x;
//					k = t3 - target - 1;
//					count2--;
//				}
//
//			}
//
//			if ((i + target) % image->x == 0 && i != 0) {
//				i += target;
//				i += (target - 1) * image->x;
//				i -= target;
//			}
//
//		}
//		//calculate the whole image average value
//		allaver = allsumr / (image->x*image->y);
//		allaveg = allsumg / (image->x*image->y);
//		allaveb = allsumb / (image->x*image->y);
//		printf("CPU Average image colour red = %d, green = %d, blue = %d \n", allaver, allaveg, allaveb);
//
//		//TODO: end timing here
//		endcp = clock();
//		totalcp = (double)(endcp - startcp) / CLOCKS_PER_SEC;
//		int a = totalcp;
//		double ms = (double)(totalcp - a) * 1000;
//		printf("CPU mode execution time took %d s and %f ms\n", a, ms);
//
//		FILE * f2 = fopen(filename, "rb");
//		double start = omp_get_wtime();
//		double total;
//
//		//TODO: calculate the average colour value
//
//		char buffer2[16];
//		if (!fgets(buffer2, sizeof(buffer2), f2)) {
//			printf("error2");
//			exit(1);
//		}
//		char type2 = buffer2[1];
//		int comments2, rgbcolor2;
//		switch (type2)
//		{
//		case('3'): {
//			comments2 = getc(f2);
//			while (comments2 == '#')
//			{
//				while (getc(f2) != '\n');
//				comments2 = getc(f2);
//			}
//			ungetc(comments2, f2);
//			fscanf(f2, "%d %d", &image->x, &image->y);
//			fscanf(f2, "%d", &rgbcolor2);
//			while (fgetc(f2) != '\n');
//			image->d = (EveryPixel *)malloc(image->x*image->y * sizeof(EveryPixel));
//			int i;
//
//			for (i = 0; i < (image->x*image->y); i++) {
//
//				fscanf(f2, "%d%d%d", &image->d[i].r, &image->d[i].g, &image->d[i].b);
//			}
//			fclose(f2);
//			break;
//		}
//		case('6'): {
//			comments2 = getc(f2);
//			while (comments2 == '#')
//			{
//				while (getc(f2) != '\n');
//				comments2 = getc(f2);
//			}
//			ungetc(comments2, f2);
//			fscanf(f2, "%d %d", &image->x, &image->y);
//			fscanf(f2, "%d", &rgbcolor2);
//			while (fgetc(f2) != '\n');
//			image->d = (EveryPixel *)malloc(image->x*image->y * sizeof(EveryPixel));
//			fread(image->d, 3 * image->x, image->y, f2);
//			fclose(f2);
//			break;
//		}
//		default:
//			break;
//		}
//		int aveRed2, aveGreen2, aveBlue2;
//		int allaver2, allaveg2, allaveb2;
//		int  allsumr2 = 0, allsumg2 = 0, allsumb2 = 0;
//		int t12 = target;
//		int i2;
//		char space[64];
//		for (i2 = 0; i2 < image->x*image->y; i2 += target) {
//			int outi = i2;
//			int count = target - 1;
//			int sumr = 0;
//			int sumg = 0;
//			int sumb = 0;
//			t12 = i2 + target;
//			int j;
//
//			for (j = i2; j < t12&&j < image->x*image->y; j++) {
//
//
//				sumr += image->d[j].r;
//
//				sumg += image->d[j].g;
//
//				sumb += image->d[j].b;
//
//
//				allsumr2 += image->d[j].r;
//
//				allsumg2 += image->d[j].g;
//
//				allsumb2 += image->d[j].b;
//
//
//				if (count != 0 && j == t12 - 1) {
//					t12 += image->x;
//					j = t12 - target - 1;
//					count--;
//				}
//
//			}
//			//calculate every cell.
//			aveRed2 = sumr / (target*target);
//
//			aveGreen2 = sumg / (target*target);
//
//			aveBlue2 = sumb / (target*target);
//
//
//
//			int t3 = target;
//			int count2 = target - 1;
//			t3 = i2 + target;
//			int k;
//			for (k = i2; k < t3&&k < image->x*image->y; k++) {
//
//				image->d[k].r = aveRed2;
//
//				image->d[k].g = aveGreen2;
//
//				image->d[k].b = aveBlue2;
//
//
//				if (count2 != 0 && k == t3 - 1) {
//					t3 += image->x;
//					k = t3 - target - 1;
//					count2--;
//				}
//			}
//
//			if ((i2 + target) % image->x == 0 && i2 != 0) {
//				i2 += target;
//				i2 += (target - 1) * image->x;
//				i2 -= target;
//			}
//
//		}
//		//calculate the whole image average value using OPENMP
//#pragma omp parallel
//#pragma omp sections nowait
//		{
//#pragma omp section
//			allaver2 = allsumr2 / (image->x*image->y);
//#pragma omp section
//			allaveg2 = allsumg2 / (image->x*image->y);
//#pragma omp section
//			allaveb2 = allsumb2 / (image->x*image->y);
//		}
//
//
//		// Output the average colour value for the image
//		printf("OPENMP Average image colour red = %d, green = %d, blue = %d \n", allaver2, allaveg2, allaveb2);
//
//		//TODO: end timing here
//		double end = omp_get_wtime();
//		total = end - start;
//		int a2 = total;
//		double ms2 = (double)(total - a2) * 1000;
//		printf("OPENMP mode execution time took %d s and %f ms\n", a2, ms2);

		break;
	}
	}
	//test

	writeFile(outname, image, target, option);
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
		return;
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