#include <Servo.h>
#include <SPI.h>
#include <MFRC522.h>
#include <esp_now.h>
#include <WiFi.h>

String success;
float dato;//Incializamos la variable datos a tipo flotante
uint8_t broadcastAddress[] = {0xEC, 0x62, 0x60, 0x93, 0x92, 0xF8};//Colocamos la MAC del Esp32 Receptor
int e=1;//Inicializamo la ariable e en 1

#define SS_PIN 21
#define RST_PIN 22

MFRC522 mfrc522(SS_PIN, RST_PIN);// Cree una instancia de MFRC522.
Servo myservo;//Declaracion de la variable del Servomotor

typedef struct struct_message {//La estructura de los datos a recibir, debe conicidir en ambos Esp32
    int id; // Declaracion del identifiacador del Esp32 Remitente para no utilizar la MAC
    int x;  // Declaracion de la variable en la que se guardaran los datos enviados del Esp32 Emisor
} 
struct_message; //Llamada de struct_message
struct_message myData; //Variable que contiene las variables del mensaje
esp_now_peer_info_t peerInfo; //Se crea una varible del tipo esp_now_peer_info_t para alamacenar informacion del Esp32 Emisor 

// función de devolución de llamada que se ejecutará cuando se reciba el estado de los dtos
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  if (status ==0){
     success = "Delivery Success :)";
  } else{
      success = "Delivery Fail :(";
    }
}
// función de devolución de llamada que se ejecutará cuando se reciban los datos
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&myData, incomingData, sizeof(myData));
  Serial.print("Bytes received: ");
  Serial.println(len);
  dato = myData.x;
}

void setup() 
{
  Serial.begin(9600);   // Inicializamos una comunicación serial
  SPI.begin();  // Se inicializa el bus SPI
  mfrc522.PCD_Init();  // Se inicializa el lector RFID-MFRC522
  Serial.println("INICIALIZACION CORRECTA MFRC522"); //Imprimimos en consola la inicializacion del lector RFID-
  Serial.println();
  //servo inicializacion
  myservo.attach(13); //PIN donde se conecta el servo-motor de salida
  myservo.write(90);  //Posicion inicial del servo-motor de salida 
  WiFi.mode(WIFI_STA);// Establecer el dispositivo como una estación Wi-Fi
  // Inicializamos ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");//Se imprime en pantall si no se inicializa carrectamente
    return;
  }

  // Una vez que ESPNow se inicie con éxito, nos registraremos para Enviar CB a
  // obtener el estado del paquete transmitido
  esp_now_register_send_cb(OnDataSent);
  
  // Registramos peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
  esp_now_register_recv_cb(OnDataRecv);// Una vez que ESPNow se inicie con éxito, nos registraremos para recv CB para obtener información del empaquetador recv
}

void loop() {
  e=1;//Inicializamos la variable e en 1.
  
  if ( ! mfrc522.PICC_IsNewCardPresent()){// Busca nuevos llaveros RFID
    return;
  }
  
  if ( ! mfrc522.PICC_ReadCardSerial()){//Se secciona una de las tarjetas.
    return;
  }
  //Mostrar UID en el monitor Serial
  Serial.print("UID tag :");
  String content= "";
  byte letter;
  //Concatenamos los numeros hexadecimales 
  for (byte i = 0; i < mfrc522.uid.size; i++) 
  {
     Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
     Serial.print(mfrc522.uid.uidByte[i], HEX);
     content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
     content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  Serial.println();
  Serial.print("Message : ");
  content.toUpperCase();//Convertimos los caracteres a mayusculas
   if (content.substring(1) == "43 87 13 BA"
      ||content.substring(1) == "75 4D 14 53"
      ||content.substring(1) == "83 AD FC AC"
      ||content.substring(1) == "9D 37 7B 25") //change here the UID of the card/cards that you want to give access
  {
    Serial.println("Acceso Autorizado");//imprimimos el mensaje de acceso autorizado en consola
    Serial.println();
    e=0; //Inicializamos la variable e igual a 0
    accesoEntrada();
    mfrc522.PICC_HaltA(); //Cerramos la conexió del RFI
  } else{
      Serial.println("Acceso Denegado");
      e=1; //Inicializamos la variable e igual a 1
      mfrc522.PICC_HaltA(); //Cerramos la conexió del RFI
      delay(3000);
     }

 // Establecer valores para enviar
  myData.id = 2;
  myData.x = e;

  // Enviar mensaje a través de ESP-NOW
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
} 

void accesoEntrada(){
   myservo.write(100);//Se establece la posicion inicil del servo-motor
   Serial.println("abre...");//Mensaje cuando el servo-motor se levanta
   delay(2000);//Pausa el programa por 2 segundos
   myservo.write(-90);//Se establece la posicion final del servo-motor
   Serial.println("cierra...");//Mensaje cuando el servo-motor se baja
   delay(1000);
}
