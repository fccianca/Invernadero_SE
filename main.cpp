/*
Temperatura: 16 a 29.5ºC
Humedad del suelo: 70% a 80%
Luz: Lugar que reciba luz solar. Al menos 6 horas de luz al día.
*/
#include <Arduino.h>
#include <Wire.h>
#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include <driver/adc.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <OneWire.h>
#include <DallasTemperature.h>

//if CONFIG_FREERTOS_UNICORE


//https://invernaderose-103fe-default-rtdb.firebaseio.com/ 

#define DHTTYPE DHT11
#define s_nivel 33
// LM35    2-150 °C    10 mV/°C
#define s_temp 32 // DHT11 ////DS18B20
#define fotoR 35
#define s_humedad 34 // Sensor suelo resistivo FC 28

#define ledNivel 18
#define bomba 23
#define ventilador 19
#define luces 5

#define tempMAX 29.5
#define tempMIN 16.0
#define humedMIN 4050
#define humedMAX 500
#define lumenMIN 2300

//Entradas
    //Fotorresistor 1.5 k (Luz) - 45k (oscuridad)
//adc1_channel_t fotoR = ADC1_CHANNEL_7; // gpio35
    // DHT11 Temp (0-50°C) y HumdRel (20-90%) de 3.3 a 5.5 V
//adc1_channel_t s_temp = ADC1_CHANNEL_4; // gpio32
    // En suelo humedo (300-700) 
//adc1_channel_t s_humedad = ADC1_CHANNEL_6; // gpio34

uint8_t estadoLed = 0;
bool estado = false;
int nivel = 0;
int static lumen = 0;
int static status = 0;
float tempC;
float tempF;
float humedDHT;
uint16_t humedad;
//DHT dht(s_temp, DHTTYPE);
//DS18B20
//OneWire oneWire(s_temp);
//DallasTemperature sensTemp(&oneWire);
/*
void TaskRiego(void *parameter);
void TaskRiegoNoche(void *parameter );
void TaskRiegoHum(void *parameter );
*/

void configIO();
void controlBomba();
void riegoNoche();
void riego();
void riegoHum();
void ventilar();
void iluminar();
void blinkLed();
void sensar();

void setup() {
    //Serial.begin(115200);
    //vTaskDelay(1000 / portTICK_PERIOD_MS);
    configIO();
}

void loop() {

    //delay(200);
    vTaskDelay(500 / portTICK_PERIOD_MS);
    controlBomba();
    riegoHum();
    riego();
    riegoNoche();
    //vTaskDelay(1000 / portTICK_PERIOD_MS);
}

void configIO() {

    pinMode(s_nivel, INPUT_PULLUP);
    pinMode(s_temp, INPUT);
    pinMode(ledNivel, OUTPUT);
    pinMode(bomba, OUTPUT);
    pinMode(ventilador, OUTPUT);
    pinMode(luces, OUTPUT);

    //ADC
    analogSetWidth(12);  // 12 bits
    analogSetAttenuation(ADC_11db); // Factor divisor 0-4095  
    digitalWrite(bomba, HIGH);
    digitalWrite(ventilador, HIGH);
    //dht.begin();
    //sensTemp.begin();

    //Tareas 
    /*
    xTaskCreatePinnedToCore (
        TaskRiego,  // Function of task
        "Riego",    // Name of task
        1024,       // Stack size, min is 768 tarea vacia
        NULL,       // Parametro a pasar a la funcion
        1,          // Priority (min 0 - max 24) (0 to configMAX_PRIORITIES -1)
        NULL,       // Task handle
        0);         // nucleo CPU 0 o cpu 1

    xTaskCreatePinnedToCore (
        TaskRiegoNoche,  // Function of task
        "RiegoNoche",    // Name of task
        1024,       // Stack size, min is 768 tarea vacia
        NULL,       // Parametro a pasar a la funcion
        1,          // Priority (min 0 - max 24) (0 to configMAX_PRIORITIES -1)
        NULL,       // Task handle
        0);         // nucleo CPU 0 o cpu 1
    
    xTaskCreatePinnedToCore (
        TaskRiegoHum,  // Function of task
        "RiegoHum",    // Name of task
        1024,       // Stack size, min is 768 tarea vacia
        NULL,       // Parametro a pasar a la funcion
        1,          // Priority (min 0 - max 24) (0 to configMAX_PRIORITIES -1)
        NULL,       // Task handle
        0);         // nucleo CPU 0 o cpu 1
    */
    //vTaskStartScheduler();
}

void blinkLed(){

    digitalWrite(ledNivel, HIGH);
    vTaskDelay(200 / portTICK_PERIOD_MS);
    digitalWrite(ledNivel, LOW);
    vTaskDelay(200 / portTICK_PERIOD_MS);
}

