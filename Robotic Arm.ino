/*Brazo robot*/

#include <Servo.h> // Librería para el control de servo motores.

// Declaracion de variables:

Servo servo_0;
Servo servo_1;
Servo servo_2;
Servo servo_3;

int sensorPin0 = A0; // Hombro
int sensorPin1 = A1; // Mano
int sensorPin2 = A2; // Codo
int sensorPin3 = A3; // Pinza

int arrayStep, arrayMax, counter, boton, steps, stepsMax, time = 1000, del = 1000, temp;

/*
arrayStep: almacena la posición en un array.
arrayMax: cantidad máxima de pasos almacenados en un array.
steps: pasos únicos para un movimiento entre posiciones almacenadas.
stepsMax: cantidad máxima de pasos que un servo debe realizar.
*/

unsigned int  dtime = 0;

long previousMillis1 = 0;
long previousMillis2 = 0;
long previousMillis3 = 0;
long previousMillis4 = 0;
long previousMicros = 0;
unsigned long currentMillis = millis();
unsigned long currentMicros = micros();

// Arrays:

int Delay[7] = {0,0,1,3,15,60,300};
int SensVal[4];
float dif[4], ist[4], sol[4],  dir[4];
int joint0[180];
int joint1[180];
int joint2[180];
int joint3[180];
int top = 179;

// Estados:

boolean playmode = false, Step = false;

void setup() // Inicialización
{
  pinMode(4, INPUT);
  pinMode(6, INPUT);
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);
  servo_0.attach(3);
  servo_1.attach(10);
  servo_2.attach(9);
  servo_3.attach(11);
  Serial.begin(115200); // Baudrate
  Serial.println("Brazo robot listo.");     
  delay(1000);
  digitalWrite(13, LOW);
}

void loop() // Programa principal
{
  currentMillis = millis();
  currentMicros = micros();
  
  Button(); // Lee el botón
  
  if(!playmode) // Modo manual
  {
    if(currentMillis - previousMillis1 > 25) // 25 milisegundos hasta la siguiente actualización manual
    {
      if (arrayStep < top)
      {
        previousMillis1 = currentMillis; // Reset
        readPot(); // Lee los potenciómetros
        mapping(); // Mapea a milisegundos para los servos
        move_servo(); // Establece una nueva posición para los servos
      }
    }
  }

  else if(playmode) // Modo reproducción
  {
    if (Step)
    {
      digitalWrite(13, HIGH);
      if (arrayStep < arrayMax)
      {
        arrayStep += 1;
        Read();
        calculate();
        Step = 0;
        digitalWrite(13, LOW);
      }
      else
      {
        arrayStep = 0; 
        calc_pause();
        counter = 0;
        while(counter < dtime)
        {
          counter += 1;
          calc_pause();
          digitalWrite(13, HIGH); delay(25);   
          digitalWrite(13, LOW); delay(975); 
        }
      }
    }
    else
    {
      if (currentMicros - previousMicros > time)
      {
        previousMicros = currentMicros;
        play_servo(); 
      }
    }
  }
  
  while (digitalRead(4) == false)
  {
    digitalWrite(13, HIGH); delay(500);
    digitalWrite(13, LOW); delay(500);
  }
}

// Subrutinas:

