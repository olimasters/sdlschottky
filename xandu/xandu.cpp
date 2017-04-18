#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <complex>
#include <SDL2/SDL.h>
#include <cmath>

struct Colour
{
	unsigned char r;
	unsigned char g;
	unsigned char b;
	unsigned char a;
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
{	//TODO: Change this into two classes by making the group theory more self-contained
	public:
		
		Schottky(double k,double v,int pixWidth,int pixHeight);	//Constructor
		void plot(void);	//Plots the current limit set, with Schottky circles, to the screen
		void setPix(int i,int j,Colour colour);
		//Setters
		void setu(double u);
		void setx(double x);
		void setPixWidth(int pixWidth){this->pixWidth=pixWidth;};
		void setPixHeight(int pixHeight){this->pixHeight=pixHeight;};
		void setWidth(int width){this->width=width;};
		void setHeight(int height){this->height=height;};
		void setThreshold(int threshold){this->threshold=threshold;};
		void setRenderer(SDL_Renderer *renderer){this->renderer=renderer;};
                void setTexture(SDL_Texture *texture){this->texture=texture;};
		
		std::complex<double> pixToC(int i,int j);	//Changes pixel (i,j) into the corresponding value in the complex plane
		//TODO: Write a CToPix function
		
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
		
		int threshold;		//Maximum number of iterations before we decide we're close enough to the limit set
		
		SDL_Renderer *renderer;	//Renderer with which to plot things
		SDL_Texture *texture;	//Similar to above
		int cpus;
		
		unsigned int *pixels;	//Raw pixel data, encoded in an unsigned int array
		
		void updateParams(void);	//Updates all parameters of the group
		void updatePixStrip(int min,int max);	//Updates the pixels with x value between min and max
		
		Colour getColour(int n);			//Takes number of iterations required to reach the fundamental domain, returns corresponding colour
		int calculate(std::complex<double> z);		//Returns the number of iterations to get z into the fundamental domain
};

void Schottky::setPix(int i,int j,Colour colour)
{
	//TODO: throw exception if we try to set a pixel which is out of range
	pixels[i + j*pixWidth] = (colour.a << 24) | (colour.r << 16) | (colour.g << 8) | colour.b;	//SDL stashes a,r,g,b into a 32 bit unsigned int
}

void Schottky::plot(void)
{
	std::vector<std::thread> threads;
	threads.reserve(cpus);
	
	//Update a strip of pixels for each logical core the machine has
	for(int i=0;i<cpus;i++)
		threads.push_back(std::thread(&Schottky::updatePixStrip,this,i * pixWidth / cpus,(i+1)*pixWidth/cpus));
	
	//Wait until they're all finished
	while(!threads.empty())
	{
		threads.back().join();
		threads.pop_back();
	}
	
	//Now we just copy over the pixel information to the screen
	
	SDL_UpdateTexture(texture,NULL,pixels,pixWidth*sizeof(pixels[0])); //TODO: Apparently this is a fairly slow function with faster alternatives
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer,texture,NULL,NULL);
	SDL_RenderPresent(renderer);
}

void Schottky::updatePixStrip(int min,int max)
{
	Colour colour;
	for(int i=min;i<max;i++)
	{
		for(int j=0;j<pixWidth;j++)
		{
			colour = getColour(calculate(pixToC(i,j)));
			setPix(i,j,colour);
		}
	}
			
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

Schottky::Schottky(double x,double u,int pixWidth,int pixHeight)
{
	this->u=u;
	this->pixWidth = pixWidth;
	this->pixHeight = pixHeight;
	cpus = std::thread::hardware_concurrency();
	pixels = new unsigned int[pixWidth*pixHeight];
	setx(x);	//Makes sure that updateParams() is called
}

void Schottky::setx(double x)
{
	this->x=x;
	updateParams();
}

void Schottky::setu(double u)
{
	this->u=u;
	updateParams();
}

void Schottky::updateParams(void)
{
	//Current mode: expecting x and/or u to be changed, none of the others
	y = sqrt(x*x - 1.0);
	v = sqrt(u*u - 1.0);
	k = (2.0/(y*v) + sqrt((4.0/(y*y*v*v)) - 4.0))/2.0;	//Quadratic formula
								//TODO: Complex roots?

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

	window = SDL_CreateWindow( "Kissing Schottky group plotter", SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED, pixWidth, pixHeight, 0 );

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
	SDL_RenderSetLogicalSize( renderer, pixWidth, pixHeight );
	SDL_Texture *texture = SDL_CreateTexture(renderer,SDL_PIXELFORMAT_ARGB8888,SDL_TEXTUREACCESS_STREAMING,pixWidth,pixHeight);

	
	Schottky group(0.5,0.5,pixWidth,pixHeight);
	group.setWidth(8.0);
	group.setHeight(8.0);
	group.setThreshold(30);
	group.setRenderer(renderer);
	group.setTexture(texture);
	group.plot();
	
	//Begin dealing with the events
	std::complex<double> params;
	SDL_Event event;
	for(bool quitting = false; !quitting;)
	{
		while(SDL_PollEvent(&event))		//Emptying the queue of all of its non-quitting events
			if(event.type == SDL_QUIT)
				break;
		
		switch(event.type)
		{
			case SDL_QUIT:
				quitting = true;
				break;
			case SDL_MOUSEMOTION:
				params = group.pixToC(event.motion.x,event.motion.y);
				group.setx(params.imag());
				group.setu(params.real());
				group.plot();
				break;
		}
	}
	/*
	int frames = 50;
	auto t = std::chrono::high_resolution_clock::now();
	for(int i = 1; i <= frames ; i++)
	{
		group.setv(0.5 + (double)i/100.0);
		group.plot();
	}
	std::chrono::duration<double> diff = std::chrono::high_resolution_clock::now() - t;
	std::cout << pixWidth << " " << diff.count() << std::endl;*/
	return 0;
}
