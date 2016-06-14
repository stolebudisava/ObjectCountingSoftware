#include "Objekat.h"



Objekat::Objekat()
{
	//set values for default constructor
	setType("null");
	setColour(Scalar(0,0,0));

}

Objekat::Objekat(string name){

	setType(name);
	
	if(name=="zelen"){

		//TODO: use "calibration mode" to find HSV min
		//and HSV max values
		//beli tel
		setHSVmin(Scalar(53,126,14));
		setHSVmax(Scalar(70,256,202));
		

		//crni tel
		//setHSVmin(Scalar(29,77,0));
		//setHSVmax(Scalar(75,256,220));

		//BGR value for Green:
		setColour(Scalar(0,255,0));

	}
	if(name=="zut"){

		//TODO: use "calibration mode" to find HSV min
		//and HSV max values
		//beli
		setHSVmin(Scalar(12,137,123));
		setHSVmax(Scalar(39,256,235));

		//crni
		//setHSVmin(Scalar(15,117,0));
		//setHSVmax(Scalar(33,256,256));
		//BGR value for Yellow:
		setColour(Scalar(0,255,255));

	}
	if(name=="plav"){

		//TODO: use "calibration mode" to find HSV min
		//and HSV max values

		//beli
		setHSVmin(Scalar(90,187,123));
		setHSVmax(Scalar(156,256,256));

		//crni
		//setHSVmin(Scalar(86,173,0));
		//setHSVmax(Scalar(188,256,182));

		//BGR value for Blue:
		setColour(Scalar(255,0,0));

	}

	if(name=="crven"){

		//TODO: use "calibration mode" to find HSV min
		//and HSV max values

		//beli
		setHSVmin(Scalar(0,121,40));
		setHSVmax(Scalar(11,256,256));

		//crni
		//setHSVmin(Scalar(0,78,0));
		//setHSVmax(Scalar(14,256,145));

		//BGR value for Red:
		setColour(Scalar(0,0,255));

	}



}



Objekat::~Objekat(void)
{
}

int Objekat::getId(){

	return Objekat::id;

}

void Objekat::setId(int id){

	Objekat::id = id;

}

int Objekat::getXPos(){

	return Objekat::xPos;

}

void Objekat::setXPos(int x){

	Objekat::xPos = x;

}

int Objekat::getYPos(){

	return Objekat::yPos;

}

void Objekat::setYPos(int y){

	Objekat::yPos = y;

}

Scalar Objekat::getHSVmin(){

	return Objekat::HSVmin;

}
Scalar Objekat::getHSVmax(){

	return Objekat::HSVmax;
}

void Objekat::setHSVmin(Scalar min){

	Objekat::HSVmin = min;
}


void Objekat::setHSVmax(Scalar max){

	Objekat::HSVmax = max;
}

