#include <iostream>
#include <complex>
#include <SDL/SDL2.h>
#include <cmath>

struct Colour
{
	double r;
	double g;
	double b;
	double a;
};

struct Circle
{
	std::complex<double> centre;
	double squaredRadius;
	bool encircles(std::complex<double> z){return std::norm(z-centre) <= squaredRadius;};
};

struct Matrix
{
	std::complex<double> a;
	std::complex<double> b;
	std::complex<double> c;
	std::complex<double> d;
	
	void seta(std::complex<double> a){this->a = a;};
	void setb(std::complex<double> b){this->b = b;};
	void setc(std::complex<double> c){this->c = c;};
	void setd(std::complex<double> d){this->d = d;};
	
	void scale(std::complex<double> k);
	std::complex<double> det(void){return a*d-b*c;};
	Matrix inv(void);
};

Matrix Matrix::inv(void)
{
	Matrix result;
	result.a = d;
	result.d = a;
	result.b = -1.0 * c;
	result.c = -1.0 * b;
	result.scale(this->det());
	return result;
}

struct Schottky
{
	public:
		void plot(void);	//Plots the current limit set, with Schottky circles, to the screen
	private:
		int calculate(void);	//Returns the number of iterations to get to the fundamental domain
		std::complex<double> pixToC(int i,int j);
		
};

void Matrix::scale(std::complex<double> k)
{
	a *= k;
	b *= k;
	c *= k;
	d *= k;
}



int main(void)
{
	return 0;
}
