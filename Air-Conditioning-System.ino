/*
 Air conditioning system 
 Sherin Varghese, 209055992,sv176@student.le.ac.uk,28/3/2021
 */

//adding pins for input devices
const int Temperature_sensor_Pin = A5;          //temperature sensor TMP36
const int Maximum_temperature_Setpin1 = 9;      //pushbutton for setting current maximum temperature
const int Minimum_temperature_Setpin2 = 7;      //pushbutton for setting current minimum temperature
const int Temperature_increment_Pin1 = 6;       //pushbutton for increase the current max/min temperature level by 1 degree celcius
const int Temperature_decrement_Pin2 = 5;       // pushbutton for decreasing the current max/min temperature level by 1 degree celcius

// adding pins for output devices
const int Red_LED_pin = 2;                      // red LED indicates heating
const int Blue_LED_pin = 3;                     // blue LED indicate cooling
const int Green_LED_pin = 4;                    // green LED indicate device is working or not

//adding pins of shift register connected to seven segments
const int Data_pin = 11;
const int Latch_pin = 8;                        // latch pins of 2 shiftregister shorted
const int Clock_pin = 12;                       // clock pins of 2 shift register shorted

//Declaration of variables for storing datas from input devices
int Sensor_output;                              // storing temperature sensor output
int setpin1_state;                              // storing the value of 9th pin
int setpin2_state;                              // storing the value of 7th pin
int increment_pin1_state;                       // storing the value of 6th pin
int decrement_pin2_state;                       // storing the value of 5th pin


float temperature;                      // variable for the calculation and coversion of voltage in to temperature in degree celcius
int temperature_celcius;
int current_maximum_threshold = 25;             //setting maximum threshold temperature,above this temperature cooling started
int current_minimum_threshold = 15;             //setting minimum threshold temperature, below this temperature heating started
int data [] = {252, 96, 218, 242, 102, 182, 190, 224, 254, 246, 238, 62, 156, 122, 158, 142}; //Decimal values for the seven segment from 0 to 9
int flag1, flag2, p, l;

//state transition state
typedef enum {TemperatureMaxThresh, TMP36Value, TemperatureMinThresh} states;     //setting 3 states,TemperatureMaxThresh to display and update current maximum temperature
states current_state = TMP36Value;                                                //TMP36Value to display ambient temperature
const int times[] = {};                                               // TemperatureMinThresh is to display and update current minimum temperature
int time_in_current_state = 0;

//SEOS function
const int MAX_TASKS = 10;

typedef struct {
  int counter;
  int period;
  void (*function)(void);
} TaskType;

typedef struct {
  TaskType tasks[MAX_TASKS];
  int number_of_tasks;
} TaskList;

TaskList tlist;

int add_task(int period, int initial, void(*fun)(void))
{
  if (tlist.number_of_tasks == MAX_TASKS) return 0;
  tlist.tasks[tlist.number_of_tasks].period = period;
  tlist.tasks[tlist.number_of_tasks].counter = initial;
  tlist.tasks[tlist.number_of_tasks].function = fun;
  tlist.number_of_tasks++;
  return tlist.number_of_tasks;
}