void calc_pause() // Lee el potenciómetro y mapea el tiempo de retardo utilizable después de realizar un movimiento completo
{
    readPot();
    temp = SensVal[3];
    if (temp < 0) temp = 0;
    temp = map(temp, 0, 680, 0 ,6); 
    dtime = Delay[temp];
}
void readPot() // Lee las entradas analógicas
{
  SensVal[0] = analogRead(sensorPin0);
  SensVal[1] = analogRead(sensorPin1);
  SensVal[2] = analogRead(sensorPin2);
  SensVal[3] = analogRead(sensorPin3);
}
void mapping() // Mapea los valores medidos de los potenciómetros a microsegundos para los servos
{
  ist[0] = map(SensVal[0], 150, 900, 600, 2400);
  ist[1] = map(SensVal[1], 1000, 100, 550, 2400);
  ist[2] = map(SensVal[2], 120, 860, 400, 2500);
  ist[3] = map(SensVal[3], 1023, 0, 500, 2500);
}
void record() // Almacena posiciones en el array del servo
{
  joint0[arrayStep] = ist[0];
  joint1[arrayStep] = ist[1];
  joint2[arrayStep] = ist[2];
  joint3[arrayStep] = ist[3];
}
void Read() // Lee el array
{
  sol[0] = joint0[arrayStep];
  sol[1] = joint1[arrayStep];
  sol[2] = joint2[arrayStep];
  sol[3] = joint3[arrayStep];
}
void move_servo() // Envia los valores de microsegundos a los servos
{
  servo_0.writeMicroseconds(ist[3]);
  servo_1.writeMicroseconds(ist[2]); 
  servo_2.writeMicroseconds(ist[0]); 
  servo_3.writeMicroseconds(ist[1]); 
}
void calculate() // Cálculo de cada paso individual
{
  // Distancia que debe recorrer cada servo:
  dif[0] = abs(ist[0]-sol[0]);
  dif[1] = abs(ist[1]-sol[1]);
  dif[2] = abs(ist[2]-sol[2]);
  dif[3] = abs(ist[3]-sol[3]);

  // Máximo recorrido de los cuatro servos:
  stepsMax = max(dif[0],dif[1]);
  stepsMax = max(stepsMax,dif[2]);
  stepsMax = max(stepsMax,dif[3]);
  // stepsMax es la mayor distancia que tiene un servo entre su posición momentánea y una nueva posición leída del array
      
  if (stepsMax < 500)
    del = 1200;
  else
    del = 600;
  
  /* 
  Calcula cada micropaso para cada servo
  Se necesita realizar esto para mover a todos los servos en un bucle con diferentes valores
  Con esto se consigue que todos recorran toda la distancia al mismo tiempo
  */
  if (sol[0] < ist[0]) dir[0] = 0-dif[0]/stepsMax; else dir[0] = dif[0]/stepsMax;
  if (sol[1] < ist[1]) dir[1] = 0-dif[1]/stepsMax; else dir[1] = dif[1]/stepsMax;
  if (sol[2] < ist[2]) dir[2] = 0-dif[2]/stepsMax; else dir[2] = dif[2]/stepsMax;
  if (sol[3] < ist[3]) dir[3] = 0-dif[3]/stepsMax; else dir[3] = dif[3]/stepsMax;

}
void play_servo()
{
  steps += 1;
  if (steps < stepsMax)
  {
    if(steps == 20) time = del*4;
    else if(steps == 40) time = del*3;
    else if(steps == 80) time = del*2;
    else if(steps == 100) time = del-1;
      
    if(steps == stepsMax-200) time = del*2;
    else if(steps == stepsMax-80) time = del*3;
    else if(steps == stepsMax-40) time = del*4;
    else if(steps == stepsMax-20) time = del*5;
      
    ist[0] += dir[0];
    ist[1] += dir[1];
    ist[2] += dir[2];
    ist[3] += dir[3];

    servo_0.writeMicroseconds(ist[3]); 
    servo_1.writeMicroseconds(ist[2]); 
    servo_2.writeMicroseconds(ist[0]);
    servo_3.writeMicroseconds(ist[1]);
  }
  else
  {
    Step = 1;
    steps = 0;
  }
}
void Button() // Verifica el click o doble click en los botones
{
  if (digitalRead(6) == false)
  {
    delay(1);
    if (digitalRead(6) == true)
    {
      if (boton == 0)
      {
        boton = 1;
        previousMillis3 = currentMillis;
      }
      else if ((boton == 1) && (currentMillis - previousMillis3 < 250))
      {
        boton = 2;
      }
    }
  }
  if ((boton == 1) && (currentMillis - previousMillis3 > 1000))
  {
    arrayStep += 1;
    arrayMax = arrayStep;
    record();
    boton = 0;
    playmode = false;
    Serial.print("Record Step: "); Serial.println(arrayStep);
    digitalWrite(13, HIGH);
    delay(100);
    digitalWrite(13, LOW);
  }
  else if (boton == 2)
  {
    arrayStep = 0;
    playmode = true;
    boton = 0;
    Step = 1;
    Serial.println("Playmode");
    delay(250);   
    digitalWrite(13, LOW);    
  }
  if (currentMillis - previousMillis3 > 2000)
  {
    boton = 0;
  }
}