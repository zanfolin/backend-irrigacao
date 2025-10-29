
#include <WiFi.h>
#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>


#define LED 2  // LED embutido no ESP3
const char* WIFI_SSID = "ME CHAMA DE LINDO";
const char* WIFI_PASS = "EU-AMO-GRAZIANI";

// GPIO34 é uma ótima escolha, pois é um pino "somente entrada"
const int pinoSensor = 34;
const int pinoAgua = 4;


// --- CALIBRAÇÃO  DO SENTOR DE HUMIDADE---
// Para obter leituras em porcentagem, você precisa calibrar o sensor.
// 1. Coloque o valor lido com o sensor totalmente seco (no ar)
// 2. Coloque o valor lido com o sensor totalmente submerso em água
// Substitua os valores abaixo pelos que você encontrar
const int VALOR_SENSOR_SECO = 2500;   // Valor com o sensor no ar (exemplo)
const int VALOR_SENSOR_UMIDO = 1070;  // Valor com o sensor na água (exemplo)

//const char* urlHttp = "http://worldtimeapi.org/api/timezone/America/Sao_Paulo";
// const char* urlHttp = "http://localhost:3000";
// const String urlHttp = "http://localhost:3000";
const char* urlHttp = "https://crowded-fishsticks-pwjgjx97p9j27v6-3000.app.github.dev/";
String token;
String statusAgua;

bool connectToWiFi(const char* ssid, const char* pass, unsigned long timeoutMs = 20000) {
  Serial.printf("Tentando conectar em \"%s\"...\n", ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED) {
    if (millis() - start >= timeoutMs) {
      Serial.println("Timeout: não foi possível conectar ao WiFi.");
      return false;
    }
    Serial.print(".");
    delay(500);
  }

  Serial.println();
  Serial.print("Conectado! IP: ");
  Serial.println(WiFi.localIP());
  return true;
}

String fetchHTTPRobust(const char* url, int timeoutMs = 15000, int maxRetries = 3, int retryDelayMs = 2000) {
  HTTPClient http;
  String payload = "";

  // Checa WiFi
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("fetchHTTPRobust: WiFi não conectado.");
    return payload;
  }

  // Serial.printf("fetchHTTPRobust: IP %s | RSSI %d dBm\n", WiFi.localIP().toString().c_str(), WiFi.RSSI());

  for (int attempt = 1; attempt <= maxRetries; ++attempt) {
    Serial.printf("fetchHTTPRobust: tentativa %d de %d -> %s\n", attempt, maxRetries, url);

    http.setTimeout(timeoutMs);
    // começa a requisição
    if (http.begin(url)) {
      // adicionar headers úteis
      http.addHeader("User-Agent", "ESP32-HTTPClient");
      // Se precisar de outros headers, adicione aqui:
      // http.addHeader("Accept", "application/json");

      int httpCode = http.GET();

      if (httpCode > 0) {
        Serial.printf("fetchHTTPRobust: HTTP status: %d\n", httpCode);
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY || httpCode == HTTP_CODE_FOUND) {
          // tenta ler a resposta completa
          payload = http.getString();
          // Serial.printf("fetchHTTPRobust: recebido %d bytes\n", payload.length());
          http.end();
          return payload;
        } else {
          Serial.printf("fetchHTTPRobust: resposta não-OK (code %d). Tentando novamente se houver tentativas.\n", httpCode);
        }
      } else {
        // httpCode <= 0 => erro de conexão/cliente
        Serial.printf("fetchHTTPRobust: Erro na requisição HTTP: %s (code %d)\n", http.errorToString(httpCode).c_str(), httpCode);
      }

      http.end();  // fecha antes de retry
    } else {
      Serial.println("fetchHTTPRobust: falha em http.begin(). Verifique URL ou DNS.");
    }

    // Se não foi sucesso, tenta reconectar WiFi se necessário
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("fetchHTTPRobust: WiFi desconectado durante retries. Tentando reconectar...");
      WiFi.reconnect();
      unsigned long waitStart = millis();
      while (WiFi.status() != WL_CONNECTED && millis() - waitStart < 8000) {
        delay(200);
      }
      if (WiFi.status() != WL_CONNECTED) {
        Serial.println("fetchHTTPRobust: falha ao reconectar WiFi.");
      } else {
        Serial.println("fetchHTTPRobust: reconectado ao WiFi.");
      }
    }

    // backoff simples
    Serial.printf("fetchHTTPRobust: aguardando %d ms antes da próxima tentativa...\n", retryDelayMs);
    delay(retryDelayMs);
  }

  Serial.println("fetchHTTPRobust: esgotadas todas as tentativas. Retornando vazio.");
  return payload;
}

