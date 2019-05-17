/* Nuestro proyecto se trata de un ascensor virtual hecho en Arduino. Tiene las caracteristicas:
 *    - Pantalla LCD que muestra la hora y la fecha de manera continua. También muestra en que piso 
 *    se encuentra el ascensor y se está en movimiento.
 *    
 *    - Indicadores LED que muestra cuando el ascensor está disponible para ser llamado y cuando
 *    está en moviemiento.
 *    
 *    - Indicador sonoro de cuando llega a su destino.
 *    
 *    - Boton de llamada para que baje el ascensor.
 *    
 *    - Selector de piso numérico.
 */
 
/////////////////////////////////////////////////////////////////////////////////////////////////////
 
          //LIBRERIAS NECESARIAS PARA LOS DISTINTOS ELEMENTOS//

  #include <Wire.h>  // PAra conectarnos con el modulo L2C de la pantalla LCD
  #include <LiquidCrystal_I2C.h> //Biblioteca para el LCD
  #include <DS3231.h> //Biblioteca para el módulo RTC (Real Time Clock)
  #include <Keypad.h> //Libreria para selccionar los pisos con el lector numérico

/////////////////////////////////////////////////////////////////////////////////////////////////////

                    //PARAMETROS Y CONEXIONES// 
                    
/*Antes de poner las funciones necesitamos definir los parametros de cada elemento 
 * y los pines a los que están conectados cada parte del ascensor.
 */

  //LCD
  DS3231  rtc(SDA, SCL); //elementos del RTC que utilizamos
  LiquidCrystal_I2C lcd(0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Pines usados para la Pantalla LCD
  
  //LEDS
  int verde=13; //Conexion de los LEDS
  int azul=12;
  
  //BOTONES
  int boton=10; 

  //INTERRUPTOR
  const int interruptor = 9;
  
  //BUZZERS
  int buzzer=11;
  
  //KEYPAD NUMÉRICO
  const byte ROWS = 4; // Número de filas que tiene nuestro KEYPAD
  const byte COLS = 4; // Número de columnas
  char keys[ROWS][COLS] = //Definimos que hay en cada fila/columna
  {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
  };
  
  byte rowPins[ROWS] = {8, 7, 6, 5}; // Pines a los que se conecta cada fila
  byte colPins[COLS] = {4, 3, 2, 1}; // Pines para las columnas
  
  Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS); //Función necesaria para el KEYPAD
  
///////////////////////////////////////////////////////////////////////////////////////////////////// 

                          //DECLARACIÓN DE FUNCIONES//
                          
/* Al igual que en C, las funciones las tenemos que definir antes de introducrilas
 *  en la "main" y después ya las definimos.
 */

  void fecha_hora(); //Display de la fecha/hora en el LCD

  int llamada (); //Lo que sucede cuando pulsamos el boton
    int valor;
    int x=0;

  int leer_interruptor (); //Lee si el interruptor de alarma está encendido
    int interruptor_emergencia = 0;
    int y=0;

  void emergencia (); //Función de modo emergencia

  void bajada (); //Función que se encarga del movimiento de bajada

  void ascensor_espera(char c); // Imprime en pantalla un mensaje de espera
    char c;
  
  char seleccionar_piso(); //Una vez baja el ascensor, seleccionamos el piso
    char piso;
   
  void subida (char b); //Después de seleccionar el piso, el ascensor subirá
    char b;
    
/*Se puede apreciar que hemos definido algunas variables, estas son de caracter global 
 * (se utilizan en más de una función) por lo que es más cómodo definirlas aquí, así podemos
 * ver también que variable pertenece a cada función.
 */
 
/////////////////////////////////////////////////////////////////////////////////////////////////////

                                     //FUNCIÓN SETUP//
                                     
/* Arduino tiene dos funciones principales, la funcion SETUP y la función LOOP. En la función SETUP, 
 *  simplemente definimos el modo de los elementos (output/input) e inicialiciamos los elementos.
 * La función loop, sería la equivalente al "main" en C, a diferencia de que está repitiendose de 
 * manera continua a no ser que se le diga que se para o que termine el programa.
*/
 
void setup() 
{
  //Inicializamos los distintos módulos
    rtc.begin(); //Comienza el módulo RTC 
    lcd.begin(20,4); //Inicilizamos la pantalla
    Serial.begin (9600); //Aunque no la usamos, para algunas pruebas fue necesaria

  //Definción de modos
    pinMode (verde,OUTPUT);
    pinMode (azul, OUTPUT);
    
    pinMode (boton,INPUT);
    pinMode (interruptor,INPUT);
  
    pinMode (buzzer,OUTPUT);
}

                                    //FUNCIÓN LOOP//

void loop() 

