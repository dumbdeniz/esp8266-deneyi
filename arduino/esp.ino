//  (               )   (         )     (               *      (      (      (             (               )
//  )\ )         ( /(   )\ )   ( /(     )\ )          (  `     )\ )   )\ )   )\ )          )\ )         ( /(
// (()/(   (     )\()) (()/(   )\())   (()/(    (     )\))(   (()/(  (()/(  (()/(    (    (()/(   (     )\())
//  /(_))  )\   ((_)\   /(_)) ((_)\     /(_))   )\   ((_)()\   /(_))  /(_))  /(_))   )\    /(_))  )\   ((_)/
//  _(_)  ((_)   _((_) (_))    _((_)   (_))_   ((_)  (_()((_) (_))   (_))   (_))_   ((_)  (_))   ((_)   _((_)
// |   \  | __| | \| | |_ _|  |_  /     |   \  | __| |  \/  | |_ _|  | _ \   |   \  | __| | |    | __| | \| |
// | |) | | _|  | .` |  | |    / /      | |) | | _|  | |\/| |  | |   |   /   | |) | | _|  | |__  | _|  | .` |
// |___/  |___| |_|\_| |___|  /___|     |___/  |___| |_|  |_| |___|  |_|_\   |___/  |___| |____| |___| |_|\_|
//
// Copyright (c) 2023 Deniz DEMIRDELEN
//
// This work is licensed under the terms of the MIT license.  
// For a copy, see https://mit-license.org.
//
// ESP8266 ile AT komutları kullanarak haberleşen ve api.demirdelen.net'ten (aslında proxyden) okunan değerlere 
// göre Arduino'ya bağlı LED'i yakıp söndüren sketch

#include <SoftwareSerial.h>

SoftwareSerial ESP(5, 6);  //!! RX ve TX pinleri ters !!

const String ssid = "SSID";
const String pass = "PASS";

const int ledPin = 2;
const int autoResetPin = 3;
const int failTrigger = 5; //kod kaç kere hata verdiğinde uyarı moduna geçilsin
const int autoResetTrigger = 2; //eğer oto reset pini bağlanmışsa, kaç kere uyarınca (led yanıp sönünce) resetlensin.

bool ledStatus = false;
bool prevLedStatus = false;

int failCount = 0;
int autoResetCount = 0;

//Normalde "api.demirdelen.net" olması gerekirdi,
//fakat ESP modülü HTTPS desteklemediğinden bilgisayarda bulunan bir proxy ile bağlanıyor. (Gerçektende PC'nin IP'si bu)
const String host = "192.168.31.31";

//!! HTTP İSTEKLERİ FATURALI !!
//Daha düşük değerler LED durumunu daha sık günceller ama aynı zamanda sunucuya daha fazla istek gönderir ve 
//seri iletişimi yorarak isteklerin yanlış okunmasına sebep olabilir!
long loopInterval = 3000;

//Akıllı İstek Azaltıcı™
//LED değeri aynı kalmışsa ve uzun süre boyunca değişmemişse loop() bekleme süresini otomatik olarak uzatır.
//Böylece dakikada ~15 istekten ~3 isteğe kadar optimizasyon sağlar. (5 dakikada 100 istek yerine 27)
const long minLoopInterval = 3000;
const long medLoopInterval = 10000;
const long maxLoopInterval = 25000; //30 saniyeden fazlaysa Arduino offline olarak algılanabilir.
const long slowModeInterval = 55000; //yavaş modun düzgün çalışması için sunucuya açık olduğunu belirtmek gerekiyor.
const long triggerCount = 10;
int currentCount = 0;
int currentStep = 0; //0 - min, 1 - med, 2 - max, 3 - yavaş mod

#pragma region Yardımcı Kodlar

//Arduinoyu resetlemek için
void(* resetFunc) (void) = 0;

//Seri iletişim ile girilen değeri gönder ve OK komutu alınana kadar tekrarla (isteğe bağlı)
void send(String cmd, bool loop = true) {
  ESP.println(cmd);
  ESP.flush();
  while (loop && !ESP.find("OK")) {
    ESP.println(cmd);
    ESP.flush();
    Serial.print(".");
  }
}

//Girilen string değerden ilk bulunan sayiyi int olarak döndür, Örn. STATUS:2 -> 2
int status(String input) {
  for (int i = 0; i <= input.length(); i++) if (isdigit(input[i])) return input[i] - '0'; return -1;
}

//Girilen string değerden OK veya ERROR bul ve int olarak döndür, Örn. OK -> 1
int result(String input) {
  if (input.indexOf("OK") != -1) return 1; if (input.indexOf("ERROR") != -1 || input.indexOf("FAIL") != -1) return 0; return -1;
}

