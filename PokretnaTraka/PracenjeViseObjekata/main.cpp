#include <sstream>
#include <string>
#include <iostream>
#include <vector>
#include <opencv\highgui.h>
#include <opencv\cv.h>
#include <time.h>
#include <windows.h>
#include <stdio.h>
#include <fstream>

#include "Objekat.h"

using namespace std;
using namespace cv;
//inicijalne min and max vrednosti za HSV filter
//menjaju se pomocu trackbarova
int H_MIN = 0;
int H_MAX = 256;
int S_MIN = 0;
int S_MAX = 256;
int V_MIN = 0;
int V_MAX = 256;
//rezolucija snimka
const int FRAME_WIDTH = 800;
const int FRAME_HEIGHT = 600;
//max broj objekata koji mogu biti detektovani u jednom frejmu
const int MAX_NUM_OBJECTS=50;
//minimum i maximum velicina objekta
const int MIN_OBJECT_AREA = 20*20;
const int MAX_OBJECT_AREA = FRAME_HEIGHT*FRAME_WIDTH/1.5;

//prozori za pracenje i kalibraciju
const string originalWindow = "Pokretna traka";
const string hsvWindow = "HSV Image";
const string thresholdWindow = "Thresholded Image";
const string trackbarWindow= "Kalibracija boja";
const string trackbarWindow2= "Kalibracija za ukupno";

//KALIBRACIJA TOLERANCIJE
int xMAX = 339;
int xMIN = 321;
int raz = xMAX - xMIN + 5;

int maksimalno = 680;
//KALIBRACIJA MILISEKUNDI
int milisek = 7;
int milisek_max = 100;





//UKUPNO
//vector<Objekat> objektiUkupno;
		vector<Objekat> cr;
		vector<Objekat> ze;
		vector<Objekat> zu;
		vector<Objekat> pl;


//manuelno snimanje
bool manualRecordingMode = false;
bool recordingM = false;
//automatsko snimanje
bool recording = false;
bool startRecording = false;
int inc = 0;


//boje za ispis
Scalar cBoja = Scalar(0,0,255);
Scalar zeBoja = Scalar(0,255,0);
Scalar pBoja = Scalar(255,0,0);
Scalar zuBoja = Scalar(0,255,255);

Scalar crBoja = Scalar(0,0,0);
Scalar beBoja = Scalar(255,255,255);



// metoda za konvertovanje int vrednosti u string
string intToString(int number){
	std::stringstream ss;
	ss << number;
	return ss.str();
}


#pragma region SnimanjeVreme
string getDateTime(){
	//sistemsko vreme
	SYSTEMTIME theTime;
	GetLocalTime(&theTime);
	//sting u koji se upisuje vreme i datum
	string dateTime;
	//konvertuj godinu u string
	string year = intToString(theTime.wYear);
	//stringstream za dodavanje 0 ispred broja meseca (npr. 3 -> 03)
	//setw(2) sirina stringa 2 karaktera, da ne bi bilo 012(ako je 12 ostace 12)
	//mesec
	std::stringstream m;
	m<<std::setfill('0')<<std::setw(2)<< theTime.wMonth;
	string month = m.str();
	//dan
	std::stringstream d;
	d<<std::setfill('0')<<std::setw(2)<< theTime.wDay;
	string day = d.str();
	//sat
	std::stringstream hr;
	hr<<setfill('0')<<std::setw(2)<<theTime.wHour;
	string hour = hr.str();
	//minut
	std::stringstream min;
	min<<setfill('0')<<std::setw(2)<<theTime.wMinute;
	string minute = min.str();
	//sekunda
	std::stringstream sec;
	sec<<setfill('0')<<std::setw(2)<<theTime.wSecond;
	string second = sec.str();


	//ispis na ekranu
	dateTime = day + "-" + month + "-" + year + "  " + hour + ":" + minute + ":" + second;

	return dateTime;
}


#pragma endregion SnimanjeVreme


fstream koordinate;

void writeFile (string s) 
{
  koordinate<<"\n"<<s; 
}

