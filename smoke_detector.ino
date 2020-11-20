// Import required libraries
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Hash.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Adafruit_Sensor.h>
#include <MQ2.h>

// Replace with your network credentials
const char* ssid = "Android samsung";
const char* password = "nova8888";

//definisi pin out MQ2
int pinAout = A0;
MQ2 dht(pinAout);

float t = 0.0;//data gas asap
float h = 0.0;//data gas LPG
String s = "";//data status LED

const char* PARAM_INPUT_1 = "output";
const char* PARAM_INPUT_2 = "state";

String outputState(int output){
  if(digitalRead(output)){
    return "checked";
  }
  else {
    return "";
  }
}
// Create AsyncWebServer object on port 80
AsyncWebServer server(80);


// The value will quickly become too large for an int to store
unsigned long previousMillis = 0;    

// Updates MQ2 readings every 5 seconds
const long interval = 5000;  

//tampilan website
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
  <style>
    html {
     font-family: fantasy;
     display: inline-block;
     text-align: center;
    }
   .topnav { 
    font-size: 4.0rem; 
  }
    p { font-size: 3.0rem; }
 body {  
    margin: 0;
  }

    h2 { font-size: 3.0rem; font-family: fantasy; }
    .units { font-size: 1.2rem; }
    .dht-labels{
      font-size: 3.0rem;
      font-family: Futura;
      vertical-align:middle;
      padding-bottom: 15px;
    }

.content { 
    padding: 20px; 
  }
  .card { 
    background-color: #e6e6fa; 
    box-shadow: 2px 2px 12px 1px rgba(140,140,140,.5); 
  }
  .cards { 
    max-width: 1000px; 
    margin: 0 auto; 
    display: grid; 
    grid-gap: 3rem; 
    grid-template-columns: repeat(auto-fit, minmax(200px, 1fr)); }
    

    .switch {position: relative; display: inline-block; width: 120px; height: 68px} 
    .switch input {display: none}
    .slider {position: absolute; top: 0; left: 0; right: 0; bottom: 0; background-color: #ccc; border-radius: 6px}
    .slider:before {position: absolute; content: ""; height: 52px; width: 52px; left: 8px; bottom: 8px; background-color: #fff; -webkit-transition: .4s; transition: .4s; border-radius: 3px}
    input:checked+.slider {background-color: #b30000}
    input:checked+.slider:before {-webkit-transform: translateX(52px); -ms-transform: translateX(52px); transform: translateX(52px)}
  </style>
</head>
<body bgcolor="#fffafa">
  <div class="topnav">
    <font font-color="#ffffff">MQ2</font>
    <font color="#008b8b">Server</font>
    <hr width="50%" align="center">
  </div>

 <div class="content">
    <div class="cards">
      <div class="card">
    <p>
    <i class="fas fa-smoking" style="color:#ffa500;"></i> 
    <span class="dht-labels" >Smoke</span><br>
    <span id="temperature" >%TEMPERATURE%</span>
    <sup class="units">ppm</sup>
  </p>
</div>

   <div class="card">
    <p>
    <i class="fas fa-burn" style="color:#008b8b;"></i> 
    <span class="dht-labels">LPG</span><br>
    <span id="humidity">%HUMIDITY%</span>
    <sup class="units">ppm</sup>
    </p>
 </div>
<div class="card">
    <p>
    <i class="fas fa-dumpster-fire" style="color:#800000;"></i> 
    <span class="dht-labels">STATUS</span><br>
    <span id="status">%STATUS%</span>
    </p>
 </div>
</div>
  </div>
  %BUTTONPLACEHOLDER%
  <script>function toggleCheckbox(element) {
    var xhr = new XMLHttpRequest();
    if(element.checked){ xhr.open("GET", "/update?output="+element.id+"&state=1", true); }
    else { xhr.open("GET", "/update?output="+element.id+"&state=0", true); }
    xhr.send();
  }
  </script>
</body>

<script>
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("temperature").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/temperature", true);
  xhttp.send();
}, 5000 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("humidity").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/humidity", true);
  xhttp.send();
}, 5000 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("status").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/status", true);
  xhttp.send();
}, 5000 ) ;
</script>
</html>)rawliteral";

// Penggantian Placeholder data MQ2
String processor(const String& var){

  //test pin
  if(var == "BUTTONPLACEHOLDER"){
    String buttons = "";
    buttons += "<label class=\"switch\"><br><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"4\" " + outputState(4) + "><span class=\"slider\"></span></label><h4>Tes Koneksi dengan LED</h4>";
   return buttons;
  }
  
  //Gas Asap
  if(var == "TEMPERATURE"){
    return String(t);
  }

  //Gas LPG
  else if(var == "HUMIDITY"){
    return String(h);
  }
   // Status
   else if(var == "STATUS"){
    return String(s);
  }
  return String();
}

void setup(){
  // Serial port for debugging purposes
  Serial.begin(115200);
  pinMode(4, OUTPUT);//PIN LED OUTPUT
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);
  dht.begin(); 
  
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println(".");
  }

  // Print ESP8266 Local IP Address
  Serial.println(WiFi.localIP());

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });
  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(t).c_str());
  });
  server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(h).c_str());
  });

  server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(s).c_str());
  });
   // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });

  // Send a GET request to <ESP_IP>/update?output=<inputMessage1>&state=<inputMessage2>
  server.on("/update", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage1;
    String inputMessage2;
    // GET input1 value on <ESP_IP>/update?output=<inputMessage1>&state=<inputMessage2>
    if (request->hasParam(PARAM_INPUT_1) && request->hasParam(PARAM_INPUT_2)) {
      inputMessage1 = request->getParam(PARAM_INPUT_1)->value();
      inputMessage2 = request->getParam(PARAM_INPUT_2)->value();
      digitalWrite(inputMessage1.toInt(), inputMessage2.toInt());
    }
    else {
      inputMessage1 = "No message sent";
      inputMessage2 = "No message sent";
    }
    Serial.print("GPIO: ");
    Serial.print(inputMessage1);
    Serial.print(" - Set to: ");
    Serial.println(inputMessage2);
    request->send(200, "text/plain", "OK");
  });
  // Start server
  server.begin();
}
 
void loop(){  
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    // save the last time you updated the MQ2 values
    previousMillis = currentMillis;
    // membaca gas asap sebagai ppm (the default)
    float newT = dht.readSmoke();

    if (isnan(newT)) {
      Serial.println("Failed to read from DHT sensor!");
    }
    else {
      t = newT;

      //kondisi LED nyala dengan parameter asap
      if(t>300){
        digitalWrite(4, HIGH);
        s="BERBAHAYA";
        }

       else{
        digitalWrite(4, LOW);
        s="AMAN";
        }
      Serial.println(t);
    }
    // membaca gas LPG as ppm
    float newH = dht.readLPG();
    
    if (isnan(newH)) {
      Serial.println("Failed to read from DHT sensor!");
    }
    else {
      h = newH;
      Serial.println(h);
    }
  }
}
