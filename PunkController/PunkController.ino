#include <Arduino.h>
#include <U8g2lib.h>
#include "ClickEncoder.h"
#include "TimerOne.h"

#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

#define pinoA 2
#define pinoB 3
#define EN 8
#define STP 9
#define dir 10

#define M0 A0
#define M1 A1
#define M2 A2
#define buzzer 5

U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);     //Parametros de inicialização do OLED

ClickEncoder *encoder;

void estadoManual();
void estadoAutomatico();
void estadoConfiguracao();
void estadoCastaldo();
void PulsoMotor(int sentido);

double tamanhoPasso = 0.0000094766;
int pulseTime = 50;
int DelayPulse = 1;
int opcao = 0;
int percorrido = 0;
int percorridoTemporario = 0;

int estadoSelecionando = 0;
int itemSelecionado = 0;

struct structItemConfig {
  int id;
  int qtdItens;
  const char *titulo;
  const char *name[10];
  int selecionado;
};

//MenuConfiguraçoes
int qtdItensConfig = 5-1;
struct structItemConfig itens[] = {
  {0, 1, "Beep", {"ON", "OFF"}, 1},
  {1, 5, "Passos", {"Full step", "1/2 step", "1/4 step", "8 uStep", "16 uStep", "32 uStep"}, 2},
  {2, 9, "Velocidade", {"10%", "20%", "30%", "40%", "50%", "60%", "70%", "80%", "90%", "100%"}, 2},
  {3, 1, "Encoder", {"x", "y"}, 0},
  {4, 1, "FimCurso", {"ON", "OFF"}, 0}
};

struct menuPrincipal {
  int id;
  const uint8_t *font;
  uint16_t icon;
  const char *name;
};

struct menuPrincipal menuItens[] = {
  { 0, u8g2_font_open_iconic_embedded_4x_t, 65, "Modo Manual"},
  { 1, u8g2_font_open_iconic_embedded_4x_t, 66, "Modo Automatico"},
  { 2, u8g2_font_open_iconic_embedded_4x_t, 72, "Configuracao"},
  { 3, u8g2_font_open_iconic_embedded_4x_t, 68, "Modo Castaldo"},
  { 4, u8g2_font_open_iconic_embedded_4x_t, 69, "Informacoes"},
  { 6, NULL, 0, NULL }
};

//Estado do modo
typedef enum estadoInterno {
  recebendo,
  funcionando,
} estadoInterno;
estadoInterno estadoDoModo = recebendo;


//Maquina de estados
typedef enum estadoPossiveis {
  Inicializa,
  SelecionandoModo,
  ModoSelecionado,
} estadoPossiveis;
estadoPossiveis estado = SelecionandoModo;

typedef enum estadoModo {
  ModoManual,
  ModoAutomatico,
  ModoConfiguracao,
  ModoCastaldo,
  ModoInformacoes,
  ModoY,

} estadoModo;
estadoModo MEModo = ModoManual;


int counter = 0;
int sentido = 0;
int last = -1;

void timerIsr() {
  encoder->service();
}

void drawButtons(U8G2 u8g2) {
  uint8_t x = 115;
  u8g2.setFont(u8g2_font_5x8_tr);  u8g2.drawRFrame(96 + 4, 54, 32, 16, 4);  u8g2.drawStr(109, 63, "OK");
  u8g2.drawTriangle(x + 0, 28 + 0, x + 10, 28 + 0, x + 5, 28 - 5);
  u8g2.drawTriangle(x + 0, 36 + 0, x + 10, 36 + 0, x + 5, 36 + 5);
}

void setup() {
  u8g2.begin();
  u8g2.setFont(u8g2_font_5x8_tr);

  Serial.begin(9600);

  encoder = new ClickEncoder(pinoA, pinoB, 4, 2);

  Timer1.initialize(1000);
  Timer1.attachInterrupt(timerIsr);

  pinMode(M0, OUTPUT);
  pinMode(M1, OUTPUT);
  pinMode(M2, OUTPUT);
  pinMode(dir, OUTPUT);
  pinMode(STP, OUTPUT);
  pinMode(EN, OUTPUT);

  pinMode(buzzer, OUTPUT);
  //digitalWrite(buzzer, HIGH);

  digitalWrite(M0, LOW);
  digitalWrite(M1, LOW);
  digitalWrite(M2, LOW);

  digitalWrite(STP, LOW);
  digitalWrite(dir, LOW);
  digitalWrite(EN, HIGH);
  //pinMode(pinoA, INPUT);
  //pinMode(pinoB, INPUT);
  //attachInterrupt(digitalPinToInterrupt(pinoA), tratamentoInterrucaoA, RISING );    //Borda de subida
  //attachInterrupt(digitalPinToInterrupt(pinoB), tratamentoInterrucaoB, FALLING);    //Borda de descida
}