//ova metoda se poziva svaki put kada se promeni vrednost trackbara
void on_trackbar( int, void* )
{


}
void trackbarWaitkey() {

	namedWindow(trackbarWindow2,WINDOW_AUTOSIZE );

	char TrackbarName[50];
	sprintf( TrackbarName, "Milisec", milisek);
	sprintf( TrackbarName, "xMIN", xMIN);
	sprintf( TrackbarName, "xMAX", xMAX);

	createTrackbar( "Milisec", trackbarWindow2, &milisek, milisek_max, on_trackbar );
	createTrackbar( "|xMINcrn", trackbarWindow2, &xMIN, maksimalno, on_trackbar );
	createTrackbar( "xMAXbeo|", trackbarWindow2, &xMAX, maksimalno, on_trackbar );

}

void createTrackbars(){
	//create window for trackbars
	namedWindow(trackbarWindow,WINDOW_AUTOSIZE );

	//create memory to store trackbar name on window
	char TrackbarName[50];
	sprintf( TrackbarName, "H_MIN", H_MIN);
	sprintf( TrackbarName, "H_MAX", H_MAX);
	sprintf( TrackbarName, "S_MIN", S_MIN);
	sprintf( TrackbarName, "S_MAX", S_MAX);
	sprintf( TrackbarName, "V_MIN", V_MIN);
	sprintf( TrackbarName, "V_MAX", V_MAX);
	//create trackbars and insert them into window
	//3 parameters are: the address of the variable that is changing when the trackbar is moved(eg.H_LOW),
	//the max value the trackbar can move (eg. H_HIGH), 
	//and the function that is called whenever the trackbar is moved(eg. on_trackbar)
	//                                  ---->    ---->     ---->      
	createTrackbar( "H_MIN", trackbarWindow, &H_MIN, H_MAX, on_trackbar );
	createTrackbar( "H_MAX", trackbarWindow, &H_MAX, H_MAX, on_trackbar );
	createTrackbar( "S_MIN", trackbarWindow, &S_MIN, S_MAX, on_trackbar );
	createTrackbar( "S_MAX", trackbarWindow, &S_MAX, S_MAX, on_trackbar );
	createTrackbar( "V_MIN", trackbarWindow, &V_MIN, V_MAX, on_trackbar );
	createTrackbar( "V_MAX", trackbarWindow, &V_MAX, V_MAX, on_trackbar );
}


void morphOps(Mat &thresh){

	//create structuring element that will be used to "dilate" and "erode" image.
	//the element chosen here is a 3px by 3px rectangle

	Mat erodeElement = getStructuringElement( MORPH_RECT,Size(3,3));
	//dilate with larger element so make sure object is nicely visible
	Mat dilateElement = getStructuringElement( MORPH_RECT,Size(8,8));

	erode(thresh,thresh,erodeElement);
	erode(thresh,thresh,erodeElement);


	dilate(thresh,thresh,dilateElement);
	dilate(thresh,thresh,dilateElement);


}

//za pracenje objekta pri kalibraciji
void trackFilteredObject(Mat threshold,Mat HSV, Mat &cameraFeed){


	vector <Objekat> objekti;
	Mat temp;
	threshold.copyTo(temp);
	//these two vectors needed for output of findContours
	vector< vector<Point> > contours;
	vector<Vec4i> hierarchy;
	//find contours of filtered image using openCV findContours function
	findContours(temp,contours,hierarchy,CV_RETR_CCOMP,CV_CHAIN_APPROX_SIMPLE );
	//use moments method to find our filtered object
	double refArea = 0;
	if (hierarchy.size() > 0) {
		int numObjects = hierarchy.size();
		//if number of objects greater than MAX_NUM_OBJECTS we have a noisy filter
		if(numObjects<MAX_NUM_OBJECTS){
			for (int index = 0; index >= 0; index = hierarchy[index][0]) {

				Moments moment = moments((cv::Mat)contours[index]);
				double area = moment.m00;

				//if the area is less than 20 px by 20px then it is probably just noise
				//if the area is the same as the 3/2 of the image size, probably just a bad filter
				//we only want the object with the largest area so we safe a reference area each
				//iteration and compare it to the area in the next iteration.
				if(area>MIN_OBJECT_AREA){

					Objekat objekat;

					objekat.setXPos(moment.m10/area);
					objekat.setYPos(moment.m01/area);

					objekti.push_back(objekat);

					cv::circle(cameraFeed,cv::Point(objekat.getXPos(),objekat.getYPos()),10,crBoja,-1);

				}

			}
			

		}else putText(cameraFeed,"PREVISE SUMA, KALIBRACIJA NIJE DOBRA!",Point(0,50),1,2,Scalar(0,0,255),2);
	}
}




