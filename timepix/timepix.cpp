#include <iostream>
#include <chrono>
#include <complex>
#include <SDL2/SDL.h>
#include <cmath>
#include <vector>

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
	bool encircles(const std::complex<double> &z){return std::norm(z-centre) <= squaredRadius;};
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
	
	
	friend std::complex<double> operator*(const Matrix &M,const std::complex<double> &z){return (M.a*z + M.b)/(M.c*z + M.d);};	//Mobius map
	
	void scale(std::complex<double> k);	//Scales the matrix by a factor of k
	std::complex<double> det(void){return a*d-b*c;};
	Matrix inv(void);
};

Matrix Matrix::inv(void)
{
	Matrix result;
	result.a = d;
	result.d = a;
	result.b = -1.0 * b;
	result.c = -1.0 * c;
	result.scale(1.0/this->det());
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
		
		Schottky(double k,double v,int pixWidth,int pixHeight);	//Constructor
		void plot(void);	//Plots the current limit set, with Schottky circles, to the screen
		//Setters
		void setk(double k);
		void setv(double v);
		void setPixWidth(int pixWidth){this->pixWidth=pixWidth;};
		void setPixHeight(int pixHeight){this->pixHeight=pixHeight;};
		void setWidth(int width){this->width=width;};
		void setHeight(int height){this->height=height;};
		void setThreshold(int threshold){this->threshold=threshold;};
		void setRenderer(SDL_Renderer *renderer){this->renderer=renderer;};
		
	private:
		Matrix gens[4];		//Generators of the group
		Circle circ[4];		//Circles of the group
		
		double k;		//Parameters of the group, see Indra's Pearls
		double v;
		double u;
		double x;
		double y;
		
		double width;
		double height;
		int pixWidth;
		int pixHeight;
		
		std::vector<std::vector<Colour>> pixels;
		
		int threshold;		//Maximum number of iterations before we decide we're close enough to the limit set
		
		SDL_Renderer *renderer;	//Renderer with which to plot things
		
		void updateParams(void);	//Updates all parameters of the group
		
		Colour getColour(int n);			//Takes number of iterations required to reach the fundamental domain, returns corresponding colour
		int calculate(std::complex<double> z);		//Returns the number of iterations to get z into the fundamental domain
		std::complex<double> pixToC(int i,int j);	//Changes pixel (i,j) into the corresponding value in the complex plane
};

void Schottky::plot(void)
{
	Colour algcolour;
	Colour timecolour;

	SDL_SetRenderDrawColor(renderer,0,0,0,0);
	SDL_RenderClear(renderer);

	
	for(int i=0;i<pixWidth;i++)
	{
		for(int j=0;j<pixHeight;j++)
		{
			auto t = std::chrono::high_resolution_clock::now();
			algcolour = getColour(calculate(pixToC(i,j)));
			std::chrono::duration<double> diff = std::chrono::high_resolution_clock::now() - t;
			double elapsed = diff.count();
			timecolour.r = elapsed * 1e8;
			timecolour.g = elapsed * 1e8;
			timecolour.b = elapsed * 1e8;
			timecolour.a = 255;
			SDL_SetRenderDrawColor(renderer,algcolour.r,algcolour.g,algcolour.b,algcolour.a);
			SDL_RenderDrawPoint(renderer,i,j);
			SDL_SetRenderDrawColor(renderer,timecolour.r,timecolour.g,timecolour.b,timecolour.a);
			SDL_RenderDrawPoint(renderer,i+pixWidth,j);
		}
	}
	SDL_RenderPresent(renderer);
}

Colour Schottky::getColour(int n)
{
	Colour pixColour;
	pixColour.a = 255;	//Full opacity for all colours (for now)
	if(n == threshold)	//We were sufficiently close enough to the limit set
	{
		pixColour.r = 255;
		pixColour.g = 255;
		pixColour.b = 255;
	}
	else
	{
		double ratio = 3.0*fmod(5.0 * (double)n/(double)threshold,1.0);       /*Google colour circle or colour pick page 200 Indra's pearls*/
		if(ratio <= 1.0)
		{
			pixColour.r = 0;
			pixColour.g = 255*ratio;
			pixColour.b = 255*(1.0-ratio);
		}
		else if(ratio <= 2.0)
		{
			ratio -= 1.0;
			pixColour.r = 255*ratio;
			pixColour.g = 255*(1.0-ratio);
			pixColour.b = 0;
		}
		else
		{
			ratio -= 2.0;
			pixColour.r = 255*(1.0-ratio);
			pixColour.g = 0;
			pixColour.b = 255*ratio;
		}
	}
	return pixColour;
}

std::complex<double> Schottky::pixToC(int i,int j)
{
	double ratio = width/(double)pixWidth;
	return std::complex<double>(i*ratio - width/2.0,width/2.0 - j * ratio);	//This is supposed to be fast, not readable
}

int Schottky::calculate(std::complex<double> z)
{
	bool acted;
	for(int count = 0; count < threshold; count++)
	{
		acted = false;
		for(int l=0;l<4;l++)
		{
			if(circ[l].encircles(z))
			{
				z = gens[(l+2)%4] * z;	//Try to move it out of circle l.  gens[(l+2)%4] is the inverse of gens[l]
				acted = true;
				break;
			}
		}
		if(!acted)
			return count;
	}
	//If we got here then we must have exceeded threshold
	return threshold;
}

Schottky::Schottky(double k,double v,int pixWidth,int pixHeight)
{
	this->k=k;
	this->pixWidth = pixWidth;
	this->pixHeight = pixHeight;
	for(int i=0;i<pixWidth;i++)
	{
		pixels.push_back(std::vector<Colour>());
		pixels[i].resize(pixHeight);
	}

	setv(v);	//Makes sure that updateParams() is called
}

void Schottky::setv(double v)
{
	this->v=v;
	updateParams();
}

void Schottky::updateParams(void)
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



int main(int argc,char *argv[])
{
	if(argc!=2)
	{
		std::cout << "Usage: " << argv[0] << " <pixel width>" << std::endl;
		return -1;
	}
	
	int pixWidth = std::stoi(argv[1]);
	int pixHeight = pixWidth;
	SDL_Window* window;
	SDL_Renderer* renderer;
	

	// Initialize SDL
	if ( SDL_Init( SDL_INIT_EVERYTHING ) != 0 )
	{
		std::cout << "Failed to initialise" << std::endl;
		return -1;
	}

	// Create and init the window
	
	window = SDL_CreateWindow( "Kissing Schottky group plotter", SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED, 2*pixWidth, pixHeight, 0 );

	if ( window == nullptr )
	{
		std::cout << "Failed to create window" << std::endl;
		return -1;
	}

	// Create and init the renderer
	renderer = SDL_CreateRenderer( window, -1, SDL_RENDERER_ACCELERATED );

	if ( renderer == nullptr )
	{
		std::cout << "Failed to create renderer" << std::endl;
		return -1;
	}

	// Set size of renderer to the same as window
	SDL_RenderSetLogicalSize( renderer, 2*pixWidth, pixHeight );
	
	Schottky group(0.5,0.5,pixWidth,pixHeight);
	group.setWidth(4.0);
	group.setHeight(4.0);
	group.setThreshold(20);
	group.setRenderer(renderer);
	
	group.plot();
	
	SDL_Delay(40000);
	
	return 0;
}
