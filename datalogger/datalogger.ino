#include "Settings.h"
#include "Translation.h"

#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <ArduinoHttpClient.h>
#include <Arduino_JSON.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <WiFiUdp.h>
#include <FS.h>
// #include <SPIFFS.h>
#include <EasyNTPClient.h>  //https://github.com/aharshac/EasyNTPClient
#include <TimeLib.h>        //https://github.com/PaulStoffregen/Time.git
#include <PubSubClient.h>   // For MQTT (in this case publishing only)

Adafruit_BME280 bme;  // I2C
WiFiUDP udp;
EasyNTPClient ntpClient(udp, NTP_SERVER, TZ_SEC + DST_SEC);

float measured_temp;
float adjusted_temp;
float measured_humi;
float adjusted_humi;
float measured_pres;
float SLpressure_hPa;  // needed for rel pressure calculation
float HeatIndex;       // Heat Index in °C
float volt;
int rel_pressure_rounded;
int accuracy_in_percent;
float DewpointTemperature;
float DewPointSpread;  // Difference between actual temperature and dewpoint

// FORECAST CALCULATION
unsigned long current_timestamp;  // Actual timestamp read from NTPtime_t now;
unsigned long saved_timestamp;    // Timestamp stored in SPIFFS

float pressure_value[12];       // Array for the historical pressure values (6 hours, all 30 mins)
                                // where as pressure_value[0] is always the most recent value
float pressure_difference[12];  // Array to calculate trend with pressure differences

// FORECAST RESULT
int accuracy;              // Counter, if enough values for accurate forecasting
String ZambrettisWords;    // Final statement about weather forecast
String trend_in_words;     // Trend in words
String forecast_in_words;  // Weather forecast in words
String pressure_in_words;  // Air pressure in words
String accuracy_in_words;  // Zambretti's prediction accuracy in words

void (*resetFunc)(void) = 0;  // declare reset function @ address 0

WiFiClient wifiClient;                   // MQTT (non-secure)
WiFiClient mqttClient;                   // MQTT (non-secure)
WiFiClientSecure httpsClient;            // HTTPS client for secure connections
PubSubClient mqttPubClient(mqttClient);  // MQTT
X509List cert(root_ca);

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("Start of Sbrubbles Weather Station V3.0");

  // Voltage divider R1 = 220k+100k+220k =540k and R2=100k
  float calib_factor = 4.2;  // change this value to calibrate the battery voltage
  unsigned long raw = analogRead(A0);
  volt = raw * calib_factor / 4095;

  Serial.print("Voltage = ");
  Serial.print(volt, 2);  // print with 2 decimal places
  Serial.println(" V");

  connectWifi();
  connectToMQTT();
  checkSpiffsInitialization();
  connectToNtp();
  measurementEvent();
  writeSpiffsOperations();
  calculateZambrettiWords();
  sendDataToServer();
  pushDataToMQTT();
  goToSleep(sleepTimeMin);
}

void loop() {  // loop is not used
}

void checkSpiffsInitialization() {
  Serial.println("SPIFFS Initialization: ");

  boolean mounted = SPIFFS.begin();  // load config if it exists. Otherwise use defaults.
  if (!mounted) {
    Serial.println("FS not formatted. Doing that now...");
    SPIFFS.format();
    Serial.println("FS formatted...");
    SPIFFS.begin();
  }
}