void ukupno(vector<Objekat> obj,Mat &cameraFeed) {

			Objekat o = obj.front();
			
			string naz = o.getType();

			int x = o.getXPos();
			int y = o.getYPos();
			if(naz == "crven") {
				cr.push_back(o);
				line(cameraFeed,Point(x,0),Point(x,480),o.getColour(),2,8,0);
				writeFile("CRVEN " "cID: "+ intToString(o.getId()) + " >> pozicija(" +intToString(x)+", "+intToString(y)+") <<"+" >> VREME  " +getDateTime()+" << ");
			} else if (naz == "zelen"){
				ze.push_back(o);
				line(cameraFeed,Point(x,0),Point(x,480),o.getColour(),2,8,0);
				writeFile("ZELEN " "zeID: "+ intToString(o.getId()) +" >> pozicija(" +intToString(x)+", "+intToString(y)+") <<"+" >> VREME  " +getDateTime()+" << ");
			} else if (naz == "zut"){
				zu.push_back(o);
				line(cameraFeed,Point(x,0),Point(x,480),o.getColour(),2,8,0);
				writeFile("ZUT   " "zuID: " + intToString(o.getId()) + " >> pozicija(" +intToString(x)+", "+intToString(y)+") <<"+" >> VREME  " +getDateTime()+" << ");
			} else if (naz == "plav"){
				pl.push_back(o);
				line(cameraFeed,Point(x,0),Point(x,480),o.getColour(),2,8,0);
				writeFile("PLAV  " "pID: " + intToString(o.getId()) + " >> pozicija(" +intToString(x)+", "+intToString(y)+")"+" >> VREME  " +getDateTime()+" << ");
			}
}

		
//za pracenje objekta nakon kalibracije
void trackFilteredObject(Objekat theObjekat,Mat threshold,Mat HSV, Mat &cameraFeed){

	vector <Objekat> objekti;

	vector<Objekat> crveniTren;
	vector<Objekat> zeleniTren;
	vector<Objekat> zutiTren;
	vector<Objekat> plaviTren;

	
	vector <Objekat> objektiUkupno;	

	vector<Objekat> uk1;
	//vector <Objekat> objektiUkupno;

	Mat temp;
	threshold.copyTo(temp);
	//these two vectors needed for output of findContours
	vector< vector<Point> > contours;
	vector<Vec4i> hierarchy;
	//find contours of filtered image using openCV findContours function
	findContours(temp,contours,hierarchy,CV_RETR_CCOMP,CV_CHAIN_APPROX_SIMPLE );
	//use moments method to find our filtered object
	

	if (hierarchy.size() > 0) {
		int numObjects = hierarchy.size();
		//if number of objects greater than MAX_NUM_OBJECTS we have a noisy filter
		if(numObjects<MAX_NUM_OBJECTS){
			for (int i = 0; i >= 0; i = hierarchy[i][0]) {

				Moments moment = moments((cv::Mat)contours[i]);
				double area = moment.m00;
				
				//if the area is less than 20 px by 20px then it is probably just noise
				//if the area is the same as the 3/2 of the image size, probably just a bad filter
				//we only want the object with the largest area so we safe a reference area each
				//iteration and compare it to the area in the next iteration.
				if(area>MIN_OBJECT_AREA){
					Objekat objekat;
					

					objekat.setXPos(moment.m10/area);
					objekat.setYPos(moment.m01/area);
					objekat.setType(theObjekat.getType());
					objekat.setColour(theObjekat.getColour());
					
					objekat.setId(rand());

					objekti.push_back(objekat);

					//krugic u centru svakog objekta
					circle(cameraFeed,cv::Point(objekat.getXPos(),objekat.getYPos()),9,objekat.getColour(),-1);
					//ispisivanje pozicije objekta i tipa objekta
					putText(cameraFeed,intToString(objekat.getXPos())+ " , " + intToString(objekat.getYPos()),cv::Point(objekat.getXPos(),objekat.getYPos()+20),1,1,Scalar(0,0,0));
					putText(cameraFeed,objekat.getType(),cv::Point(objekat.getXPos(),objekat.getYPos()-30),1,2,objekat.getColour());
					
					//LOGIKA ZA ISPIS

					//ZA TRENUTNO
					
					if(objekat.getType() == "crven") {
						crveniTren.push_back(objekat);
					} else if(objekat.getType() == "zelen") {
						zeleniTren.push_back(objekat);
					} else if(objekat.getType() == "zut") {
						zutiTren.push_back(objekat);
					} else if(objekat.getType() == "plav") {
						plaviTren.push_back(objekat);
					}

					//ZA UKUPNO
					if(objekat.getXPos() < xMAX && objekat.getXPos() > xMIN) {
						
						int x = objekat.getXPos()-raz;
						int x1 = objekat.getXPos()+raz;

						
						if(x<xMIN || x1>xMAX) {

							objektiUkupno.push_back(objekat);

							if(objektiUkupno.size() == 1) {
								ukupno(objektiUkupno,cameraFeed);
							} else {
								objektiUkupno.pop_back();
							}
							/*if(objektiUkupno.size() == 1) {
								ukupno(objektiUkupno,cameraFeed);
							}*/
							
							/*if(objektiUkupno.size() == 2) {
								Objekat o = objektiUkupno.front();
								Objekat o2 = objektiUkupno.back();

								string naz = o.getType();
								string naz2 = o2.getType();

								int a = abs(o.getYPos() - o2.getYPos());
								
								if(naz != naz2 && a > 5) {
									ukupno(objektiUkupno,cameraFeed);
								} else if(naz == naz2 && a < 5) {
									objektiUkupno.erase(objektiUkupno.begin()+1);
									ukupno(objektiUkupno,cameraFeed);
								}
							} else if(objektiUkupno.size() > 2) {
								objektiUkupno.clear();
							} */
						} else {objekti.clear();}
					} 
				}
			} 
			
			
			//ISPIS TRENUTNOG BROJA OBJEKATA
			if(crveniTren.size()>0) {
				putText(cameraFeed,"CRVENIH",cv::Point(200,400),1,1,cBoja);
				putText(cameraFeed,intToString(crveniTren.size()),cv::Point(200,420),1,1,cBoja);
			}
			if(zeleniTren.size()>0) {
				putText(cameraFeed,"ZELENIH",cv::Point(300,400),1,1,zeBoja);
				putText(cameraFeed,intToString(zeleniTren.size()),cv::Point(300,420),1,1,zeBoja);
			}
		
			if(zutiTren.size()>0) {
				putText(cameraFeed,"ZUTIH",cv::Point(400,400),1,1,zuBoja);
				putText(cameraFeed,intToString(zutiTren.size()),cv::Point(400,420),1,1,zuBoja);
			}
		
			if(plaviTren.size()>0) {
				putText(cameraFeed,"PLAVIH",cv::Point(500,400),1,1,pBoja);
				putText(cameraFeed,intToString(plaviTren.size()),cv::Point(500,420),1,1,pBoja);
			}


			if(objekti.size() > 0) {
				recording = true;
			} else {
				recording = false;
			}
			
		}else putText(cameraFeed,"PREVISE SUMA, KALIBRACIJA NIJE DOBRA!",Point(0,50),1,2,Scalar(0,0,255),2);

		objekti.clear();
		
		//objektiUkupno.clear();

		crveniTren.clear();
		zeleniTren.clear();
		plaviTren.clear();
		zutiTren.clear();
			
	}
}