//Göz kırp ;)
void blink() {
  for(int i = 1; i <= 2; i++) {
    digitalWrite(ledPin, HIGH);
    delay(100);
    digitalWrite(ledPin, LOW);
    delay(100);
  }
}

void reset() {
  Serial.println("------ RESET ------");
  send("AT+RST", false);
  delay(1000);
  resetFunc();  
}

//Akıllı İstek Azaltıcı™
void onLoop() {
  ledStatus == prevLedStatus ? currentCount++ : currentCount = 0;
  prevLedStatus = ledStatus;

  if (currentCount == 0) {
    loopInterval = minLoopInterval;
    currentStep = 0;
  }

  if (currentCount >= triggerCount) {
    int prevStep = currentStep;
    currentStep == 3 ? currentStep = 3 : currentStep++;

    switch(currentStep) {
      case 1: loopInterval = medLoopInterval; break;
      case 2: loopInterval = maxLoopInterval; break;
      case 3: loopInterval = slowModeInterval; break;
    }

    if (prevStep != currentStep) currentCount = 0;
  }

  //yavaş mod fix, ilk defa geçildiğinde sunucuya 30 saniye içinde bildirmesi gerekiyor.
  if (currentStep == 3 && currentCount == 0) loopInterval = maxLoopInterval; 
}

#pragma endregion

void setup() {
  pinMode(ledPin, OUTPUT);

  Serial.begin(9600);  //Arduino Serial
  ESP.begin(9600);  //ESP Yazılımsal Serial
  
  Serial.println("setup() -----------");

  send("AT"); //ESP cevap verene kadar ilerlemez
  Serial.println("AT : OK");

  send("AT+CWMODE?", false); //istasyon modunu sorgula (loop kapalı çünkü find() serial bufferi sıfırlar)
  int cwMode = status(ESP.readString());

  Serial.print("CWMODE? : ");
  Serial.println(cwMode);

  if (cwMode != 1) send("AT+CWMODE=1"); //1 değilse (2 veya 3 ise) istasyon olarak ayarla
  Serial.println("CWMODE : OK");

  send("AT+CIPSTATUS", false); //Bağlantı durumunu sorgula
  int cipStatus = status(ESP.readString());

  Serial.print("CIPSTATUS? : ");
  Serial.println(cipStatus);

  if (cipStatus == 5) {
    send("AT+CWJAP=\""+ ssid +"\",\""+ pass +"\"", false);

    String data;
    while(result(data) == -1) {
      data = ESP.readString();
      Serial.print(".");
    }

    int cwResult = result(data);
    Serial.print("CWJAP : ");
    Serial.println(cwResult == 1 ? "OK" : "FAIL");
  }
  
  Serial.println("-------------------");
  blink();
}

void loop() {
  //UYARI MODU
  if (digitalRead(autoResetPin) == HIGH && autoResetCount >= autoResetTrigger) reset();

  if (failCount >= failTrigger) {
    autoResetCount++;
    blink();
    delay(1000);
    return;
  }

  //NORMAL MOD
  Serial.println("\nloop() ------------");

  while(ESP.available() > 0) char dummy = ESP.read(); //Serial buffer temizleme

  bool cipSend = false;
  for(int i = 0; i < 3 && !cipSend; i++) {
    send("AT+CIPSTART=\"TCP\",\""+ host +"\",80", false);
    int startResult = result(ESP.readString());

    Serial.print("CIPSTART : ");
    Serial.println(startResult == 1 ? "OK" : "FAIL");
  
    send("AT+CIPSEND=60", false);
    if (ESP.find(">")) cipSend = true;;
    delay(1000);
  }

  if (!cipSend) {
    Serial.println("CIPSEND : FAIL");
    send("AT+CIPCLOSE", false);
    Serial.println("-------------------");
    failCount++;
    delay(minLoopInterval);
    return;    
  }

  String slowMode = currentStep == 3 ? "1" : "0"; //Akıllı İstek Azaltıcı™
  send("GET /esp-proxy?slow="+ slowMode +" HTTP/1.1\r\nHost: "+ host +"\r\n\r\n\r\n\r\n", false); //192.168.31.31/esp-proxy adresine GET isteği gönder
  Serial.println("GET /esp-proxy");

  String response = ESP.readString();
  if (response.indexOf("+IPD") == -1 || response.indexOf("200 OK") == -1) {
    Serial.println("Geçersiz yanıt");
    Serial.println("-------------------");
    failCount++;
    delay(minLoopInterval);
    return;
  }

  failCount = 0;
  ledStatus = status(response.substring(response.lastIndexOf("led:"))) == 1;
  Serial.print("LED : ");
  Serial.println(ledStatus);
  
  digitalWrite(ledPin, ledStatus);

  onLoop();

  Serial.print("INTERVAL : ");
  Serial.println(loopInterval);
  Serial.println("-------------------");
  
  delay(loopInterval);
}