void pushDataToMQTT() {
  Serial.println("---> Push data to MQTT");

  char _adjusted_temp[8];
  dtostrf(adjusted_temp, 4, 1, _adjusted_temp);

  bool result = mqttPubClient.publish("sbrubbles/garten-2/temperature", _adjusted_temp, 1);
  if (!result) {
    Serial.println("---> BAD MQTT");
  }
  delay(50);


  char _adjusted_humi[8];                        // Buffer big enough for 7-character float
  dtostrf(adjusted_humi, 3, 1, _adjusted_humi);  // Leave room for too large numbers!

  mqttPubClient.publish("sbrubbles/garten-2/humidity", _adjusted_humi, 1);  // ,1 = retained
  delay(50);

  char _measured_pres[8];                        // Buffer big enough for 7-character float
  dtostrf(measured_pres, 3, 0, _measured_pres);  // Leave room for too large numbers!

  mqttPubClient.publish("sbrubbles/garten-2/abs_pressure", _measured_pres, 1);  // ,1 = retained
  delay(50);

  char _rel_pressure_rounded[8];                               // Buffer big enough for 7-character float
  dtostrf(rel_pressure_rounded, 3, 0, _rel_pressure_rounded);  // Leave room for too large numbers!

  mqttPubClient.publish("sbrubbles/garten-2/rel_pressure", _rel_pressure_rounded, 1);  // ,1 = retained
  delay(50);

  char _volt[8];               // Buffer big enough for 7-character float
  dtostrf(volt, 3, 2, _volt);  // Leave room for too large numbers!

  mqttPubClient.publish("sbrubbles/garten-2/batterry", _volt, 1);  // ,1 = retained
  delay(50);

  char _DewpointTemperature[8];                              // Buffer big enough for 7-character float
  dtostrf(DewpointTemperature, 3, 1, _DewpointTemperature);  // Leave room for too large numbers!

  mqttPubClient.publish("sbrubbles/garten-2/dewpoint", _DewpointTemperature, 1);  // ,1 = retained
  delay(50);

  char _HeatIndex[8];                    // Buffer big enough for 7-character float
  dtostrf(HeatIndex, 3, 1, _HeatIndex);  // Leave room for too large numbers!

  mqttPubClient.publish("sbrubbles/garten-2/heatindex", _HeatIndex, 1);  // ,1 = retained
  delay(50);

  char _accuracy_in_percent[8];                              // Buffer big enough for 7-character float
  dtostrf(accuracy_in_percent, 3, 0, _accuracy_in_percent);  // Leave room for too large numbers!

  mqttPubClient.publish("sbrubbles/garten-2/accuracy", _accuracy_in_percent, 1);  // ,1 = retained
  delay(50);

  char _DewPointSpread[8];                         // Buffer big enough for 7-character float
  dtostrf(DewPointSpread, 3, 1, _DewPointSpread);  // Leave room for too large numbers!

  mqttPubClient.publish("sbrubbles/garten-2/dewpointspread", _DewPointSpread, 1);  // ,1 = retained
  delay(50);

  char tmp1[128];
  ZambrettisWords.toCharArray(tmp1, 128);
  mqttPubClient.publish("sbrubbles/garten-2/zambrettisays", tmp1, 1);
  delay(50);

  char tmp2[128];
  trend_in_words.toCharArray(tmp2, 128);
  mqttPubClient.publish("sbrubbles/garten-2/trendinwords", tmp2, 1);
  delay(50);

  char _trend[8];                                  // Buffer big enough for 7-character float
  dtostrf(pressure_difference[11], 3, 2, _trend);  // Leave room for too large numbers!

  mqttPubClient.publish("sbrubbles/garten-2/trend", _trend, 1);  // ,1 = retained
  delay(50);
}

void sendDataToServer() {
  Serial.println("---> Send Data to Server");

  JSONVar doc;

  doc["sensor"] = sensor_name;
  doc["time"] = current_timestamp;

  doc["temperature"] = adjusted_temp;
  doc["humidity"] = adjusted_humi;
  doc["pressure"] = measured_pres;
  doc["rel_pressure"] = rel_pressure_rounded;
  doc["volt"] = volt;
  doc["battery"] = volt;
  doc["dew_point"] = DewpointTemperature;
  doc["heat_index"] = HeatIndex;
  doc["zambretti"] = ZambrettisWords;
  doc["accuracy"] = accuracy_in_percent;
  doc["trend"] = trend_in_words;
  doc["dew_point_spread"] = DewPointSpread;

  String contentType = "application/json";
  String jsonString = JSON.stringify(doc);

  Serial.println("---> Sbrubbles Server");

  // Check if we're using HTTPS (port 443 or if explicitly needed)
  bool useHTTPS = (neofelis_port == 443) || (String(neofelis_server_address).indexOf("https") >= 0);

  if (useHTTPS) {
    Serial.println("Using HTTPS connection");
    // Configure HTTPS client
     httpsClient.setInsecure(); // Skip certificate validation (use only for testing)
    //httpsClient.setTrustAnchors(&cert);
    // For production, use: httpsClient.setCACert(rootCA); with proper certificate

    HttpClient sbrubbles_client = HttpClient(httpsClient, neofelis_server_address, neofelis_port);
    sbrubbles_client.post(neofelis_path, contentType, jsonString);

    int neofelis_status_code = sbrubbles_client.responseStatusCode();
    Serial.println(sbrubbles_client.responseBody());

    Serial.print("Neofelis Status code: ");
    Serial.println(neofelis_status_code);
  } else {
    Serial.println("Using HTTP connection");
    HttpClient sbrubbles_client = HttpClient(wifiClient, neofelis_server_address, neofelis_port);
    sbrubbles_client.post(neofelis_path, contentType, jsonString);

    int neofelis_status_code = sbrubbles_client.responseStatusCode();
    Serial.println(sbrubbles_client.responseBody());

    Serial.print("Neofelis Status code: ");
    Serial.println(neofelis_status_code);
  }
}

