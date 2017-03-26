#include "HX711.h"

// HX711.DOUT	- pin #A1
// HX711.PD_SCK	- pin #A0

HX711 scale(A0, A1);		// parameter "gain" is ommited; the default value 128 is used by the library
bool GOT_IT = false;
bool SPINDLE_RELEASE = false;
bool PRELOAD = true;


float len_inc = 0.8177;  // "(1.562 * PI) / 6" --- line spool circumfrence by 6 holes
float len = 0.0;

float max_slope = 250.0;

float grams = 0.0, x1 = 0.0, x2 = 0.0, y1 = 0.0, y2 = 0.0, slope = 0.0;
bool need_pull_len = true;
float pull_len = 0.0;

 
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
  int hold = millis();
  while( digitalRead( 13 ) == LOW )
    delay(10);
  if( (millis() - hold) > 3000 )
  {
    stepTest();

    Serial.println("\nPush start button\n");
    
    while( digitalRead( 13 ) == HIGH )
      delay(10);
    int hold = millis();
    while( digitalRead( 13 ) == LOW )
      delay(10);
  }  

    
  SPINDLE_RELEASE = false;
  //spindlePull();
  Serial.println( "\n\nEnter the pull length in inches\n\n");
}

void spindleStop()
{
  digitalWrite( 11, HIGH );
  digitalWrite( 10, HIGH );
  digitalWrite( 9, HIGH );
  digitalWrite( 8, HIGH );

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

void stepTest()
{
  bool ilock = false;

  Serial.println("Step mode");
  spindleStop();
  
  while( true )
  {
    while( digitalRead( 13 ) == HIGH )
      delay(10);
      
    int hold = millis();
    
    while( digitalRead( 13 ) == LOW )
      delay(10);
      
/*
    if( (millis() - hold) > 5000 )
    {
      spindleStop();
      return;
    }
*/
    spindlePull();

    while( digitalRead(12) == HIGH )
    {
      delay(5);
    }

    while( digitalRead(12) == LOW )
    {
      delay(5);
    }

    spindleStop();
      
  }
  
}


void loop() {
     
  while( need_pull_len && Serial.available() > 0 )
  {
    pull_len = Serial.parseFloat();

    if( Serial.read() == '\n' )
    {
      if( pull_len > 0.0 )
      {
        need_pull_len = false;
        Serial.print("\nPull Length: ");
        Serial.println( pull_len );
        spindlePull();
      }
    }
  }

  ///Serial.print("\nPull Length: ");
  ///Serial.println( pull_len );

  if( !need_pull_len && pull_len > 1.0 )
  {
    {
      if( digitalRead( 12 ) == HIGH && GOT_IT == false )
      {
        spindleStop();
    
        
        
        x1 = len;
        y1 = grams;
    
        if( SPINDLE_RELEASE )
          len -= len_inc;
        else
          len += len_inc;
    
        grams = scale.get_units();
    
        x2 = len;
        y2 = grams;
    
        slope = ( (y2-y1) / (x2-x1) );
    
        //if( grams > 1700.0 && slope >= max_slope && !SPINDLE_RELEASE )
        if( len >= pull_len )
        {
          SPINDLE_RELEASE = true;
          Serial.println("\nReleasing\n");
          delay( 1500 );
    
        }
        
        //Serial.print("one reading:\t");
    
        Serial.print( len, 3 );
        Serial.print( "\t" );
        Serial.print(grams*.001, 3);
        /*
        Serial.print( "\t" );
        Serial.print(y2, 3);
        Serial.print( "\t" );
        Serial.print(y1, 3);
        Serial.print( "\t" );
        Serial.print(x2, 3);
        Serial.print( "\t" );
        Serial.print(x1, 3);
        */
        Serial.print( "\t" );
        Serial.println(slope, 3);
        //Serial.print("\t| average:\t");
        //Serial.print(scale.get_units(4), 1);
        //Serial.print("\t| value:\t");
        //Serial.println(scale.get_value(), 1);
        GOT_IT = true;
      
        // preload 
        if( PRELOAD && grams < 10 )
        {
          Serial.println("Preloading...");
          spindleStop();
          delay(200);
        }    
        else if( PRELOAD && grams >= 10 )
        {
          spindleStop();
          Serial.println("Preloaded:");
          Serial.println("\nPush button to stretch\n");
    
          PRELOAD = false;
          //len = len_inc;
          len = 0.0;
          
          while( digitalRead( 13 ) == HIGH )
            delay(10);
          int hold = millis();
          while( digitalRead( 13 ) == LOW )
            delay(10);
        }
        else if( SPINDLE_RELEASE && grams < 500 )
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
    }
  }
  //scale.power_down();			        // put the ADC in sleep mode
  //delay(50);
  //scale.power_up();
}
