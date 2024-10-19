#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN         0           // Configurable, see typical pin layout above
#define SS_PIN          5           // Configurable, see typical pin layout above

#include <WiFi.h>
#include <WebServer.h>

// Nom du réseau et mot de passe du point d'accès
const char *ssid = "ESP32_AP";
const char *password = "12345678";

// Crée un serveur web sur le port 80
WebServer server(80);

// Fonction pour gérer la racine ("/") et afficher "Hello World"
String dataToSend = "";  // Variable pour stocker les données à envoyer

  void handleRoot() {
   String html = "<html><body><h1>RFID Data</h1>";
   html += "<p>" + dataToSend + "</p>";
   html += "</body></html>";
   server.send(200, "text/html", html);
}



MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance

void setup() {
  Serial.begin(115200);                                           // Initialize serial communications with the PC
  SPI.begin();                                                    // Init SPI bus
  mfrc522.PCD_Init(); 
  WiFi.softAP(ssid, password);

  // Afficher l'adresse IP du point d'accès
  IPAddress IP = WiFi.softAPIP();
  Serial.print("Point d'accès démarré. IP: ");
  Serial.println(IP);

  // Définir la route de la page principale "/"
  server.on("/", handleRoot);

  // Démarrage du serveur web
  server.begin();
  Serial.println("Serveur web démarré.");                                            // Init MFRC522 card
  Serial.println(F("Ready to read Block 8, Block 9, Block 10, and Block 11 data on a MIFARE PICC:"));
}

void loop() {
    server.handleClient();

    MFRC522::MIFARE_Key key;
    for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;

    byte block8 = 25;  // Block number for first name (Prenom)
    byte block9 = 26;  // Block number for last name (Nom de famille)
    byte block10 = 28; // Block for description (part 1)
    byte block11 = 29; // Block for description (part 2)
    byte buffer[18];    // Buffer to store the data from the blocks
    byte len = 18;      // Expected length of data to read
    MFRC522::StatusCode status;

    if (!mfrc522.PICC_IsNewCardPresent()) {
        return;
    }

    if (!mfrc522.PICC_ReadCardSerial()) {
        return;
    }

    Serial.println(F("**Card Detected:**"));
    mfrc522.PICC_DumpDetailsToSerial(&(mfrc522.uid)); // Dump card details

    //--- Reading data from Block 8 (Prenom) ---
    Serial.print(F("Reading Prenom from block "));
    Serial.println(block8);

    status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block8, &key, &(mfrc522.uid));
    if (status != MFRC522::STATUS_OK) {
        Serial.print(F("Authentication failed for Prenom : "));
        Serial.println(mfrc522.GetStatusCodeName(status));
        return;
    }

    status = mfrc522.MIFARE_Read(block8, buffer, &len);
    if (status != MFRC522::STATUS_OK) {
        Serial.print(F("Reading failed for Prenom :"));
        Serial.println(mfrc522.GetStatusCodeName(status));
        return;
    }

    // Print the data from Block 8 (first name)
    Serial.print(F("Data in block 8: "));
    for (uint8_t i = 0; i < 16; i++) {
        Serial.print(buffer[i], HEX); // Print hex values for debugging
        Serial.print(" ");
        if (buffer[i] >= 32 && buffer[i] <= 126) {  // Only print valid ASCII characters
            Serial.write(buffer[i]);
        } else {
            break;  // Stop if we encounter padding or invalid characters
        }
    }
    Serial.println(); // New line after block data

    // Store the Prenom in dataToSend
    // Après avoir lu le prénom
dataToSend = "Prenom: ";
for (uint8_t i = 0; i < 16; i++) {
    if (buffer[i] >= 32 && buffer[i] <= 126) {
        dataToSend += (char)buffer[i];
    } else {
        break;
    }
}
dataToSend += "<br>";

// Debug output for dataToSend
Serial.print(F("Data to send: "));
Serial.println(dataToSend);

//--- Reading data from Block 9 (Nom de famille) ---
Serial.print(F("Nom de famille : "));

status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block9, &key, &(mfrc522.uid));
if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Authentication failed for Nom de famille : "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
}

status = mfrc522.MIFARE_Read(block9, buffer, &len);
if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Reading failed for Nom de famille :"));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
}

// Print the data from Block 9 (last name)
Serial.print(F("Data in block 9: "));
for (uint8_t i = 0; i < 16; i++) {
    if (buffer[i] >= 32 && buffer[i] <= 126) {  // Only print valid ASCII characters
        Serial.write(buffer[i]);
    } else {
        break;  // Stop if we encounter padding or invalid characters
    }
}
Serial.println(); // New line after block data

// Store the Nom de famille in dataToSend
dataToSend += "Nom de famille: "; // Ajoutez au lieu de réinitialiser
for (uint8_t i = 0; i < 16; i++) {
    if (buffer[i] >= 32 && buffer[i] <= 126) {
        dataToSend += (char)buffer[i];
    } else {
        break;
    }
}
dataToSend += "<br>";

// Debug output for dataToSend
Serial.print(F("Data to send: "));
Serial.println(dataToSend);

  Serial.println();  // Add a new line

  //--- Reading data from Block 10 and Block 11 (Description) ---
  Serial.print(F("Description  "));

  // Reading Block 10
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block10, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Authentication failed for Description "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  status = mfrc522.MIFARE_Read(block10, buffer, &len);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Reading failed for Desc1 "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  // Print the data from Block 10
  for (uint8_t i = 0; i < 16; i++) {
    if (buffer[i] >= 32 && buffer[i] <= 126) {  // Only print valid ASCII characters
      Serial.write(buffer[i]);
    } else {
      break;
    }
  }

  // Reading Block 11
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block11, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Authentication failed for Desc2 "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  status = mfrc522.MIFARE_Read(block11, buffer, &len);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Reading failed for Desc2 "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  // Print the data from Block 11
  for (uint8_t i = 0; i < 16; i++) {
    if (buffer[i] >= 32 && buffer[i] <= 126) {  // Only print valid ASCII characters
      Serial.write(buffer[i]);
    } else {
      break;
    }
  }

  Serial.println();  // Add a new line

//--- Reading data from Block 30 (ID) ---
byte block30 = 30;  // Block number for the ID
Serial.print(F("User ID: "));

status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block30, &key, &(mfrc522.uid));
if (status != MFRC522::STATUS_OK) {
  Serial.print(F("Authentication failed for UserID "));
  Serial.println(mfrc522.GetStatusCodeName(status));
  return;
}

status = mfrc522.MIFARE_Read(block30, buffer, &len);
if (status != MFRC522::STATUS_OK) {
  Serial.print(F("Reading failed for block 30: "));
  Serial.println(mfrc522.GetStatusCodeName(status));
  return;
}

// Print the data from Block 30 (ID)
for (uint8_t i = 0; i < 16; i++) {
  if (buffer[i] >= 32 && buffer[i] <= 126) {  // Only print valid ASCII characters
    Serial.write(buffer[i]);
  } else {
    break;  // Stop if we encounter padding or invalid characters
  }
}

Serial.println();  // Add a new line


  Serial.println(F("**End Reading Blocks**\n"));

  delay(1000); // Adjust the delay if you want to read cards faster

  mfrc522.PICC_HaltA();          // Halt PICC
  mfrc522.PCD_StopCrypto1(); 
      // Stop encryption on PCD
}