void calculateZambrettiWords() {
  accuracy_in_percent = accuracy * 94 / 12;  // 94% is the max predicion accuracy of Zambretti
  ZambrettisWords = ZambrettiSays(char(ZambrettiLetter()));
  forecast_in_words = TEXT_ZAMBRETTI_FORECAST;
  pressure_in_words = TEXT_AIR_PRESSURE;
  accuracy_in_words = TEXT_ZAMBRETTI_ACCURACY;

  Serial.println("********************************************************");
  Serial.print("Zambretti says: ");
  Serial.print(ZambrettisWords);
  Serial.print(", ");
  Serial.println(trend_in_words);
  Serial.print("Prediction accuracy: ");
  Serial.print(accuracy_in_percent);
  Serial.println("%");

  if (accuracy < 12) {
    Serial.println("Reason: Not enough weather data yet.");
    Serial.print("We need ");
    Serial.print((12 - accuracy) / 2);
    Serial.println(" hours more to get sufficient data.");
  }

  Serial.println("********************************************************");
}

void writeSpiffsOperations() {
  ReadFromSPIFFS();  // read stored values and update data if more recent data is available

  Serial.print("Timestamp difference: ");
  Serial.println(current_timestamp - saved_timestamp);

  if (current_timestamp - saved_timestamp > 21600) {  // last save older than 6 hours -> re-initialize values
    FirstTimeRun();
  } else if (current_timestamp - saved_timestamp > 1800) {  // it is time for pressure update (1800 sec = 30 min)

    for (int i = 11; i >= 1; i = i - 1) {
      pressure_value[i] = pressure_value[i - 1];  // shifting values one to the right
    }

    pressure_value[0] = rel_pressure_rounded;  // updating with acutal rel pressure (newest value)

    if (accuracy < 12) {
      accuracy = accuracy + 1;  // one value more -> accuracy rises (up to 12 = 100%)
    }
    WriteToSPIFFS(current_timestamp);  // update timestamp on storage
  } else {
    WriteToSPIFFS(saved_timestamp);  // do not update timestamp on storage
  }
}

void connectWifi() {
  WiFi.mode(WIFI_STA);
  WiFi.setHostname("sbrubbles-sensor-2");  // This changes the hostname of the ESP8266 to display neatly on the network esp on router.

  WiFi.begin(ssid, pass);
  Serial.print("---> Connecting to WiFi ");
  int i = 0;

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);

    i++;
    if (i > 20) {
      //goToSleep(10);  // go to sleep and retry after 10 min
      Serial.println("---> Wifi connection not ok. Continue without Wifi");
      return;
    }

    Serial.print(".");
  }
  Serial.println(".");
  Serial.println("---> Wifi connected ok");
}


