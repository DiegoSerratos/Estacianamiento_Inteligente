#include <Servo.h>
#include <SPI.h>
#include <MFRC522.h>
#include <esp_now.h>
#include <WiFi.h>
#include <LiquidCrystal.h>

String success;
#define COLS 16
#define ROWS 2
#define SS_PIN 21
#define RST_PIN 22
MFRC522 mfrc522(SS_PIN, RST_PIN); // Cree una instancia de MFRC522.
LiquidCrystal lcd_1(2,4,5,17,16,15);//Declaracion de los pines en el Display
Servo myservo;//Declaracion de la variable del Servomotor

typedef struct struct_message {//La estructura de los datos a recibir, debe conicidir en ambos Esp32
    int id; // Declaracion del identifiacador del Esp32 Remitente para no utilizar la MAC
    int x;  // Declaracion de la variable en la que se guardaran los datos enviados del Esp32 Emisor
} 
struct_message;//Llamada de struct_message
struct_message myData; //Variable que contiene las variables del mensaje

// Se crear una estructura para contener las lecturas de cada tablero
struct_message board1;

// Se crear un Array con todas las estructuras.
struct_message boardsStruct[1] = {board1};

esp_now_peer_info_t peerInfo;//Se crea una varible del tipo esp_now_peer_info_t para alamacenar informacion del Esp32 Emisor 

// función de devolución de llamada que se ejecutará cuando se reciban los datos
void OnDataRecv(const uint8_t * mac_addr, const uint8_t *incomingData, int len){
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  memcpy(&myData, incomingData, sizeof(myData));
  boardsStruct[1].x = myData.x;
  boardsStruct[1].id = myData.id;
}
int conteo=6;//Inicializamos el contador en el máximo de los cajones de estacionamiento y lo guardamos en la varible contador.

// Establecemos el número LCD de columnas y filas
int lcdColumns = 16;
int lcdRows = 2;

void setup() 
{
  lcd_1.begin(COLS, ROWS);//Especifica la configuracion concreta en el Display de las filas y las columnas.
  Serial.begin(9600);   // Se inicializa una comunicación serial.
  SPI.begin();      // Se inicializa el bus SPI
  mfrc522.PCD_Init();   // Se inicializa el lector RFID-MFRC522
  Serial.println("INICIALIZACION CORRECTA MFRC522");//Imprimimos en consola la inicializacion del lector RFID-MFRC522
  
  //  Creamos la inicializacion de Servo
  myservo.attach(13); //PIN donde se conecta el servo-motor de entrada
  myservo.write(90);//Posicion inicial del servo-motor de entra

  //Se establece el dispositivo como una estación Wi-Fi
  WiFi.mode(WIFI_STA);

  //Incializacion del ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");//Se imprime en pantall si no se inicializa carrectamente
    return;
  }

  // Una vez que ESPNow se inicie con éxito, nos registraremos para recv CB para obtener información del empaquetador recv
  esp_now_register_recv_cb(OnDataRecv);

  boardsStruct[1].x=1;//Inicializamos el envialdo por el Emisor de X a 1.
  Serial.println(conteo);//Imprimimos la variable contador.
  lcd_1.setCursor(0,0);//Establecemos la posicion del Display para el mensaje de Bienvenida.
  lcd_1.print("   Bienvenido");//Creamos el mensaje a plasmar en el Display.
  lcd_1.setCursor(0,1);//Establecemos la posicion del Display para los lugares disponibles.
  lcd_1.print("Disponibles:");//Creamos el mensaje a plasmar en el Display.
  lcd_1.setCursor(13,1);//Establecemos la posicion del Display para el contador.
  lcd_1.print(conteo);//Mostramos en pantalla el numero de espacios disponibles guardado en la varible conteo.
}

void loop(){
  if(boardsStruct[1].x == 0){//Cuando un carro sale del estacionamiento recibimos un 0 del Esp32 emisor y entramos al ciclo.
    conteo++;//Incrementamos un lugar disponible en el estacionamiento y lo guardamos en la variable conteo.
    lcd_1.setCursor(0,1);//Establecemos la posicion del Display para los lugares disponibles.
    lcd_1.print("Disponibles:" );//Creamos el mensaje a plasmar en el Display.
    lcd_1.setCursor(13,1);//Establecemos la posicion del Display para el contador.
    lcd_1.print(conteo);//Mostramos en pantalla el numero de espacios disponibles guardado en la varible conteo.
    Serial.println(conteo);//Imprimimos el contador en consola.
    mfrc522.PICC_HaltA();//Cerramos la comunicacion con la tarjeta RFID.
    boardsStruct[1].x = 1;//Ponemos a boardsStruct[1].x en 1.
  }
  //Buscamos una nuevas tarjetas.
  if ( ! mfrc522.PICC_IsNewCardPresent()){
    return;
  }
  //Se secciona una de las tarjetas.
  if ( ! mfrc522.PICC_ReadCardSerial()){
    return;
  }
  
  String content= "";
  byte letter;
  //Concatenamos los numeros hexadecimales 
  for (byte i = 0; i < mfrc522.uid.size; i++){
    content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  
  content.toUpperCase();//Convertimos los caracteres a mayusculas
  if (content.substring(1) == "43 87 13 BA"
    ||content.substring(1) == "75 4D 14 53"
    ||content.substring(1) == "83 AD FC AC"
    ||content.substring(1) == "9D 37 7B 25") //cambia aquí el UID de la/s tarjeta/s a las que quieres dar acceso
  {    
      if(conteo>=1&&conteo<=6){//Declaramos un limite inferior en el contador
          accesoEntrada();//Invocamos la funcion  accesoEntrada              
          mfrc522.PICC_HaltA();//Cerramos la conexió del RFI
      }else 
          if(conteo==0){//Creamos el ciclo del espacio lleno en el estacionamiento
              //Serial.println("Cupo lleno");
              conteo=0;//Delvolvemos a 0 la variable contador.
              sinCupo();//Invocamos la funcion sinCupo
          }      
  }else{
    Serial.println("Acceso Denegado");//Devolvemos el mensaje de acceso denegado en caso de no tener una targeta registrada
    mfrc522.PICC_HaltA(); //Cerramos la conexió del RFI
    delay(3000);
  }

  lcd_1.setCursor(0,1);//Establecemos la posicion del Display para los lugares disponibles.
  lcd_1.print("Disponibles:" );//Creamos el mensaje a plasmar en el Display.
  lcd_1.setCursor(13,1);//Establecemos la posicion del Display para el contador.
  lcd_1.print(conteo);//Mostramos en pantalla el numero de espacios disponibles guardado en la varible conteo.
  Serial.println(conteo);//Mostramos en consola el numero de espacios disponibles guardado en la varible conteo.
} 


void accesoEntrada(){
                    
   myservo.write(-90);//Se establece la posicion inicial del servo-motor
   Serial.println("abre...");//Mensaje cuando el servo-motor se levanta
   delay(2000);//Pausa el programa por 2 segundos
   myservo.write(100);//Se establece la posicion final del servo-motor
   Serial.println("cierra...");//Mensaje cuando el servo-motor se baja
   delay(1000);
   conteo--;//Se le resta un valor en el display al contador y se guarda en la variable contador 
}

int sinCupo(){
   myservo.write(100);//Se establece la posicion final del servo-motor
   delay(2000);//Pausa el programa por 2 segundo
}



                     
