#include "FrameWeb.h"
FrameWeb frame;

#include <WebSocketsServer.h>
#include <HTTPClient.h>

#include <Arduino.h>
#include <SPI.h>
#include "SparkFunBME280.h"

// Server
#define SERVER "opadegp-datagateway.cern.ch"
#define PORT 8080
#define PAGE "/setvalue/set?"

// Debug macro 
#define DEBUG_MAIN

// Variable used in this sample
BME280 sensor01;
BME280 sensor02;
BME280 sensor03;
BME280 sensor04;
float h01, h02, h03, h04;
float t01, t02, t03, t04;
float p01, p02, p03, p04;
const char VERSION[] ="0.0.1";
int8_t wifiLost = 0;
uint32_t wdCounter = 0;
int second =  0;
HTTPClient http;
char temp[1024];

// Internal led
#define EspLedBlue 2
long previousMillis = 0;

// Frame option config and web-socket NOT USE
void saveConfigCallback() {}
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {}

#define sendGet(format, ...) { \
  snprintf(temp, 1024, format, __VA_ARGS__); \
  String url =  String(PAGE) + String(temp); \
  http.begin(SERVER, PORT, url); \
  int httpCode = http.GET(); \
  http.end(); \
}

// -------- WatchDog ----------------------------
void watchdog(void *pvParameter) {
  while (1) {
    vTaskDelay(5000/portTICK_RATE_MS); // Wait 5 sec
    wdCounter++;
    if (wdCounter > 400) { // 
      // We have a problem no connection if crash or waitting 
      if (wdCounter == 401 ) {
      } else {
        // Perhapse force ??? WiFi.begin(ssid, password);
        ESP.restart(); // Restart after 5sec * 180 => 15min
        delay(2000);
      }
    }
  }
}

//  configModeCallback callback when entering into AP mode
void configModeCallback (WiFiManager *myWiFiManager) {
  #ifdef DEBUG_MAIN
    Serial.printf("Mode Access Point is running MAC:%s \n\r", WiFi.macAddress().c_str());
  #endif
}

// setup -------------------------------------------------------------------------
void setup() {

  Serial.begin(115200);
#ifdef DEBUG_MAIN
  Serial.printf("Start setup Ver:%s\n\r",VERSION);
#endif

  // Set pin mode  I/O Directions
  pinMode(EspLedBlue, OUTPUT);     // Led is BLUE at statup
  digitalWrite(EspLedBlue, HIGH);  // After 5 seconds blinking indicate WiFI ids OK

   // Start WatchDog used to reset AP evey 15m (Some time after general cut off Wifi host is started after Eps)
  xTaskCreate(&watchdog, "wd task", 2048, NULL, 5, NULL);
  
  // Start Html framework
  frame.setup();

  // Append /websocket access html 
  // frame.externalHtmlTools="Specific home page is visible at :<a class='button' href='/websocket.html'>Web Socket Demo</a>";

  if (sensor01.beginSPI(5) == false) {
    Serial.println("Sensor01 fault");
    while(1);
  }
  if (sensor02.beginSPI(4) == false) {
    Serial.println("Sensor01 fault");
    while(1);
  }
  if (sensor03.beginSPI(16) == false) {
    Serial.println("Sensor01 fault");
    while(1);
  }
  if (sensor04.beginSPI(17) == false) {
    Serial.println("Sensor01 fault");
    while(1);
  }

  #ifdef DEBUG_MAIN
    Serial.printf("End setup MAC:%s IPLocal:%s \n\r",WiFi.macAddress().c_str(), WiFi.localIP().toString().c_str() );
  #endif
}

// Main loop -----------------------------------------------------------------
void loop() {

  // Call Html_frame loop
  frame.loop();

  // Is alive executed every 1 sec.
  if ( millis() - previousMillis > 1000L) {
    previousMillis = millis();
    digitalWrite(EspLedBlue, !digitalRead(EspLedBlue));

    int wifistat = WiFi.status();
    // if wifi is down, try reconnecting every 60 seconds
    if (wifistat != WL_CONNECTED) {
      wifiLost++;
      if (wifiLost==10) {
        #ifdef DEBUG_MAIN
          Serial.printf("-WiFi Lost:%s wifiLost:%d sec. localIP:%s", frame.wifiStatus(wifistat), wifiLost, WiFi.localIP().toString().c_str() );
        #endif
      }
      if (wifiLost == 50) {
        #ifdef DEBUG_MAIN
          Serial.printf("-WiFi disconnect OK after 50s (%s).", frame.wifiStatus(wifistat));
        #endif
        WiFi.disconnect();
      }
      if (wifiLost == 60) {
        if (WiFi.reconnect()) {
          wifistat = WL_CONNECTED;
        }
      }
    } else {
      wdCounter = 0;
      wifiLost = 0;
    }

    // read some sensors
    if (second==0) {
      h01 = sensor01.readFloatHumidity();
      h02 = sensor02.readFloatHumidity();
      h03 = sensor03.readFloatHumidity();
      h04 = sensor04.readFloatHumidity();
      p01 = sensor01.readFloatPressure();
      p02 = sensor02.readFloatPressure();
      p03 = sensor03.readFloatPressure();
      p04 = sensor04.readFloatPressure();
      t01 = sensor01.readTempC();
      t02 = sensor02.readTempC();
      t03 = sensor03.readTempC();
      t04 = sensor04.readTempC();
    }

    // Notify
    if (second==1) {
      sendGet("dcr_temp01=%g", t01);
      sendGet("dcr_temp02=%g", t02);
      sendGet("dcr_temp03=%g", t03);
      sendGet("dcr_temp04=%g", t04);
    }
    else if (second==2) {
      sendGet("dcr_rh01=%g", h01);
      sendGet("dcr_rh02=%g", h02);
      sendGet("dcr_rh03=%g", h03);
      sendGet("dcr_rh04=%g", h04);
    }
    else if (second==4) {
      sendGet("dcr_ap01=%g", p01);
      sendGet("dcr_ap02=%g", p02);
      sendGet("dcr_ap03=%g", p03);
      sendGet("dcr_ap04=%g", p04);
    }
    
    second++;
    if (second>4) {
      second=0;
    }

  } // End 1 second

} // End loop
