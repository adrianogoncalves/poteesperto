#include <ESP8266WiFi.h>  //essa biblioteca já vem com a IDE. Portanto, não é preciso baixar nenhuma biblioteca adicional
#include <PubSubClient.h> //Importa biblioteca MQTT

#define SSID_REDE     "iPhone de Adriano"  //coloque aqui o nome da rede que se deseja conectar
#define SENHA_REDE    "liviagoncalves"  //coloque aqui a senha da rede que se deseja conectar

#define ID_MQTT "poteesperto"
#define MQTT_SERVER "iot2017.nc2.iff.edu.br"
#define MQTT_USER   "saeg2017"
#define MQTT_PASS   "semsenha"
#define PORTA   443

#define TOPICOPORCENTAGEM "/poteesperto/arroz"


// defines pins numbers
const int trigPin = 2;  //D4
const int echoPin = 0;  //D3
const int CHEIO = 2;
const int VAZIO = 12;
const int ledFalta = D1;
const int ledOK = D8;

// defines variables
long duration;
int distance;
float porcentagem;

float calcularPorcentagem(int distancia);

WiFiClient cliente;
PubSubClient clienteMQTT(cliente);


//Função: faz a conexão WiFI
//Parâmetros: nenhum
//Retorno: nenhum
boolean conectaWiFi(void)
{
    
    cliente.stop();
        
    delay(500);
    Serial.println("Conectando-se à rede WiFi...");
    Serial.println();  
    delay(1000);
    WiFi.begin(SSID_REDE, SENHA_REDE);
    
    int contDelay = 0;
    while ((WiFi.status() != WL_CONNECTED) && (contDelay < 20) ) 
    {
        delay(500);
        Serial.print(".");
        contDelay++;
        
    }
    if(WiFi.status() != WL_CONNECTED){
       Serial.println("");
       Serial.println("WiFi não connectado");
       return false;
    }
      
    Serial.println("");
    Serial.println("WiFi connectado com sucesso!");  
    Serial.println("IP obtido: ");
    Serial.println(WiFi.localIP());

    delay(500);
    return true;
}

//Função: inicializa parâmetros de conexão clienteMQTT(endereço do 
//        broker, porta e seta função de callback)
//Parâmetros: nenhum
//Retorno: nenhum
void iniciaMQTT(void){
  clienteMQTT.setServer(MQTT_SERVER, PORTA);
  clienteMQTT.setCallback(mqtt_callback); 
}

// 
//Função: conectando ao servidor por MQTT
//Parâmetros: nenhum
//Retorno: nenhum
void connectaClienteMQTT(void) {
  // Espera até estar conectado ao servidor
  while (!clienteMQTT.connected()) {
    Serial.print("Attempting MQTT connection...");
    
    // Tentativa de conexão
    if( clienteMQTT.connect(ID_MQTT, MQTT_USER, MQTT_PASS )) {
      Serial.println("connected");
      clienteMQTT.subscribe(TOPICOPORCENTAGEM);
    } else {
      Serial.print("failed, rc=");
      Serial.print(clienteMQTT.state());
      Serial.println(" try again in 5 seconds");
      // Espera 5 segundo e tenta novamente
      delay(5000);
    }
  }
}

String mensagem(byte* payload, unsigned int length){

  String msg;
 
  //obtem a string do payload recebido
  for(int i = 0; i < length; i++) 
  {
     char c = (char)payload[i];
     msg += c;
  }
  return msg;
}

// 
//Função: Trata o valor do Topico
//Parâmetros: nenhum
//Retorno: nenhum
void trataTopico(char* topic,String msg){
  
   

}




//Função: função de callback 
//        esta função é chamada toda vez que uma informação de 
//        um dos tópicos subescritos chega)
//Parâmetros: nenhum
//Retorno: nenhum
void mqtt_callback(char* topic, byte* payload, unsigned int length) 
{     
    String msg = mensagem(payload,length);
    
    trataTopico(topic,msg);
    
}
void iniciaGPIO(void){
  
 
    
}


void setup() {
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input
  pinMode(ledFalta, OUTPUT); // Sets the echoPin as an Input
  pinMode(ledOK, OUTPUT); // Sets the echoPin as an Input
  Serial.begin(9600); // Starts the serial communication

   //Serial.begin(115200);
  delay(10);
  iniciaGPIO();
  if (conectaWiFi()){
     iniciaMQTT();
    // conectaSensorTemperatura();
  }
}

void loop() {
  // Clears the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);

  // Calculating the distance
  distance= duration*0.034/2;
  // Prints the distance on the Serial Monitor
  Serial.print("Distance: ");
  Serial.println(distance);

  porcentagem = calcularPorcentagem(distance);
  
  Serial.print("Porcentagem: ");
  Serial.println(porcentagem);

  if(distance <= VAZIO)
  {
      if(porcentagem <= 33.)
      {
        analogWrite(ledFalta, 100);
        analogWrite(ledOK, 0);
      }
      else
      {
        analogWrite(ledFalta, 0);
        analogWrite(ledOK, 100);
      }
     clienteMQTT.publish(TOPICOPORCENTAGEM, String(porcentagem).c_str());
  }
  else{
    analogWrite(ledOK, 0);
    analogWrite(ledFalta, 0);
  }

  if (WiFi.status() == WL_CONNECTED){
      if (!clienteMQTT.connected()) {
        connectaClienteMQTT();
      }
     
      clienteMQTT.loop(); 
  }else{
   if (conectaWiFi()){
     iniciaMQTT();
   }   
      
  }
  
  delay(2000);
}


float calcularPorcentagem(int distancia)
{
  if(distancia >= VAZIO)
  {
    return 0.;
  }

  if(distancia <= CHEIO)
  {
    return 100.;
  }
  
  return 100. - (((float)distancia - (float)CHEIO) / ((float)VAZIO - (float)CHEIO)) * 100.;
}

