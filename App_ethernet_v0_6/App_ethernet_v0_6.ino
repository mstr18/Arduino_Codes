#include <RTClib.h>
#include <SPI.h>
#include <String.h>
#include <Ethernet.h>
#include <ctype.h>
#include <DS1307.h>
#include "Timer.h"
#include <EEPROM.h>

/*Timers trigger tasks that occur after a specified interval of time has passed.
The timer interval can be specified in seconds, or in hour, minutes and seconds.
  Alarm.timerRepeat(15, Repeats);            // timer task every 15 seconds    
This calls the Repeats() function in your sketch every 15 seconds.
Ligar por 5 minutos, de hora em hora, por exemplo
*/


//Modulo timer ligado as portas a4 e a5
//DS1307 rtc(A4, A5);
RTC_DS1307 RTC;

byte mac[] = { 0x90, 0xA2, 0xDA, 0x00, 0x9B, 0x36 }; // Endereço Mac
byte ip[] = { 192, 168, 1, 130 }; // Endereço de Ip da Rede
EthernetServer server(8090); // Porta de serviço

int solenoidPin = 4;// Pino onde deve ser ligada a valvula
int id;
String readString = String(30); // string para buscar dados de endereço
boolean statusSolenoid = false; // Variável para o status do led
Timer t; //Instancia do timer
int *ptrHora, *ptrMinuto;
DateTime now;


int agenda(int hora, int minuto)
{
  ptrHora = &hora;
  ptrMinuto = &minuto;
}

void ativaAlarme()
{
  if((now.hour() == *ptrHora)&& (now.minute() == *ptrMinuto))
  {
    digitalWrite(solenoidPin, HIGH);
    statusSolenoid = true;
  }
  else if((now.hour() > *ptrHora)||(now.minute() > *ptrMinuto))
  {
    Serial.print("Passou da hora");
    Serial.print("\n");
  }
  else
  {
    Serial.print("Ainda nao esta na hora");
    Serial.print("\n");
    
  }
}

void setOff()
{
  t.stop(id);
  digitalWrite(solenoidPin, LOW);
  statusSolenoid = false;
}


void setup() {
  //Aciona o relogio
  RTC.begin();
  //A linha abaixo acertam a data e hora do modulo
  //e podem ser comentada apos a primeira utilizacao
  //RTC.adjust(DateTime("01/14/2017", "19:09:00"));
  // Inicia o Ethernet
  Ethernet.begin(mac, ip);
  // Define o pino como saída
  pinMode(solenoidPin, OUTPUT);
  // Inicia a comunicação Serial
  Serial.begin(9600);
}

void loop() {
  // Criar uma conexão de cliente
  //digitalClockDisplay();
  EthernetClient client = server.available();
  //inicializa o RTC
  now = RTC.now();
 
  //Atualiza os processos em segundo plano
  t.update();
  ativaAlarme();
  if (client) {
    while (client.connected())
    {

      if (client.available())
      {
        char c = client.read();
        // ler caractere por caractere vindo do HTTP
        if (readString.length() < 30)
        {
          // armazena os caracteres para string
          readString += (c);
        }

        //se o pedido HTTP terminou
        if (c == '\n')
        {
          unsigned long tempo;
          String leitura = String(10);
          String strHora(3), strMinuto(3);
          int hora = 0, minuto = 0;
          // vamos verificar se a valvula deve ser ligada
          if (readString.indexOf("Tempo") >= 0)
          {
            leitura = readString.substring(10, 12);
            tempo = leitura.toInt();
            tempo = (tempo * 60000);
            digitalWrite(solenoidPin, HIGH);
            statusSolenoid = true;
            id = t.after(tempo, setOff);
          }
          if (readString.indexOf("agen") >= 0)
          {
            leitura = readString.substring(9, 15);
            strHora = leitura.substring(1, 3);
            hora = strHora.toInt();
            strMinuto = leitura.substring(4, 6);
            minuto = strMinuto.toInt();
            agenda(hora, minuto);
          }
          // Se a string possui o texto Ligar
          if (readString.indexOf("Ligar") >= 0)
          {
            // A valvula vai ser ligada
            digitalWrite(solenoidPin, HIGH);
            statusSolenoid = true;
          }
          // Se a string possui o texto Desligar
          if (readString.indexOf("Desligar") >= 0)
          {
            // A valvula vai ser desligada
            digitalWrite(solenoidPin, LOW);
            statusSolenoid = false;
          }



          // dados HTML de saída começando com cabeçalho padrão
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println();
          client.println("<!doctype html>");
          client.println("<html>");
          client.println("<head>");
          client.println("<title></title>");
          client.println("<meta name=\"viewport\" content=\"width=320\">");
          client.println("<meta name=\"viewport\" content=\"width=device-width\">");
          client.println("<meta charset=\"utf-8\">");
          client.println("<meta name=\"viewport\" content=\"initial-scale=1.0, user-scalable=no\">");
          client.println("<meta http-equiv=\"refresh\" content=\"2,URL=http://192.168.1.130:8090\">");

          client.println("</head>");
          client.println("<body>");
          client.println("<center>");

          client.println("<font size =\"5\" face=\"verdana\" color=\"green\"></font>");
          if (statusSolenoid)
          {
            client.print("Ligado");
            client.print("</p>");
            client.print(now.hour());
            client.print(":");
            client.print(now.minute());
            client.print("</p>");
          }
          else
          {
            client.print("Desligado");
            client.print("</p>");
            client.print(now.hour());
            client.print(":");
            client.print(now.minute());
            client.print("</p>");
          }
          //limpa string para a próxima leitura
          readString = "";

          // parar cliente
          client.stop();
        }
      }
    }
  }
}
/*
  // digital clock display of the time
  Serial.print(now.hour());
  printDigits(now.minute());
  printDigits(now.second());
  Serial.println(); 
}
void printDigits(int digits)
{
  Serial.print(":");
  if(now.minute < 10)
    Serial.print('0');
  Serial.print(digits);
}*/

