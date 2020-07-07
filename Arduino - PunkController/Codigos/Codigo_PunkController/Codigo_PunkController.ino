#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//Definicao de pinos
#define BotaoOK 7
#define encoderPinA  2
#define encoderPinB  3

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);

//LOGO
#define LOGO_HEIGHT   16
#define LOGO_WIDTH    16

static const unsigned char PROGMEM logo_bmp[] = { B00000000, B11000000,
                                                  B00000001, B11000000,
                                                  B00000001, B11000000,
                                                  B00000011, B11100000,
                                                  B11110011, B11100000,
                                                  B11111110, B11111000,
                                                  B01111110, B11111111,
                                                  B00110011, B10011111,
                                                  B00011111, B11111100,
                                                  B00001101, B01110000,
                                                  B00011011, B10100000,
                                                  B00111111, B11100000,
                                                  B00111111, B11110000,
                                                  B01111100, B11110000,
                                                  B01110000, B01110000,
                                                  B00000000, B00110000
                                                };

void controleManualPasso();


int const tempoAtualizarDisplay = 30;
int const modosOperacao = 4;
String modosOperacaoString[modosOperacao] = { "Controle de distancia", "Controle de passos", "Controle manual", "Configuracoes" };

volatile int direcaoEncoder = 0;
int operacaoSelecionada = 0;

//------------------MAQUINA DE ESTADOS-----------------------------
typedef enum ME {EstadoInicio,
                 EstadoSelecao,
                 EstadoModos,
                 EstadoConfiguracao,
                } estadosME;

typedef enum ME_modo {MODO_distancia,
                      MODO_passos = 1,
                      MODOmanual,
                      MODOconfiguracao,
                      MODOnenhum
                     } modoME;

estadosME estado =  EstadoSelecao;
modoME modo = MODO_passos;


//------------------CODIGO-----------------------------
void setup() {
  Serial.begin(9600);

  //Configura OLED
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.display();
  delay(100); // Pause for 2 seconds
  display.clearDisplay();

  //Botao
  pinMode(BotaoOK, INPUT);
  digitalWrite(BotaoOK, LOW);       // turn on pullup resistor

  //Configura Interrupçao
  pinMode(encoderPinA, INPUT);
  pinMode(encoderPinB, INPUT);
  digitalWrite(encoderPinA, HIGH);       // turn on pullup resistor
  digitalWrite(encoderPinB, HIGH);       // turn on pullup resistor
  attachInterrupt(digitalPinToInterrupt(2), interrupcaoA, RISING);       //Borda de subida
  attachInterrupt(digitalPinToInterrupt(3), interrupcaoB, FALLING);       //Borda de descida
  modoSelecao();
  estado = EstadoSelecao;
}

void loop() {
  switch (estado) {
    case EstadoInicio:
      //selecaoModo();
      break;
    case EstadoSelecao:
      selecaoModo();
      break;
    case EstadoModos:
      apresentaModo();
      break;
    default:
      return;
  }
}

void apresentaModo() {
  switch (modo) {
    case MODO_distancia:
      //selecaoModo();
      break;
    case 1:
      Serial.println("Dentro do modo");
      controleManualPasso();
      break;
    case EstadoConfiguracao:
      //selecaoModo();
      break;
    default:
      return;
  }
}

void pohaNenhuma() {
  Serial.println("se fude");
}
void controleManualPasso() {
  Serial.println("Dentro do modo controle manual");
  delay(500);
  int passos = 0;

  display.clearDisplay();
  display.setTextColor(BLACK, WHITE); display.setTextSize(1); display.setCursor(0, 0);
  display.println(modosOperacaoString[2]);

  display.setTextColor(WHITE); display.setTextSize(1); //display.setCursor(0, 0);
  display.println("Passos: ");
  display.println("Velocidade: ");

  display.setTextColor(WHITE); display.setTextSize(1); display.setCursor(70, 32);
  display.print("passos");

  while (digitalRead(BotaoOK) == 0) {
    if (direcaoEncoder > 0) {
      direcaoEncoder = 0;
      passos++;
      display.setTextColor(WHITE, BLACK); display.setTextSize(4); display.setCursor(0, 24);
      display.print(passos);
      display.display();
      delay(10);
    } else if (direcaoEncoder < 0 && passos > 0) {
      direcaoEncoder = 0;
      passos++;
      display.setTextColor(WHITE, BLACK); display.setTextSize(4); display.setCursor(0, 24);
      display.print(passos);
      display.display();
      delay(10);
    }
  }

/*
  for (int i = 0; i < passos; i++) {
    digital
  }
*/
}

