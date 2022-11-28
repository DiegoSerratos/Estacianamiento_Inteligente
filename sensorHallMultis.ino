//variables para sacar el campo magnetico
const float sensibilidad = 0.0014;
float campoGauss;
//variables para los ciclos de sensores y led (total)
#define SENSORS 6
#define LEDS    6
//cajones en total
int conteo=6;

//array con los pines de todos los sensores
const int sensorN[SENSORS] = {A0, A1, A2, A3, A4, A5};
//array con los pines de todos los LEDS
const int ledN[LEDS] = {2, 3, 4, 5, 6, 7};

void setup() {//llave de inicio setup

  //CICLO PARA recorrer la cantidad de terminales y marcarlas como entrada
  for(int i; i < SENSORS; i++){
    pinMode(sensorN[i], INPUT);
  }

  //CICLO PARA recorrer la cantidad de leds y marcarlos como salida
  for(int i; i < LEDS; i++) {
    pinMode(ledN[i], OUTPUT);
  }
  Serial.begin(9600);
}//llave de cierre setup


void loop() {//llave de inicio loop
  //variable para que corra el codigo sin parar 
  int i = 0;
  //ciclo para recorrer varias veces el codigo
  while(i < SENSORS) {//llave inicio del while

        //variable que guarda el resultado de la funcion medicion(array que contiene los pines de los sensores);
        int result = medicion(sensorN[i]);
              Serial.println("Campo magnético en Gauss sensor ");//etiqueta campo magnetico
              Serial.println(i);//etiqueta del sensor
              Serial.println(result);//etiqueta del resultado arrojado por la funcion
        //condicional para saber si el sensor sufre una alteracion en el rango 300 a -300 enciende el led correspondiente 
        if(result>100 || result < -200){
            digitalWrite(ledN[i], HIGH);//enciende el led rojo

        }else{
            digitalWrite(ledN[i], LOW);//apaga el led rojo
                            
        } 
        i++; //aumento del while
   } //llave inicio del while
        
  delay(1000);

}//llave de cierre loop

 //funcion para saber el campoGaus (sabremos la alteracion del sensor hall)
int medicion(int sensor){
  //Promedio de 100 medidas para mayor precisión
  long medicion = 0;
  for (int i = 0; i < 10; i++) {
    int value =
    medicion += analogRead(sensor);
  }
  medicion /= 10;
                  
  //Calculo del voltaje
  float outputV = medicion * 5.0 / 1023;
                  
  //Calculo de la densidad de flujo magnético
  campoGauss =  (outputV - 2.5) / sensibilidad;
                    
  return campoGauss;
}
