// Menambahkan library sensor DHT
#include <SimpleDHT.h>

// Menambahkan library LCD
#include <LiquidCrystal_I2C.h>

// Mendefinsikan alamat pin
#define pinLED1  D0 // LED kuning
#define pinLED2  D3 // LED Orange
#define pinLED3  D4 // LED Hijau
#define pinLED4  D5 // LED Biru
#define pinLED5  D6 // LED Merah
#define pinDHT   D7 // SD3 pin signal sensor DHT

// Membuat kustom karakter derajat
// https://maxpromer.github.io/LCD-Character-Creator/
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

// Deklarasi variabel global
byte temperature = 0;
byte humidity = 0;

// Deklarasi instance object constructor DHT11
SimpleDHT11 dht11(pinDHT);
LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  // Mengatur baudrate serial Monitor
  Serial.begin(115200);

  // Mengatur mode pin LED sebagai output
  pinMode(pinLED1, OUTPUT);
  pinMode(pinLED2, OUTPUT);
  pinMode(pinLED3, OUTPUT);
  pinMode(pinLED4, OUTPUT);
  pinMode(pinLED5, OUTPUT);

  // Mengatur baudrate serial Monitor
  Serial.begin(115200);

  // Mengatur mode pin DHT sebagai input
  pinMode(pinDHT, INPUT);

  // Menampilkan pesan string pada LCD
  lcd.begin();                // Inisialisasi LCD
  lcd.clear();                // Bersihkan layar
  lcd.createChar(0, derajat);
  lcd.setCursor(0, 0);        // Kursor X,Y
  lcd.print("Humidity");      // Tampilkan ke layar
  lcd.setCursor(0, 1);
  lcd.print("Sensor");

  // Tunda untuk tampil selama 5 detik
  delay(5000);

  // Bersihkan layat
  lcd.clear();
}

void loop() {
  // Memanggil fungsi suhu dan kelembaban
  // yang akan dipanggil berulang-ulang
  KelembabanSuhu();
}

void KelembabanSuhu()
{
  int err = SimpleDHTErrSuccess;

  if ((err = dht11.read(&temperature, &humidity, NULL)) != SimpleDHTErrSuccess)
  {
    Serial.print("Pembacaan DHT11 gagal, err=");
    Serial.println(err);
    delay(1000);
    return;
  }

  // Tampilkan hasil pembacaan sensor ke serial monitor
  Serial.print("Temperatur: ");
  Serial.print((int)temperature);
  Serial.print(" *C, - ");
  Serial.print((int)humidity);
  Serial.println(" H");

  // misal jika suhu lebi dari 30 derajat celcius
  if (temperature > 30 ) {
    AnimLED1();
  } else {
    AnimLED2();
  }
  
  // Tampilkan hasil pembacaan sensor ke LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Temp: " + String((int)temperature));
  lcd.write(0); // Simbol derajat
  lcd.print("C ");
  lcd.setCursor(0, 1);
  lcd.print("Humi: " + String((int)humidity) + "H");
  
  // Tunda pembacaan setiap 2 detik
  // kemampuan baca DHT11 idealnya adalah 1,5 detik
  delay(2000);
}

void AnimLED1() {
  //buat animasi led
  digitalWrite(pinLED1, HIGH);
  digitalWrite(pinLED2, LOW);
  // dan seterusnya
}

void AnimLED2() {
  //buat animasi led
  digitalWrite(pinLED1, LOW);
  digitalWrite(pinLED2, HIGH);
  // dan seterusnya
}
