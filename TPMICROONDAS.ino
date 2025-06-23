#include <Stepper.h>
#include <Keypad.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>

#define BUZZER_PIN 3
#define PUERTA_PIN 2
#define LED_INTERIOR 4
#define MOTOR_PLATO A3

/**************Keypad****************/
const byte FILAS = 4;
const byte COLS =4;

char teclas[FILAS][COLS]=
{
  '1','2','3','A',
  '4','5','6','B',
  '7','8','9','C',
  '*','0','#','D'  
};

byte pinesFilas[FILAS] = {12,11,10,9};
byte pinesCols[COLS] = {8,7,6,5};

Keypad kp = Keypad(makeKeymap(teclas), pinesFilas, pinesCols, FILAS, COLS);

/*****************Display********************/

const byte MAX_CHARS = 16;
const byte MAX_ROWS = 2;

short i = 0;
byte filaActual = 0;

LiquidCrystal_I2C lcd(0x20, MAX_CHARS, MAX_ROWS);

/*******************Programas*****************/
struct Programa {
  int calentar;
  int apagado;
  int repeticiones;
};

Programa programas[4]; // A, B, C, D

bool programaActivo = false;
bool cancelado = false;


//funciones
bool puertaAbierta();
void cargarProgramasPorDefecto();
int leerNumero();
void configurarPrograma();
void cancelarPrograma();
void iniciarPrograma(int index);
void repetirPrograma(int rep);

void setup() {
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(PUERTA_PIN, INPUT_PULLUP);
  pinMode(LED_INTERIOR, OUTPUT);
  pinMode(MOTOR_PLATO, OUTPUT);

  lcd.init();
  lcd.backlight();

  cargarProgramasPorDefecto();

  lcd.setCursor(0, 0);
  lcd.print("Listo");
  delay(1000);
  lcd.clear();

  Serial.begin(9600);
}

void loop() {
  //puerta abierta
  if(puertaAbierta() && !programaActivo){
    digitalWrite(LED_INTERIOR, HIGH);
  }else if(!programaActivo){
    digitalWrite(LED_INTERIOR, HIGH);
  }

  char tecla = kp.getKey();
  if(tecla){
    switch(tecla){
      case 'A':
        iniciarPrograma(0);
        break;
      case 'B':
        iniciarPrograma(1);
        break;
      case 'C':
        iniciarPrograma(2);
        break;
      case 'D': 
        iniciarPrograma(3);
        break;
      case '#':
        if(!programaActivo){
          configurarPrograma();
        }
        break;
      case '*':
        cancelarPrograma();
        break;
      case '1':
        repetirPrograma(1);
        break;
      case '2':
        repetirPrograma(2);
        break;
      case '3':
        repetirPrograma(3);
        break;
      case '4':
        repetirPrograma(4);
        break;
      case '5':
        repetirPrograma(5);
        break;
      case '6':
        repetirPrograma(6);
        break;
      case '7':
        repetirPrograma(7);
        break;
      case '8':
        repetirPrograma(8);
        break;
      case '9':
        repetirPrograma(9);
        break;
    }
  }

  //pausar si se abre la puerta
  while(programaActivo && puertaAbierta()){
    lcd.clear();
    lcd.print("Puerta abierta");
    digitalWrite(MOTOR_PLATO,LOW);
    digitalWrite(LUZ_INTERIOR,HIGH);
    noTone(BUZZER_PIN);
    delay(1000);
  }
}

//verificar puerta
bool puertaAbierta(){
  return digitalRead(PUERTA_PIN) == LOW; // LOW = la puerta esta abierta
}

//programas por defecto
void cargarProgramasPorDefecto() {
  programas[0] = {30, 0, 1}; // A
  programas[1] = {20, 10, 5}; // B
  programas[2] = {15, 3, 3}; // C

  // Leer desde EEPROM si hay uno personalizado guardado
  EEPROM.get(0, programas[3]);
  if (programas[3].calentar <= 0 || programas[3].calentar > 180) {
    // No hay guardado válido → cargar uno default
    programas[3] = {10, 5, 2};
  }
}

//permitir ingresar multiples digitos
int leerNumero(){
  String num = "";
  while(true){
    char t = kp.getKey();
    if(t == '#')break; //confirmar 
    if(t >= '0' && t <= '9'){
      num += t;
      lcd.setCursor(0,1);
      lcd.print(num);
    }else if(t == '*')return -1;//cancelar
  }
  return num.toInt();
}

//configurar programa(tecla D)
void configurarPrograma(){
  lcd.clear();
  lcd.print("tiempo calentar:");
  int calentar = leerNumero();

  lcd.clear();
  lcd.print("tiempo apagado");
  int apagado = leerNumero();

  lcd.clear();
  lcd.print("repeticiones");
  int repeticiones = leerNumero();

  programas[3] = {calentar,apagado,repeticiones};
  EEPROM.put(0,programas[3]);

  lcd.clear();
  lcd.print("Guardado");
  delay(1000);
}

//cancelar programa
void cancelarPrograma(){
  cancelado = true;
  programaActivo = false;
  noTone(BUZZER_PIN);
  digitalWrite(MOTOR_PLATO, LOW);
  digitalWrite(LUZ_INTERIOR, LOW);
  lcd.clear();
  lcd.print("Cancelado");
  delay(1000);
}
//ejecutar programa
void iniciarPrograma(int index){
  if(puertaAbierta()){
    lcd.clear();
    lcd.print("Cierre la puerta");
    delay(2000);
    return;
  }

  Programa p = programas[index];
  int tiempoTotal = (p.calentar + p.apagado) * p.repeticiones;
  programaActivo = true;
  cancelado = false;

  for(int r = 0; r <p.repeticiones && !cancelado; r++){
    //calentar
    lcd.clear();
    lcd.print("Calentando...");
    digitalWrite(LUZ_INTERIOR, HIGH);
    digitalWrite(MOTOR_PLATO, HIGH);
    tone(BUZZER_PIN, 500);
    for (int i = 0; i < p.calentar && !puertaAbierta() && !cancelado; i++) {
      delay(1000);
    }
    //apagado
    lcd.clear();
    lcd.print("En pausa...");
    digitalWrite(MOTOR_PLATO, LOW);
    tone(BUZZER_PIN, 300);
    for (int i = 0; i < p.apagado && !puertaAbierta() && !cancelado; i++) {
      delay(1000);
    }
  }
  
  noTone(BUZZER_PIN);
  digitalWrite(MOTOR_PLATO, LOW);
  digitalWrite(LUZ_INTERIOR, LOW);

  if(!cancelado){
    lcd.clear();
    lcd.print("Listo");
    tone(BUZZER_PIN, 2000, 1000);
    delay(1000);
  }

  programaActivo = false;
}
//repeticiones de coccion rapida
void repetirPrograma(int rep){
  programas[0].repeticiones = rep;// modifico la cantidad de repeticiones de coccion rapida
  iniciarPrograma(0);// inicio la coccion rapida modificada
}




