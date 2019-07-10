#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <omp.h>

#define FAILURE 0
#define SUCCESS !FAILURE

#define USER_NAME "acw18mg"		//replace with your user name

#pragma warning(disable : 4996)

void print_help();
int process_command_line(int argc, char *argv[]);
int readFile();
int allocateMemory();
int runCPU(unsigned short *red_temp, unsigned short *green_temp, unsigned short *blue_temp);
int runOPENMP(unsigned short *red_temp, unsigned short *green_temp, unsigned short *blue_temp);
int writeBinary();
int writePlainText();
int freeMemory();

typedef enum MODE { CPU, OPENMP, CUDA, ALL } MODE;
typedef enum FORMAT { PPM_BINARY, PPM_PLAIN_TEXT } FORMAT;

unsigned char magic_number[3];
char *input_image_name = NULL;
char *output_image_name = NULL;
char comment[64] = "";
unsigned short width = 0;
unsigned short height = 0;
unsigned short max_color_value = 0;
unsigned short **red;
unsigned short **green;
unsigned short **blue;
unsigned int c = 0;
MODE execution_mode = OPENMP;
FORMAT image_format = PPM_BINARY;

/*
  use parallel inner loop
*/
int main(int argc, char *argv[]) {
	unsigned short red_average = 0;
	unsigned short blue_average = 0;
	unsigned short green_average = 0;

	clock_t begin, end;
	double begin_openmp, end_openmp;

	
	if (process_command_line(argc, argv) == FAILURE) {
		//return 1;

		//Test (Remove when finished)
		printf("arguments wrong!\n");
		//return 0;
	}
		

	
	//TODO: read input image file (either binary or plain text PPM) 
	readFile();

	//TODO: execute the mosaic filter based on the mode
	switch (execution_mode){
		case (CPU) : {
			//TODO: starting timing here
			begin = clock();

			//TODO: calculate the average colour value
			runCPU(&red_average, &green_average, &blue_average);

			// Output the average colour value for the image
			printf("CPU Average image colour red = %hu, green = %hu, blue = %hu \n", red_average, green_average, blue_average);

			//TODO: end timing here
			end = clock();
			printf("CPU mode execution time took %.0f s and %.0f ms\n", (end - begin) / (float)CLOCKS_PER_SEC, ((end - begin) / (float)CLOCKS_PER_SEC)*1000.0);
			
			//save the output image file (from last executed mode)
			switch (image_format) {
			case (PPM_BINARY): {
				writeBinary();
				break;
			}
			case (PPM_PLAIN_TEXT): {
				writePlainText();
				break;
			}
			}
			
			break;
		}
		case (OPENMP) : {
			//TODO: starting timing here
			begin_openmp = omp_get_wtime();

			//TODO: calculate the average colour value
			runOPENMP(&red_average, &green_average, &blue_average);

			// Output the average colour value for the image
			printf("OPENMP Average image colour red = %d, green = %d, blue = %d \n", red_average, green_average, blue_average);

			//TODO: end timing here
			end_openmp = omp_get_wtime();
			printf("OPENMP mode execution time took %.0f s and %.0f ms\n", (end_openmp - begin_openmp), (end_openmp - begin_openmp)*1000.0);
			
			//save the output image file (from last executed mode)
			switch (image_format) {
			case (PPM_BINARY): {
				writeBinary();
				break;
			}
			case (PPM_PLAIN_TEXT): {
				writePlainText();
				break;
			}
			}
			
			break;
		}
		case (CUDA) : {
			printf("CUDA Implementation not required for assignment part 1\n");
			break;
		}
		case (ALL) : {
			// starting timing here
			begin = clock();

			// calculate the average colour value
			runCPU(&red_average, &green_average, &blue_average);

			// Output the average colour value for the image
			printf("CPU Average image colour red = %d, green = %d, blue = %d \n", red_average, green_average, blue_average);

			// end timing here
			end = clock();
			printf("CPU mode execution time took %.0f s and %.0f ms\n", (end - begin) / (float)CLOCKS_PER_SEC, ((end - begin) / (float)CLOCKS_PER_SEC)*1000.0);
			
			//save the output image file (from last executed mode)
			switch (image_format) {
			case (PPM_BINARY): {
				writeBinary();
				break;
			}
			case (PPM_PLAIN_TEXT): {
				writePlainText();
				break;
			}
			}



			// read the file again for openmp
			readFile();
			
			// starting timing here
			begin_openmp = omp_get_wtime();

			// calculate the average colour value
			runOPENMP(&red_average, &green_average, &blue_average);

			// Output the average colour value for the image
			printf("OPENMP Average image colour red = %d, green = %d, blue = %d \n", red_average, green_average, blue_average);

			//end timing here
			end_openmp = omp_get_wtime();
			printf("OPENMP mode execution time took %.0f s and %.0f ms\n", (end_openmp - begin_openmp), (end_openmp - begin_openmp)*1000.0);
			
			//save the output image file (from last executed mode)
			switch (image_format) {
			case (PPM_BINARY): {
				writeBinary();
				break;
			}
			case (PPM_PLAIN_TEXT): {
				writePlainText();
				break;
			}
			}
			
			break;
		}
	}

	freeMemory();
	getchar();
	return 0;
}

