#include "AESLib.h"
AESLib aesLib;
  
#include <SPI.h>
#include <LoRa.h>

const int csPin = 5;          // LoRa radio chip select
const int resetPin = 26;        // LoRa radio reset
const int irqPin = 13;          // change for your board; must be a hardware interrupt pin

char cleartext[256];
char ciphertext[512];
// AES Encryption Key
byte aes_key[] = { 0x15, 0x2B, 0x7E, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C };
// General initialization vector (you must use your own IV's in production for full security!!!)
byte aes_iv[N_BLOCK] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
// Generate IV (once)

int loopcount = 0;

void aes_init() {
  aesLib.gen_iv(aes_iv);
  // workaround for incorrect B64 functionality on first run...
  encrypt("HELLO WORLD!", aes_iv);
}

String encrypt(char * msg, byte iv[]) {  
  int msgLen = strlen(msg);
  char encrypted[2 * msgLen];
  aesLib.encrypt64(msg, msgLen, encrypted, aes_key,sizeof(aes_key) ,iv);  
  return String(encrypted);
}

/*String decrypt(char * msg, byte iv[]) {
  unsigned long ms = micros();
  int msgLen = strlen(msg);
  char decrypted[msgLen]; // half may be enough
  aesLib.decrypt64(msg, msgLen, decrypted, aes_key,sizeof(aes_key) ,iv);  
  return String(decrypted);
}
*/

void setup() {
  Serial.begin(115200);
  while (!Serial);
  aes_init();
  Serial.println("LoRa Sender");
  LoRa.setPins(csPin, resetPin, irqPin);
  if (!LoRa.begin(915E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
}

void loop() {
  loopcount++;
  
  // Encrypt
  sprintf(cleartext, "data : %i \n", loopcount);  
  byte enc_iv[N_BLOCK] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }; // iv_block gets written to, provide own fresh copy...
  String encrypted = encrypt(cleartext, enc_iv);
  sprintf(ciphertext, "%s", encrypted.c_str());
  Serial.print("Ciphertext: ");
  Serial.println(encrypted);
  
   // send packet
  LoRa.beginPacket();
  LoRa.print(encrypted);
  LoRa.endPacket();

  // Decrypt
  //byte dec_iv[N_BLOCK] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }; // iv_block gets written to, provide own fresh copy...
  //String decrypted = decrypt(ciphertext, dec_iv);  
  //Serial.print("Cleartext: ");
  //Serial.println(decrypted);  
  delay(3000);
}
