//define wifi
#include <WiFi.h>
#define WIFI_SSID "Y O Y O"
#define WIFI_PASSWORD "yoyo1617"

//define firebase
#include <FirebaseESP32.h>
#define FIREBASE_HOST "https://testaja-b89a4-default-rtdb.firebaseio.com/"
#define FIREBASE_AUTH "QQ6cZmnVyGfKm2djRfqM0MZqMpHfeG8e6UFKA3LL"
FirebaseData firebaseData;

//define soil moisture sensor
#define soilMoisturePin_1 32
#define soilMoisturePin_2 33
#define soilMoisturePin_3 34
#define soilMoisturePin_4 35
#define soilMoisturePin_5 25
#define soilMoisturePin_6 26

//define dht11 sensor
#include <DHT.h>
#define DHTTYPE DHT11
#define DHTPIN 15
DHT dht(DHTPIN, DHTTYPE);

//define relay
#define relayPin1 0
#define relayPin2 4
#define relayPin3 12
#define relayPin4 13
#define relayPin5 14

void setup() {
  Serial.begin(9600);
  Serial.print("Watering Control System");

  // Initialize DHT11
  dht.begin();
  
  // Initialize Relay
  pinMode(relayPin1, OUTPUT);
  pinMode(relayPin2, OUTPUT);
  pinMode(relayPin3, OUTPUT);
  pinMode(relayPin4, OUTPUT);
  pinMode(relayPin5, OUTPUT);

  // Connect to WIFI
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.println("Menghubungkan");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println(".");
    delay(500);
  }
  Serial.println();
  Serial.println("Terhubung:");
  Serial.println(WiFi.localIP());

  // Initialize Firebase
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
}

//set default soil moisture
int sm1, sm2, sm3, sm4, sm5, sm6 = 0;

//set low and high level soil moisture
const int AirValue = 4095;
const int WaterValue = 1250;

//set value for parameter otomatis
int low = 10;
int high = 50;