void print_help(){
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

int process_command_line(int argc, char *argv[]){
	if (argc < 7){
		fprintf(stderr, "Error: Missing program arguments. Correct usage is...\n");
		print_help();
		return FAILURE;
	}

	//first argument is always the executable name
    
	//read in the non optional command line arguments
	c = (unsigned int)atoi(argv[1]);

	//TODO: read in the mode
	if (!strcmp(argv[2], "CPU"))
		execution_mode = CPU;
	else if (!strcmp(argv[2], "OPENMP"))
		execution_mode = OPENMP;
	else if (!strcmp(argv[2], "CUDA"))
		execution_mode = CUDA;
	else if (!strcmp(argv[2], "ALL"))
		execution_mode = ALL;
	else 
		fprintf(stderr, "Error: Wrong mode argument. Correct usage is CPU, OPENMP, CUDA or ALL.\n");

	//TODO: read in the input image name
	input_image_name = argv[4];

	//TODO: read in the output image name
	output_image_name = argv[6];

	//TODO: read in any optional part 3 arguments
	if (argc == 9)
		if (!strcmp(argv[8], "PPM_BINARY"))
			image_format = PPM_BINARY;
		else if (!strcmp(argv[8], "PPM_PLAIN_TEXT"))
			image_format = PPM_PLAIN_TEXT;
		else
			fprintf(stderr, "Error: Wrong image format argument. Correct usage is PPM_BINARY or PPM_PLAIN_TEXT.\n");

	return SUCCESS;
}

int readFile() {
	char a[2] = "e";
	char *temp; // temporarily store each read result 
	temp = a;

	//TODO improve this, using unsigned short
	unsigned char *temp_value = (char*)malloc(sizeof(char) * 4); // temporarily store the reading results

	FILE *f = NULL;

	// open file as binary, read
	f = fopen(input_image_name, "rb");

	if (f == NULL) {
		fprintf(stderr, "Could not open file\n");
	}

	
	// Read magic number
	fread(magic_number, 3, 1, f);
	magic_number[2] = '\0';
	if (strncmp(magic_number, "P3", 2) == 0) {
		// Plain Text
		printf("Read plain text file is begining\n");

		for (int i = 0; i < 3; i++) {
			fgets(comment, 64, f);
			
			if (strncmp(comment, "#", 1) == 0)
				i--;
			else if (width == 0)
				width = (unsigned short) atoi(comment);
			else if (height == 0)
				height = (unsigned short)atoi(comment);
			else 
				max_color_value = (unsigned short)atoi(comment);

		}
		
		// allocate memory for arrays
		allocateMemory();

		// assign value for arrays
		for (int i = 0; i < height; i++) {
			for (int j = 0; j < width; j++) {
				fscanf(f, "%hu %hu %hu", &red[i][j], &green[i][j], &blue[i][j]);
				//printf("red is: %hu, %hu, %hu\n", red[i][j], green[i][j], blue[i][j]);
			}
		}
		printf("Read plain text file is finished\n");
	}
	else {
		// Binary File
		printf("Read binary file is begining\n");
		int i = 0;
		int j = 0;
		int k = 0;// iterate binary file without line break


		// read width
		while (1) {
			fread(temp, 1, 1, f);
			if (!strcmp("\n", temp))
				break;
			else 
				temp_value[i++] = temp[0];
		}
		temp_value[i] = '\0';
		width = atoi(temp_value);


		// read height
		i = 0;
		while (1) {
			fread(temp, 1, 1, f);
			if (!strcmp("\n", temp))
				break;
			else
				temp_value[i++] = temp[0];
		}
		temp_value[i] = '\0';
		height = atoi(temp_value);


		// read max color value
		i = 0;
		while (1) {
			fread(temp, 1, 1, f);
			if (!strcmp("\n", temp))
				break;
			else
				temp_value[i++] = temp[0];
		}
		temp_value[i] = '\0';
		max_color_value = atoi(temp_value);

		// allocate memory for arrays
		allocateMemory();

		unsigned char *color_temp = (char*)malloc(sizeof(char) * 3 * width * height);
		fread(color_temp, 3, width * height, f);

		// divide red, green and blue into 3 arrays.
		for (i = 0; i < height; i++) {
			for (j = 0; j < width; j++) {
				red[i][j] = *(color_temp + k * 3);
				green[i][j] = *(color_temp + k * 3 + 1);
				blue[i][j] = *(color_temp + k * 3 + 2);
				k++;
			}
		}
		printf("Read binary file is finished\n");
	}
		

	return 1;
}

/*
  dynamically allocate the memory for two dimension array
*/
int allocateMemory() {
	red = (unsigned short **)malloc(sizeof(unsigned short *) * height);
	for (int i = 0; i < height; i++)
		red[i] = (unsigned short *)malloc(sizeof(unsigned short) * width);

	green = (unsigned short **)malloc(sizeof(unsigned short *) * height);
	for (int i = 0; i < height; i++)
		green[i] = (unsigned short *)malloc(sizeof(unsigned short) * width);

	blue = (unsigned short **)malloc(sizeof(unsigned short *) * height);
	for (int i = 0; i < height; i++)
		blue[i] = (unsigned short *)malloc(sizeof(unsigned short) * width);

	return 1;
}

/*
  CPU
*/
int runCPU(unsigned short *red_temp, unsigned short *green_temp, unsigned short *blue_temp) {
	unsigned short i, j, k, l;
	unsigned short quotient_row = 0;
	unsigned short remainder_row = 0;
	unsigned short quotient_column = 0;
	unsigned short remainder_column = 0;
	unsigned short limitation_row = 0;
	unsigned short limitation_column = 0;
	unsigned long red_average_part = 0;
	unsigned long green_average_part = 0;
	unsigned long blue_average_part = 0;
	unsigned long red_average_all = 0;
	unsigned long green_average_all = 0;
	unsigned long blue_average_all = 0;

	quotient_row = width / c;
	remainder_row = width % c;
	quotient_column = height / c;
	remainder_column = height % c;

	// there are ( width / c + width % c) cells in a row
	// there are ( height / c + height % c) cells in a column
	int cells_per_row;
	int cells_per_column;

	if (remainder_row)
		cells_per_row = quotient_row + 1;
	else
		cells_per_row = quotient_row;

	if (remainder_column)
		cells_per_column = quotient_column + 1;
	else
		cells_per_column = quotient_column;

	int cells = cells_per_row * cells_per_column;

	for (i = 0; i < cells_per_column; i++) {
		// loop cells in columnwq

		// change limitaion of the cell, if there is not 
		if (i == quotient_column)
			limitation_column = remainder_column;
		else
			limitation_column = c;

		for (j = 0; j < cells_per_row; j++) {
			// loop cells in row
			red_average_part = 0;
			green_average_part = 0;
			blue_average_part = 0;

			// change for the rest cell
			if (j == quotient_row)
				limitation_row = remainder_row;
			else
				limitation_row = c;
			// sum all pixel in a cell
			for (k = 0; k < limitation_column; k++) {
				// loop row in cell
				for (l = 0; l < limitation_row; l++) {
					// loop column in cell
					red_average_part += red[i*c + k][j*c + l];
					green_average_part += green[i*c + k][j*c + l];
					blue_average_part += blue[i*c + k][j*c + l];
				}
			}

			// change the pixel to average
			for (k = 0; k < limitation_column; k++) {
				// loop column in cell
				for (l = 0; l < limitation_row; l++) {
					// loop row in cell

					red[i*c + k][j*c + l] = (unsigned short) red_average_part / (limitation_row * limitation_column);
					green[i*c + k][j*c + l] = (unsigned short) green_average_part / (limitation_row * limitation_column);
					blue[i*c + k][j*c + l] = (unsigned short) blue_average_part / (limitation_row * limitation_column);

				}
			}
			red_average_all += red_average_part;
			green_average_all += green_average_part;
			blue_average_all += blue_average_part;
		}
	}

	red_average_all /= width * height;
	green_average_all /= width * height;
	blue_average_all /= width * height;

	*red_temp = (unsigned short) red_average_all ;
	*green_temp = (unsigned short) green_average_all;
	*blue_temp = (unsigned short) blue_average_all;

	return 1;
}

/*
  OPENMP
*/
int runOPENMP(unsigned short *red_temp, unsigned short *green_temp, unsigned short *blue_temp) {
	signed short i, j, k, l;
	unsigned short quotient_row = 0;
	unsigned short remainder_row = 0;
	unsigned short quotient_column = 0;
	unsigned short remainder_column = 0;
	unsigned long red_average_all = 0;
	unsigned long green_average_all = 0;
	unsigned long blue_average_all = 0;
	unsigned short limitation_row = 0;
	unsigned short limitation_column = 0;
	unsigned long red_average_part = 0;
	unsigned long green_average_part = 0;
	unsigned long blue_average_part = 0;
	unsigned long red_average_part_temp = 0;
	unsigned long green_average_part_temp = 0;
	unsigned long blue_average_part_temp = 0;

	quotient_row = width / c;
	remainder_row = width % c;
	quotient_column = height / c;
	remainder_column = height % c;

	// there are ( width / c + width % c) cells in a row
	// there are ( height / c + height % c) cells in a column
	int cells_per_row;
	int cells_per_column;

	if (remainder_row)
		cells_per_row = quotient_row + 1;
	else
		cells_per_row = quotient_row;

	if (remainder_column)
		cells_per_column = quotient_column + 1;
	else
		cells_per_column = quotient_column;

	int cells = cells_per_row * cells_per_column;


	for (i = 0; i < cells_per_column; i++) {
		//printf("Thread %d: i = %d\n", omp_get_thread_num(), i);
		
		// loop cells in columnwq

		// change limitaion of the cell, if there is not 
		if (i == quotient_column)
			limitation_column = remainder_column;
		else
			limitation_column = c;

		for (j = 0; j < cells_per_row; j++) {
			// loop cells in row

			// change for the rest cell
			if (j == quotient_row)
				limitation_row = remainder_row;
			else
				limitation_row = c;

			red_average_part = 0;
			green_average_part = 0;
			blue_average_part = 0;

			// sum all pixel in a cell
#pragma omp parallel private (k, l, red_average_part_temp, green_average_part_temp, blue_average_part_temp) 
			{
				//printf("Thread %d\n", omp_get_thread_num());
				red_average_part_temp = 0;
				green_average_part_temp = 0;
				blue_average_part_temp = 0;
#pragma omp for schedule(static, 2)
				for (k = 0; k < limitation_column; k++) {
					// loop row in cell
					for (l = 0; l < limitation_row; l++) {
						// loop column in cell
						red_average_part_temp += red[i*c + k][j*c + l];
						green_average_part_temp += green[i*c + k][j*c + l];
						blue_average_part_temp += blue[i*c + k][j*c + l];
					}
				}
#pragma omp atomic
				red_average_part += red_average_part_temp;
#pragma omp atomic
				green_average_part += green_average_part_temp;
#pragma omp atomic
				blue_average_part += blue_average_part_temp;
			}
			

			// change the pixel to average
			for (k = 0; k < limitation_column; k++) {
				// loop column in cell
				for (l = 0; l < limitation_row; l++) {
					// loop row in cell
					red[i*c + k][j*c + l] = (unsigned short) red_average_part / (limitation_row * limitation_column);
					green[i*c + k][j*c + l] = (unsigned short) green_average_part / (limitation_row * limitation_column);
					blue[i*c + k][j*c + l] = (unsigned short) blue_average_part / (limitation_row * limitation_column);
				}
			}

			red_average_all += red_average_part;
			green_average_all += green_average_part;
			blue_average_all += blue_average_part;
		}// end for (j)
	}// end for (i)

	red_average_all /= width * height;
	green_average_all /= width * height;
	blue_average_all /= width * height;
	
	*red_temp = (unsigned short) red_average_all;
	*green_temp = (unsigned short) green_average_all;
	*blue_temp = (unsigned short) blue_average_all;
	
	return 1;
}
/*
  write to binary file
*/
int writeBinary(){
	int i, j;
	FILE *f = NULL;

	printf("Start write binary file\n");

	// open file as binary, read
	f = fopen(output_image_name, "wb");

	fwrite("P6\n", sizeof(char), sizeof(magic_number) / sizeof(char), f);

	// store chache for writing
	char itoa_temp[64] = "";

	// write width
	itoa(width, itoa_temp, 10);
	itoa_temp[strlen(itoa_temp)] = '\n';
	fwrite(itoa_temp, sizeof(char), strlen(itoa_temp), f);

	// write height
	itoa(height, itoa_temp, 10);
	itoa_temp[strlen(itoa_temp)] = '\n';
	fwrite(itoa_temp, sizeof(char), strlen(itoa_temp), f);

	// write max color value
	itoa(max_color_value, itoa_temp, 10);
	itoa_temp[strlen(itoa_temp)] = '\n';
	fwrite(itoa_temp, 1, 4, f);

	// write pixel
	for (i = 0; i < height; i++) {
		for (j = 0; j < width; j++) {
			fwrite(&red[i][j], 1, 1, f);
			fwrite(&green[i][j], 1, 1, f);
			fwrite(&blue[i][j], 1, 1, f);
		}
	}


	fclose(f);
	printf("Write binary file is finished!\n");

	return 1;
}

/*
  write to plain text file
*/
int writePlainText() {
	int i, j;
	FILE *f = NULL;

	printf("Start write plain text file\n");

	// open file as binary, read
	f = fopen(output_image_name, "wb");

	// write header
	fprintf(f, "%s\n%d\n%d\n%d\n", "P3", width, height, max_color_value);

	// write pixel information
	for (i = 0; i < height; i++) {
		for (j = 0; j < width; j++) {
			fprintf(f, "%d %d %d", red[i][j], green[i][j], blue[i][j]);

			// change for end of a line
			if ((j + 1) < width)
				fprintf(f, "\t");
			else
				fprintf(f, "\n");
		}
	}

	fclose(f);
	printf("Write plain text file is finished!\n");

	return 1;
}

/*
  free memory
*/
int freeMemory() {
	int i;

	for (i = 0; i < height; i++)
		free(red[i]);
	free(red);
	for (i = 0; i < height; i++)
		free(green[i]);
	free(green);
	for (i = 0; i < height; i++)
		free(blue[i]);
	free(blue);

	return 1;
}