void controlBomba(){    
    
    estado = true;
    nivel = digitalRead(s_nivel);
    if (nivel == 1){
        digitalWrite(bomba, HIGH);
        printf(">>>>>Cisterna vacia\n");
        while (nivel == 1){ 
            blinkLed();
            nivel = digitalRead(s_nivel);
        }
        digitalWrite(ledNivel, LOW);
        estado = false;
    }
    else{
        digitalWrite(ledNivel, LOW);
    }
}

void sensar(){
}

void riegoHum(){

    controlBomba();
    humedad = analogRead(s_humedad);
    printf("Humedad: %d\n", humedad);
    if(humedad > humedMIN){             
        digitalWrite(bomba, 0);
        while(humedad > humedMIN && estado){
            controlBomba();
            humedad = analogRead(s_humedad);
            delay(800);
        }
    }
    else{
        digitalWrite(bomba, 1);
    } 
}

void riegoNoche(){
    controlBomba();
    //fotoR  con luz = 1.5 K (0), sin luz 20 Kohm (1) 
    //uso 2000 ADC
    lumen = analogRead(fotoR);
    printf("Lumen: %d\n\n", lumen);
   
    if(!digitalRead(s_nivel) && (lumen < lumenMIN)){
        digitalWrite(bomba, LOW);
        digitalWrite(luces, HIGH);     
        vTaskDelay(4000 / portTICK_PERIOD_MS);
        digitalWrite(bomba, HIGH);
        while(lumen < lumenMIN && estado){
            controlBomba();
            riegoHum();
            lumen = analogRead(fotoR);
            delay(500);              
        }
    }
    if(lumen > 2000){
        digitalWrite(luces, LOW);
        delay(200);
    }
}

void riego(){
    
    controlBomba();
    //sensTemp.requestTemperatures(); 
    //tempC = sensTemp.getTempCByIndex(0);
    //tempF = sensTemp.getTempFByIndex(0);
    /*
    tempC = dht.readTemperature();
    tempF = dht.readTemperature(true);
    humedDHT = dht.readHumidity();
    if (isnan(humedDHT) || isnan(tempC) || isnan(tempF)) {
        printf("Failed to read from DHT sensor!");
        //return;
    }
    */
    // LM35    2-150 °C    10 mV/°C
    tempC = analogReadMilliVolts(s_temp)/10.0;
    tempF = tempC * 9.0 / 5.0 + 32.0;

    printf("Temperatura: %.2f °C - %.2f °F\n", tempC, tempF);
    //printf("HumedadDHT: %2.f %\n", humedDHT);

    if(tempC > tempMAX && !digitalRead(s_nivel)){
        
        digitalWrite(bomba, 0);
        digitalWrite(ventilador, 0);
        vTaskDelay(5000 / portTICK_PERIOD_MS);
        digitalWrite(bomba, 1);
        while(tempC > tempMAX && estado){
            controlBomba();
            riegoHum();
            //tempC = dht.readTemperature();
            //sensTemp.requestTemperatures(); 
            //tempC = sensTemp.getTempCByIndex(0);
            //tempF = sensTemp.getTempFByIndex(0);
            tempC = analogReadMilliVolts(s_temp)/10.0;
            tempF = tempC * 9.0 / 5.0 + 32.0;

            printf("Temperatura: %.2f °C - %.2f °F\n", tempC, tempF);
            vTaskDelay(3000 / portTICK_PERIOD_MS);
        }
        //vTaskDelay(3000 / portTICK_PERIOD_MS);
        digitalWrite(ventilador, 1);
        //vTaskDelay(500 / portTICK_PERIOD_MS);
    }  
    else{
        //digitalWrite(ventilador, 1);
        //digitalWrite(bomba, 1);
        printf(">>>Temperatura estable entre 16 y 30 °C\n");
        //digitalWrite(luces, 1);
    }
    vTaskDelay(2000 / portTICK_PERIOD_MS);
}

void ventilar(){

    humedad = analogRead(s_humedad);
    printf("Humedad: %d\n", humedad);
    if(humedad > 2000){
        digitalWrite(ventilador, 1);
        vTaskDelay(3000 / portTICK_RATE_MS);
        digitalWrite(ventilador, 0);
    }  
    else if(humedad < 1000 && !digitalRead(s_nivel)){
        digitalWrite(bomba, 1);
        vTaskDelay(3000 / portTICK_RATE_MS);
        digitalWrite(bomba, 0);
    }
}

void iluminar(){

    if(tempC < 20){
        digitalWrite(luces, 1);
        vTaskDelay(3000 / portTICK_RATE_MS);
        digitalWrite(luces, 0);
    }  
}

/*
void TaskRiego(void *parameter){
   while(1){
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        riego();
        //vTaskSuspend(thRiego);    
    }
}

void TaskRiegoNoche(void *parameter ){
    //while(1){
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        riegoNoche();
    //}
}

void TaskRiegoHum(void *parameter ){
    while(1){
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        riegoHum();
    }
}
*/
