#include <Arduino.h>
#include <DHT.h>
bool dht_data_valid = false;

//---------DHT sensor (I/O digital - GPIO)--------//
#define DHTPIN 14     // Digital pin connected to the DHT sensor --> GPIO14 (
#define DHTTYPE DHT11 // DHT 11
//#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321

#define DHT_SAMPLING_RATE	5000	//5s

DHT dht(DHTPIN, DHTTYPE);
bool dht_en = false;

unsigned long dht_sampling;
unsigned char num_dht_err = 0;
float dht_tem, dht_hum;

bool read_dht();

/////////////////////////////////////////////////////////////
double avergearray(int* arr, int number){
  int i;
  int max,min;
  double avg;
  long amount = 0;
  
  if(number <= 0){
    Serial.println("Error number for the array to avraging!/n");
    return 0;
  }
  if(number < 5){   //less than 5, calculated directly statistics
    for(i = 0; i < number; i++){
      amount += arr[i];
    }
    avg = amount / number;
    return avg;
  }
  else{
    if(arr[0] < arr[1]){
      min = arr[0];
      max = arr[1];
    }
    else{
      min = arr[1];
      max = arr[0];
    }
    for(i = 2; i < number; i++){
      if(arr[i] < min){
        amount += min;        //arr < min
        min = arr[i];
      }
      else if(arr[i] > max){
        amount += max;    //arr > max
        max = arr[i];
      }
      else{
        amount += arr[i]; //min <= arr <= max
      }      
    }//for
    avg = (double)amount/(number-2);
  }//if
  return avg;
}