int counterMulti = 0;
int multiplicadores[] = {1, 2, 4, 10, 100, 1000};
int multiplicador = 1;
int inicial = 21;

void loop(void) {
  switch (estado) {
    case Inicializa:
      Serial.println(sentido);
      break;
    case SelecionandoModo:
      selecaoModo();
      break;
    case ModoSelecionado:
      switch (MEModo) {
        case ModoManual:
          estadoManual();
          break;
        case ModoAutomatico:
          estadoAutomatico();
          break;
        case ModoConfiguracao:
          estadoConfiguracao();
          break;
        case ModoCastaldo:
          estadoCastaldo();
          break;
        case ModoInformacoes:
          estadoInformacoes();
          break;
      }
      break;
  }
}

void estadoInformacoes() {
  int inicial = 12;
  ClickEncoder::Button b = encoder->getButton();
  if (b == ClickEncoder::Open) {
    sentido += encoder->getValue();
    if (sentido != last) {
      last = sentido;

      u8g2.firstPage();
      do {
        u8g2.setDrawColor(1); u8g2.drawBox(0, 0, 128, 12); //x, y, largura, altura
        u8g2.setDrawColor(0); u8g2.setFont(u8g2_font_8x13B_tr); u8g2.drawStr(20, 11, "Informaçoes");
        u8g2.setDrawColor(1);

        u8g2.setFont(u8g2_font_6x12_tf); u8g2.setCursor(0, inicial + (9 * 1)); u8g2.print("https://github.com/");
        u8g2.setFont(u8g2_font_6x12_tf); u8g2.setCursor(0, inicial + (9 * 2)); u8g2.print("/LeonardoFurini");
        u8g2.setFont(u8g2_font_6x12_tf); u8g2.setCursor(0, inicial + (9 * 3)); u8g2.print("/PunkControler");
        u8g2.setFont(u8g2_font_6x12_tf); u8g2.setCursor(0, inicial + (9 * 4)); u8g2.print("Desenvolvido EFAB");
        u8g2.setFont(u8g2_font_6x12_tf); u8g2.setCursor(0, inicial + (9 * 5)); u8g2.print("UTFPR - CURITIBA");
      } while ( u8g2.nextPage() );
    }
  } else if (b == ClickEncoder::DoubleClicked) {
    estado = SelecionandoModo;
    sentido = 0;
    last = 1;
    return;
  }
}

void selecaoModo() {
  sentido += encoder->getValue();
  ClickEncoder::Button b = encoder->getButton();
  if (b != ClickEncoder::Clicked) {
    int indicadorY = 55;
    int indicadorX = 34;
    int textoY = 48;
    if (sentido != last) {
      last = sentido;
      if (sentido > 4)
        sentido = 0;
      if (sentido < 0)
        sentido = 4;
      u8g2.firstPage();
      do {
        u8g2.setDrawColor(1);
        u8g2.drawCircle(indicadorX + (15 * 0), indicadorY, 4);
        u8g2.drawCircle(indicadorX + (15 * 1), indicadorY, 4);
        u8g2.drawCircle(indicadorX + (15 * 2), indicadorY, 4);
        u8g2.drawCircle(indicadorX + (15 * 3), indicadorY, 4);
        u8g2.drawCircle(indicadorX + (15 * 4), indicadorY, 4);

        String buffString = String(menuItens[sentido].name);
        u8g2.setFont(u8g2_font_8x13B_tr );
        u8g2.setCursor(((128 / 2) - (buffString.length() * 8 / 2)), textoY); u8g2.print(buffString);
        u8g2.drawDisc(indicadorX + (15 * sentido), indicadorY, 2);

        //Icone
        u8g2.setFont(u8g2_font_open_iconic_embedded_2x_t);
        u8g2.drawGlyph(56, 30 , menuItens[sentido].icon);

      } while ( u8g2.nextPage() );
    }
  } else {
    estado = ModoSelecionado;
    MEModo = sentido;
    estadoDoModo = recebendo;
    sentido = 0;
    last = 1;
    return;
  }
}

