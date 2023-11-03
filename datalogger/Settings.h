/*----------------------------------------------------------------------------------------------------
  Project Name : Solar Powered WiFi Weather Station V2.31
  Features: temperature, dewpoint, dewpoint spread, heat index, humidity, absolute pressure, relative pressure, battery status and
  the famous Zambretti Forecaster (multi lingual)
  Authors: Keith Hungerford, Debasish Dutta and Marc Stähli
  Website : www.opengreenenergy.com

----------------------------------------------------------------------------------------------------*/

char ssid[] = "xxxx";            // WiFi Router ssid
char pass[] = "xxxx"; // WiFi Router password

const char *mqtt_server = "xxxx";
const char *mqtt_user = "xxxx";
const char *mqtt_password = "xxxx";
const char *mqtt_client = "xxxx";

const char *sensor_name = "xxxx";

const char neofelis_server_address[] = "xxxx";
const char neofelis_path[] = "/v1/sensors";
const int neofelis_port = 9000;

const char dweet_server_address[] = "dweet.io";  // server address
const int dweet_port = 80;

#define TEMP_CORR (0)   
#define ELEVATION (290) 

#define sleepTimeMin (21) 

#define LANGUAGE 'DE'

// NTP
#define NTP_SERVER "pool.ntp.org"
#define TZ 0      // (utc+) TZ in hours
#define DST_MN 60 // use 60mn for summer time in some countries

#define TZ_SEC ((TZ)*3600)    // don't change this
#define DST_SEC ((DST_MN)*60) // don't change this

