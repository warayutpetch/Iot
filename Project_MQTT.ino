
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <DS3231.h>

DS3231 clock;
RTCDateTime dt;

// Update these with values suitable for your network.
const char* ssid = "COMPUTER";
const char* password = "571733022";

// Config MQTT Server
#define mqtt_server "your server"
#define mqtt_port 
#define mqtt_user "your user"
#define mqtt_password "your password"

#define LED_PIN D3
#define LED_PIN1 D4
#define LED_PIN2 D5
#define LED_PIN3 D6
#define F_PIN1 D7
#define F_PIN2 D0


WiFiClient espClient;
PubSubClient client(espClient);

float tmp;
char buf[3];
char timeall1[15] = "999999", timeall2[15] = "999999", timeall3[15] = "999999", timeall4 [15] = "999999";
char sendtime[20];
String H1_on, M1_on, S1_on,
H1_off, M1_off, S1_off,

H2_on, M2_on, S2_on,
H2_off, M2_off, S2_off,

H3_on, M3_on, S3_on,
H3_off, M3_off, S3_off,

H4_on, M4_on, S4_on,
H4_off, M4_off, S4_off;


int on_Hour1, on_Min1, on_Sec1,
off_Hour1, off_Min1, off_Sec1,

on_Hour2, on_Min2, on_Sec2,
off_Hour2, off_Min2, off_Sec2,

on_Hour3, on_Min3, on_Sec3,
off_Hour3, off_Min3, off_Sec3,

on_Hour4, on_Min4, on_Sec4,
off_Hour4, off_Min4, off_Sec4;

char state_F1_1[3], state_F1_2[3],
state_F2_1[3], state_F2_2[3],
state_F3_1[3], state_F3_2[3],
state_F4_1[3], state_F4_2[3];


char Smart[10] = "00000000";
int smart_time = 0 , moisture = 0;
String moiture_S, time_S;
int val1, val2, val3, val4;

char statusVal[5] = "0000";

int F1_1 = 0, F1_2 = 0,
F2_1 = 0, F2_2 = 0,
F3_1 = 0, F3_2 = 0,
F4_1 = 0, F4_2 = 0;
int soli_State = 0;
int sensorPin = A0;
int sensorValue = 0;
int Moisture = 0; // variable to store the value coming from the sensor
int status_smart;
int start_count;
void setup() {


  pinMode(LED_PIN, OUTPUT);
  pinMode(LED_PIN1, OUTPUT);
  pinMode(LED_PIN2, OUTPUT);
  pinMode(LED_PIN3, OUTPUT);
  pinMode(F_PIN1, OUTPUT);
  pinMode(F_PIN2, OUTPUT);
  //digitalWrite(F_PIN2,1);
  Serial.begin(115200);
  delay(10);
  clock.begin();
  // disable 32kHz
  //clock.enable32kHz(false);

  // Select output as rate to 1Hz
  clock.setOutput(DS3231_1HZ);

  // Enable output
  clock.enableOutput(true);

  if (!clock.isReady())
  {
    // Set sketch compiling time
    clock.setDateTime(__DATE__, __TIME__);
  }
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  attachInterrupt(D8, checkDevice, RISING);



}

void loop() {

  connectMQTT();
  //sensors.requestTemperatures();
  //tmp = sensors.getTempCByIndex(0);
  //dtostrf(tmp, 3, 3, buf);
  //client.publish("/ESP/temp", buf);
  //Serial.println(buf);
  //delay(300);
  //checkDevice();
  if (soli_State == 1) {
    sensorValue = analogRead(sensorPin);
    Serial.print("Moisture of Soil:  ");
    Serial.println(sensorValue);
    Moisture = map(sensorValue, 1, 1024, 1, 100);
    Moisture = 100 - Moisture;
    Serial.println(Moisture);
    char buff[3];
    dtostrf(Moisture, 3, 0, buff);
    client.publish("/ESP/temp", buff);
  }
  delay(200);
}

