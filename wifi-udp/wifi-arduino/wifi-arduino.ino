//BridgeBlink.ino
#include <Bridge.h>
#include <SoftwareSerial.h>

unsigned char buf[100];
SoftwareSerial m_serial(2, 3); // (rx, tx)
byte speed_stop[] = {0xAA, 0x55, 0x06, 0x01, 0x04, 0x00, 0x00, 0x00, 0x00};

struct SpeedMessageFormat
{
  uint8_t header_0;
  uint8_t header_1;
  uint8_t payload_len;
  uint8_t sub_payload_id;
  uint8_t sub_payload_len;
  uint16_t speed;
  uint16_t radius;
  uint8_t crc;
};
void setup() {

  //Serial.begin(115200);
  //while (!Serial) {
    //;
  //}
  //Serial.println("Initiailize the arduino yun");

  m_serial.begin(115200);
  while (!m_serial) {
    ;
  }
  //Serial.println("Success to initialize the arduino yun");
  
  // put your setup code here, to run once:
  pinMode(13,OUTPUT);
  
  digitalWrite(13,LOW);
  Bridge.begin();
  digitalWrite(13,HIGH);
  
  delay(1000);
  
  digitalWrite(13,LOW);
  Bridge.put("D13", "0");
  //Serial.println("You're connected to the Console!!!");
  //digitalWrite(13,HIGH);
}

void loop() {
  // put your main code here, to run repeatedly:
  memset(buf, 0, 100);
  int buf_len = Bridge.get("D13",buf,100);
  //struct SpeedMessageFormat *temp = (struct SpeedMessageFormat *)buf;
    
  if (buf[0] != '0') {
    if (digitalRead(13) == HIGH) digitalWrite(13, LOW);
    else digitalWrite(13, HIGH);
    m_serial.write(buf, buf_len);
    //Serial.print(timer);
    //Serial.print(": ");
    //Serial.println((int16_t)temp->speed);
    //Serial.write((char *)buf, buf_len);
    //m_serial.write((char *)buf, buf_len);
    //Bridge.put("D13", "0");
  }
  else {
    if (digitalRead(13) == HIGH) digitalWrite(13, LOW);
    else digitalWrite(13, HIGH);
    m_serial.write(speed_stop, sizeof(speed_stop)/sizeof(byte));
    //Serial.println("command stop");
  }
  delay(50);
  return;
}
