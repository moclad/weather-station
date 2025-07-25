/*----------------------------------------------------------------------------------------------------
  Project Name : Solar Powered WiFi Weather Station V2.31
  Features: temperature, dewpoint, dewpoint spread, heat index, humidity, absolute pressure, relative pressure, battery status and
  the famous Zambretti Forecaster (multi lingual)
  Authors: Keith Hungerford, Debasish Dutta and Marc Stähli
  Website : www.opengreenenergy.com

----------------------------------------------------------------------------------------------------*/

char ssid[] = "sbrubbles";            // WiFi Router ssid
char pass[] = "99835070995297020670"; // WiFi Router password

const char *mqtt_server = "sbrubbles-srv3";
const char *mqtt_user = "moclad";
const char *mqtt_password = "masterkey_mosquitto";
const char *mqtt_client = "sbrubbles-garten-2";

const char *sensor_name = "sbrubbles-garten-2";

const char neofelis_server_address[] = "neofelis.sbrubbles.duckdns.org";
const char neofelis_path[] = "/v1/sensors";
const int neofelis_port = 443;

const char root_ca [] PROGMEM = R"CERT(
    -----BEGIN CERTIFICATE-----
    MIIDzTCCA1SgAwIBAgISBvHtTgKg1byR4/2UkamAuSFEMAoGCCqGSM49BAMDMDIx
    CzAJBgNVBAYTAlVTMRYwFAYDVQQKEw1MZXQncyBFbmNyeXB0MQswCQYDVQQDEwJF
    NTAeFw0yNTA1MTYyMDUyNDRaFw0yNTA4MTQyMDUyNDNaMCIxIDAeBgNVBAMMFyou
    c2JydWJibGVzLmR1Y2tkbnMub3JnMHYwEAYHKoZIzj0CAQYFK4EEACIDYgAEFXmf
    +33NeNyaW+ZjwQ6QbppeHFN9VltvBXta5JjysKOmdhjAXLUk4F0lkX3L+bCS6mBP
    xu0mtxwjfjXAQjsjijJ88cUosCak0bUunJjrKX4TokqKyRaHup4xkeXIL6yOo4IC
    OzCCAjcwDgYDVR0PAQH/BAQDAgeAMB0GA1UdJQQWMBQGCCsGAQUFBwMBBggrBgEF
    BQcDAjAMBgNVHRMBAf8EAjAAMB0GA1UdDgQWBBSh3KG5feUlxUKI7E323UFHfbql
    mjAfBgNVHSMEGDAWgBSfK1/PPCFPnQS37SssxMZwi9LXDTAyBggrBgEFBQcBAQQm
    MCQwIgYIKwYBBQUHMAKGFmh0dHA6Ly9lNS5pLmxlbmNyLm9yZy8wOQYDVR0RBDIw
    MIIXKi5zYnJ1YmJsZXMuZHVja2Rucy5vcmeCFXNicnViYmxlcy5kdWNrZG5zLm9y
    ZzATBgNVHSAEDDAKMAgGBmeBDAECATAtBgNVHR8EJjAkMCKgIKAehhxodHRwOi8v
    ZTUuYy5sZW5jci5vcmcvNjkuY3JsMIIBAwYKKwYBBAHWeQIEAgSB9ASB8QDvAHYA
    7TxL1ugGwqSiAFfbyyTiOAHfUS/txIbFcA8g3bc+P+AAAAGW2xR8ewAABAMARzBF
    AiEArnrX2qJtE4vnzKup4QA2p+Dq8ugGds9keBcRQ6Gh1vsCIDwugkyvOwv+YX6N
    PrnkRkf6DnXfK1dX6IZpeJQNeBT4AHUAEvFONL1TckyEBhnDjz96E/jntWKHiJxt
    MAWE6+WGJjoAAAGW2xSEXAAABAMARjBEAiAqzhwMUogE0uMZ1a01RFKLh1SZbxat
    7HGF3qNj4rCaYgIgHeYSV4AErBcsXch63BDAKGyLiD12PqiTaFFkJ8KuL3QwCgYI
    KoZIzj0EAwMDZwAwZAIwemGfeZZgW/++WsatojRWhRCANBjyoUtnWjh7Itn2F5rg
    D6j7+EJ5uZ/81jgtoVqaAjB4Yj4Z7UtrXRJ3Ya4W5ttPApDiWdfUzXlfHwTN8ux6
    90TrzBhLdCBVBZ1wYZxiznU=
    -----END CERTIFICATE-----
)CERT";    

#define TEMP_CORR (0)
#define ELEVATION (290)

#define sleepTimeMin (20)

#define LANGUAGE 2

// NTP
#define NTP_SERVER "pool.ntp.org"
#define TZ 0      // (utc+) TZ in hours
#define DST_MN 60 // use 60mn for summer time in some countries

#define TZ_SEC ((TZ) * 3600)    // don't change this
#define DST_SEC ((DST_MN) * 60) // don't change this