void loop() {
  // Humidtiy n Temperature DHT11
  float h= dht.readHumidity(); //baca kelembapan
  float t = dht.readTemperature(); //baca suhu

  // Soil Moisture Sensor 1
  int sm1 = analogRead(soilMoisturePin_1);
  int percentageValue1 = map(sm1, AirValue, WaterValue, 0, 100);
  Serial.print("SM1: ");
  Serial.print(sm1);
  Serial.print("%");

  // Soil Moisture Sensor 2
  int sm2 = analogRead(soilMoisturePin_2);
  int percentageValue2 = map(sm2, AirValue, WaterValue, 0, 100);
  Serial.print("  SM2: ");
  Serial.print(sm2);
  Serial.print("%");

  // Soil Moisture Sensor 3
  int sm3 = analogRead(soilMoisturePin_3);
  int percentageValue3 = map(sm3, AirValue, WaterValue, 0, 100);
  Serial.print("  SM3: ");
  Serial.print(sm3);
  Serial.print("%");

  // Soil Moisture Sensor 4
  int sm4 = analogRead(soilMoisturePin_4);
  int percentageValue4 = map(sm4, AirValue, WaterValue, 0, 100);
  Serial.print("  SM4: ");
  Serial.print(sm4);
  Serial.print("%");

  // Soil Moisture Sensor 5
  int sm5 = analogRead(soilMoisturePin_5);
  int percentageValue5 = map(sm5, AirValue, WaterValue, 0, 100);
  Serial.print("  SM5: ");
  Serial.print(sm5);
  Serial.print("%");

  // Soil Moisture Sensor 6
  int sm6 = analogRead(soilMoisturePin_6);
  int percentageValue6 = map(sm6, AirValue, WaterValue, 0, 100);
  Serial.print("  SM6: ");
  Serial.print(sm6);
  Serial.print("%");

  // Calculate the average soil moisture
  int averageSM = (percentageValue1 + percentageValue2 + percentageValue3 + percentageValue4 + percentageValue5 + percentageValue6) / 6;

  //print average sm
  Serial.print("  Average SM: ");
  Serial.print(averageSM);
  Serial.print("%");

  // DHT11 not read
  if (isnan(h) || isnan(t)) { //jika tidak ada hasil
    Serial.println("Sensor DHT11 tidak terbaca... !");
    return;
  }
  Serial.print("  Suhu: ");  //kirim serial "Suhu"
  Serial.print(t);     //kirim serial nilai suhu
  Serial.print("*C");    //kirim serial "C" Celcius

  Serial.print("  Kelembapan: ");  //kirim serial "Humi"
  Serial.print(h);     //kirim serial nilai kelembaban
  Serial.print("%");  //kirim serial "%RH"

  //Sent t to firebase
  Firebase.setFloat(firebaseData, "/Suhu", t);
  
  //Sent h to firebase
  Firebase.setFloat(firebaseData, "/Kelembapan", h);
  
  //Sent sm to firebase
  Firebase.setInt(firebaseData, "/SoilMoisture", averageSM);

  // Message Successfully
  Serial.println("  Successfully Sent!");

  // Read manual and auto mode from Firebase
  Firebase.getString(firebaseData, "/manualMode");
  bool isManualMode = firebaseData.stringData().equals("true");

  Firebase.getString(firebaseData, "/autoMode");
  bool isAutoMode = firebaseData.stringData().equals("true");

  Firebase.getString(firebaseData, "/timer/timerMode");
  bool isTimerMode = firebaseData.stringData().equals("true");

  Firebase.getString(firebaseData, "/penyiramanMode");
  bool isPenyiramanMode = firebaseData.stringData().equals("true");

  Firebase.getString(firebaseData, "/manualModePestisida");
  bool isPestisidaMode = firebaseData.stringData().equals("true");

  // Logika Control
  if(isPenyiramanMode){
    if (isTimerMode) {
      Firebase.getString(firebaseData, "/timer/duration");
      long countdownValue = firebaseData.stringData().toInt();
      // Mengonfigurasi relay berdasarkan nilai countdown
      if (countdownValue == 0) {
        digitalWrite(relayPin3, HIGH);
        digitalWrite(relayPin5, HIGH);  // Relay dalam kondisi HIGH
      } else if (countdownValue == -1) {
        digitalWrite(relayPin3, LOW);
        digitalWrite(relayPin5, LOW);   // Relay dalam kondisi LOW
      } else {
        digitalWrite(relayPin3, LOW);
        digitalWrite(relayPin5, LOW);
      }
    } else if (isAutoMode) {
      // Logika penyiraman otomatis
      if (averageSM <= low) {
        digitalWrite(relayPin3, HIGH);
        digitalWrite(relayPin5, HIGH); // Aktifkan relay
      } else if (averageSM >= high) {
        digitalWrite(relayPin3, LOW);
        digitalWrite(relayPin5, LOW); // Matikan relay
      }
    } else if (isManualMode) {
      Firebase.getString(firebaseData, "/manualControl");
      if (strcmp(firebaseData.stringData().c_str(), "on") == 0) {
          digitalWrite(relayPin3, HIGH);
          digitalWrite(relayPin5, HIGH); // Aktifkan relay manual
      } else {
          digitalWrite(relayPin3, LOW);
          digitalWrite(relayPin5, LOW); // Matikan relay manual
      } // Aktifkan relay manual
    } else {
      digitalWrite(relayPin3, LOW);
      digitalWrite(relayPin5, LOW);
    }
  } else{
    digitalWrite(relayPin1, LOW);
    digitalWrite(relayPin2, HIGH);
    digitalWrite(relayPin3, LOW);
  }
  if(isPestisidaMode){
    Firebase.getString(firebaseData, "/manualControlPestisida");
    if (strcmp(firebaseData.stringData().c_str(), "on") == 0) {
      digitalWrite(relayPin4, HIGH);
      digitalWrite(relayPin5, HIGH); // Aktifkan relay manual
    } else if(strcmp(firebaseData.stringData().c_str(), "off") == 0) {
      digitalWrite(relayPin4, LOW);
      digitalWrite(relayPin5, LOW); // Matikan relay manual
    }
  } else {
    digitalWrite(relayPin1, HIGH);
    digitalWrite(relayPin2, LOW);
    digitalWrite(relayPin4, LOW);
  }

  delay(1000);
}