void connectToNtp() {
  //******** GETTING THE TIME FROM NTP SERVER  ***********************************

  Serial.println("---> Now reading time from NTP Server");
  int ii = 0;
  while (!ntpClient.getUnixTime()) {
    delay(100);
    ii++;
    if (ii > 20) {
      Serial.println("Could not connect to NTP Server. Continue without NTP Time.");
      //resetFunc();
      return;
    }
    Serial.print(".");
  }
  current_timestamp = ntpClient.getUnixTime();  // get UNIX timestamp (seconds from 1.1.1970 on)

  Serial.print("Current UNIX Timestamp: ");
  Serial.println(current_timestamp);

  Serial.print("Time & Date: ");
  Serial.print(hour(current_timestamp));
  Serial.print(":");
  Serial.print(minute(current_timestamp));
  Serial.print(":");
  Serial.print(second(current_timestamp));
  Serial.print("; ");
  Serial.print(day(current_timestamp));
  Serial.print(".");
  Serial.print(month(current_timestamp));  // needed later: month as integer for Zambretti calcualtion
  Serial.print(".");
  Serial.println(year(current_timestamp));
}

void connectToMQTT() {
  Serial.print("---> Connecting to MQTT . ");

  mqttPubClient.setServer(mqtt_server, 1883);
  mqttPubClient.connect(mqtt_client, mqtt_user, mqtt_password);

  int failed = 0;
  while (!mqttPubClient.connected() && failed < 5) {
    delay(500);

    // Attempt to connect
    if (mqttPubClient.connect(mqtt_client, mqtt_user, mqtt_password)) {
      Serial.println("connected");
    } else {
      Serial.print(".");
      failed++;
    }
  }

  if (failed > 4) {
    Serial.println("Failed to connect to MQTT. Continue without MQTT.");
    return;
  }

  Serial.println(".");
  Serial.println("---> Wifi connected ok");

  mqttPubClient.publish("sbrubbles/garten-2/status", "Sensor started", 1);
}


void measurementEvent() {
  bool bme_status;
  bme_status = bme.begin(0x76);  // address either 0x76 or 0x77
  if (!bme_status) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
  }

  Serial.println("---> Measuring...");
  Serial.println("forced mode, 1x temperature / 1x humidity / 1x pressure oversampling,");
  Serial.println("filter off");

  bme.setSampling(Adafruit_BME280::MODE_FORCED,
                  Adafruit_BME280::SAMPLING_X1,  // temperature
                  Adafruit_BME280::SAMPLING_X1,  // pressure
                  Adafruit_BME280::SAMPLING_X1,  // humidity
                  Adafruit_BME280::FILTER_OFF);

  // Measures absolute Pressure, Temperature, Humidity, Voltage, calculate relative pressure,
  // Dewpoint, Dewpoint Spread, Heat Index

  bme.takeForcedMeasurement();

  // Get temperature
  measured_temp = bme.readTemperature();
  // print on serial monitor
  Serial.print("Temp: ");
  Serial.print(measured_temp);
  Serial.print("°C; ");

  // Get humidity
  measured_humi = bme.readHumidity();
  // print on serial monitor
  Serial.print("Humidity: ");
  Serial.print(measured_humi);
  Serial.print("%; ");

  // Get pressure
  measured_pres = bme.readPressure() / 100.0F;
  // print on serial monitor
  Serial.print("Pressure: ");
  Serial.print(measured_pres);
  Serial.print("hPa; ");

  // Calculate and print relative pressure
  SLpressure_hPa = (((measured_pres * 100.0) / pow((1 - ((float)(ELEVATION)) / 44330), 5.255)) / 100.0);
  rel_pressure_rounded = (int)(SLpressure_hPa + .5);
  // print on serial monitor
  Serial.print("Pressure rel: ");
  Serial.print(rel_pressure_rounded);
  Serial.print("hPa; ");

  // Calculate dewpoint
  float a = 17.625;
  float b = 243.04;

  float tempcalc = (a * measured_temp) / (b + measured_temp) + log(measured_humi * 0.01);

  DewpointTemperature = (b * tempcalc) / (a - tempcalc);
  Serial.print("Dewpoint: ");
  Serial.print(DewpointTemperature);
  Serial.println("°C; ");

  // With the dewpoint calculated we can correct temp and automatically calculate humidity
  adjusted_temp = measured_temp + TEMP_CORR;
  if (adjusted_temp < DewpointTemperature)
    adjusted_temp = DewpointTemperature;  // compensation, if offset too high

  // August-Roche-Magnus approximation (http://bmcnoldy.rsmas.miami.edu/Humidity.html)
  adjusted_humi = 100 * (exp((a * DewpointTemperature) / (b + DewpointTemperature)) / exp((a * adjusted_temp) / (b + adjusted_temp)));
  if (adjusted_humi > 100)
    adjusted_humi = 100;  // just in case

  Serial.print("Temp adjusted: ");
  Serial.print(adjusted_temp);
  Serial.print("°C; ");
  Serial.print("Humidity adjusted: ");
  Serial.print(adjusted_humi);
  Serial.print("%; ");

  // Calculate dewpoint spread (difference between actual temp and dewpoint -> the smaller the number: rain or fog

  DewPointSpread = adjusted_temp - DewpointTemperature;
  Serial.print("Dewpoint Spread: ");
  Serial.print(DewPointSpread);
  Serial.println("°C; ");

  // Calculate HI (heatindex in °C) --> HI starts working above 26,7 °C
  if (adjusted_temp > 26.7) {
    double c1 = -8.784, c2 = 1.611, c3 = 2.338, c4 = -0.146, c5 = -1.230e-2, c6 = -1.642e-2, c7 = 2.211e-3, c8 = 7.254e-4, c9 = -2.582e-6;
    double T = adjusted_temp;
    double R = adjusted_humi;

    double A = ((c5 * T) + c2) * T + c1;
    double B = ((c7 * T) + c4) * T + c3;
    double C = ((c9 * T) + c8) * T + c6;
    HeatIndex = (C * R + B) * R + A;
  } else {
    HeatIndex = adjusted_temp;
    Serial.println("Not warm enough (less than 26.7 °C) for Heatindex");
  }

  Serial.print("HeatIndex: ");
  Serial.print(HeatIndex);
  Serial.print("°C; ");

}  // end of void measurementEvent()