// Faz login e retorna o token ("" se falhar)
String loginAndGetToken(const String& url, const char* usuario, const char* senha) {
  HTTPClient http;
  WiFiClientSecure client;
  client.setInsecure();  // para testes: ignora validação do certificado TLS
                         // REMOVER/ALTERAR em produção!

  // Monta JSON
  DynamicJsonDocument reqDoc(256);
  reqDoc["usuario"] = usuario;
  reqDoc["senha"] = senha;
  String requestBody;
  serializeJson(reqDoc, requestBody);

  Serial.println(url);
  Serial.print("Body: ");
  Serial.println(requestBody);

  token = "";

  // Inicia conexão HTTPS
  if (http.begin(client, url)) {
    http.addHeader("Content-Type", "application/json");
    int httpCode = http.POST(requestBody);
    Serial.print("HTTP code: ");
    Serial.println(httpCode);

    if (httpCode > 0) {
      String payload = http.getString();
      Serial.println("Resposta:");
      Serial.println(payload);

      // Parse JSON
      DynamicJsonDocument resDoc(1024);
      DeserializationError err = deserializeJson(resDoc, payload);
      if (!err) {
        if (resDoc.containsKey("token")) {
          token = String(resDoc["token"].as<const char*>());
          Serial.print("Token recebido: ");
          Serial.println(token);
        } else {
          Serial.println("JSON recebido não contém 'token'.");
        }
      } else {
        Serial.print("Erro parse JSON: ");
        Serial.println(err.c_str());
      }
    } else {
      Serial.print("Falha na requisição, erro: ");
      Serial.println(httpCode);  // -1 timeout etc
    }
    http.end();
  } else {
    Serial.println("http.begin() falhou");
  }

  return token;
}

String getWithAuth(const String& url, const String& bearerToken) {
  HTTPClient http;
  WiFiClientSecure client;
  client.setInsecure();  // testar rápido
  String response = "";

  if (http.begin(client, url)) {
    http.addHeader("Authorization", bearerToken);  // já com "Bearer ..."
    int httpCode = http.GET();
    Serial.print("GET code: ");
    Serial.println(httpCode);

    if (httpCode > 0) {
      response = http.getString();
      Serial.println("GET resposta:");
      Serial.println(response);
    } else {
      Serial.print("Erro GET: ");
      Serial.println(httpCode);
    }
    http.end();
  } else {
    Serial.println("Erro iniciando GET");
  }

  return response;
}


String fetchHTTPS(const char* url, int timeoutMs = 15000) {
  WiFiClientSecure client;
  client.setTimeout(timeoutMs / 1000);
  client.setInsecure();  // simplifica: aceita qualquer certificado (inseguro)

  HTTPClient https;
  String payload = "";

  if (https.begin(client, url)) {
    int httpCode = https.GET();
    if (httpCode > 0) {
      Serial.printf("HTTPS status: %d\n", httpCode);
      if (httpCode == HTTP_CODE_OK) {
        payload = https.getString();
      }
    } else {
      Serial.printf("Erro na requisição HTTPS: %s\n", https.errorToString(httpCode).c_str());
    }
    https.end();
  } else {
    Serial.println("Falha ao iniciar HTTPS (begin). Verifique URL.");
  }

  return payload;
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println();
  Serial.println("Iniciando scan de redes WiFi...");
  WiFi.mode(WIFI_MODE_STA);  // modo estação (scan)
  WiFi.disconnect();         // garante que não está conectado (ajuda no scan)
  delay(100);

  //Procurar e exibir as redes
  scanNetworks();
  pinMode(LED, OUTPUT);
  pinMode(pinoAgua, OUTPUT);
  digitalWrite(pinoAgua, HIGH);

  //Para se conectar na rede Wifi
  bool ok = connectToWiFi(WIFI_SSID, WIFI_PASS, 20000);
  if (!ok) {
    Serial.println("Falha ao conectar. Verifique SSID/senha, drivers ou alcance.");
    // aqui você pode decidir reiniciar, tentar novamente ou entrar em modo AP
    return;
  }

  delay(1000);
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
  Serial.print("RSSI: ");
  Serial.println(WiFi.RSSI());

  token = loginAndGetToken((String(urlHttp) + "api/signin").c_str(), "adm", "minha_senha_123");
  if (token.length() > 0) {
    Serial.println("token.");
    // salvar em variável global ou usar diretamente
    // String resp = getWithAuth("http://seu-servidor/api/protegido", token);
  } else {
    Serial.println("Não obteve token.");
  }

  statusAgua = "";
}


int getLigadaFromJson(const String& resp) {
  DynamicJsonDocument doc(512);
  DeserializationError error = deserializeJson(doc, resp);

  if (error) {
    Serial.print("Erro ao parsear JSON: ");
    Serial.println(error.c_str());
    return -1;  // retorna -1 em caso de erro
  }

  // Retorna o valor da propriedade "ligada"
  if (doc.containsKey("ligada")) {
    return doc["ligada"].as<int>();
  }
  // Caso o servidor use "ligado" no lugar de "ligada"
  else if (doc.containsKey("ligado")) {
    return doc["ligado"].as<int>();
  }

  // Se não encontrou nenhuma das propriedades
  return -1;
}