void modoSelecao(void) {
  display.clearDisplay();
  display.setTextSize(1); display.setTextColor(WHITE); display.setCursor(0, 0);          // Start at top-left corner
  for (int i = 0; i < modosOperacao; i++) {
    display.println(modosOperacaoString[i]);
  }
  display.display();
  delay(100);
}

void selecaoModo() {
  while (digitalRead(BotaoOK) == 0) {
    //Sentido anti horario do encoder
    if (direcaoEncoder > 1) {
      //Arruma o valor anterior
      if (operacaoSelecionada == 0) {
        display.setTextColor(WHITE, BLACK); display.setTextSize(1); display.setCursor(0, (modosOperacao - 1) * 8); // Start at top-left corner
        display.print(modosOperacaoString[modosOperacao - 1]);
        //display.display();
        //delay(tempoAtualizarDisplay);
      } else {
        display.setTextColor(WHITE, BLACK); display.setTextSize(1); display.setCursor(0, (operacaoSelecionada - 1) * 8);     // Start at top-left corner
        display.print(modosOperacaoString[operacaoSelecionada - 1]);
        //display.display();
        //delay(tempoAtualizarDisplay);
      }

      display.setTextColor(BLACK, WHITE); display.setTextSize(1); display.setCursor(0, operacaoSelecionada * 8);
      display.print(modosOperacaoString[operacaoSelecionada]);
      display.display();
      delay(tempoAtualizarDisplay);
      direcaoEncoder = 0;

      operacaoSelecionada++;
      if (operacaoSelecionada > (modosOperacao - 1)) {
        operacaoSelecionada = 0;
      }
    }

    //Sentido anti horario do encoder
    else if (direcaoEncoder < 0) {
      operacaoSelecionada--;
      if (operacaoSelecionada < 0) {
        operacaoSelecionada = (modosOperacao - 1);
      }

      //Imprime a opção selecionada
      display.setTextColor(BLACK, WHITE); display.setTextSize(1); display.setCursor(0, operacaoSelecionada * 8);      //Texto selecionado
      display.print(modosOperacaoString[operacaoSelecionada]);


      //Arruma o valor anterior
      if (operacaoSelecionada == (modosOperacao - 1)) {
        display.setTextColor(WHITE, BLACK); display.setTextSize(1); display.setCursor(0, 0); //texto normal
        display.print(modosOperacaoString[0]);
      } else {
        display.setTextColor(WHITE, BLACK); display.setTextSize(1); display.setCursor(0, (operacaoSelecionada + 1) * 8);     // Start at top-left corner
        display.print(modosOperacaoString[operacaoSelecionada + 1]);
      }

      display.display();                  //Atualiza as modificaçoes feitas
      delay(tempoAtualizarDisplay);
      direcaoEncoder = 0;
    }
  }


  estado = EstadoModos;
  modo = operacaoSelecionada;
  Serial.println(modo);
  return;
}

void interrupcaoA() {
  delay(1);
  if (digitalRead(encoderPinA) == 1 && digitalRead(encoderPinB) == 0)
    direcaoEncoder++;
}

void interrupcaoB() {
  delay(1);
  if (digitalRead(encoderPinA) == 1 && digitalRead(encoderPinB) == 0)
    direcaoEncoder--;
}


void selectMenu(String *itens, int selecionado) {
  display.clearDisplay();

  display.fillRect(0, selecionado*8, display.width(), 10, INVERSE);       //x,y largura, altura, color
  
  
  display.display();
}