void estadoCastaldo() {
  int inicial = 21;
  ClickEncoder::Button b = encoder->getButton();
  if (b == ClickEncoder::Open) {
    sentido += encoder->getValue();
    if (sentido != last) {
      last = sentido;
      if (sentido > 1)
        sentido = 0;
      if (sentido < 0)
        sentido = 1;
      u8g2.firstPage();
      do {
        u8g2.setDrawColor(1); u8g2.drawBox(0, 0, 128, 12); //x, y, largura, altura
        u8g2.setDrawColor(0); u8g2.setFont(u8g2_font_8x13B_tr); u8g2.drawStr(20, 11, "Modo Castaldo");
        u8g2.setDrawColor(1);

        if (sentido == 0) {
          u8g2.setFont(u8g2_font_6x12_tf); u8g2.setCursor(0, inicial + (9 * 1)); u8g2.print("HORARIO");
        } else if (sentido == 1) {
          u8g2.setFont(u8g2_font_6x12_tf); u8g2.setCursor(0, inicial + (9 * 1)); u8g2.print("ANTI-HORARIO");
        }

        u8g2.setFont(u8g2_font_6x12_tf); u8g2.setCursor(0, inicial + (9 * 2)); u8g2.print(notacao(tamanhoPasso * percorrido));
        //u8g2.drawFrame(0, 50, 128, 14);
        //String passosString = String(sentido);
        //u8g2.setCursor(((128 / 2) - (passosString.length() * 8 / 2)), 62); u8g2.setFont(u8g2_font_8x13B_tr );    u8g2.print(passosString);

      } while ( u8g2.nextPage() );
    }
  } else if (b == ClickEncoder::Held) {
    if (sentido == 0) {
      DelayPulse = 2;
      PulsoMotor(0);
      percorridoTemporario++;
    } else if (sentido == 1) {
      DelayPulse = 2;
      PulsoMotor(1);
      percorridoTemporario++;
    }
  } else if (b == ClickEncoder::Released) {
    som();
    percorrido = percorridoTemporario;
    percorridoTemporario = 0;
    last = 9853;
    return;
  }  else if (b == ClickEncoder::Clicked) {
    percorrido = 0;
    last = 9853;
    return;
  } else if (b == ClickEncoder::DoubleClicked) {
    estado = SelecionandoModo;
    sentido = 0;
    last = 1;
    return;
  }
}

int j = 0;
void drawMenu(U8G2 u8g2, uint8_t selected, int opcao) {
  int PrimeiroVisivel = 0;
  j = 0;
  if (selected > 6) {
    PrimeiroVisivel = selected - 6;
  }
  //Titulo
  u8g2.setFont(u8g2_font_5x8_tr); u8g2.drawStr(1, 7, "Configuracao:"); u8g2.drawHLine(0, 8, 128);

  for (uint8_t i = PrimeiroVisivel; i < PrimeiroVisivel + 8; i++, j++) {
    if (selected == i) { u8g2.setFont(u8g2_font_5x8_tr); u8g2.setCursor(1, 8 * (j + 1) + 7);  u8g2.print("> ");}
    u8g2.setFont(u8g2_font_5x8_tr); u8g2.setCursor(7, 8 * (j + 2) + 0); u8g2.print(itens[j].titulo); u8g2.print(": ");  
    
    if(estadoSelecionando == 0){
      //Printa o subITEM salvo
      u8g2.print( itens[j].name[(itens[j].selecionado)]);
    }else{
      if(selected == i){u8g2.print( itens[selected].name[opcao]);}else{u8g2.print( itens[j].name[(itens[j].selecionado)]);}
    }
  }
}

