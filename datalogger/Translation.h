/*----------------------------------------------------------------------------------------------------
  Project Name : Solar Powered WiFi Weather Station V2.31
  Features: temperature, dewpoint, dewpoint spread, heat index, humidity, absolute pressure, relative pressure, battery status and
  the famous Zambretti Forecaster (multi lingual)
  Authors: Keith Hungerford, Debasish Dutta and Marc Stähli
  Website : www.opengreenenergy.com

******* Transaltion tables ****************************************/

#if LANGUAGE == 'EN'
const char TEXT_AIR_PRESSURE[] = "Pressure";
const char TEXT_RISING_FAST[] = "Rising fast";
const char TEXT_RISING[] = "Rising";
const char TEXT_RISING_SLOW[] = "Rising slow";
const char TEXT_STEADY[] = "Steady";
const char TEXT_FALLING_SLOW[] = "Falling slow";
const char TEXT_FALLING[] = "Falling";
const char TEXT_FALLING_FAST[] = "Falling fast";

const char TEXT_ZAMBRETTI_FORECAST[] = "Forecast";
const char TEXT_ZAMBRETTI_ACCURACY[] = "Prediction accuracy";
const char TEXT_ZAMBRETTI_A[] = "Settled fine weather";
const char TEXT_ZAMBRETTI_B[] = "Fine weather";
const char TEXT_ZAMBRETTI_C[] = "Becoming fine";
const char TEXT_ZAMBRETTI_D[] = "Fine, becoming less settled";
const char TEXT_ZAMBRETTI_E[] = "Fine, possibly showers";
const char TEXT_ZAMBRETTI_F[] = "Fairly fine, improving";
const char TEXT_ZAMBRETTI_G[] = "Fairly fine, possibly showers early";
const char TEXT_ZAMBRETTI_H[] = "Fairly fine, showers later";
const char TEXT_ZAMBRETTI_I[] = "Showery early, improving";
const char TEXT_ZAMBRETTI_J[] = "Changeable, improving";
const char TEXT_ZAMBRETTI_K[] = "Fairly fine, showers likely";
const char TEXT_ZAMBRETTI_L[] = "Rather unsettled, clearing later";
const char TEXT_ZAMBRETTI_M[] = "Unsettled, probably improving";
const char TEXT_ZAMBRETTI_N[] = "Showery, bright intervals";
const char TEXT_ZAMBRETTI_O[] = "Showery, becoming unsettled";
const char TEXT_ZAMBRETTI_P[] = "Changeable, some rain";
const char TEXT_ZAMBRETTI_Q[] = "Unsettled, short fine intervals";
const char TEXT_ZAMBRETTI_R[] = "Unsettled, rain later";
const char TEXT_ZAMBRETTI_S[] = "Unsettled, rain at times";
const char TEXT_ZAMBRETTI_T[] = "Very unsettled, finer at times";
const char TEXT_ZAMBRETTI_U[] = "Rain at times, worse later";
const char TEXT_ZAMBRETTI_V[] = "Rain at times, becoming very unsettled";
const char TEXT_ZAMBRETTI_W[] = "Rain at frequent intervals";
const char TEXT_ZAMBRETTI_X[] = "Very unsettled, rain";
const char TEXT_ZAMBRETTI_Y[] = "Stormy, possibly improving";
const char TEXT_ZAMBRETTI_Z[] = "Stormy, much rain";
const char TEXT_ZAMBRETTI_0[] = "Battery empty, please recharge!";
const char TEXT_ZAMBRETTI_DEFAULT[] = "Sorry, no forecast at the moment";

#elif LANGUAGE == 'DE'
const char TEXT_AIR_PRESSURE[] = "Luftdruck";
const char TEXT_RISING_FAST[] = "rasch steigend";
const char TEXT_RISING[] = "steigend";
const char TEXT_RISING_SLOW[] = "langsam steigend";
const char TEXT_STEADY[] = "beständig";
const char TEXT_FALLING_SLOW[] = "langsam fallend";
const char TEXT_FALLING[] = "fallend";
const char TEXT_FALLING_FAST[] = "rasch fallend";

const char TEXT_ZAMBRETTI_FORECAST[] = "Wettervorhersage";
const char TEXT_ZAMBRETTI_ACCURACY[] = "Vorhersagegenauigkeit";
const char TEXT_ZAMBRETTI_A[] = "Beständiges Schönwetter";
const char TEXT_ZAMBRETTI_B[] = "Schönes Wetter";
const char TEXT_ZAMBRETTI_C[] = "Wetter wird gut";
const char TEXT_ZAMBRETTI_D[] = "Schön, wird wechselhaft";
const char TEXT_ZAMBRETTI_E[] = "Schön, Regenschauer möglich";
const char TEXT_ZAMBRETTI_F[] = "Ziemlich gut, verbessert sich";
const char TEXT_ZAMBRETTI_G[] = "Ziemlich gut, frühe Regenschauer möglich";
const char TEXT_ZAMBRETTI_H[] = "Ziemlich gut, spätere Regenschauer";
const char TEXT_ZAMBRETTI_I[] = "Früh schauerhaft, verbessert sich";
const char TEXT_ZAMBRETTI_J[] = "Wechselhaft, verbessert sich";
const char TEXT_ZAMBRETTI_K[] = "Ziemlich gut, Regenschauer möglich";
const char TEXT_ZAMBRETTI_L[] = "Eher veränderlich, klart später auf";
const char TEXT_ZAMBRETTI_M[] = "Veränderlich, verbessert sich wahrscheinlich";
const char TEXT_ZAMBRETTI_N[] = "Regnerisch mit Aufhellungen";
const char TEXT_ZAMBRETTI_O[] = "Regnerisch, wird veränderlich";
const char TEXT_ZAMBRETTI_P[] = "Veränderlich mit wenig Regen";
const char TEXT_ZAMBRETTI_Q[] = "Veränderlich, mit kurzen schönen Intervallen";
const char TEXT_ZAMBRETTI_R[] = "Veränderlich, später Regen";
const char TEXT_ZAMBRETTI_S[] = "Veränderlich, zeitweise Regen";
const char TEXT_ZAMBRETTI_T[] = "Stark wechselnd, zeitweise schöner";
const char TEXT_ZAMBRETTI_U[] = "Zeitweise Regen, verschlechtert sich";
const char TEXT_ZAMBRETTI_V[] = "Zeitweise Regen, wird sehr unruhig";
const char TEXT_ZAMBRETTI_W[] = "Regen in regelmässigen Abständen";
const char TEXT_ZAMBRETTI_X[] = "Sehr veränderlich, Regen";
const char TEXT_ZAMBRETTI_Y[] = "Stürmisch, verbessert sich wahrscheinlich";
const char TEXT_ZAMBRETTI_Z[] = "Stürmisch, viel Regen";
const char TEXT_ZAMBRETTI_0[] = "Batterie leer, bitte nachladen!";
const char TEXT_ZAMBRETTI_DEFAULT[] = "Im Moment keine Prognose möglich";
#endif
