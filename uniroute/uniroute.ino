#include <Ethernet.h>
#include <Modbus.h>
#include <ModbusIP.h>
#include "SoftWare.h"
#include "route2.h"

// regular constants
const bool      db            = 0;
// modbus instances
const int       start_coil    = 100,
                alarm_coil    = 101,
                score_ireg    = 102,
                prep_coil     = 103;
ModbusIP        mb;
// web interface instances
EthernetServer  server(80);
String string = "";
// loop's functions
void mr_refresh();
void web_show();
void test_time();

void setup() {
  if( db ){
    Serial.begin(19200);
    Serial.println("Setup");
  }
  //modbus slave
  mb.config(MacAd, IPAd);
  mb.addCoil(start_coil,false);
  mb.addCoil(alarm_coil,false);
  mb.addCoil(prep_coil,false);
  mb.addIreg(score_ireg);
  //web server
  Ethernet.begin(MacAd, IPAd);
  server.begin();
  if( db ){ 
    Serial.print("IP address: ");
    Serial.println(Ethernet.localIP());
  }
  //route instance
  route.mount();
  if( db ){ Serial.println("begin"); }
}

void loop() {
  // modbus update
  mr_refresh();
  //web-server update
  web_show();
  //route refresh
  route.refresh();
  // debug functions
  //test_time();
}

void mr_refresh(){
  mb.task();
  if( mb.Coil(start_coil) == 1 ){
    route.start();
  }
  if( mb.Coil(alarm_coil) == 1 ){
    route.alarm();
  }
  if( mb.Coil(prep_coil) == 1 ){
    route.stop();
  }
  mb.Ireg(score_ireg, route.score());
}

void web_show(){  
  EthernetClient client = server.available();
  if (client) {
    while (client.connected()) { 
      if (client.available()) {
        char c = client.read();
        if( c == '\n' ) {
          route.takeGet(&string);
          string=""; 
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println();
          client.println("<HTML>");
            client.println("<HEAD>");
              client.print("<TITLE>"); client.print(Name); client.println("</TITLE>");
            client.println("</HEAD>");
            client.println("<BODY>");
              client.println("<hr />");
              client.println("<br />");
              client.print("<H2>"); client.print(Name); client.println("</H2>");
              client.println("<br />");
              client.println("<form method=\"get\">");
              route.giveForm(&string);
              client.println(string);
              string = "";
              client.println("</form>");
              client.print("score = ");client.print(route.score());client.println(";");
            client.println("</BODY>");
          client.println("</HTML>");
          client.stop(); // stopping client
        } // end HTTP REQUEST
        else {
          string += c;
        }
      } // end Client.Available
    } // end Client.Connected
  } // end if(client)
}

void test_time(){
  static uint32_t print_time = 0;
  static uint32_t loop_counter = 0;
  ++loop_counter;
  if( timeInterval(print_time) > 1000 ){
    if(db){ 
      Serial.print(">>> ");
      Serial.print(loop_counter);
      Serial.print(" loops in one second; Step: ");
      Serial.print(route.stat());
      Serial.print("; Score: ");
      Serial.println(route.score());
    }
    print_time = millis();
    loop_counter = 0;
  }
}
