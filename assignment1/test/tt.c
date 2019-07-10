///*#include <stdio.h>
//#include <stdlib.h>
//#include <time.h>
//*/
//#include <omp.h>
//int main()
//{
//	int wid = 8, hei = 10;
//	//FILE *f = fopen("Sheffield512x512.ppm", "rb");
//	int red = 0, green = 0, blue = 0;
//#pragma omp parallel for 
//	for (int i = 0; i < (wid*hei); i++) {
//		int r3=0, g3=0, b3=0;
//		//fscanf(f, "%d%d%d", &r3, &g3, &b3);
//		red += r3;
//		green += g3;
//		blue += b3;
//	}
//	return 0;
//}

#include <stdio.h>
#include <omp.h>
int main()
{
	int i;
	#pragma omp parallel for 
	for ( i = 0; i <8; i++) {
	
		int thread = omp_get_thread_num();
		int max_threads = omp_get_max_threads();
		printf("Hello World (Thread %d of %d)\n", thread, max_threads);
	}
	return 0;
}