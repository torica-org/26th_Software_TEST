#include <Arduino.h>
#include <Wire.h>

/* for SDP3X */
#include <Arduino.h>
#include <SensirionI2CSdp.h>
#include <Wire.h>

SensirionI2CSdp sdp;

uint8_t sdp31_init(void) {
  uint16_t error;
  char errorMessage[256];

  sdp.begin(Wire, SDP3X_I2C_ADDRESS_2);  // SDP3Xを選択＆I2Cアドレスが0x23なのでADDRESS_2で

  uint32_t productNumber;
  uint8_t serialNumber[8];
  uint8_t serialNumberSize = 8;

  sdp.stopContinuousMeasurement();

  error = sdp.readProductIdentifier(productNumber, serialNumber,
                                    serialNumberSize);
  if (error) {
    Serial.print("Error trying to execute readProductIdentifier(): ");
    errorToString(error, errorMessage, 256);
    Serial.println(errorMessage);
  } else {
    Serial.print("ProductNumber:");
    Serial.print(productNumber);
    Serial.print("\t");
    Serial.print("SerialNumber:");
    Serial.print("0x");
    for (size_t i = 0; i < serialNumberSize; i++) {
      Serial.print(serialNumber[i], HEX);
    }
    Serial.println();
  }

  error = sdp.startContinuousMeasurementWithDiffPressureTCompAndAveraging();

  // 正常に初期化できたらerror=0
  if (error) {
    Serial.print(
      "Error trying to execute "
      "startContinuousMeasurementWithDiffPressureTCompAndAveraging(): ");
    errorToString(error, errorMessage, 256);
    Serial.println(errorMessage);
  }

  return error;  // 正常な場合はerror = 0
}



uint16_t read_sdp31(float &diff_Pressure, float &temp) {
  uint16_t error;
  char errorMessage[256];

  error = sdp.readMeasurement(diff_Pressure, temp);

  if (error) {
    Serial.print("Error trying to execute readMeasurement(): ");
    errorToString(error, errorMessage, 256);
    Serial.println(errorMessage);
  }

  return error;  // 正常な場合はerror = 0
}



/* for BMP3XX */
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP3XX.h>

#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BMP3XX bmp;

bool BMP3XX_init(void) {
  if (!bmp.begin_I2C()) {
    Serial.println("BMP3XX init error");
    return false;
  }

  // Set up oversampling and filter initialization
  bmp.setTemperatureOversampling(BMP3_NO_OVERSAMPLING);
  bmp.setPressureOversampling(BMP3_NO_OVERSAMPLING);
  bmp.setIIRFilterCoeff(BMP3_IIR_FILTER_DISABLE);
  bmp.setOutputDataRate(BMP3_ODR_100_HZ);

  return true;
}


float read_bmp(float &temperature, float &pressure){
if (! bmp.performReading()) {
  Serial.println("Failed to perform reading :(");
  return 0.0;
  }
  
  temperature = bmp.temperature; // ℃で返す
  pressure = bmp.pressure / 100.0; // hPaで返す

  // Serial.print(bmp.readAltitude(SEALEVELPRESSURE_HPA));
  // Serial.println(" m");
}


// float pressure_altitude(pressure_alt){
//   return bmp.readAltitude(SEALEVELPRESSURE_HPA);
// }


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Wire.begin();
  Wire.setClock(400000);

  sdp31_init();
  BMP3XX_init();
}

void loop() {
  // put your main code here, to run repeatedly:
  float diff_Pressure;
  float sdp_temperature;
  float bmp_temperature;
  float bmp_pressure;
  float airspeed;

  read_sdp31(diff_Pressure, sdp_temperature);
  read_bmp(bmp_temperature, bmp_pressure);

  Serial.print("airspeed: ");
  airspeed = sqrt(abs(2.0 * diff_Pressure * ((bmp_temperature + 273.15) / (bmp_pressure * 100.0)) * 287.026));
  Serial.println(airspeed);

  delay(10);
}