int CalculateTrend() {
  int trend;  // -1 falling; 0 steady; 1 raising
  Serial.println("---> Calculating trend");

  //--> giving the most recent pressure reads more weight
  pressure_difference[0] = (pressure_value[0] - pressure_value[1]) * 1.5;
  pressure_difference[1] = (pressure_value[0] - pressure_value[2]);
  pressure_difference[2] = (pressure_value[0] - pressure_value[3]) / 1.5;
  pressure_difference[3] = (pressure_value[0] - pressure_value[4]) / 2;
  pressure_difference[4] = (pressure_value[0] - pressure_value[5]) / 2.5;
  pressure_difference[5] = (pressure_value[0] - pressure_value[6]) / 3;
  pressure_difference[6] = (pressure_value[0] - pressure_value[7]) / 3.5;
  pressure_difference[7] = (pressure_value[0] - pressure_value[8]) / 4;
  pressure_difference[8] = (pressure_value[0] - pressure_value[9]) / 4.5;
  pressure_difference[9] = (pressure_value[0] - pressure_value[10]) / 5;
  pressure_difference[10] = (pressure_value[0] - pressure_value[11]) / 5.5;

  //--> calculating the average and storing it into [11]
  pressure_difference[11] = (pressure_difference[0] + pressure_difference[1] + pressure_difference[2] + pressure_difference[3] + pressure_difference[4] + pressure_difference[5] + pressure_difference[6] + pressure_difference[7] + pressure_difference[8] + pressure_difference[9] + pressure_difference[10]) / 11;

  Serial.print("Current trend: ");
  Serial.println(pressure_difference[11]);

  if (pressure_difference[11] > 3.5) {
    trend_in_words = TEXT_RISING_FAST;
    trend = 1;
  } else if (pressure_difference[11] > 1.5 && pressure_difference[11] <= 3.5) {
    trend_in_words = TEXT_RISING;
    trend = 1;
  } else if (pressure_difference[11] > 0.25 && pressure_difference[11] <= 1.5) {
    trend_in_words = TEXT_RISING_SLOW;
    trend = 1;
  } else if (pressure_difference[11] > -0.25 && pressure_difference[11] < 0.25) {
    trend_in_words = TEXT_STEADY;
    trend = 0;
  } else if (pressure_difference[11] >= -1.5 && pressure_difference[11] < -0.25) {
    trend_in_words = TEXT_FALLING_SLOW;
    trend = -1;
  } else if (pressure_difference[11] >= -3.5 && pressure_difference[11] < -1.5) {
    trend_in_words = TEXT_FALLING;
    trend = -1;
  } else if (pressure_difference[11] <= -3.5) {
    trend_in_words = TEXT_FALLING_FAST;
    trend = -1;
  }

  Serial.println(trend_in_words);
  return trend;
}

