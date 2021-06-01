/*  Comparação entre os serviços em nuvem Amazon AWS 
 *   e Microsoft Azure em um contexto de Internet das coisas
 *  
 *  Autor: Jorge Durvalino Junior
 *         Mathues Gontijo Dias 
 */


#include "FS.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

// Dados de conexão WiFi
const char* ssid 	= "Jorge";
const char* password 	= "GaloDoido";

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");


// EndereÃo do Broker MQTT da Amazon AWs
const char* AWS_endpoint = "aojfefigchcsh-ats.iot.us-east-2.amazonaws.com"; 

// EndereÃo do Broker MQTT no Azure
const char* AZURE_endpoint = "INSERIR_AQUI";


void callback(char* topic, byte* payload, unsigned int length) {
	Serial.print("Message arrived [");
	Serial.print(topic);
	Serial.print("] ");
	for (int i = 0; i < length; i++) 
		Serial.print((char)payload[i]);}

	Serial.println();
}

WiFiClientSecure espClient;

// Definindo a porta 8883
PubSubClient client(AWS_endpoint, 8883, callback, espClient); 
long lastMsg = 0;
char msg[50];
int value = 0;

void setup_wifi() {
	delay(10);
	
	// Tentativa de conexão na rede WiFi
	espClient.setBufferSizes(512, 512);
	Serial.println();
	Serial.print("Connecting to ");
	Serial.println(ssid);

	WiFi.begin(ssid, password);

	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}

	Serial.println("");
	Serial.println("WiFi connected");
	Serial.println("IP address: ");
	Serial.println(WiFi.localIP());

	timeClient.begin();
	while(!timeClient.update()){
		timeClient.forceUpdate();
	}

	espClient.setX509Time(timeClient.getEpochTime());
}

void reconnect() {
	// Loop até que esteja conectado 
	while (!client.connected()) {
		Serial.print("Attempting MQTT connection...");
		
		// Tentativa de conexão
		if (client.connect("ESPthing")) {
			Serial.println("connected");
			
			// Publicando uma mensagem no tópico outTopic
			client.publish("outTopic", "hello world");
			
			// se inscrevendo no tópico inTopic 
			client.subscribe("inTopic");
		} else {
			Serial.print("failed, rc=");
			Serial.print(client.state());
			Serial.println(" try again in 5 seconds");

			char buf[256];
			espClient.getLastSSLError(buf,256);
			Serial.print("WiFiClientSecure SSL error: ");
			Serial.println(buf);

			// Aguarda 5 segundo para tentar se reconectar
			delay(5000);
		}
	}
}

void setup() {
	Serial.begin(115200);
	Serial.setDebugOutput(true);
	
	pinMode(LED_BUILTIN, OUTPUT);
	setup_wifi();
	
	delay(1000);
	
	if (!SPIFFS.begin()) {
		Serial.println("Failed to mount file system");
		return;
	}

	Serial.print("Heap: "); 
	Serial.println(ESP.getFreeHeap());

	// Carrega o certificado
	File cert = SPIFFS.open("/cert.der", "r"); 
	if (!cert)
		Serial.println("Failed to open cert file");
 	else
		Serial.println("Success to open cert file");

	delay(1000);

	if (espClient.loadCertificate(cert))
		Serial.println("cert loaded");
	else
		Serial.println("cert not loaded");

	// carrega do certificado private.der
	File private_key = SPIFFS.open("/private.der", "r"); 
	if (!private_key)
		Serial.println("Failed to open private cert file");
	else
		Serial.println("Success to open private cert file");

	delay(1000);

	if (espClient.loadPrivateKey(private_key))
		Serial.println("private key loaded");
	else
		Serial.println("private key not loaded");

	
	// Carrega o certificado ca.cer
	File ca = SPIFFS.open("/ca.der", "r"); //replace ca eith your uploaded file name
	if (!ca)
		Serial.println("Failed to open ca ");
	else
		Serial.println("Success to open ca");

	delay(1000);

	if(espClient.loadCACert(ca))
		Serial.println("ca loaded");
	else
		Serial.println("ca failed");

	Serial.print("Heap: "); Serial.println(ESP.getFreeHeap());
}

void loop() {

	if (!client.connected())
		reconnect();

	client.loop();

	long now = millis();
	if (now - lastMsg > 2000) {
		lastMsg = now;
		++value;
		snprintf (msg, 75, "{\"message\": \"hello world #%ld\"}", value);
		Serial.print("Publish message: ");
		Serial.println(msg);
		client.publish("outTopic", msg);
		Serial.print("Heap: "); Serial.println(ESP.getFreeHeap()); 
	}
	
	// Apenas pisca o led para sabermos que esta o loop
	digitalWrite(LED_BUILTIN, HIGH); 
	delay(100); // wait for a second
	digitalWrite(LED_BUILTIN, LOW); 
	delay(100); // wait for a second
}


