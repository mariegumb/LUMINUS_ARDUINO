// Load Wi-Fi library
#include <WiFi.h>
#include <HTTPClient.h>
#include <Adafruit_NeoPixel.h>
#include <Arduino_JSON.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH1106.h>

#define PIN       23
#define NUMPIXELS 60
#define DELAYVAL  10

HTTPClient http;
Adafruit_SH1106 display(23); 


Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

const char* ssid = "Livebox-8260";
const char* password = "uT5Pnx7vWnL4biCFqH";

int colorSoundState = 0;
//const char* ssid = "SFR_D990";
//const char* password = "bxv54aahz6stc45snvq8";

WiFiServer server(80);
unsigned long previousTime = millis();
JSONVar deviceState;
int decalage = 0;

void setup() {
    Serial.begin(115200);

    display.begin();  // initialisation de l'afficheur
    display.clearDisplay();   // ça efface à la fois le buffer et l'écran

    displayLog("Connecting to wifi");
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    displayLog("WiFi connected.");
    Serial.println("Mac address: ");
    Serial.println(WiFi.macAddress());
    
    pixels.begin();
    pinMode(18, INPUT);
    fetchDeviceState();

    displayLog("luminus"); //Our team name
}

void loop(){
     if (millis() - previousTime >= 5*1000UL){ // toute les 5s
       previousTime = millis();
       fetchDeviceState();
     }

    String selectedMode = JSON.stringify(deviceState["mode"]);
    selectedMode.replace("\"", "");
    
    if(selectedMode.equals("color")){
        displayLog("color mode on");
        setColor();
        delay(50);
    } else if(selectedMode.equals("sound")){
        displayLog("sound mode on");
        soundMode();
        delay(50);
    } else if(selectedMode.equals("gradient")){
        displayLog("gradient mode on");
        gradientMode();
        delay(50);
    } else if(selectedMode.equals("animated-gradient")){
        displayLog("animated gradient mode on");
        animatedGradientMode();
    } else {
        displayLog("no mode recognized");
    }
}

void setColor(){
    Serial.println(deviceState);
    JSONVar color = deviceState["firstColor"];
  
    int red = (int) color["red"];
    int green = (int) color["green"];
    int blue = (int) color["blue"];

    for(int i = 0; i < NUMPIXELS; i++) {
        pixels.setPixelColor(i, pixels.Color(red, green, blue));
    }

    pixels.show();
}

void gradientMode(){
    JSONVar color = deviceState["gradient"];

    Serial.println(color[0]);

    for(int i = 0; i < NUMPIXELS; i++) {
        int r = (int) color[i][0];
        int g = (int) color[i][1];
        int b = (int) color[i][2];
        pixels.setPixelColor(i, pixels.Color(r, g, b));
        delay(10);
    }
    decalage ++;
    pixels.show();
}

void animatedGradientMode(){
    JSONVar color = deviceState["gradient"];

    Serial.println(color[0]);

    for(int i = 0; i < NUMPIXELS; i++) {
        int r = (int) color[i][0];
        int g = (int) color[i][1];
        int b = (int) color[i][2];
        int numPixel = (i + decalage) % NUMPIXELS;
        pixels.setPixelColor(numPixel, pixels.Color(r, g, b));
    }
    decalage ++;
    pixels.show();
}

void soundMode(){
    Serial.println(deviceState);

    JSONVar firstColor = deviceState["firstColor"];
    JSONVar secondColor = deviceState["secondColor"];

    // first color rgb
    int fr = (int) firstColor["red"];
    int fg = (int) firstColor["green"];
    int fb = (int) firstColor["blue"];

    // second color rgb
    int sr = (int) secondColor["red"];
    int sg = (int) secondColor["green"];
    int sb = (int) secondColor["blue"];

    int soundSensor = digitalRead(18);
    Serial.println(soundSensor);
  
    if(soundSensor == 0){
      for(int i = 0; i < NUMPIXELS / 2; i++) {
        int pixelUp = (NUMPIXELS / 2);
        int pixelDown = (NUMPIXELS / 2) - 1;
        if(colorSoundState == 0){
          pixels.setPixelColor((pixelDown) - i, pixels.Color(fr, fg, fb));
          pixels.setPixelColor((pixelUp) + i, pixels.Color(fr, fg, fb));
        } else {
          pixels.setPixelColor((pixelDown) - i, pixels.Color(sr, sg, sb));
          pixels.setPixelColor((pixelUp) + i, pixels.Color(sr, sg, sb));
        }
        pixels.show();
        delay(1); 
      } 
    } else {
      for(int i = 0; i < NUMPIXELS / 2; i++) {
        int pixelUp = (NUMPIXELS / 2);
        int pixelDown = (NUMPIXELS / 2) - 1;
        if(colorSoundState == 0){
          pixels.setPixelColor((pixelDown) - i, pixels.Color(sr, sg, sb));
          pixels.setPixelColor((pixelUp) + i, pixels.Color(sr, sg, sb));
        } else {
          pixels.setPixelColor((pixelDown) - i, pixels.Color(fr, fg, fb));
          pixels.setPixelColor((pixelUp) + i, pixels.Color(fr, fg, fb));
        }
        pixels.show();
        delay(1);
      }
      if(colorSoundState == 0){
        colorSoundState = 1;
      } else {
        colorSoundState = 0;
      }
    }

    
   
    
}


void fetchDeviceState(){
    String payload = fetchDatabase();
    JSONVar stripeData = JSON.parse(payload);
    if (JSON.typeof(stripeData) == "undefined") {
      Serial.println("Parsing input failed!");
    } else {
      deviceState = stripeData;
    }   
}

String fetchDatabase(){
    String DEVICE_ID = WiFi.macAddress();
    String DATABASE_URL = "https://luminus-efrei-default-rtdb.europe-west1.firebasedatabase.app/devices/" + DEVICE_ID + ".json";

    http.setTimeout(500);
    http.begin(DATABASE_URL);

    int status = http.GET();

    if (status <= 0) {
        Serial.printf("HTTP error: %s\n", http.errorToString(status).c_str());
        return "";
    }

    String payload = http.getString();
    return payload;
}

JSONVar parse(String input){

  Serial.println(input);

  JSONVar myObject = JSON.parse(input);

  if (JSON.typeof(myObject) == "undefined") {
    Serial.println("Parsing input failed!");
    return null;
  }

  return myObject;

}

void displayLog(String text){
    display.clearDisplay();
    display.setCursor(15, 15);  // Place
    display.setTextColor(WHITE);
    display.setTextSize(1);  // default size
    display.println(text);
    display.display();
}