{
  leer_interruptor (); //Lo primero que hacemos es ver si es modo emergencia está activado
  if (y == 0) //En caso de no estar activado, entramos en el modo "normal"
  {
    do
    {
      fecha_hora(); //Esta funcion está siempre activa
      llamada(); //Llamamos al ascensor, la cual devuelve un booleano
        if (x == 1 && y == 0) //Para que entre en el resto de comandos, tenemos que pulsar el botón (x==1 y que el no esté en modo emergencia)
        {
          lcd.clear(); //Limpiamos el lcd
          fecha_hora(); //Como lo hemos limpiado la tenemos que volver a llamar
          bajada(); 
          seleccionar_piso();
          b=seleccionar_piso(); //Asignamos el piso (b) al retorno de la función seleccionar piso
          subida (b); //Se lo mandamos a subida() para que sepa en que piso estamos
        }
        else
        {
          ascensor_espera(c); //Mensaje de espera cuando no estamos haciendo nada
        }
    }while (y == 0); //Mientras no haya ninguna emergencia
  }
  else
  {
    emergencia (); 
  } 
}


/////////////////////////////////////////////////////////////////////////////////////////////////////

                                    //FUNCIONES USADAS//

void fecha_hora ()
{
  lcd.setCursor (0,0); 
  lcd.print ("------");
  lcd.setCursor (14,0);
  lcd.print ("------");
  
  lcd.setCursor(6,0); 
  lcd.print(rtc.getTimeStr()); //Funcion de la libreria para obtener la hora

  lcd.setCursor (0,3);
  lcd.print ("-----");
  lcd.setCursor (15,3);
  lcd.print ("-----");

  lcd.setCursor(5,4);
  lcd.print(rtc.getDateStr()); //Funcion de la libreria para obtener la fecha
}

int llamada ()
{
  valor=digitalRead (boton); //Leemos el estado del botón
  if (valor == HIGH)
  {
    x = 1;
    digitalWrite(azul,LOW); //Apagamos el led de movimiento
  }
  else 
  {
    x = 0;
  }
  return x; //devolvemos el estado del boton
}

int leer_interruptor ()
{ 
  interruptor_emergencia = digitalRead (interruptor); //Leemos el estado del interruptor
  if (interruptor_emergencia == HIGH) 
  {
    y = 1;
  }
  else 
  {
    y = 0;
  }
  delay (1000);
  return y; //devolvemos un booleano
}

void emergencia ()
{
  int i=0;
  
  lcd.setCursor (0,0);
  lcd.print ("--------------------");
  lcd.setCursor (4,1);
  lcd.print ("Ascensor en:");
  lcd.setCursor (3,2);
  lcd.print ("!!EMERGENCIA!!");
  lcd.setCursor (0,3);
  lcd.print ("--------------------"); //Display en el LCD
  
  for (i=0;i<5;i++)
  {
      digitalWrite (azul,HIGH);
      digitalWrite (verde,HIGH);
      tone(buzzer,500,500);
      delay (250);
      digitalWrite (azul,LOW);
      digitalWrite (verde,LOW);
      tone(buzzer,350,300);
      delay (250); //Parpadeos de los LED y sonido del buzzer
  }  
}

void ascensor_espera(char c)
{

  digitalWrite (verde,HIGH); 
  lcd.setCursor (0,1);
  lcd.print ("Pulse el boton para"); 
  lcd.setCursor (0,2);
  lcd.print ("que baje el ascensor"); 
  
}

void bajada()
{
  int i;
  lcd.setCursor (2,1);
  lcd.print ("El ascensor esta:"); 
  lcd.setCursor (3,2);
  lcd.print ("...bajando..."); 
  digitalWrite (verde,LOW); 
  for (i=0;i<5;i++)
    {
      digitalWrite (azul,HIGH);
      delay (250);
      digitalWrite (azul,LOW);
      delay (250);
    }
    
  tone(buzzer,500,500);
  delay(300);
  tone(buzzer,350,300);
  delay(300);
  tone(buzzer,250,600); //Sonido que hace al llegar
  digitalWrite (verde,HIGH); //Indica que el ascensor está listo
  
  digitalWrite (azul,LOW); //Apagamos el led azul, ya que no estamos en movimiento
  
}

char seleccionar_piso () 
{
  lcd.clear();
  fecha_hora(); //La volvemos a llamar porque hemmos limpiado la pantalla
  
  lcd.setCursor (1,1);
  lcd.print ("Seleccione un piso");
  
  char piso = keypad.waitForKey(); //Funcion de la libreria keyPad para recoger el valor del teclado numérico
  
  lcd.setCursor (9,2);
  lcd.print ("-");
  
  lcd.setCursor (11,2);
  lcd.print (piso);
  delay (250);
  
  return piso;
  }

void subida(char b)
{
  int i;
    lcd.clear();
    fecha_hora();
    lcd.setCursor (0,1);
    lcd.print ("Subiendo al Piso: ");
  
    lcd.setCursor (18,1);
    lcd.print (b);
    digitalWrite (verde,LOW);
    for (i=0;i<5;i++)
    {
      digitalWrite (azul,HIGH);
      delay (250);
      digitalWrite (azul,LOW);
      delay (250);
    }
    
    tone(buzzer,500,500);
    delay(300);
    tone(buzzer,350,300);
    delay(300);
    tone(buzzer,250,600);
    digitalWrite (verde,HIGH);
}



  
    


 

