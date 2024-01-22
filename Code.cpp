#include <Arduino.h>
#include <Wire.h>
#include <HTTPClient.h>
#include <WiFiManager.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

#define PIN_SDA 21
#define PIN_SCL 22
#define PIN_PASO_X 2
#define PIN_PASO_Y 3
#define PIN_PASO_Z 4
#define PIN_DIRECCION_X 5
#define PIN_DIRECCION_Y 6
#define PIN_DIRECCION_Z 7

char ssid[32];      // Variable para almacenar el SSID de la red WiFi
char password[32];  // Variable para almacenar la contraseña de la red WiFi

unsigned long tiempoInicio = 0;  // Variable para almacenar el tiempo de inicio
bool motoresEnMovimiento = false;  // Variable para rastrear si los motores están en movimiento

// Configuración del bot de Telegram
const char* botToken = "6964930916:AAF9C6yqMXgwJHOv3ago0Gy8LLCQuyGxsB0";
const unsigned long botMBTS = 1000;
unsigned long botLastScan;
const int botPort = 443;
const char* host = "api.telegram.org";
const char* chatId = "1554837753";  // Puedes obtenerlo hablando con el bot de "get_id_bot" en Telegram
WiFiClientSecure secured_client;
UniversalTelegramBot bot(botToken, secured_client);
void actualizarEstadoMotores(int datosArduino);
void enviarDatosThingsLinker(int valor1, int valor2, int valor3, int valor4);
void verificarComandosTelegram();
void enviarMensajeTelegram(String mensaje, String chatID);  // Agregamos el prototipo

void setup() {
  // Inicia la comunicación serial
  Serial.begin(115200);

  // Configura el modo de WiFi
  WiFi.mode(WIFI_STA);

  // Configura la red WiFi usando WiFiManager
  WiFiManager wiFiManager;
  wiFiManager.resetSettings();

  // Intenta conectar o crea una red WiFi si no hay configuración previa
  bool res = wiFiManager.autoConnect("AutoconnectESP32S", "1111");
  secured_client.setCACert(TELEGRAM_CERTIFICATE_ROOT);
  // Si la conexión fue exitosa
  if (!res) {
    Serial.println("Connected to WiFi");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    // Continúa con la lógica principal
  } else {
    Serial.println("Failed to connect to WiFi");
  }

 
}

void loop() {
  // Coloca aquí tu lógica principal

  // Lectura de datos desde el Arduino Uno a través de I2C
  Wire.requestFrom(8, 1);  // Reemplaza '8' con la dirección I2C del Arduino Uno y '1' con la cantidad de bytes esperados

  while (Wire.available()) {
    // Aquí procesamos los datos recibidos desde el Arduino Uno
    int datosArduino = Wire.read();

    // Hacer algo con los datosArduino
    actualizarEstadoMotores(datosArduino);
  }
  verificarComandosTelegram();
  delay(1000);  // Agrega un breve retardo si es necesario
}

void actualizarEstadoMotores(int datosArduino) {
  // Desglosar datos y actualizar en ThingsLinker
  int valor1 = (datosArduino & 0b00000001) != 0;
  int valor2 = (datosArduino & 0b00000010) != 0;
  int valor3 = (datosArduino & 0b00000100) != 0;
  int valor4 = (datosArduino & 0b00001000) != 0;

  // Si los motores están en movimiento
  if (valor1 || valor2 || valor3) {
    // Si es la primera vez que detectamos movimiento, registramos el tiempo de inicio
    if (!motoresEnMovimiento) {
      tiempoInicio = millis();
      motoresEnMovimiento = true;
    }
  } else {
    // Si los motores están en reposo, reiniciamos el contador de tiempo
    tiempoInicio = 0;
    motoresEnMovimiento = false;
  }

  // Enviar datos a ThingsLinker
  enviarDatosThingsLinker(valor1, valor2, valor3, valor4);

  // Enviar mensaje a Telegram si los motores están en reposo después de cierto tiempo
  //if (!motoresEnMovimiento && (millis() - tiempoInicio > 5000)) {
  //  enviarMensajeTelegram("¡Los motores están en reposo!");
  //}
}

void enviarDatosThingsLinker(int valor1, int valor2, int valor3, int valor4) {
  // Reemplaza 'TU_API_KEY' y 'thing_key' con tus propias credenciales de ThingsLinker
  String apiKey = "TU_API_KEY";

  // Construir la URL para enviar datos a ThingsLinker
  String url = "https://api.thingslinker.com/things/" + apiKey + "/data";

  // Crear un objeto HTTPClient
  HTTPClient http;

  // Construir el cuerpo de la solicitud
  String payload = "{\"V1\":" + String(valor1) + ",\"V2\":" + String(valor2) + ",\"V3\":" + String(valor3) + ",\"V4\":" + String(valor4) + ",\"TiempoEnMovimiento\":" + String(millis() - tiempoInicio) + "}";

  // Agregar las credenciales a la solicitud HTTP
  http.begin(url);
  http.addHeader("Content-Type", "application/json");
  int httpCode = http.sendRequest("POST", payload);
  http.end();
  
  
  Serial.println("HTTP Code: " + String(httpCode));
}

//void enviarMensajeTelegram(const char *mensaje) {
//  Serial.print("Enviando mensaje a Telegram: ");
//  Serial.println(mensaje);

  // Enviar mensaje a través del bot de Telegram
//  String response = bot.sendMessage(chatId, mensaje, "");
//  Serial.println("Respuesta de Telegram: " + response);
//}

void verificarComandosTelegram() {
  int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

  for (int i = 0; i < numNewMessages; i++) {
    String chatId = String(bot.messages[i].chat_id);

    // Process the message
    String text = bot.messages[i].text;

    if (text.indexOf("Estado de trabajo") != -1) {
      // Send message about work status
      if (motoresEnMovimiento) {
        enviarMensajeTelegram("Trabajo en proceso", chatId);
      } else {
        enviarMensajeTelegram("Trabajo realizado", chatId);
      }
    } else if (text.indexOf("Tiempo de trabajo") != -1) {
      // Send work time
      if (motoresEnMovimiento) {
        enviarMensajeTelegram("Los motores aún están en movimiento", chatId);
      } else {
        enviarMensajeTelegram("Tiempo de trabajo: " + String(millis() - tiempoInicio) + " ms", chatId);
      }
    }
  }
}


void enviarMensajeTelegram(String mensaje, String chatId) {
  // Reemplaza 'TU_CHAT_ID' con tu chat ID de Telegram
  //String chatId = "TU_CHAT_ID";
  
  // Enviar el mensaje
  bot.sendMessage(chatId, mensaje, "");
}