void setup()
{
   Serial.begin(9600);                                 // baud rate for serial communication

   // setting pinmode for the input and output devices
   pinMode(Temperature_sensor_Pin, INPUT);
   pinMode(Maximum_temperature_Setpin1, INPUT);
   pinMode( Minimum_temperature_Setpin2, INPUT);
   pinMode(Temperature_increment_Pin1, INPUT);
   pinMode(Temperature_decrement_Pin2, INPUT);
   pinMode(Red_LED_pin, OUTPUT);
   pinMode(Blue_LED_pin, OUTPUT);
   pinMode(Green_LED_pin, OUTPUT);
   pinMode(Latch_pin, OUTPUT);
   pinMode(Data_pin, OUTPUT);
   pinMode(Clock_pin, OUTPUT);

   //setting 3 task
  
   tlist.number_of_tasks = 0;
   if (add_task(2000,0, greenLEDcheck ))                //greenLEDcheck for checking the blinking of green led at every 2s to know the system is working
   {
      Serial.println("Task 1 added successfully");
   }
   if (add_task(500, 0, temperatureSensorCheck))         // temperatureSensorCheck gives the ambient temperature at every 0.5s
   {
      Serial.println("Task 2 added successfully");
   }
   
   if (add_task(100,0, update_checkButtonsanddisplay))  //update_checkButtonsanddisplay will update the ambient temperature, current maximum and minimum temperature in seven segment
   {
      Serial.println("Task 3 added successfully");
   }


   //setting interrupts
   noInterrupts();
   TCCR1A = 0;
   TCCR1B = 0;
   TCNT1 = 0;
   OCR1A = 16000;
   TCCR1B |= (1 << WGM12);
   TCCR1B |= (1 << CS10);
   TIMSK1 |= (1 << OCIE1A);
   interrupts();
}

ISR(TIMER1_COMPA_vect)         
{
  for (int i = 0; i < tlist.number_of_tasks; i++)
  {
    if (tlist.tasks[i].counter == 0)
    {
      tlist.tasks[i].function();
      tlist.tasks[i].counter = tlist.tasks[i].period;
    }
    tlist.tasks[i].counter--;
  }
}

void greenLEDcheck (void)
{
   // The variable "ok" controls whether the green LED
   // is on or off, while it is flashing.
   static bool ok = true;
   digitalWrite(Green_LED_pin, ok ? LOW : HIGH);
   ok = !ok;
}

void temperatureSensorCheck(void)
{
   Sensor_output = analogRead(Temperature_sensor_Pin);     

   //converting the analog value in to temperature in degree celcius
   temperature = Sensor_output * 5;                             // converting that reading to voltage, multiplying by 5v
   temperature /= 1024;                                         //converting from 10 mv per degree with 500 mV offset
   temperature = (temperature - 0.5) * 100;             //for getting temperature in  degree celcius ((voltage - 500mV) multiplied by 100)
   temperature_celcius = temperature;
  Serial.println(temperature_celcius);

   //setting temperature value range in between 0 to 99)
   if ( temperature_celcius < 0 )
   {
       temperature_celcius = 0;                                         // for the values less than 0 it is equal to 0
   }
   else if ( temperature_celcius > 99)
   {
       temperature_celcius = 99;                                        // for values greater than 99 temperature_celcius value will be eual to 99
   }
   else
   {
       temperature_celcius = temperature_celcius ;
   }

   //indication of heat and cold using LEDs

   if (temperature_celcius <= current_minimum_threshold)
   {
       digitalWrite(Red_LED_pin, HIGH);
       digitalWrite(Blue_LED_pin, LOW);
   }
   else if (temperature_celcius >= current_maximum_threshold)
   {
       digitalWrite(Red_LED_pin, LOW);
       digitalWrite(Blue_LED_pin, HIGH);
   }
   else
   {
       digitalWrite(Red_LED_pin, LOW);
       digitalWrite(Blue_LED_pin, LOW);
   }
}
//function for pushbutton to displaying and setting new current maximum temperature
void ButtonCheckmax(void)
{

   setpin1_state = digitalRead(Maximum_temperature_Setpin1);
   if (setpin1_state == 1)
   {
      flag1++;                                                                // whenever the setpin1_state goes high(button pressed) a counter called flag1 will increment by 1
   }
   if (flag1 == 1)                                                            // if counter value equal to 1 the system changes its state from TMP36Value to TemperatureMaxThresh
   {
      ButtonCheckDown();                                                     //at the same time by using ButtonCheckDown and ButtonCheckUp function we can increment or decrement the current maximum temperature
      ButtonCheckUp();
      current_state = TemperatureMaxThresh;
      time_in_current_state = 0;
   }
   else if (flag1 == 2)                                                     //if the counter value equal to 2 the system returns back to TMP36Value state also current_maximum_threshold contains the last updated value
   {
      current_state = TMP36Value ;
      time_in_current_state = 0;
      flag1 = 0;                                                            //counter reset to 0
   }
}

