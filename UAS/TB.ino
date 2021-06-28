/*------------------------------------------------
IoT Smart Device Development Board
by Dodit Suprianto | DSP-TECH
https://doditsuprianto.blogspot.com/
https://doditsuprianto.gitbook.io/dsp-tech/
https://github.com/doditsuprianto
Email: doditsuprianto@gmail.com
------------------------------------------------
Library Link & Credit:
1. https://github.com/bblanchon/ArduinoJson
2. https://github.com/winlinvip/SimpleDHT
3. https://github.com/kiryanenko/SimpleTimer
------------------------------------------------*/
#include <Arduino.h>
#include <SimpleDHT.h> // library sensor DHT
#include <LiquidCrystal_I2C.h> // library LCD I2C
#include <SimpleTimer.h> // thread millis
#include <ArduinoJson.h>
#include <ESP8266WiFi.h> // library ESP8266 Wifi
#include <PubSubClient.h> // library MQTT Message
#define pinLEDMerah 0 // atau pin D3
#define pinLEDHijau 2 // atau pin D4
#define pinLEDBiru 14 // atau pin D5
#define pinDHT D1
#define pinLDR D0 // pin analog
/*-------------------------------------
Buffering memory untuk
Serialisasi String ke JSON Library
-------------------------------------*/
#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE];
DynamicJsonDocument dhtData(1024);
/*-----------------
Variable global
------------------*/
// variabel object LCD I2C
LiquidCrystal_I2C lcd(0x27, 16, 2);
// variable global simple timer
SimpleTimer TimerDHT, TimerLDR;
// Konstruktor instance Sensor DHT11
SimpleDHT11 dht11(pinDHT);
/*---------------------------------------------------
Login dan Password Access Point jaringan internet
----------------------------------------------------*/
const char* wifiName = "instan speedy"; // sesuaikan
const char* wifiPass = "@F1ndels21"; // sesuaikan
/*-------------------------------------------
Login dan Password ke Message Broker
sesuaikan konfigurasiya mengi
-------------------------------------------*/
const char* brokerUser = "fidelatrisaktiyani01@gmail.com"; // sesuaikan
const char* brokerPass = "5d99558a"; //sesuaikan
const char* brokerHost = "192.168.210.62";
/*--------------------------------------------
Daftar nama Topic MQTT sebagai Publisher:
1. Publisher DHT11: Suhu & Kelembaband9
2. Publisher LDR: Intensitas Cahaya
--------------------------------------------*/
const char* outTopicDHT = "/dht"; // suhu dan kelembaban
const char* outTopicLDR = "/ldr"; // intensitas cahaya ()
byte suhu, hum; // Deklarasi variable suhu dan kelembaban
WiFiClient espClient; // Deklaasi client wifi
PubSubClient client(espClient); // Deklarasi MQTT Client
long lastReconnectAttempt = 0;
//membuat kustom karakter simbol derajat
//https://maxpromer.github.io/LCD-Character-Creator/
byte derajat[] = {
B01110,
B10001,
B10001,
B10001,
B01110,
B00000,
B00000,
B00000
};
void setup() {
// pengaturan baudrate serial monitor
Serial.begin(115200);
// menentukan mode pin
pinMode(pinLEDMerah, OUTPUT);
pinMode(pinLEDHijau, OUTPUT);
pinMode(pinLEDBiru, OUTPUT);
pinMode(pinDHT, INPUT);
pinMode(pinLDR, INPUT);
/*-------------------------------------
Atur interval pengecekan LDR dan DHT
-------------------------------------*/
TimerDHT.setInterval(1500); // interval 1,5 detik
TimerLDR.setInterval(1000); // interval 1 detik
/*-----------------------------------
Koneksi jaringan WIFI Access Point
-----------------------------------*/
KoneksiWIFI();
/*----------------------------
Koneksi TCP ke Broker MQTT
----------------------------*/
client.setServer(brokerHost, 1883);
lcd.begin(); // inisialisasi LCD, atau gunakan lcd.init()
lcd.clear(); // hapus semua tampilan layar LCD
lcd.createChar(0, derajat);
lcd.backlight(); // hidupkan latar layar
// menapilkan string ke LCD
lcd.setCursor(0, 0); lcd.print("Projek");
lcd.setCursor(0, 1); lcd.print("Pertemuan 12");
// tunda selama 4 detik
delay(4000);
// hapus isi layar LCD
lcd.clear();
}
void loop() {
/*---------------------------------------
Koneksi ulang ke broker jika terputus
---------------------------------------*/
if (!client.connected()) {
reconnect();
}
client.loop();
// memaanggil function pembacaan sensor DHT11
// untuk suhu dan kelembaban setiap 1.5 detik
if (TimerDHT.isReady()) {
SensorDHT(); // memanggil fungsi suhu & kelembaban
updateLCD(); // mengupdate tampilan pada LCD
// data dalam bentuk array
dhtData["suhu"] = suhu;
dhtData["kelembaban"] = hum;
// serialisasi data dari array menjadi format JSON
char buffer[256];
size_t n = serializeJson(dhtData, buffer);
client.publish(outTopicDHT, buffer, n);
// menghidupkan atau mematikan LED Hijau atau LED merah
// jika kelembaban berada di bawah atau di atas 60
if (hum >= 60) {
digitalWrite(pinLEDHijau, HIGH); // LED Hijau hidup
digitalWrite(pinLEDMerah, LOW); // LED Merah mati
} else {
digitalWrite(pinLEDHijau, LOW); // LED hijau mati
digitalWrite(pinLEDMerah, HIGH); // LED merah mati
}
TimerDHT.reset(); // reset counter = 0
}
if (TimerLDR.isReady()) {
// kirim data LDR ke message broker
snprintf (msg, MSG_BUFFER_SIZE, "%d", SensorLDR());
client.publish(outTopicLDR, msg);
updateLCD(); // mengupdate tampilan pada LCD, sekaligus membaca sensor LDR
// menghidupkan atau mematikan LED biru
// jika intensitas cahaya berada di atas atau di bawah 100 LUX
if (SensorLDR() >= 100) {
digitalWrite(pinLEDBiru, HIGH);
} else {
digitalWrite(pinLEDBiru, LOW);
}
TimerLDR.reset(); // reset counter = 0
}
}
/*------------------------------------
Fungsi membaca suhu dan kelembaban
dengan sensor DHT11
-------------------------------------*/
void SensorDHT() {
suhu = 0;
hum = 0;
int err = SimpleDHTErrSuccess;
if ((err = dht11.read(&suhu, &hum, NULL)) !=
SimpleDHTErrSuccess)
{
Serial.print("Read DHT11 failed, err="); Serial.println(err);
delay(100);
return;
}
Serial.print("Sample OK: ");
Serial.print((int)suhu); Serial.print(" *C, ");
Serial.print((int)hum); Serial.println(" H");
}
/*-------------------------------------
Fungsi menghitung intensitas cahaya
dengan sensor LDR secara analog
-------------------------------------*/
int SensorLDR() {
int nilaiAnalogLDR = analogRead(pinLDR);
/*--------------------------------------------------------------
------------------
Referensi perhitung Lux
https://arduinodiy.wordpress.com/2013/11/03/measuring-lightwith-an-arduino/
https://emant.com/316002
VOut = nilaiAnalogLDR * (3.3 / 1023) = nilaiAnalogLDR *
0.0032258064516129
Perhitungan ini tidak dikalibrasi
--------------------------------------------------------------
------------------*/
double Vout = nilaiAnalogLDR * 0.0032258064516129;
int lux = 330 / (10 * ((3.3 - Vout) / Vout));
Serial.println("Lux Intensity= " + String(int(lux)));
return lux;
}
void updateLCD() {
lcd.clear();
// baris pertama LCD
lcd.setCursor(0, 0);
// menampilkan suhu
lcd.print("T:" + String(suhu)); // nilai suhu
lcd.write(0); // simbol derajat
lcd.print("C "); // Celcius
// menampilkan kelembaban
lcd.print("H:" + String(hum)); // nilai kelembaban
// baris kedua LCD
// menampilkan intensitas cahaya dalam satuan 'Lux'
lcd.setCursor(0, 1);
lcd.print("LDR:" + String(SensorLDR()) + "Lux");
}
/*----------------------------------------
Fungsi koneksi jaringan ke Access Point
----------------------------------------*/
void KoneksiWIFI() {
Serial.print("Connecting to ");
Serial.println(wifiName);
// Memposisikan NodeMCU sebagai station
// NodeMCU dihubungkan ke Access Point
WiFi.mode(WIFI_STA);
WiFi.begin(wifiName, wifiPass);
while (WiFi.status() != WL_CONNECTED) {
delay(500);
Serial.print(".");
}
// NodeMCU telah terhubung ke Access Point
Serial.println();
Serial.println("WiFi connected");
Serial.print("IP address: ");
Serial.println(WiFi.localIP());
}
void reconnect() {
while (!client.connected()) {
Serial.print("Attempting MQTT connection...");
// Attempt to connect
if (client.connect("ESP8266Client", brokerUser, brokerPass)) {
Serial.println("connected");
//client.subscribe("");
} else {
Serial.print("failed, rc=");
Serial.print(client.state());
Serial.println(" try again in 5 seconds");
delay(5000);
}
}
}
