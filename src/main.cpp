#include <Arduino.h>
#include <ESP8266WiFi.h> //https://github.com/esp8266/Arduino
#include <ModbusIP_ESP8266.h>
#include <Modbus.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h> //https://github.com/tzapu/WiFiManager
#include <MatrixKeypad.h>

const uint8_t rown = 4; // 4 rows
const uint8_t coln = 3; // 3 columns
uint8_t rowPins[rown] = {4, 0, 2, 14};
// byte colPins[n_cols] = {D0, D1, D6};
uint8_t colPins[coln] = {16, 5, 12}; // frist column is connect to pin 6, second to 5...
char keymap[rown][coln] =
    {{'1', '2', '3'}, // key of the frist row frist column is '1', frist row second column column is '2'
     {'4', '5', '6'}, // key of the second row frist column is '4', second row second column column is '5'
     {'7', '8', '9'},
     {'*', '0', '#'}};
MatrixKeypad_t *keypad; // keypad is the variable that you will need to pass to the other functions
int hitung = 0, ad;
char key;
bool valkey = false;
const int LED_COIL = 100;
const int ledPin = 10; // Builtin ESP8266 LED
ModbusIP mb;

bool cbConn(IPAddress ip)
{
  Serial.println(ip);
  return true;
}

void setup()
{
  Serial.begin(115200);
  pinMode(12, OUTPUT);
  pinMode(14, INPUT_PULLUP);
  digitalWrite(12, LOW);
  delay(1000);
  WiFiManager wm;
  if (digitalRead(14) == LOW)
  {
    wm.resetSettings();
  }
  bool res;
  res = wm.autoConnect("keypad"); // password protected ap

  if (!res)
  {
    Serial.println("Failed to connect");
    // ESP.restart();
  }
  else
  {
    // if you get here you have connected to the WiFi
    Serial.println("connected...yeey :)");
  }
  mb.onConnect(cbConn); // Add callback on connection event
  mb.server();
  pinMode(ledPin, OUTPUT);
  for (int i = 0; i < 100; i++)
  {
    mb.addCoil(i);
  }

  //  mb.onSetCoil(LED_COIL, cbLed); // Add callback on Coil LED_COIL value set
  mb.addHreg(0);
  keypad = MatrixKeypad_create((char *)keymap /* don't forget to do this cast */, rowPins, colPins, rown, coln); // creates the keypad object
}

void loop()
{
  mb.task();
  MatrixKeypad_scan(keypad);
  if (MatrixKeypad_hasKey(keypad))
  {
    key = MatrixKeypad_waitForKey(keypad);
    Serial.println(key);
    ad = String(key).toInt();
    valkey = !mb.Coil(ad);
    if (key == '*')
    {
      valkey = !mb.Coil(10);
      mb.Coil(10, valkey);
    }
    else if (key == '#')
    {
      valkey = !mb.Coil(11);
      mb.Coil(11, valkey);
    }
    else
    {
      mb.Coil(ad, valkey);
    }
  }
  mb.Hreg(0, key);
  delay(10); // do something
}

/* blinks a led roughly each second */
void blink()
{
  static int led_state = LOW;
  static long time = 0;
  if (millis() - time > 500)
  {
    if (led_state == HIGH)
    {
      led_state = LOW;
    }
    else
    {
      led_state = HIGH;
    }
    digitalWrite(ledPin, led_state);
    time = millis();
  }
}