#include<stdio.h>
#include<omp.h>
#include<math.h>

int main(){
int n;
double result = 0.0;
double x = 1.0;
#pragma omp parallel for

for (n = 0; n < 10; n++) {
	double r = pow(-1, n - 1) * pow(x, 2 * n - 1) ;
#pragma omp critical
	{
	result -= r; 
	}
}
printf("Approximation is %f, value is %f\n", result, cos(x));
	getchar();
	return 0;

}