void loop() {
  // Se quiser re-scan periódico:
  // delay(10000);
  // scanNetworks();
  // Para este exemplo, loop fica vazio.
  digitalWrite(LED, HIGH);
  delay(1000);

  Serial.println("Leitor de Umidade do Solo com ESP32");
  // Faz a leitura do valor analógico do pino do sensor
  // O ADC do ESP32 tem 12 bits, então os valores vão de 0 a 4095
  int valorLido = analogRead(pinoSensor);
  // Mapeia o valor lido para uma porcentagem (0 a 100%)
  // Note que os valores são invertidos: mais seco = valor maior, mais úmido = valor menor
  int porcentagemUmidade = map(valorLido, VALOR_SENSOR_SECO, VALOR_SENSOR_UMIDO, 0, 100);

  // Garante que a porcentagem não saia do intervalo 0-100
  if (porcentagemUmidade > 100) {
    porcentagemUmidade = 100;
  }
  if (porcentagemUmidade < 0) {
    porcentagemUmidade = 0;
  }
  Serial.print("Umidade: ");
  Serial.print(porcentagemUmidade);
  Serial.println("%");


  // String urlUmidade = urlHttp + "/umidade/" + String(porcentagemUmidade);
  // Serial.println("Fazendo requisição Umidade...");
  // //tring respHttp = fetchHTTPRobust(urlHttp);
  // //Serial.println("Resposta HTTP:");
  // //Serial.println(respHttp);
  // Serial.println(urlUmidade);

  // String urlAgua = urlHttp + "/agua";
  // Serial.println("Fazendo requisição Água...");
  // //tring respHttp = fetchHTTPRobust(urlHttp);
  // //Serial.println("Resposta HTTP:");
  // //Serial.println(respHttp);
  // Serial.println(urlAgua);

  const char* urlApto = urlHttp;
  Serial.println("Testando Servidor...");
  String respHttp = fetchHTTPRobust(urlApto);
  if (respHttp.indexOf("API") >= 0) {
    Serial.println("Servidor Ok");
  }
  // Serial.println("Resposta HTTP:");
  // Serial.println(urlApto);
  // Serial.println(urlAgua);

  String resp = getWithAuth((String(urlHttp) + "api/agua").c_str(), token);
  Serial.println(resp);
  int estadoAgua = getLigadaFromJson(resp);

  if (estadoAgua == 1) {
    Serial.println("Água LIGADA");
    digitalWrite(pinoAgua, HIGH);  
  } else if (estadoAgua == 0) {
    Serial.println("Água DESLIGADA");
    digitalWrite(pinoAgua, LOW);  
  } else {
    Serial.println("Erro ao interpretar resposta.");
    digitalWrite(pinoAgua, LOW);
  }

  digitalWrite(LED, LOW);
  delay(1000);
}

const char* authTypeToString(wifi_auth_mode_t auth) {
  switch (auth) {
    case WIFI_AUTH_OPEN: return "OPEN";
    case WIFI_AUTH_WEP: return "WEP";
    case WIFI_AUTH_WPA_PSK: return "WPA_PSK";
    case WIFI_AUTH_WPA2_PSK: return "WPA2_PSK";
    case WIFI_AUTH_WPA_WPA2_PSK: return "WPA/WPA2_PSK";
    case WIFI_AUTH_WPA2_ENTERPRISE: return "WPA2_ENTERPRISE";
    case WIFI_AUTH_WPA3_PSK: return "WPA3_PSK";
    case WIFI_AUTH_WPA2_WPA3_PSK: return "WPA2/WPA3_PSK";
    default: return "UNKNOWN";
  }
}

void scanNetworks() {
  int n = WiFi.scanNetworks(false, true);  // (async, show_hidden) -> here blocking scan, show hidden true
  Serial.printf("Encontradas %d redes:\n", n);
  if (n == 0) {
    Serial.println("Nenhuma rede encontrada");
    return;
  }

  // Imprime cabeçalho
  Serial.println("Idx | RSSI  | Canal | Auth           | SSID");
  Serial.println("----+-------+-------+----------------+--------------------------");
  for (int i = 0; i < n; ++i) {
    String ssid = WiFi.SSID(i);
    int32_t rssi = WiFi.RSSI(i);
    int32_t channel = WiFi.channel(i);
    wifi_auth_mode_t auth = WiFi.encryptionType(i);
    const char* authStr = authTypeToString(auth);

    Serial.printf("%3d | %4d dBm | %5d | %-14s | %s\n",
                  i, rssi, channel, authStr, ssid.c_str());
  }
  // libera memória usada pelo scan
  WiFi.scanDelete();
}
