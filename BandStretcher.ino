#include "HX711.h"

// HX711.DOUT	- pin #A1
// HX711.PD_SCK	- pin #A0

HX711 scale(A0, A1);		// parameter "gain" is ommited; the default value 128 is used by the library
bool GOT_IT = false;
bool SPINDLE_RELEASE = false;

void setup() {
  Serial.begin(115200);
  Serial.println("HX711 Demo");

  pinMode( 13, INPUT_PULLUP );
  pinMode( 12, INPUT );
  pinMode( 11, OUTPUT );
  pinMode( 10, OUTPUT );
  pinMode( 9, OUTPUT );
  pinMode( 8, OUTPUT );

  spindleStop();

  Serial.println("Before setting up the scale:");
  Serial.print("read: \t\t");
  Serial.println(scale.read());			// print a raw reading from the ADC

  Serial.print("read average: \t\t");
  Serial.println(scale.read_average(10));  	// print the average of 20 readings from the ADC

  Serial.print("get value: \t\t");
  Serial.println(scale.get_value(5));		// print the average of 5 readings from the ADC minus the tare weight (not set yet)

  Serial.print("get units: \t\t");
  Serial.println(scale.get_units(5), 1);	// print the average of 5 readings from the ADC minus tare weight (not set) divided 
						// by the SCALE parameter (not set yet)  

  scale.set_scale(-45100.0/500.0);                      // this value is obtained by calibrating the scale with known weights; see the README for details
  scale.tare();				        // reset the scale to 0

  Serial.println("After setting up the scale:");

  Serial.print("read: \t\t");
  Serial.println(scale.read());                 // print a raw reading from the ADC

  Serial.print("read average: \t\t");
  Serial.println(scale.read_average(10));       // print the average of 20 readings from the ADC

  Serial.print("get value: \t\t");
  Serial.println(scale.get_value(5));		// print the average of 5 readings from the ADC minus the tare weight, set with tare()

  Serial.print("get units: \t\t");
  Serial.println(scale.get_units(5), 1);        // print the average of 5 readings from the ADC minus tare weight, divided 
						// by the SCALE parameter set with set_scale

  Serial.println("Readings:");
  Serial.println("\nPush start button\n");

  while( digitalRead( 13 ) == HIGH )
    delay(10);
  while( digitalRead( 13 ) == LOW )
    delay(10);
    
  SPINDLE_RELEASE = false;
  spindlePull();
}


void spindleStopOld()
{
  digitalWrite( 11, LOW );
  digitalWrite( 10, LOW );
  digitalWrite( 9, LOW );
  digitalWrite( 8, LOW );
}

void spindleStop()
{
  digitalWrite( 11, HIGH );
  digitalWrite( 10, HIGH );
  digitalWrite( 9, HIGH );
  digitalWrite( 8, HIGH );
  delay(3);
}

void spindlePull()
{
  spindleStop();
  digitalWrite( 11, HIGH );
  digitalWrite( 10, LOW );
  digitalWrite( 9, HIGH );
  digitalWrite( 8, LOW );
}

void spindleRelease()
{
  spindleStop();
  digitalWrite( 11, LOW );
  digitalWrite( 10, HIGH );
  digitalWrite( 9, LOW );
  digitalWrite( 8, HIGH );
}

void loop() {
  //Serial.print("one reading:\t");
  //Serial.print(scale.get_units(), 1);
  //Serial.print("\t| average:\t");
  //Serial.print(scale.get_units(4), 1);
  //Serial.print("\t| value:\t");
  //Serial.println(scale.get_value(), 1);

  if( digitalRead( 12 ) == HIGH && GOT_IT == false )
  {
    spindleStop();
    float grams = scale.get_units();
    //Serial.print("one reading:\t");
    Serial.println(grams*.001, 1);
    //Serial.print("\t| average:\t");
    //Serial.print(scale.get_units(4), 1);
    //Serial.print("\t| value:\t");
    //Serial.println(scale.get_value(), 1);
    GOT_IT = true;

    if( SPINDLE_RELEASE && grams < 200 )
    {
      spindleStop();
      Serial.println("Stopped");
      
      while(true)
        ;
    }

    if( !SPINDLE_RELEASE ) 
      spindlePull();
    else
      spindleRelease();
  }
  else if( digitalRead( 12 ) == LOW && GOT_IT == true )
  {
    GOT_IT = false;
    
    if( !SPINDLE_RELEASE ) 
      spindlePull();
    else
      spindleRelease();
  }

  if( digitalRead( 13 ) == LOW && SPINDLE_RELEASE == false )
  {
    Serial.println("\nReleasing\n");
    SPINDLE_RELEASE = true;
    spindleRelease();
  }

  //scale.power_down();			        // put the ADC in sleep mode
  //delay(50);
  //scale.power_up();
}
