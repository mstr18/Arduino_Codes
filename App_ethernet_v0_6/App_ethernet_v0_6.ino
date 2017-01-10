#include <SPI.h>
#include <String.h>
#include <Ethernet.h>
#include <ctype.h>
#include <DS1307.h>
#include "Timer.h"

//Modulo timer ligado as portas a4 e a5
DS1307 rtc(A4, A5);

byte mac[] = { 0x90, 0xA2, 0xDA, 0x00, 0x9B, 0x36 }; // Endereço Mac
byte ip[] = { 192, 168, 1, 130 }; // Endereço de Ip da Rede
EthernetServer server(8090); // Porta de serviço

int solenoidPin = 4;// Pino onde deve ser ligada a valvula
int id;
String readString = String(30); // string para buscar dados de endereço
boolean statusSolenoid = false; // Variável para o status do led
Timer t; //Instancia do timer

void setOff()
{
  t.stop(id);
  digitalWrite(solenoidPin, LOW);
  statusSolenoid = false;
}

void setup(){
  //Aciona o relogio
  rtc.halt(false);

  //As linhas abaixo acertam a data e hora do modulo
  //e podem ser comentada apos a primeira utilizacao
  //rtc.setDOW(THURSDAY); //Define o dia da semana
  //rtc.setTime(20, 04, 0); //Define o horario
  //rtc.setDate(29, 12, 2016); //Define o dia, mes e ano

  //Definicoes do pino SQW/Out
  rtc.setSQWRate(SQW_RATE_1);
  rtc.enableSQW(true);
  
  // Inicia o Ethernet
  Ethernet.begin(mac, ip);
  // Define o pino como saída
  pinMode(solenoidPin, OUTPUT);
  // Inicia a comunicação Serial
  Serial.begin(9600); 
}

void loop(){
  // Criar uma conexão de cliente
  EthernetClient client = server.available();
  t.update();
  
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
          // vamos verificar se a valvula deve ser ligada
          if(readString.indexOf("Tempo")>=0)
          {   
              leitura = readString.substring(10, 12);
              tempo = leitura.toInt();
              tempo = (tempo*60000);
              digitalWrite(solenoidPin, HIGH);
              statusSolenoid = true;
              id = t.after(tempo, setOff);
          }
          if(readString.indexOf("agen")>=0)
          {
              leitura = readString.substring(9,15);
              Serial.print(leitura);
              Serial.print("\n");
          }
          // Se a string possui o texto Ligar
          if(readString.indexOf("Ligar")>=0)
          {
            // A valvula vai ser ligada
            digitalWrite(solenoidPin, HIGH);
            statusSolenoid = true;
          }
          // Se a string possui o texto Desligar
          if(readString.indexOf("Desligar")>=0)
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
              client.print(readString);
              //client.print(rtc.getTimeStr());
              client.print("</p>");
              //client.print(rtc.getDateStr());
          } 
          else
          {
              client.print("Desligado");
              client.print("</p>");
              client.print(readString);
             //client.print(rtc.getTimeStr());
              client.print("</p>");
              //client.print(rtc.getDateStr());
          }
        //limpa string para a próxima leitura
        readString="";
        
        // parar cliente
        client.stop();
        }
      }
    }
  }
}
