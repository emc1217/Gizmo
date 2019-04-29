#include <Wire.h>  //import the library allowing communication with the I2C through pins SDA and SCL
#include <skywriter.h>   //import the library of Skywriter (the 3D position sensor used)

#define PIN_TRFR 12  //define pins used for skywriter
#define PIN_RESET 13

#include <Stepper.h>  //import library for stepper motor control
#define STEPS 2048

Stepper stepperx(STEPS, 2,4,3,5); //define pins used by each motor
Stepper steppery(STEPS, A0, A2, A1, A3);
Stepper stepperz(STEPS, 9, 7, 8, 6);
// motors x and y are reversed in the machine so the orientation of x and y in the code is the one of the sensor's coordinate system

int x, y, z;  //initialise variables x,y,z
int posx = 0, posy = 0, prevx = 0, prevy = 0, prevz = 0; //intitialise variables used to caluclate the difference between current and desired positions of the motors
// posx, posy are the desired positions of motors x and y, updated by extracting the stored data in the array 'image'
// prevx and prevy are the current positions of motors x and y, updated after each time the motors rotate 

int image[8][3]; //array in which the x,y,z positional data of the 8 points of the drawing will be stored
int touched = 0; //initialise the variable 'touched' as zero. It changes value in the function 'touch', when the sensor is touched (in the function 'touch' defined at the end

void setup() {
  Serial.begin(9600); //delay added to ensure setup is made correctly

  Skywriter.begin(PIN_TRFR, PIN_RESET); //initialise the skywriter
  Skywriter.onTouch(touch); //initialise the function 'touch'
  Skywriter.onXYZ(xyz); //initialise the function 'xyz'

  //define the speed of each stepper motor using the stepper library function 'setSpeed'
  stepperx.setSpeed(15); // maximum speeds were found for each motor. Speed of x (vertical lead screw) is lower because motor exerts more torque
  steppery.setSpeed(18); 
  stepperz.setSpeed(18);

  pinMode(10, OUTPUT); //define the pins for the LEDs
  pinMode(11, OUTPUT);
}


void loop() {
  Skywriter.poll();  //the data sensed by the skywriter is polled (using the built in function '.poll()' of its library) infinitely until it detects a touch. 
  if (touched == 1) { // if a touch is detected, the value o variable 'touched' changes from 0 to 1 (see function 'touch' at the end of the code)
    digitalWrite(10, HIGH); //first LEDs light up
    for (int i = 0; i < 8; i = i + 1) {  //repeats 8 times
      Skywriter.poll(); //read xyz positional data from the sensor
      //x, y and z are extracted using an instance of the class 'SkyWriter' defined in the skywriter library
      x = 15 - (Skywriter.x) / 4369; //calibrate the readings (given on a scale from 0 to 65530) to give values in the scale of the pinpression board (15x10x3 cm)
      y = (Skywriter.y) / 6553;
      z = 3 - (Skywriter.z) / 21844;
      image[i][0] = x; //store the readings in the first row of the array 'image'
      image[i][1] = y;
      image[i][2] = z;
      delay(250); //wait 1/4 of a second. Therefore, 8 samples will be taken in 2 seconds.
    }
    digitalWrite(11, HIGH); //second LEDs light up (signals the end of the drawing)
    delay(250); // a delay is added to allow time before the LED turns back off
    digitalWrite(10, LOW); // all LEDs turn off
    digitalWrite(11, LOW);


    for (int j = 0; j <= 8; j = j + 1) {      //repeats 8 times
      x = image[j][0];  //x, y, z values are extracted from the array 'image' row by row
      y = image[j][1];
      z = image[j][2];
      posx = (x-prevx);  //the difference between the current and desired position of the x and y motors is calculated
      posy = (y-prevy);

      stepperx.step(posx *1.3* STEPS);  //the motors rotate to correct this difference. The number of steps is empirically adjusted to match the actual size of the pinpression board
      steppery.step(posy * STEPS);

      if ((prevx!=x) || (prevz!=z)){   //only if x or y motors have moved, z axis motor moves. This condition prevents the z motor from moving for nothing if x and y position are multiple times the same 
        stepperz.step(STEPS * 0.5 * z);
        stepperz.step(-STEPS * 0.5 * z); //motor z goes back to position zero after each time it moves
      }

      // current position of motors x and y are updated
      prevx = x; 
      prevy = y;
    }

    //drawing reproduction is now finished
    touched = 0;  //variable 'touched' returns to zero
    stepperx.step(-STEPS * 1.3 * x); //motors x and y return to position zero
    steppery.step(-STEPS * y);
    prevx=0; //current motor positions are updated
    prevy=0;
  }
}


int touch(unsigned char) { //touch is defined in the skywriter library
  touched = 1; //if a touch is detected, the value of variable 'touched' changes from 0 to 1
  return touched;
}

int xyz(unsigned int x, unsigned int y, unsigned int z) {   //This function is empty but because of the way the .h file is written, it needs to be defined to exploit the x, y, z reading of the sensor
  Serial.print("\nxyz poll");
}