void connectMQTT() {
  if (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP8266Client", mqtt_user, mqtt_password)) {
      Serial.println("connected");
      client.subscribe("/ESP/LED");
      client.subscribe("/setdevice1");
      client.subscribe("/setdevice2");
      client.subscribe("/setdevice3");
      client.subscribe("/setdevice4");
      client.subscribe("/ESP/SMART");

      } else {
        Serial.print("failed, rc=");
        Serial.print(client.state());
        Serial.println(" try again in 5 seconds");
        delay(5000);
        return;
      }

    }

    client.loop();
  }

  void callback(char* topic, byte* payload, unsigned int length) {
    Serial.println(topic);
    String msg = "";
    int i = 0;
    while (i < length) msg += (char)payload[i++];

    Serial.println(msg);

    if (msg == "SMART" ) {
      client.publish("/ESP/SMART", Smart);
      return;
    }

    
    if (msg == "GET") {
      digitalRead(LED_PIN) ? statusVal[0]=1 : statusVal[0]=0;
      digitalRead(LED_PIN1) ? statusVal[1]=1 : statusVal[1]=0;
      digitalRead(LED_PIN2) ? statusVal[2]=1 : statusVal[2]=0;
      digitalRead(LED_PIN3) ? statusVal[3]=1 : statusVal[3]=0;
      client.publish("/ESP/LED", statusVal);
      Serial.println("Send !");
      return;
    }


    if (msg == "Soil") {
      soli_State = 1;
      return;
    }
    if (msg == "Closed" ) {
      soli_State = 0;
      return;
    }

    if (msg == "GET-State_F1" ) {
      client.publish("/ESP/LED", state_F1_1);
      client.publish("/ESP/LED", state_F1_2);
      return;
    }
    if (msg == "GET=State_F2" ) {
      client.publish("/ESP/LED", state_F2_1);
      client.publish("/ESP/LED", state_F2_2);
      return;
    }
    if (msg == "GET=State_F3" ) {
      client.publish("/ESP/LED", state_F3_1);
      client.publish("/ESP/LED", state_F3_2);
      return;
    }
    if (msg == "GET=State_F4" ) {
      client.publish("/ESP/LED", state_F4_1);
      client.publish("/ESP/LED", state_F4_2);
      return;
    }

    if (strcmp(topic, "/setdevice1") == 0 && msg == "GET-TIME") {
      client.publish("/setdevice1", timeall1);
      return;

    }
    if (strcmp(topic, "/setdevice2") == 0 && msg == "GET-TIME") {
      client.publish("/setdevice2", timeall2);
      return;
    }
    if (strcmp(topic, "/setdevice3") == 0 && msg == "GET-TIME") {
      client.publish("/setdevice3", timeall3);
      return;
    }
    if (strcmp(topic, "/setdevice4") == 0 && msg == "GET-TIME") {
      client.publish("/setdevice4", timeall4);
      return;
    }
    if (strcmp(topic, "/ESP/SMART") == 0 && msg == "GET") {
      msg.toCharArray(Smart, 8);
      moiture_S += Smart[0];
      moiture_S += Smart[1];
      time_S += Smart[2];
      time_S += Smart[3];
      moisture = moiture_S.toInt();
      smart_time = time_S.toInt();
      val1 = Smart[4] - '0';
      val1 = Smart[5] - '0';
      val1 = Smart[6] - '0';
      val1 = Smart[7] - '0';
    }
    if (msg == "LEDON") {
      digitalWrite(LED_PIN, HIGH);
    }
    if (msg == "LEDOFF") {
      digitalWrite(LED_PIN, LOW);
    }
    if (msg == "LEDON1") {
      digitalWrite(LED_PIN1, HIGH);
    }
    if (msg == "LEDOFF1") {
      digitalWrite(LED_PIN1, LOW);
    }
    if (msg == "LEDON2") {
      digitalWrite(LED_PIN2, HIGH);
    }
    if (msg == "LEDOFF2") {
      digitalWrite(LED_PIN2, LOW);
    }
    if (msg == "LEDON3") {
      digitalWrite(LED_PIN3, HIGH);
    }
    if (msg == "LEDOFF3") {
      digitalWrite(LED_PIN3, LOW);
    }

    if (msg == "F1_1_1") {
      state_F1_1[0] = '1';
      state_F1_1[1] = '1';
      state_F1_1[2] = '1';
      F1_1 = 1;
    }
    if (msg == "F1_1_0") {
      state_F1_1[0] = '1';
      state_F1_1[1] = '1';
      state_F1_1[2] = '0';
      F1_1 = 0;
    }
    if (msg == "F1_2_1") {
      state_F1_2[0] = '1';
      state_F1_2[1] = '2';
      state_F1_2[2] = '1';
      F1_2 = 1;
    }
    if (msg == "F1_2_0") {
      state_F1_2[0] = '1';
      state_F1_2[1] = '2';
      state_F1_2[2] = '0';
      F1_2 = 0;
    }
    if (msg == "F2_1_1") {
      state_F2_1[0] = '2';
      state_F2_1[1] = '1';
      state_F2_1[2] = '1';
      F2_1 = 1;
    }
    if (msg == "F2_1_0") {
      state_F2_1[0] = '2';
      state_F2_1[1] = '1';
      state_F2_1[2] = '0';
      F2_1 = 0;
    }
    if (msg == "F2_2_1") {
      state_F2_2[0] = '2';
      state_F2_2[1] = '2';
      state_F2_2[2] = '1';
      F2_2 = 1;
    }
    if (msg == "F2_2_0") {
      state_F2_2[0] = '2';
      state_F2_2[1] = '2';
      state_F2_2[2] = '0';
      F2_2 = 0;
    }
    if (msg == "F3_1_1") {
      state_F3_1[0] = '3';
      state_F3_1[1] = '1';
      state_F3_1[2] = '1';
      F3_1 = 1;
    }
    if (msg == "F3_1_0") {
      state_F3_1[0] = '3';
      state_F3_1[1] = '1';
      state_F3_1[2] = '0';
      F3_1 = 0;
    }
    if (msg == "F3_2_1") {
      state_F3_2[0] = '3';
      state_F3_2[1] = '2';
      state_F3_2[2] = '1';
      F3_2 = 1;
    }
    if (msg == "F3_2_0") {
      state_F3_2[0] = '3';
      state_F3_2[1] = '2';
      state_F3_2[2] = '0';
      F3_2 = 0;
    }
    if (msg == "F4_1_1") {
      state_F4_1[0] = '4';
      state_F4_1[1] = '1';
      state_F4_1[2] = '1';
      F4_1 = 1;
    }
    if (msg == "F4_1_0") {
      state_F4_1[0] = '4';
      state_F4_1[1] = '1';
      state_F4_1[2] = '0';
      F4_1 = 0;
    }
    if (msg == "F4_2_1") {
      state_F4_2[0] = '4';
      state_F4_2[1] = '2';
      state_F4_2[2] = '1';
      F4_2 = 1;
    }
    if (msg == "F4_2_0") {
      state_F4_2[0] = '4';
      state_F4_2[1] = '2';
      state_F4_2[2] = '0';
      F4_2 = 0;
    }


    if (strcmp(topic, "/setdevice1") == 0 && msg != "999999" && msg != "GET-TIME") {
      msg.toCharArray(timeall1, 15);
      H1_on += timeall1[0];
      H1_on += timeall1[1];

      M1_on += timeall1[2];
      M1_on += timeall1[3];

      S1_on += timeall1[4];
      S1_on += timeall1[5];

      H1_off += timeall1[6];
      H1_off += timeall1[7];

      M1_off += timeall1[8];
      M1_off += timeall1[9];

      S1_off += timeall1[10];
      S1_off += timeall1[11];

      on_Hour1 = H1_on.toInt();
      on_Min1 = M1_on.toInt();
      on_Sec1 = S1_on.toInt();

      off_Hour1 = H1_off.toInt();
      off_Min1 = M1_off.toInt();
      off_Sec1 = S1_off.toInt();

      Serial.println("Set!1");
      Serial.print(H1_on);   Serial.print(":");
      Serial.print(M1_on); Serial.print(":");
      Serial.print(S1_on); Serial.println("");
      Serial.print(H1_off);   Serial.print(":");
      Serial.print(M1_off); Serial.print(":");
      Serial.print(S1_off); Serial.println("");
      H1_on = "";
      M1_on = "";
      S1_on = "";

      H1_off = "";
      M1_off = "";
      S1_off = "";
    }
    if (strcmp(topic, "/setdevice2") == 0 && msg != "999999" && msg != "GET-TIME") {
      msg.toCharArray(timeall2, 15);
      H2_on += timeall2[0];
      H2_on += timeall2[1];

      M2_on += timeall2[2];
      M2_on += timeall2[3];

      S2_on += timeall2[4];
      S2_on += timeall2[5];

      H2_off += timeall2[6];
      H2_off += timeall2[7];

      M2_off += timeall2[8];
      M2_off += timeall2[9];

      S2_off += timeall2[10];
      S2_off += timeall2[11];

      on_Hour2 = H2_on.toInt();
      on_Min2 = M2_on.toInt();
      on_Sec2 = S2_on.toInt();

      off_Hour2 = H2_off.toInt();
      off_Min2 = M2_off.toInt();
      off_Sec2 = S2_off.toInt();

      Serial.println("Set!2");
      Serial.print(H2_on);   Serial.print(":");
      Serial.print(M2_on); Serial.print(":");
      Serial.print(S2_on); Serial.println("");
      Serial.print(H2_off);   Serial.print(":");
      Serial.print(M2_off); Serial.print(":");
      Serial.print(S2_off); Serial.println("");
      H2_on = "";
      M2_on = "";
      S2_on = "";

      H2_off = "";
      M2_off = "";
      S2_off = "";

    }
    if (strcmp(topic, "/setdevice3") == 0  && msg != "999999" && msg != "GET-TIME") {
      msg.toCharArray(timeall3, 15);
      H3_on += timeall3[0];
      H3_on += timeall3[1];

      M3_on += timeall3[2];
      M3_on += timeall3[3];

      S3_on += timeall3[4];
      S3_on += timeall3[5];

      H3_off += timeall3[6];
      H3_off += timeall3[7];

      M3_off += timeall3[8];
      M3_off += timeall3[9];

      S3_off += timeall3[10];
      S3_off += timeall3[11];

      on_Hour3 = H3_on.toInt();
      on_Min3 = M3_on.toInt();
      on_Sec3 = S3_on.toInt();

      off_Hour3 = H3_off.toInt();
      off_Min3 = M3_off.toInt();
      off_Sec3 = S3_off.toInt();

      Serial.println("Set!3");
      Serial.print(H3_on);   Serial.print(":");
      Serial.print(M3_on); Serial.print(":");
      Serial.print(S3_on); Serial.println("");
      Serial.print(H3_off);   Serial.print(":");
      Serial.print(M3_off); Serial.print(":");
      Serial.print(S3_off); Serial.println("");
      H3_on = "";
      M3_on = "";
      S3_on = "";

      H3_off = "";
      M3_off = "";
      S3_off = "";

    }

    if (strcmp(topic, "/setdevice4" ) == 0 && msg != "999999" && msg != "GET-TIME") {
      msg.toCharArray(timeall4, 15);
      H4_on += timeall4[0];
      H4_on += timeall4[1];

      M4_on += timeall4[2];
      M4_on += timeall4[3];

      S4_on += timeall4[4];
      S4_on += timeall4[5];

      H4_off += timeall4[6];
      H4_off += timeall4[7];

      M4_off += timeall4[8];
      M4_off += timeall4[9];

      S4_off += timeall4[10];
      S4_off += timeall4[11];

      on_Hour4 = H4_on.toInt();
      on_Min4 = M4_on.toInt();
      on_Sec4 = S4_on.toInt();

      off_Hour4 = H4_off.toInt();
      off_Min4 = M4_off.toInt();
      off_Sec4 = S4_off.toInt();

      Serial.println("Set!4");
      Serial.print(H4_on);   Serial.print(":");
      Serial.print(M4_on); Serial.print(":");
      Serial.print(S4_on); Serial.println("");
      Serial.print(H4_off);   Serial.print(":");
      Serial.print(M4_off); Serial.print(":");
      Serial.print(S4_off); Serial.println("");
      H4_on = "";
      M4_on = "";
      S4_on = "";

      H4_off = "";
      M4_off = "";
      S4_off = "";
    }

    msg = "";

  }


  void checkDevice() {
    dt = clock.getDateTime();
    Serial.print(dt.hour);   Serial.print(":");
    Serial.print(dt.minute); Serial.print(":");
    Serial.print(dt.second); Serial.println("");
    sensorValue = analogRead(sensorPin);
    Serial.print("Moisture of Soil:  ");
    Serial.println(sensorValue);
    Moisture = map(sensorValue, 1, 1024, 1, 100);
    Moisture = 100 - Moisture;
  /*
    Serial.print(on_Hour1);   Serial.print(":");
    Serial.print(on_Min1); Serial.print(":");
    Serial.print(on_Sec1); Serial.println("");

    Serial.print(off_Hour1);   Serial.print(":");
    Serial.print(off_Min1); Serial.print(":");
    Serial.print(off_Sec1); Serial.println("");

    Serial.print(on_Hour2);   Serial.print(":");
    Serial.print(on_Min2); Serial.print(":");
    Serial.print(on_Sec2); Serial.println("");

    Serial.print(off_Hour2);   Serial.print(":");
    Serial.print(off_Min2); Serial.print(":");
    Serial.print(off_Sec2); Serial.println("");
    */
    if (soli_State == 1) {

      Serial.println(Moisture);
      char buff[3];
      dtostrf(Moisture, 3, 0, buff);
      client.publish("/ESP/temp", buff);
    }

    if (dt.hour == on_Hour1 && dt.minute == on_Min1 && dt.second == on_Sec1) {
      digitalWrite(LED_PIN, HIGH);
      client.publish("/ESP/LED", "LEDON");

      digitalWrite(F_PIN1, F1_1);
      digitalWrite(F_PIN2, F1_2);

    }
    if (dt.hour == off_Hour1 && dt.minute == off_Min1 && dt.second == off_Sec1) {
      digitalWrite(LED_PIN, LOW);
      client.publish("/ESP/LED", "LEDOFF");

      digitalWrite(F_PIN1, LOW);
      digitalWrite(F_PIN2, LOW);

    }

    if (dt.hour == on_Hour2 && dt.minute == on_Min2 && dt.second == on_Sec2) {
      digitalWrite(LED_PIN1, HIGH);
      client.publish("/ESP/LED", "LEDON1");

      digitalWrite(F_PIN1, F2_1);
      digitalWrite(F_PIN2, F2_2);
    }
    if (dt.hour == off_Hour2 && dt.minute == off_Min2 && dt.second == off_Sec2) {
      digitalWrite(LED_PIN1, LOW);
      client.publish("/ESP/LED", "LEDOFF1");

      digitalWrite(F_PIN1, LOW);
      digitalWrite(F_PIN2, LOW);
    }

    if (dt.hour == on_Hour3 && dt.minute == on_Min3 && dt.second == on_Sec3) {
      digitalWrite(LED_PIN2, HIGH);
      client.publish("/ESP/LED", "LEDON2");

      digitalWrite(F_PIN1, F3_1);
      digitalWrite(F_PIN2, F3_2);

    }
    if (dt.hour == off_Hour3 && dt.minute == off_Min3 && dt.second == off_Sec3) {
      digitalWrite(LED_PIN2, LOW);
      client.publish("/ESP/LED", "LEDOFF2");

      digitalWrite(F_PIN1, LOW);
      digitalWrite(F_PIN2, LOW);

    }


    if (dt.hour == on_Hour4 && dt.minute == on_Min4 && dt.second == on_Sec4) {
      digitalWrite(LED_PIN3, HIGH);
      client.publish("/ESP/LED", "LEDON3");

      digitalWrite(F_PIN1, F4_1);
      digitalWrite(F_PIN2, F4_2);
    }
    if (dt.hour == off_Hour4 && dt.minute == off_Min4 && dt.second == off_Sec4) {
      digitalWrite(LED_PIN3, LOW);
      client.publish("/ESP/LED", "LEDOFF3");

      digitalWrite(F_PIN1, LOW);
      digitalWrite(F_PIN2, LOW);
    }
/*
  if (Moisture < moisture ) {
      status_smart = 1;
  }

  if(status_smart == 1){
    
    start_count == dt.second;
    status_smart ==0;
    }
    */
  }