char ZambrettiLetter() {
  Serial.println("---> Calculating Zambretti letter");
  char z_letter;
  int(z_trend) = CalculateTrend();
  // Case trend is falling
  if (z_trend == -1) {
    float zambretti = 0.0009746 * rel_pressure_rounded * rel_pressure_rounded - 2.1068 * rel_pressure_rounded + 1138.7019;
    // A Winter falling generally results in a Z value lower by 1 unit
    if (month(current_timestamp) < 4 || month(current_timestamp) > 9)
      zambretti = zambretti + 1;
    if (zambretti > 9)
      zambretti = 9;
    Serial.print("Calculated and rounded Zambretti in numbers: ");
    Serial.println(round(zambretti));
    switch (int(round(zambretti))) {
      case 0:
        z_letter = 'A';
        break;  // Settled Fine
      case 1:
        z_letter = 'A';
        break;  // Settled Fine
      case 2:
        z_letter = 'B';
        break;  // Fine Weather
      case 3:
        z_letter = 'D';
        break;  // Fine Becoming Less Settled
      case 4:
        z_letter = 'H';
        break;  // Fairly Fine Showers Later
      case 5:
        z_letter = 'O';
        break;  // Showery Becoming unsettled
      case 6:
        z_letter = 'R';
        break;  // Unsettled, Rain later
      case 7:
        z_letter = 'U';
        break;  // Rain at times, worse later
      case 8:
        z_letter = 'V';
        break;  // Rain at times, becoming very unsettled
      case 9:
        z_letter = 'X';
        break;  // Very Unsettled, Rain
    }
  }
  // Case trend is steady
  if (z_trend == 0) {
    float zambretti = 138.24 - 0.133 * rel_pressure_rounded;
    Serial.print("Calculated and rounded Zambretti in numbers: ");
    Serial.println(round(zambretti));
    switch (int(round(zambretti))) {
      case 0:
        z_letter = 'A';
        break;  // Settled Fine
      case 1:
        z_letter = 'A';
        break;  // Settled Fine
      case 2:
        z_letter = 'B';
        break;  // Fine Weather
      case 3:
        z_letter = 'E';
        break;  // Fine, Possibly showers
      case 4:
        z_letter = 'K';
        break;  // Fairly Fine, Showers likely
      case 5:
        z_letter = 'N';
        break;  // Showery Bright Intervals
      case 6:
        z_letter = 'P';
        break;  // Changeable some rain
      case 7:
        z_letter = 'S';
        break;  // Unsettled, rain at times
      case 8:
        z_letter = 'W';
        break;  // Rain at Frequent Intervals
      case 9:
        z_letter = 'X';
        break;  // Very Unsettled, Rain
      case 10:
        z_letter = 'Z';
        break;  // Stormy, much rain
    }
  }
  // Case trend is rising
  if (z_trend == 1) {
    float zambretti = 142.57 - 0.1376 * rel_pressure_rounded;
    // A Summer rising, improves the prospects by 1 unit over a Winter rising
    if (month(current_timestamp) >= 4 && month(current_timestamp) <= 9)
      zambretti = zambretti - 1;
    if (zambretti < 0)
      zambretti = 0;
    Serial.print("Calculated and rounded Zambretti in numbers: ");
    Serial.println(round(zambretti));
    switch (int(round(zambretti))) {
      case 0:
        z_letter = 'A';
        break;  // Settled Fine
      case 1:
        z_letter = 'A';
        break;  // Settled Fine
      case 2:
        z_letter = 'B';
        break;  // Fine Weather
      case 3:
        z_letter = 'C';
        break;  // Becoming Fine
      case 4:
        z_letter = 'F';
        break;  // Fairly Fine, Improving
      case 5:
        z_letter = 'G';
        break;  // Fairly Fine, Possibly showers, early
      case 6:
        z_letter = 'I';
        break;  // Showery Early, Improving
      case 7:
        z_letter = 'J';
        break;  // Changeable, Improving
      case 8:
        z_letter = 'L';
        break;  // Rather Unsettled Clearing Later
      case 9:
        z_letter = 'M';
        break;  // Unsettled, Probably Improving
      case 10:
        z_letter = 'Q';
        break;  // Unsettled, short fine Intervals
      case 11:
        z_letter = 'T';
        break;  // Very Unsettled, Finer at times
      case 12:
        z_letter = 'Y';
        break;  // Stormy, possibly improving
      case 13:
        z_letter = 'Z';
        break;
        ;  // Stormy, much rain
    }
  }
  char *tmp1 = &z_letter;
  mqttPubClient.publish("sbrubbles/garten-2/zletter", tmp1, 1);  // ,1 = retained
  delay(50);
  Serial.print("This is Zambretti's famous letter: ");
  Serial.println(z_letter);
  return z_letter;
}

