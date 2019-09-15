#include <SoftwareSerial.h>
#define CNST_OFF_TIME 10000UL //10 s
#define ON_LIMIT 900000UL //90s
#define WAITING_LIMIT 120000UL// 120s
#define LED 17

#define OFFON 0
#define CHECK 1
#define KEEPON 2

SoftwareSerial mySerial(14, 16); // RX, TX

void setup() {
  Serial.begin(9600); // ハードウェアシリアルを準備
  while (!Serial) {
    ; // シリアルポートの準備ができるのを待つ(Leonardoのみ必要)
  }
  Serial.println("ONOFF test start");
  mySerial.begin(9600); // ソフトウェアシリアルの初期化
  mySerial.setTimeout(15000UL); // タイムアウト時間（15秒）
  mySerial.println("Are you ready?");
  long on_time = 10000UL; // ON時間初期値10s

  pinMode(LED, OUTPUT);
}

void loop() {
  //if (mySerial.available()) Serial.write(mySerial.read());
  //if (Serial.available()) mySerial.write(Serial.read());

  long on_time = 10000UL;
  int first_time;
  int state = OFFON;
  unsigned long start_time;
  unsigned long now;
  unsigned long interval;
  int test_cnt = 1;
  int recovery_count = 0;

  while(1) {
    switch (state) {
      case OFFON:
        Serial.print("ON testing...(Cycle = ");
        Serial.print(test_cnt);
        Serial.println(" )");
        //Constant off time
        digitalWrite(LED, HIGH);
        delay(CNST_OFF_TIME);

        //Variable on time
        Serial.print(on_time / 1000);
        Serial.println("s ON");
        digitalWrite(LED, LOW);
        delay(on_time);

        //ON time increment
        if ( on_time > ON_LIMIT){
          on_time = 10000;  //
        } else {
          on_time = on_time + 1000; //ever 1s increment 
        }
        
        test_cnt++;
        first_time = 0; //flag for detecting Recovery screen
        state = CHECK;
        break;

      case CHECK:
        //Constant off time
        Serial.print("Power Down!!! for ");
        Serial.print(CNST_OFF_TIME/1000);
        Serial.println("s");
        digitalWrite(LED, HIGH);
        delay(CNST_OFF_TIME);
        //Checking
        Serial.println("Checking...");
        //Constant on
        digitalWrite(LED, LOW);

        start_time = millis();
        while (1) {
          now = millis();
          interval = now - start_time;
          Serial.print("\r");
          Serial.print(interval / 1000);
          Serial.print("s");

          if ( mySerial.available() > 0 ) {
            String str = mySerial.readStringUntil('\r');
            if ( str == "booted") {
              Serial.println("\n\rNomarlly Started!\n\r");
              state = OFFON;
              break;
            }
          }

          if (interval > WAITING_LIMIT) {
            Serial.println("\n\rTime out!");
            if (first_time > 0) {
              recovery_count++;
              Serial.print("\nMaybe OS broken! The test was cancelled.");
              state = KEEPON;
              break;
            } else {
              recovery_count++;
              Serial.print("\nMaybe recovery occured!(Count = ");
              Serial.print(recovery_count);
              Serial.println(" )");
              state = CHECK;
              first_time++;
              break;
            }
          }
          delay(1000);
        }
        break;

      case KEEPON:
        digitalWrite(LED, LOW);
        break;
      default:
        break;
    }
  }
}
