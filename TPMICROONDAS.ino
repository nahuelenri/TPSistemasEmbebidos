#include <Keypad.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>

#define BUZZER_PIN 3
#define PUERTA_PIN 2
#define LUZ_INTERIOR 4
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
void menuPrincipal();
bool puertaAbierta();
void cargarProgramasPorDefecto();
int leerNumero();
void configurarPrograma();
void cancelarPrograma();
void iniciarPrograma(int index);
void repetirPrograma(int rep);

/*********************************************************************************************/
void setup() {
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(PUERTA_PIN, INPUT_PULLUP);
  pinMode(LUZ_INTERIOR, OUTPUT);
  pinMode(MOTOR_PLATO, OUTPUT);

  lcd.init();
  lcd.backlight();

  cargarProgramasPorDefecto();
  menuPrincipal();

  Serial.begin(9600);
}

void loop() {
  //puerta abierta
  if(puertaAbierta() && !programaActivo){
    digitalWrite(LUZ_INTERIOR, HIGH);
  }else if(!programaActivo){
    digitalWrite(LUZ_INTERIOR, HIGH);
  }

  char tecla = kp.getKey();
  opcionesMenu(tecla);

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

/**************************************MENU PRINCIPAL**************************************************/
void menuPrincipal(){
  
  String texto = "A-Coccion B-Descongelar C-Recalentar D-Usuario #-Configurar ";
  while(true){
    lcd.clear();
    
    for (int i = 0; i <= texto.length() - 16; i++) {
      lcd.setCursor(0, 0);
      lcd.print(texto.substring(i, i + 16));
      char tecla = kp.getKey();
      if(tecla){
        opcionesMenu(tecla);
      }
      delay(100); // velocidad del scroll
    }
  }
}

void opcionesMenu(char tecla){
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
      	menuPrincipal();
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
/*******************************verificar puerta*********************************/
bool puertaAbierta(){
  return digitalRead(PUERTA_PIN) == LOW; // LOW = la puerta esta abierta
}

/***********************************programas por defecto****************************************/
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

/****************************permitir ingresar multiples digitos********************************/
int leerNumero(){
  String num = "";
  while(true){
    char t = kp.getKey();
    if(t == '#')break; //confirmar 
    if(t >= '0' && t <= '9'){
      num += t;
      lcd.setCursor(0,1);
      lcd.print(num);
    }else if(t == '*')menuPrincipal();//cancelar
  }
  return num.toInt();
}

/*****************************configurar programa(tecla D)**************************************/
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

/*********************cancelar programa*****************************/
void volverAlMenu(){
  char tecla = kp.getKey();
  if(tecla == '*'){
    menuPrincipal();
  }
}
void cancelarPrograma(){
  cancelado = true;
  programaActivo = false;
  noTone(BUZZER_PIN);
  digitalWrite(MOTOR_PLATO, LOW);
  digitalWrite(LUZ_INTERIOR, LOW);
  lcd.clear();
  lcd.print("Cancelado");
  delay(3000);
  menuPrincipal();
}
void verificarCancelacion(){
  char tecla = kp.getKey();
  if(tecla){
    if(tecla == '*'){
      cancelarPrograma();
    }
  }
}
/************************************ejecutar programa****************************************************/
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
  int r = 0;
  lcd.clear();
  lcd.setCursor(0,0);
  switch(index){
    case 0:
    lcd.print("Coccion Rapida");
    break;
    case 1:
    lcd.print("Descongelar");
    break;
    case 2:
    lcd.print("Recalentar");
    break;
    case 3:
    lcd.print("Personalizada");
    break;
  }
  while(r <p.repeticiones && !cancelado){
    int i = 0;
    //calentar
    digitalWrite(LUZ_INTERIOR, HIGH);
    digitalWrite(MOTOR_PLATO, HIGH);
    tone(BUZZER_PIN, 300);
    while(i < p.calentar && !puertaAbierta() && !cancelado) {
      verificarCancelacion();
      lcd.setCursor(0,1);
      lcd.print(tiempoTotal);
      lcd.print(" seg");
      tiempoTotal--;
      delay(1000);
      i++;
    }
    //apagado
    i = 0;
    digitalWrite(MOTOR_PLATO, LOW);
    tone(BUZZER_PIN, 100);
    while(i < p.apagado && !puertaAbierta() && !cancelado) {
      verificarCancelacion();
      lcd.setCursor(0,1);
      lcd.print(tiempoTotal);
      lcd.print(" seg");
      tiempoTotal--;
      delay(1000);
      i++;
    }
    r++;
  }
  
  noTone(BUZZER_PIN);
  digitalWrite(MOTOR_PLATO, LOW);
  digitalWrite(LUZ_INTERIOR, LOW);

  if(!cancelado){
    lcd.clear();
    lcd.print("Listo");
    tone(BUZZER_PIN, 2000, 1000);
    delay(3000);
  }
  programaActivo = false;
}
/******************************repeticiones de coccion rapida***********************************/
void repetirPrograma(int rep){
  programas[0].repeticiones = rep;// modifico la cantidad de repeticiones 
  iniciarPrograma(0);
}