String ZambrettiSays(char code) {
  String zambrettis_words = "";
  switch (code) {
    case 'A':
      zambrettis_words = TEXT_ZAMBRETTI_A;
      break;  // see Tranlation.h
    case 'B':
      zambrettis_words = TEXT_ZAMBRETTI_B;
      break;
    case 'C':
      zambrettis_words = TEXT_ZAMBRETTI_C;
      break;
    case 'D':
      zambrettis_words = TEXT_ZAMBRETTI_D;
      break;
    case 'E':
      zambrettis_words = TEXT_ZAMBRETTI_E;
      break;
    case 'F':
      zambrettis_words = TEXT_ZAMBRETTI_F;
      break;
    case 'G':
      zambrettis_words = TEXT_ZAMBRETTI_G;
      break;
    case 'H':
      zambrettis_words = TEXT_ZAMBRETTI_H;
      break;
    case 'I':
      zambrettis_words = TEXT_ZAMBRETTI_I;
      break;
    case 'J':
      zambrettis_words = TEXT_ZAMBRETTI_J;
      break;
    case 'K':
      zambrettis_words = TEXT_ZAMBRETTI_K;
      break;
    case 'L':
      zambrettis_words = TEXT_ZAMBRETTI_L;
      break;
    case 'M':
      zambrettis_words = TEXT_ZAMBRETTI_M;
      break;
    case 'N':
      zambrettis_words = TEXT_ZAMBRETTI_N;
      break;
    case 'O':
      zambrettis_words = TEXT_ZAMBRETTI_O;
      break;
    case 'P':
      zambrettis_words = TEXT_ZAMBRETTI_P;
      break;
    case 'Q':
      zambrettis_words = TEXT_ZAMBRETTI_Q;
      break;
    case 'R':
      zambrettis_words = TEXT_ZAMBRETTI_R;
      break;
    case 'S':
      zambrettis_words = TEXT_ZAMBRETTI_S;
      break;
    case 'T':
      zambrettis_words = TEXT_ZAMBRETTI_T;
      break;
    case 'U':
      zambrettis_words = TEXT_ZAMBRETTI_U;
      break;
    case 'V':
      zambrettis_words = TEXT_ZAMBRETTI_V;
      break;
    case 'W':
      zambrettis_words = TEXT_ZAMBRETTI_W;
      break;
    case 'X':
      zambrettis_words = TEXT_ZAMBRETTI_X;
      break;
    case 'Y':
      zambrettis_words = TEXT_ZAMBRETTI_Y;
      break;
    case 'Z':
      zambrettis_words = TEXT_ZAMBRETTI_Z;
      break;
    case '0':
      zambrettis_words = TEXT_ZAMBRETTI_0;
      break;
    default:
      zambrettis_words = TEXT_ZAMBRETTI_DEFAULT;
      break;
  }
  return zambrettis_words;
}

