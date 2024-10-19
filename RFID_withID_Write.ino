#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 5
#define RST_PIN 0
MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance

MFRC522::MIFARE_Key key;  // Create an instance of MIFARE_Key

void setup() {
  Serial.begin(115200);  // Initialize serial communications with the PC
  SPI.begin();           // Init SPI bus
  mfrc522.PCD_Init();    // Init MFRC522
  Serial.println("Place your card to write to ...");

  // Initialize the key to the default key (0xFF for each byte)
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }
}
void writeIDToBlock30(int idNumber) {
    MFRC522::StatusCode status;
    byte block30 = 30;  // Block number for ID
    char idBuffer[16];  // Buffer to store the ID as a string
    byte buffer[18];    // Buffer for writing to the block

    // Create the ID string in the format "TDx"
    snprintf(idBuffer, sizeof(idBuffer), "%d", idNumber);

    // Ensure the buffer is padded with zeros (null terminator)
    memset(buffer, 0, sizeof(buffer));
    memcpy(buffer, idBuffer, strlen(idBuffer));  // Copy the ID string into the buffer

    // Authenticate before writing to Block 30
    status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block30, &key, &(mfrc522.uid));
    if (status != MFRC522::STATUS_OK) {
        Serial.print(F("Authentication failed for block 30: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
        return;
    }

    // Write the ID to Block 30
    status = mfrc522.MIFARE_Write(block30, buffer, 16);
    if (status != MFRC522::STATUS_OK) {
        Serial.print(F("Writing failed for block 30: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
        return;
    }

    Serial.println(idBuffer);
}
  int currentID = 1;
void loop() {
  // Look for a card
  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  // Show UID on serial monitor
  Serial.print("Card UID:");
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
  }
  Serial.println();

  // Get Prénom, Nom de famille, and Description from user input
  Serial.println("Enter Prénom:");
  while (!Serial.available());  // Wait for input
  String prenom = Serial.readStringUntil('\n');  // Read until newline
  Serial.flush();  // Clear serial buffer for next input

  Serial.println("Enter Nom de famille:");
  while (!Serial.available());  // Wait for input
  String nomDeFamille = Serial.readStringUntil('\n');  // Read until newline
  Serial.flush();  // Clear serial buffer for next input

  Serial.println("Enter Description:");
  while (!Serial.available());  // Wait for input
  String description = Serial.readStringUntil('\n');  // Read until newline
  Serial.flush();  // Clear serial buffer for next input

  // Write to Block 8 for Prénom, Block 9 for Nom de famille
  writeDataToBlock(25, prenom);
  writeDataToBlock(26, nomDeFamille);

  // Write Description split across Block 5 and Block 6
  writeDescriptionToBlocks(28,29, description); ///////////////////////////////////////////////

  // Halt and stop communication with the card
  delay(1000);  // Wait a second before halting


  writeIDToBlock30(currentID);
  currentID++;
  mfrc522.PICC_HaltA();  // Halt the card
  mfrc522.PCD_StopCrypto1();  // Stop encryption on PCD

  delay(1000);  // Wait before reading the next card
}

void writeDataToBlock(byte block, String data) {
  MFRC522::StatusCode status;

  byte buffer[18]; // Buffer to hold the data to write
  byte len = data.length();

  if (len > 16) {
    len = 16;  // Truncate data if too long for a block (16 bytes max)
  }

  // Fill the buffer with the data
  for (byte i = 0; i < len; i++) {
    buffer[i] = data[i];
  }

  // Fill remaining buffer with 0x00 if data is less than 16 bytes
  for (byte i = len; i < 16; i++) {
    buffer[i] = 0x00;
  }

  // Authenticate and write to the block
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Authentication failed: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  // Write data to the block
  status = mfrc522.MIFARE_Write(block, buffer, 16);
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Write failed: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  Serial.print("Data written to block ");
  Serial.println(block);
}

void writeDescriptionToBlocks(byte block1, byte block2, String description) {
  // Split description into two parts if it exceeds 16 characters
  String part1 = description.substring(0, 16);  // First 16 characters
  String part2 = description.substring(16);     // Remaining characters

  // Ensure each part is exactly 16 characters long by padding with spaces
  while (part1.length() < 16) part1 += ' ';
  while (part2.length() < 16) part2 += ' ';

  // Write the first part to block1
  writeDataToBlock(block1, part1);

  // Write the second part to block2
  writeDataToBlock(block2, part2);
}