//function for incrementing the current maximum and minimum temperature
void ButtonCheckUp(void)
{
   increment_pin1_state = digitalRead(Temperature_increment_Pin1);
   if (flag1 == 1 && increment_pin1_state == 1 )
   {
       current_maximum_threshold++;                                         //if the flag1 which is incremented when the Maximum_temperature_Setpin1 button pressed, which is equal to 1
   }                                                                        // if the Temperature_increment_Pin1 is pressed the increment_pin1_state will become 1.if both are 1
   else if (flag2 == 1 && increment_pin1_state == 1)                       //the current_maximum_threshold incremented by 1
   {
       current_minimum_threshold++;                                        //if the flag1 which is incremented when the Maximum_temperature_Setpin1 button pressed,which is equal to 1
   }                                                                       // if the Temperature_decrement_Pin1 is pressed the increment_pin1_state will become 1.if both are 1
}                                                                         //the current_minimum_threshold incremented by 1

//function for pushbutton to displaying and setting new current minimum temperature
void ButtonCheckDown(void)
{
   decrement_pin2_state = digitalRead(Temperature_decrement_Pin2);
   if (flag1 == 1 && decrement_pin2_state == 1 )                          //same function as that of ButtonCheckUp,instead of incrementing currentmaximum and minimum threshold will decremented
   {  
      current_maximum_threshold--;
   }
   else if (flag2 == 1 && decrement_pin2_state == 1)
   {
      current_minimum_threshold--;
   }
}
void ButtonCheckmin(void)
{
   setpin2_state = digitalRead(Minimum_temperature_Setpin2);
   if (setpin2_state == 1)
   {
      flag2++;
   }
   if (flag2 == 1)
   {
      ButtonCheckDown();                                                   // same function as that of ButtonCheckmax here we use flag2 as counter, if flag 2 equal to 1 the current state
      ButtonCheckUp();
      current_state = TemperatureMinThresh;                                  //changes from TMP36Value to TemperatureMinThresh and if flag2 equal to 2 viceversa
      time_in_current_state = 0;
   }
   else if (flag2 == 2)
   {
      current_state = TMP36Value;
      time_in_current_state = 0;
      flag2 = 0;
   }
}

//task 3 function
void  update_checkButtonsanddisplay()
{
   ButtonCheckmax();
   ButtonCheckmin();
   bool change = false;
   // Handle the timing to switch between states.
   time_in_current_state++;
   if (time_in_current_state == times[current_state])
   {
      change = true;
      time_in_current_state = 0;
   }
  switch (current_state)
  {
      case TemperatureMinThresh:
        displayNumber(current_minimum_threshold);                             //sevensegment function calling to display current minimum threshold
        if (change) current_state = TMP36Value ;                              //if Minimum_temperature_Setpin2 button pressed current state changes to TMP36Value
        break;
      case TMP36Value:
        displayNumber(temperature_celcius);                                  //stay on TMP36Value state untile a button press occur
        break;
      case TemperatureMaxThresh:
        displayNumber(current_maximum_threshold);
        if (change) current_state = TMP36Value;                             // if Maximum_temperature_Setpin1 button pressed current state changes to TMP36Value
        break;
  }
}

//function for seven segment display
void displayNumber(int n)
{
   int t = n;
   p = (t / 10);
   l = (t - ((t / 10) * 10));
   digitalWrite(Latch_pin, LOW);
   shiftOut(Data_pin, Clock_pin, LSBFIRST, data [p]);
   shiftOut(Data_pin, Clock_pin, LSBFIRST, data [l]);
   digitalWrite(Latch_pin, HIGH);
}
void loop()
{
  //do something else
}