VideoCapture capture;


int main(int argc, char* argv[])
{
	
	koordinate.open("F:/TRAKASNIMCI/log.txt",fstream::app);

	//pauza i resume koda
	bool pause = false;

	//kalibracija boja
	bool calibrationMode = true;
	//UPUTSTVO
	cout<<"CONTROLS\n";
	cout<<"************************************\n";
	cout<<"Press C to reset UKUPNO and SVI \n";
	cout<<"Press P to pause program \n";
	cout<<"************************************\n";
	cout<<"Press M to enter manual record  mode\n";
	cout<<"Press A to return to automatic record mode \n";
	cout<<"Press N to start new record \n";
	cout<<"************************************\n";
	cout<<"Current record mode > AUTOMATIC\n";
	cout<<"************************************\n";

	


	//Matrix to store each frame of the webcam feed
	Mat cameraFeed;
	Mat threshold;
	Mat HSV;
	
	capture.open(0);


	if(calibrationMode){
		//kreiraj slajdere na treshold prozoru
		createTrackbars();
	} else {
		//kreiraj slajdere na glavnom prozoru pokretna traka
		trackbarWaitkey();
	}
	capture.set(CV_CAP_PROP_FRAME_WIDTH,FRAME_WIDTH);
	capture.set(CV_CAP_PROP_FRAME_HEIGHT,FRAME_HEIGHT);
	
	
	
	//Video writer

	VideoWriter oVideoWriter;//create videoWriter object, not initialized yet
	double dWidth = capture.get(CV_CAP_PROP_FRAME_WIDTH); //get the width of frames of the video
	double dHeight = capture.get(CV_CAP_PROP_FRAME_HEIGHT); //get the height of frames of the video
	//set framesize for use with videoWriter
	Size frameSize(static_cast<int>(dWidth), static_cast<int>(dHeight));

	if(!capture.isOpened()){
		cout<<"GRESKA PRILIKOM PREUZIMANJA VIDEA\n";
		getchar();
		return -1;
	}

	
	Objekat crven("crven"), zelen("zelen"), zut("zut"), plav("plav");	
	
	//start an infinite loop where webcam feed is copied to cameraFeed matrix
	//all of our operations will be performed within this loop
	while(1){
		
		//store image to matrix
		capture.read(cameraFeed);

		
		//convert frame from BGR to HSV colorspace
		cvtColor(cameraFeed,HSV,COLOR_BGR2HSV);


		if(calibrationMode==true){
			//if in calibration mode, we track objects based on the HSV slider values.
			inRange(HSV,Scalar(H_MIN,S_MIN,V_MIN),Scalar(H_MAX,S_MAX,V_MAX),threshold);
			morphOps(threshold);
			imshow(thresholdWindow,threshold);
			trackFilteredObject(threshold,HSV,cameraFeed);
		} else {

			//crni kvadrat
			rectangle(cameraFeed,Point(200,380),Point(650,460),crBoja,-1);
		
			//crvene
			inRange(HSV,crven.getHSVmin(),crven.getHSVmax(),threshold);
			//morphOps(threshold);
			trackFilteredObject(crven,threshold,HSV,cameraFeed);

			//zute
			inRange(HSV,zut.getHSVmin(),zut.getHSVmax(),threshold);
			//morphOps(threshold);
			trackFilteredObject(zut,threshold,HSV,cameraFeed);
			
			//zelene
			inRange(HSV,zelen.getHSVmin(),zelen.getHSVmax(),threshold);
			//morphOps(threshold);
			trackFilteredObject(zelen,threshold,HSV,cameraFeed);
			
			//plave
			inRange(HSV,plav.getHSVmin(),plav.getHSVmax(),threshold);
			//morphOps(threshold);
			trackFilteredObject(plav,threshold,HSV,cameraFeed);
			
		
			line(cameraFeed,Point(xMIN,0),Point(xMIN,480),crBoja,2,8,0);

			line(cameraFeed,Point(xMAX,0),Point(xMAX,480),beBoja,2,8,0);

			//ISPIS DATUMA I VREMENA
			rectangle(cameraFeed,Point(0,460),Point(200,480),beBoja,-1);
			putText(cameraFeed,getDateTime(),Point(0,480),1,1,Scalar(0,0,0),2);

			//ukupno crvenih
			putText(cameraFeed,"UKUPNO",cv::Point(200,440),1,1,cBoja);
			putText(cameraFeed,intToString(cr.size()),cv::Point(200,460),1,1,cBoja);

			//ukupno zelenih
			putText(cameraFeed,"UKUPNO",cv::Point(300,440),1,1,zeBoja);
			putText(cameraFeed,intToString(ze.size()),cv::Point(300,460),1,1,zeBoja);

			//ukupno zutih
			putText(cameraFeed,"UKUPNO",cv::Point(400,440),1,1,zuBoja);
			putText(cameraFeed,intToString(zu.size()),cv::Point(400,460),1,1,zuBoja);

			//ukupno plavih
			putText(cameraFeed,"UKUPNO",cv::Point(500,440),1,1,pBoja);
			putText(cameraFeed,intToString(pl.size()),cv::Point(500,460),1,1,pBoja);

			//ukupno svi
			putText(cameraFeed,"SVI",cv::Point(600,440),1,1,beBoja);
			putText(cameraFeed,intToString(pl.size()+cr.size()+ze.size()+zu.size()),cv::Point(600,460),1,1,beBoja);
		}

		if(startRecording == true) {

				oVideoWriter  = VideoWriter("F:/TRAKASNIMCI/Video"+intToString(inc)+".avi", CV_FOURCC('D', 'I', 'V', '3'), 15, frameSize, true); //initialize the VideoWriter object 
				cout<<"New video file created F:/TRAKASNIMCI/Video"+intToString(inc)+".avi "<<endl;
				startRecording = false;
				if ( !oVideoWriter.isOpened() ) //if not initialize the VideoWriter successfully, exit the program
				{
					cout << "ERROR: Failed to initialize video writing" << endl;
					getchar();
					return -1;
				}
			}

		//automatsko snimanje
		if(manualRecordingMode == false) {
			if(recording) {
				oVideoWriter.write(cameraFeed);
				//show "REC" in top left corner in red
				//be sure to do this AFTER you write to the file so that "REC" doesn't show up
				//on the recorded video.
				putText(cameraFeed,"REC",Point(0,60),1,2,Scalar(0,0,255),2);
			}
		}

		//manualno snimanje 

		if(manualRecordingMode == true) {
			if(recordingM) {
				oVideoWriter.write(cameraFeed);
				//show "REC" in top left corner in red
				//be sure to do this AFTER you write to the file so that "REC" doesn't show up
				//on the recorded video.
				putText(cameraFeed,"mREC",Point(0,60),1,2,Scalar(0,0,255),2);
			}
		}

		//prikaz videa
		imshow(originalWindow,cameraFeed);
		//imshow(hsvWindow,HSV);
		//imshow(thresholdWindow,threshold);
		
		//KONTROLA

		switch(waitKey(milisek)){

		case 27: //'esc' key has been pressed, exit program.
			return 0;
		case 112: //'p' has been pressed. this will pause/resume the code.
			pause = !pause;
			if(pause == true){ cout<<"Code paused, press 'p' again to resume\n";
			cout<<"****************************************\n";
			while (pause == true){
				//stay in this loop until 
				switch (waitKey()){
					//a switch statement inside a switch statement? Mind blown.
				case 112: 
					//change pause back to false
					pause = false;
					cout<<"Code Resumed\n"<<endl;
					cout<<"****************************************\n";
					break;
				}
			}

		case 114:
			//'r' has been pressed.
			//toggle recording mode
			if(manualRecordingMode) {
			recordingM = true;
			cout << "Recording Started\n" << endl;
			cout<<"****************************************\n";
			}
			break;

		case 115:
			//'s' has been pressed.
			//toggle recording mode
			if(manualRecordingMode) {
			recordingM = false;
			cout << "Recording Stopped, press R to continue recording to"<< "Video"<<intToString(inc)
				<< "\n or press N to save current video and start recording new video\n";
			cout<<"****************************************\n";
			}

			break;

		case 99:
			//'c' has been pressed
			cr.clear();
			ze.clear();
			zu.clear();
			pl.clear();
			cout<<"Counters reseted\n";
			cout<<"****************************************\n";

			break;
		
		case 109:
			//'m' has been pressed
			//Manual recording
			manualRecordingMode = true;
			cout << "Manual recording mode \n New Video recording started, press R to record\n" << endl;
			cout<<"****************************************\n";
			//increment video file name
			inc+=1;

			break;

		case 97:

			manualRecordingMode = false;
			cout<<" Automatic recording mode \n";
			cout<<"****************************************\n";
			break;

		case 110:
			//'n' has been pressed
			//start new video file
			startRecording = true;
			cout<<"NOW RECORDING \n";
			cout<<"****************************************\n";
			//increment video file name
			inc+=1;
			break; 

			}
			
		} 
			
	}
	capture.release();
	koordinate.close();
	return 0;

}
