#include <SoftwareSerial.h>;

#define RxPin 4
#define TxPin 3
#define TxBauds 115200
#define RxBauds 9600

char mensaje;

SoftwareSerial TXArdu(RxPin,TxPin);

void setup() {
  Serial.begin(TxBauds);
  TXArdu.begin(RxBauds);
}

void loop() {
  if(Serial.available()>0){
    mensaje = TXArdu.read();
    Serial.print(mensaje);
    delay(1000);
  }
}
