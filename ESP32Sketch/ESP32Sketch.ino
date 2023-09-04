/**************************
  Aux Library
***************************/
// #include <SPI.h> // Untuk koneksi SPI
#include <Wire.h>   // Untuk koneksi I2C
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>   // Untuk OLED 1.3" 
// #include <Adafruit_SSD1306> // Untuk OLED 0.96"

/********************************
  OLED Setting & Method
*********************************/
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET 4 // Reset pin # (or -1 if sharing Arduino reset pin)
// Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Adafruit_SH1106G display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void splashScreen(){
  
  display.setTextColor(SH110X_WHITE);

  display.setTextSize(2);
  display.setCursor(0,0);
  display.println("GreenTRUST");
  display.setTextSize(1);
  display.println();
  display.println("by");display.println();
  display.println("AdaLovelace Team");
  display.display();
  delay(5000);
  display.clearDisplay();
  display.display();
}

void tampilan(int volume, float berat, float suhu, int kelembapan){
  display.clearDisplay();
  display.cp437(true);

  display.setTextSize(1);
  display.setCursor(0,57);
  display.print("SUHU:");display.print(suhu);display.write(0xf8);display.print("C");
  display.print(" KLMB:");display.print(kelembapan);display.print("%");

  // Grafik volume
  display.setTextSize(1);
  display.setCursor(0,0);
  display.print("VOLUME");
  display.setTextSize(2);
  display.setCursor(0,10);
  display.print(volume); display.print("%");

  display.fillRect(1,32,(int)(((float)volume/100) * 55),12,SH110X_WHITE);
  display.drawRect(0,31,56,13,SH110X_WHITE);

  // Grafik berat
  display.setTextSize(1);
  display.setCursor(68,0);
  display.print("BERAT (kg)");
  display.setTextSize(2);
  display.setCursor(68,10);
  display.print(berat);

  display.fillRect(69,32,(int)((berat/10) * 55),12,SH110X_WHITE);
  display.drawRect(68,31,56,13,SH110X_WHITE);

  // Garis kompartemen
  display.drawFastHLine(0,52,128,SH110X_WHITE);
  display.drawFastVLine(63,0,52,SH110X_WHITE);

  display.display();
}
/********************************
  WiFi Library, variable, method
*********************************/
#include <WiFi.h>

#define WIFI_SSID "studio.01FO"
#define WIFI_PASS "Baru872131"


void initWifi()
{
  bool koneksiSukses = false;
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  display.setCursor(0,0);
  display.print("Menyambung WiFI ");

  while(!koneksiSukses){
    display.print(".");
    display.display();
    delay(500);
    
    if (WiFi.status() == WL_CONNECTED)
    {
      display.println("");
      display.println("WiFi Terkoneksi");
      display.println("Alamat IP: ");
      display.println(WiFi.localIP());
      koneksiSukses = true;
      display.display();
      delay(2000);
      display.clearDisplay();
      display.display();
    }
    else if (WiFi.status() == WL_NO_SSID_AVAIL)
    {
      display.println("==> GAGAL!");
      display.println("SSID tdk ditemukan");
      display.println("Cek SSID atau ganti SSID.");
      display.display();
      while (true)
      {
        ;
      }
    }
    else if (WiFi.status() == WL_CONNECT_FAILED)
    {
      display.println("");
      display.println("Koneksi WiFi Gagal...");
      display.println("Restart ESP32...");
      display.display();
      delay(2000);
      ESP.restart();
    }
  }
}

void cekKoneksiWiFi()
{
  // Jika WiFi putus, restart ESP32
  if ((WiFi.status() != WL_CONNECTED))
  {
    display.clearDisplay();
    display.setCursor(0,0);
    display.println("WiFi terputus... ");
    display.println("Restart ESP32..");
    delay(2000);
    ESP.restart();
  }
}

/********************************
  MQTT Library, variable, method
*********************************/
#include <PubSubClient.h>

WiFiClient raspi;
PubSubClient client(raspi);

#define MQTT_BROKER_IP "192.168.1.2"
#define MQTT_BROKER_PORT 1883
#define TOKEN "abcdefghijklm"

#define MQTT_CLIENT_NAME "GreenTRUST"
#define DEVICE_LABEL "GreenTRUST_1"

#define SUHU "suhu"
#define KELEMBAPAN "kelembapan"
#define VOLUME "volume"
#define BERAT "berat"

// Mendapatkan koneksi MQTT
void mqttReconnect()
{
  // Ulang sampai terkoneksi
  while (!client.connected())
  {
    Serial.println("Membuka koneksi MQTT...");

    // Melakukan koneksi
    if (client.connect(MQTT_CLIENT_NAME, TOKEN, ""))
    {
      Serial.println("MQTT terkoneksi!");
    }
    else
    {
      Serial.print("Koneksi MQTT gagal, kode gagal=");
      Serial.print(client.state());
      Serial.println(" coba lagi dalam 2 detik");
      delay(2000);
    }
  }
}

// Cek koneksi MQTT
void cekKoneksiMQTT()
{
  cekKoneksiWiFi();
  if (!client.connected())
  {
    //    client.subscribe(topicSubscribe);
    mqttReconnect();
  }
}

// Callback koneksi MQTT
void callback(char *topic, byte *payload, unsigned int length)
{
  char p[length + 1];
  memcpy(p, payload, length);
  p[length] = NULL;
  String message(p);
  Serial.write(payload, length);
  Serial.println(topic);
}


/**************************************
  Main program
***************************************/
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  
  if (!display.begin(0x3C,true))
  { // Alamat default I2C untuk oleh 0x3C, kalo tdk tampil silahkan gunakan script I2C Finder untuk mencari alamatnya.
    Serial.println(F("SH1106G allocation failed"));
    for (;;)
      ;
  }

  display.clearDisplay();
  display.display();
  delay(1000);

  splashScreen();   // Tampilkan splash screen

  initWiFi();       // Inisiasi WiFi

  // Inisiasi MQTT
  client.setServer(MQTT_BROKER_IP, MQTT_BROKER_PORT);
  client.setCallback(callback);

  
}

void loop() {
  // put your main code here, to run repeatedly:
  // int volume = random(0,100);
  int volume[5] = {20,30,40,50,60};
  // float berat = random(0,20);
  float berat[5] = {0.56,1.1,2.2,5.0,7.5};
  // float suhu = random(10,100);
  float suhu = 27.50;
  // int kelembapan = random(0,99);
  int kelembapan = 78;
  for(int i=0;i<5;i++){
    tampilan(volume[i],berat[i],suhu,kelembapan);
    delay(10000);
  }
}
