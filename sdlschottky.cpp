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
	
	void scale(std::complex<double> k);	//Scales the matrix by a factor of k
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

void Matrix::scale(std::complex<double> k)
{
	a *= k;
	b *= k;
	c *= k;
	d *= k;
}

class Schottky
{
	public:
		
		Schottky(double k,double v);
		void setk(double k);
		void setv(double v);
		void plot(void);	//Plots the current limit set, with Schottky circles, to the screen
	private:
		double k;
		double v;
		double u;
		double x;
		double y;
		
		double width;
		double height;
		int pixWidth;
		int pixHeight;
		
		Matrix gens[4];
		
		void updateParams(void);	//Updates all parameters of the group
		
		int calculate(void);	//Returns the number of iterations to get to the fundamental domain
		std::complex<double> pixToC(int i,int j);
		
};

void Schottky::Schottky(double k,double v)
{
	this->k=k;
	setv(v);	//Makes sure that updateParams() is called
}

void schottky::updateParams(void)
{
	u = sqrt(1.0 + v*v);
	y = 2.0 / (v*(k+1.0/k));
	x = sqrt(1.0 + y*y);

	gens[0].a = u;
	gens[0].b = std::complex<double>(0,k*v);
	gens[0].c = std::complex<double>(0,-1.0*v/k);
	gens[0].d = u;
	gens[1].a = x;
	gens[1].b = y;
	gens[1].c = y;
	gens[1].d = x;
	gens[2] = gens[0].inv();
	gens[3] = gens[1].inv();

	circ[0].centre = std::complex<double>(0,k*u/v);
	circ[0].squaredRadius = k*k/(v*v);
	circ[1].centre = x/y;
	circ[1].squaredRadius = 1.0/(y*y);
	circ[2].centre = std::complex<double>(0,-1.0*k*u/v);
	circ[2].squaredRadius = k*k/(v*v);
	circ[3].centre = -x/y;
	circ[3].squaredRadius = 1.0/(y*y);

}



int main(void)
{
	return 0;
}