void estadoConfiguracao() {
  ClickEncoder::Button b = encoder->getButton();
  if (b == ClickEncoder::Open) {
    sentido += encoder->getValue();
    if (sentido != last) {
      last = sentido;
      if(estadoSelecionando == 0){
      if (sentido > qtdItensConfig)
        sentido = 0;
      if (sentido < 0)
        sentido = qtdItensConfig;
      }else{
        if (sentido > itens[itemSelecionado].qtdItens)
          sentido = 0;
        if (sentido < 0)
          sentido = itens[itemSelecionado].qtdItens;
      }

      //Item ainda nao selecionado
      if (estadoSelecionando == 0) {
        u8g2.firstPage();
        do {
          u8g2.setDrawColor(1);
          drawButtons(u8g2);
          drawMenu(u8g2, sentido, opcao);
        } while ( u8g2.nextPage() );
      }

      //Escolhendo o SUBITEM
      else if (estadoSelecionando == 1) {
        u8g2.firstPage();
        do {
          u8g2.setDrawColor(1);
          drawButtons(u8g2);
          drawMenu(u8g2, itemSelecionado, sentido);
        } while ( u8g2.nextPage() );
      }
      
    }
  }
  
  else if (b == ClickEncoder::Clicked) {
    //Escolher Item 
    if (estadoSelecionando == 0) {
      itemSelecionado = sentido;
      estadoSelecionando = 1;
      sentido = 0;
    } 
    //Escolhendo subItem
    else {
      itens[itemSelecionado].selecionado = sentido;
      estadoSelecionando = 0;
    }
    sentido = 0;
    last = 1;
    return;
  } 
  
  else if (b == ClickEncoder::DoubleClicked) {
    estado = SelecionandoModo;
    sentido = 0;
    last = 1;
    return;
  }
}

void estadoManual() {

  ClickEncoder::Button b = encoder->getButton();
  sentido += (encoder->getValue()) * multiplicador;
  if (b == ClickEncoder::Open) {

    if (sentido != last) {


      int diferenca = sentido - last;
      if (diferenca < 0) {
        for (int i = 0; i < abs(diferenca); i++) {
          DelayPulse = 3;
          PulsoMotor(0);
        }
      } else if (diferenca > 0) {
        for (int i = 0; i < abs(diferenca); i++) {
          DelayPulse = 3;
          PulsoMotor(1);
        }
      }
      last = sentido;

      u8g2.firstPage();
      do {
        u8g2.setDrawColor(1); u8g2.drawBox(0, 0, 128, 12); //x, y, largura, altura
        u8g2.setDrawColor(0); u8g2.setFont(u8g2_font_8x13B_tr); u8g2.drawStr(20, 11, "Modo Manual");
        u8g2.setDrawColor(1);
        u8g2.setFont(u8g2_font_6x12_tf); u8g2.setCursor(0, inicial + (9 * 0)); u8g2.print("Passo: "); u8g2.print(notacao(tamanhoPasso));
        u8g2.setFont(u8g2_font_6x12_tf); u8g2.setCursor(0, inicial + (9 * 1)); u8g2.print("Modo: ");  u8g2.print("Unipolar");
        u8g2.setFont(u8g2_font_6x12_tf); u8g2.setCursor(0, inicial + (9 * 2)); u8g2.print("Space: "); u8g2.print(notacao(tamanhoPasso * sentido));
        u8g2.setFont(u8g2_font_6x12_tf); u8g2.setCursor(0, inicial + (9 * 3)); u8g2.print("Multi: "); u8g2.print(multiplicador);
        u8g2.drawFrame(0, 50, 128, 14);
        String passosString = String(sentido);
        u8g2.setCursor(((128 / 2) - (passosString.length() * 8 / 2)), 62); u8g2.setFont(u8g2_font_8x13B_tr );    u8g2.print(passosString);
      } while ( u8g2.nextPage() );
    }
  }
  else if (b == ClickEncoder::Held) {
    multiplicador = multiplicadores[counterMulti];

    u8g2.firstPage();
    do {
      u8g2.setDrawColor(1); u8g2.drawBox(0, 0, 128, 12); //x, y, largura, altura
      u8g2.setDrawColor(0); u8g2.setFont(u8g2_font_8x13B_tr); u8g2.drawStr(20, 11, "Modo Manual");
      u8g2.setDrawColor(1);
      u8g2.setFont(u8g2_font_6x12_tf); u8g2.setCursor(0, inicial + (9 * 0)); u8g2.print("Passo: "); u8g2.print(notacao(tamanhoPasso));
      u8g2.setFont(u8g2_font_6x12_tf); u8g2.setCursor(0, inicial + (9 * 1)); u8g2.print("Modo: ");  u8g2.print("Unipolar");
      u8g2.setFont(u8g2_font_6x12_tf); u8g2.setCursor(0, inicial + (9 * 2)); u8g2.print("Space: "); u8g2.print(notacao(tamanhoPasso * sentido));
      u8g2.setFont(u8g2_font_6x12_tf); u8g2.setCursor(0, inicial + (9 * 3)); u8g2.print("Multi: "); u8g2.print(multiplicador);
      u8g2.drawFrame(0, 50, 128, 14);
      String passosString = String(sentido);
      u8g2.setCursor(((128 / 2) - (passosString.length() * 8 / 2)), 62); u8g2.setFont(u8g2_font_8x13B_tr );    u8g2.print(passosString);
    } while ( u8g2.nextPage() );

    delay(500);
    counterMulti++;
    if (counterMulti > 5)
      counterMulti = 0;
  } else if (b == ClickEncoder::Clicked) {
    sentido = 0;
    last = 1;
    return;
  }  else if (b == ClickEncoder::DoubleClicked) {
    estado = SelecionandoModo;
    sentido = 0;
    last = 1;
    return;
  }
}