void ReadFromSPIFFS() {
  char filename[] = "/data.txt";
  File myDataFile = SPIFFS.open(filename, "r");  // Open file for reading
  if (!myDataFile) {
    Serial.println("Failed to open file");
    FirstTimeRun();  // no file there -> initializing
  }

  Serial.println("---> Now reading from SPIFFS");

  String temp_data;

  temp_data = myDataFile.readStringUntil('\n');
  saved_timestamp = temp_data.toInt();
  Serial.print("Timestamp from SPIFFS: ");
  Serial.println(saved_timestamp);

  temp_data = myDataFile.readStringUntil('\n');
  accuracy = temp_data.toInt();
  Serial.print("Accuracy value read from SPIFFS: ");
  Serial.println(accuracy);

  Serial.print("Last 12 saved pressure values: ");
  for (int i = 0; i <= 11; i++) {
    temp_data = myDataFile.readStringUntil('\n');
    pressure_value[i] = temp_data.toInt();
    Serial.print(pressure_value[i]);
    Serial.print("; ");
  }
  myDataFile.close();
  Serial.println();
}

void WriteToSPIFFS(int write_timestamp) {
  char filename[] = "/data.txt";
  File myDataFile = SPIFFS.open(filename, "w");  // Open file for writing (appending)
  if (!myDataFile) {
    Serial.println("Failed to open file");
  }

  Serial.println("---> Now writing to SPIFFS");

  myDataFile.println(write_timestamp);  // Saving timestamp to /data.txt
  myDataFile.println(accuracy);         // Saving accuracy value to /data.txt

  for (int i = 0; i <= 11; i++) {
    myDataFile.println(pressure_value[i]);  // Filling pressure array with updated values
  }

  myDataFile.close();

  Serial.println("File written. Now reading file again.");
  myDataFile = SPIFFS.open(filename, "r");  // Open file for reading
  Serial.print("Found in /data.txt = ");
  while (myDataFile.available()) {
    Serial.print(myDataFile.readStringUntil('\n'));
    Serial.print("; ");
  }
  Serial.println();
  myDataFile.close();
}

void FirstTimeRun() {
  Serial.println("---> Starting initializing process.");
  accuracy = 1;
  char filename[] = "/data.txt";
  File myDataFile = SPIFFS.open(filename, "w");  // Open a file for writing
  if (!myDataFile) {
    Serial.println("Failed to open file");
    Serial.println("Stopping process - maybe flash size not set (SPIFFS).");
    exit(0);
  }
  myDataFile.println(current_timestamp);  // Saving timestamp to /data.txt
  myDataFile.println(accuracy);           // Saving accuracy value to /data.txt
  for (int i = 0; i < 12; i++) {
    myDataFile.println(rel_pressure_rounded);  // Filling pressure array with current pressure
  }
  Serial.println("** Saved initial pressure data. **");
  myDataFile.close();
  Serial.println("---> Doing a reset now.");
  resetFunc();  // call reset
}

void closeMQTTConnection(unsigned int sleepmin) {
  if (!mqttPubClient.connected()) {
    return;
  }

  char tmp[128];
  String sleepmessage = "Taking a nap for " + String(sleepmin) + " Minutes";
  sleepmessage.toCharArray(tmp, 128);
  mqttPubClient.publish("sbrubbles/garten-2/sleep", tmp);

  delay(50);

  Serial.println("INFO: Closing the MQTT connection");
  mqttPubClient.disconnect();
  delay(50);
}

void closeWifiConnection() {
  Serial.println("INFO: Closing the Wifi connection");
  WiFi.disconnect();

  while (mqttPubClient.connected() || (WiFi.status() == WL_CONNECTED)) {
    Serial.println("Waiting for shutdown before sleeping");
    delay(10);
  }
}

void goToSleep(unsigned int sleepmin) {
  closeMQTTConnection(sleepmin);
  closeWifiConnection();

  Serial.print("Going to sleep now for ");
  Serial.print(sleepmin);
  Serial.print(" Minute(s).");
  ESP.deepSleep(sleepmin * 60 * 1000000);  // convert to microseconds

  //delay(1 * 60 * 1000000);
  //resetFunc();

  Serial.println("Going to sleep for a while.");
}