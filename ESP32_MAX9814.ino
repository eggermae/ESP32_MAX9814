/* sources for program are
 *  https://github.com/acrobotic/Ai_Demos_ESP32/tree/master/vu_meter
 *  
 *  
  */

#define BLYNK_PRINT Serial
#include <BlynkSimpleEsp32.h>
#include "WiFi.h"
#include "WiFiConfig.h"
#include "esp_system.h"


// include a watchdog timer to restart the esp32 if not feeded every 10 s
const int wdtTimeout = 10000;  //time in ms to trigger the watchdog
hw_timer_t *timer_ = NULL;

void IRAM_ATTR resetModule() {
  //ets_printf("reboot\n");
  esp_restart();
}

#include<Filter.h>

// define necessary parameters
#define MIC_PIN   32

#define NOISE 550


// define the variables needed for the audio levels
int lvl = 0, minLvl = 0, maxLvl = 300; // tweak the min and max as needed

// instantiate the filter class for smoothing the raw audio signal
ExponentialFilter<long> ADCFilter(5,0);


char auth[]="rFtcBtKkEjrkcXasE2p7LhzOCn33emeF";

BlynkTimer timer;

// Use Virtual pin 5 for uptime display
#define ACC V5


// This function tells Arduino what to do if there is a Widget
// which is requesting data for Virtual Pin (5)
BLYNK_READ(ACC)
{
  // read the audio signal and filter it
  int n, height;
  n = analogRead(MIC_PIN);
  // remove the MX9614 bias of 1.25VDC
  n = abs(1023 - n);
  // hard limit noise/hum
  n = (n <= NOISE) ? 0 : abs(n - NOISE);
  // apply the exponential filter to smooth the raw signal
  ADCFilter.Filter(n);
  lvl = ADCFilter.Current();
  Blynk.virtualWrite(ACC,lvl);
}

void myTimerEvent()
{


  
  // read the audio signal and filter it
  int n, height;
  n = analogRead(MIC_PIN);
  // remove the MX9814 bias of 1.25VDC
  n = abs(1023 - n);
  // hard limit noise/hum
  n = (n <= NOISE) ? 0 : abs(n - NOISE);
  // apply the exponential filter to smooth the raw signal
  ADCFilter.Filter(n);
  lvl = ADCFilter.Current();
  Blynk.virtualWrite(ACC,lvl);
  //Serial.println(lvl);
}

void setup_wifi(){
    delay(10);
    WiFi.disconnect();
    //WiFi.begin(ssid, password);
    Blynk.begin(auth, ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
    }
  }

void setup() {
  Serial.begin(9600);
  Blynk.begin(auth, ssid, password);
  // Setup a function to be called every second
  timer.setInterval(100L, myTimerEvent);

// use without Blynk to connect to Wifi
// Wifi setup
/*  //Serial.begin(115200);
  //WiFi.mode(WIFI_STA);
  WiFi.begin(ssid,password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
    Serial.println(ssid);
  }
  Serial.println("Connected to the Wifi network");
  */

// watchdog timer
  timer_ = timerBegin(0, 80, true);//timer 0, div 80, ESP32 counters is 80 MHz, divide by 80 to get 1us
  timerAttachInterrupt(timer_, &resetModule, true);  //attach callback
  timerAlarmWrite(timer_, wdtTimeout * 1000000, false); //set time in s
  timerAlarmEnable(timer_);                          //enable interrupt
  
}

unsigned long check_wifi = 30000;

void loop() {
  /*if (accel.available()) {      // Wait for new data from accelerometer
    // Acceleration of x, y, and z directions in g units
    Serial.print(accel.getCalculatedX(), 3);
    Serial.print("\t");
    Serial.print(accel.getCalculatedY(), 3);
    Serial.print("\t");
    Serial.print(accel.getCalculatedZ(), 3);
    Serial.println();
  }*/
  if ((WiFi.status() != WL_CONNECTED) && (millis() > check_wifi)) {
    setup_wifi();
    check_wifi = millis() + 30000;
  }
  timerWrite(timer_,0);
  Blynk.run();
  timer.run();
}