void estadoAutomatico() {
  sentido += (encoder->getValue()) * multiplicador;
  ClickEncoder::Button b = encoder->getButton();
  if (b == ClickEncoder::Open) {
    switch (estadoDoModo) {
      case recebendo:
        if (sentido != last) {
          last = sentido;
          u8g2.firstPage();
          do {
            String tituloString = "Modo Automatico";
            u8g2.setDrawColor(1); u8g2.drawBox(0, 0, 128, 12); //x, y, largura, altura
            u8g2.setDrawColor(0); u8g2.setFont(u8g2_font_8x13B_tr); u8g2.drawStr(((128 / 2) - (tituloString.length() * 8 / 2)), 11, "Modo Automatico");
            u8g2.setDrawColor(1);
            u8g2.setFont(u8g2_font_6x12_tf); u8g2.setCursor(0, inicial + (9 * 0)); u8g2.print("Passo: "); u8g2.print(notacao(tamanhoPasso));
            u8g2.setFont(u8g2_font_6x12_tf); u8g2.setCursor(0, inicial + (9 * 1)); u8g2.print("Multi: "); u8g2.print(multiplicador);
            u8g2.setFont(u8g2_font_6x12_tf); u8g2.setCursor(0, inicial + (9 * 2)); u8g2.print("Space: "); u8g2.print(notacao(tamanhoPasso * sentido));
            //u8g2.setFont(u8g2_font_6x12_tf); u8g2.setCursor(0, inicial + (9 * 3)); u8g2.print("Velocidade: ");
            u8g2.drawRFrame(0, 43, 128, 6, 2);
            String passosString = String(sentido);
            u8g2.drawFrame(0, 50, 128, 14); u8g2.setCursor(((128 / 2) - (passosString.length() * 8 / 2)), 62); u8g2.setFont(u8g2_font_8x13B_tr );    u8g2.print(passosString);
          } while ( u8g2.nextPage() );
        }
        break;
      case funcionando:
        for (int i = 0; i < abs(sentido); i++) {
          if (sentido < 0) {
            DelayPulse = 3;
            PulsoMotor(0);
          } else {
            DelayPulse = 3;
            PulsoMotor(1);
          }

          if (((i % 10) == 0) || (i == abs(sentido))) {
            u8g2.firstPage();
            do {
              String tituloString = "Funcionando";
              int porcentagem = map(i, 0, abs(sentido), 2, 125);
              u8g2.setDrawColor(1); u8g2.drawBox(0, 0, 128, 12);
              u8g2.setDrawColor(0); u8g2.setFont(u8g2_font_8x13B_tr); u8g2.setCursor(((128 / 2) - (tituloString.length() * 8 / 2)), 11); u8g2.print(tituloString);
              u8g2.setDrawColor(1);
              u8g2.setFont(u8g2_font_6x12_tf); u8g2.setCursor(0, inicial + (9 * 0)); u8g2.print("Passo: "); u8g2.print(notacao(tamanhoPasso));
              u8g2.setFont(u8g2_font_6x12_tf); u8g2.setCursor(0, inicial + (9 * 1)); u8g2.print("Multi: "); u8g2.print(multiplicador);
              u8g2.setFont(u8g2_font_6x12_tf); u8g2.setCursor(0, inicial + (9 * 2)); u8g2.print("Space: "); u8g2.print(notacao(tamanhoPasso * sentido));
              u8g2.drawRFrame(0, 43, 128, 6, 2);              //Loading Externo
              u8g2.drawFrame (2, 45, porcentagem, 2);          //Porcentagem
              String passosString = String(sentido);
              u8g2.drawFrame(0, 50, 128, 14); u8g2.setCursor(((128 / 2) - (passosString.length() * 8 / 2)), 62); u8g2.setFont(u8g2_font_8x13B_tr );    u8g2.print(passosString);
            } while ( u8g2.nextPage() );
          }
        }
        som();
        estadoDoModo = recebendo;
        break;
    }
  } else if (b == ClickEncoder::Clicked) {
    estadoDoModo = funcionando;
    Serial.print("Funcionando");
    return;
  } else if (b == ClickEncoder::Held) {
    multiplicador = multiplicadores[counterMulti];
    u8g2.firstPage();
    do {
      String tituloString = "Modo Automatico";
      u8g2.setDrawColor(1); u8g2.drawBox(0, 0, 128, 12); //x, y, largura, altura
      u8g2.setDrawColor(0); u8g2.setFont(u8g2_font_8x13B_tr); u8g2.drawStr(((128 / 2) - (tituloString.length() * 8 / 2)), 11, "Modo Automatico");
      u8g2.setDrawColor(1);
      u8g2.setFont(u8g2_font_6x12_tf); u8g2.setCursor(0, inicial + (9 * 0)); u8g2.print("Passo: "); u8g2.print(notacao(tamanhoPasso));
      u8g2.setFont(u8g2_font_6x12_tf); u8g2.setCursor(0, inicial + (9 * 1)); u8g2.print("Multi: "); u8g2.print(multiplicador);
      u8g2.setFont(u8g2_font_6x12_tf); u8g2.setCursor(0, inicial + (9 * 2)); u8g2.print("Space: "); u8g2.print(notacao(tamanhoPasso * sentido));
      //u8g2.setFont(u8g2_font_6x12_tf); u8g2.setCursor(0, inicial + (9 * 3)); u8g2.print("Velocidade: ");
      u8g2.drawRFrame(0, 43, 128, 6, 2);
      String passosString = String(sentido);
      u8g2.drawFrame(0, 50, 128, 14); u8g2.setCursor(((128 / 2) - (passosString.length() * 8 / 2)), 62); u8g2.setFont(u8g2_font_8x13B_tr );    u8g2.print(passosString);
    } while ( u8g2.nextPage() );

    delay(500);
    counterMulti++;
    if (counterMulti > 5)
      counterMulti = 0;
  }  else if (b == ClickEncoder::DoubleClicked)  {
    estado = SelecionandoModo;
    sentido = 0;
    return;
  }
}

String notacao(double valorBruto) {
  double valor = abs(valorBruto);
  String buff;
  //metro
  if (valor >= 1) {
    buff = String(valor, 3) + " m";
    return buff;
  }
  //centimetro
  else if (valor < 1 && valor > 0.09) {
    buff = String((valor * 100), 2) + " cm";
    return buff;
  }
  //milimetro
  else if (valor < 0.1 && valor > 0.0009) {
    buff = String(valor * 1000.000) + " mm";
    return buff;
  }
  //micrometro
  else if (valor < 0.001 && valor > 0.0000009) {
    buff = String(valor * 1000000) + " um";
    return buff;
  }
}

void PulsoMotor(int sentido) {
  digitalWrite(EN, LOW);
  if (sentido == 0) {
    digitalWrite(dir, HIGH);
  } else if (sentido == 1) {
    digitalWrite(dir, LOW);
  }
  digitalWrite(STP, HIGH);
  delayMicroseconds(pulseTime);    //1 millisegundo
  digitalWrite(STP, LOW);
  delay(DelayPulse);
  digitalWrite(EN, HIGH);
}

void som() {
  tone(buzzer, 330, 300); //RE
  delay(100);
  noTone(buzzer);
}
