/*
  Version 2.0 - smart system with http server
  Instructions
   keep all devices in the LAN network within ip range from 0 to 100
   let LAN ip range from 100 to 200 for arduino devices
   to change arduino sttings you have to connect from android mobile directly to arduino AP unit
   to connect to AP unit use SSID smart_home_###### password sh123456
   default device Name=default device password= device_ID= ChipId()

   EEPROM 0-10 sw status
   EEPROM 10-110 sw names
   EEPROM 110-111 device No
   EEPROM 112-126 device Name
   EEPROM 127-141 device_pass
   EEPROM 142-143 Refresh Rate
   EEPROM 144-145 No of devices
   EEPROM 146-160 WIFI SSID
   EEPROM 161-175 WIFI PASSWORD
   EEPROM 176-196 Host Name
   EEPROM 197-254 encoded host username&password
*/
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <base64.h>
#include <ESP8266Ping.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <WiFiClientSecure.h>

String AP_NAME = "smart_home_";
String AP_PASS = "" ;
String router_ssid = "SSID1";
String router_password = "Data2020";
bool isConnected = false;
const int sw_count = 10;
int Pins[sw_count] = {10, 2, 0, 4, 5, 9, 13, 12, 14, 16};
String sw_status[10];
String d_status[10];
String old_status[10];
String d_names[10];
String dev_sw_status[100][10];
String dev_sw_names[100];
String x1, x2;
String SW_Status = "";
String SW_Names = "";
unsigned int localport = 4000;
unsigned int httpport = 2000;
unsigned int clientport = 6000;
String rcv_String, device_pass, data_received, device_ID;
int memoryLoc;
uint8_t pin_led = 2;
int order;
String device_No, dev_no;
String device_Name;
String dev_names[100];
String dev_numbers[100];
String d_name, d_no;
String unitNo = "";
String sw_no, sw_state;
int unitNoI = 0;
String A, B, C;
int clientNo;
int ref_status = 0;
String LIP0, LIP1, LIP2, LIP3;
String RIP0, RIP1, RIP2, RIP3;
String routerIP;
String devList;
ESP8266WebServer wep_server(localport);
String dataSend = "";
String sw__Status;
bool sw_all_get_action = false;
bool sw_all_get_done = false;
bool scan = false;
unsigned long sw_all_get_Time;
int sw_all_index = 0;
String nod = "";   //number of devices
String refRate = "10"; //refresh rate
String QrefRate = "5"; //Quick refresh rate
String device_setup_data;
String noipServer = "dynupdate.no-ip.com";
String HostName = "";
String encoded_host_username_password = "";
String deviceIP;
//-----------http server----------------
WiFiServer httpserver(httpport);
WiFiClient client;
WiFiClient client2;
// Variable to store the HTTP request
String header;
// Current time
unsigned long currentTime = millis();
unsigned long timePeriod = 0 ;
// Previous time
unsigned long previousTime = 0;
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 5000;
int counter = 0;
int xcount = 0;
String rcv_pass = "";
String WIFI_STATUS;
bool deviceOrder = false;
IPAddress AP_IP(192, 168, 1, 200);
//-----update firmware from Git-----
unsigned long currentMillis = millis();
unsigned long previousMillis = 0;
const long interval = 60000;
const String FirmwareVer = {"1.6"};
#define URL_fw_Version "/goodjamal2000/smart_home/main/version.txt"
#define URL_fw_Bin "https://raw.githubusercontent.com/goodjamal2000/smart_home/main/firmware.bin"
const char* update_host = "raw.githubusercontent.com";
const int update_httpsPort = 443;
//-----------http server----------------
void setup() {
  Serial.begin(9600);
  EEPROM.begin(256);
  Serial.println("");
  Serial.println("ESP ID ..." + String(ESP.getChipId()) );
  Serial.println("FreeSketchSpace ..." + String(ESP.getFreeSketchSpace()));
  Serial.println("FirmWare Version:" + FirmwareVer);
  AP_NAME = AP_NAME + String(ESP.getChipId());
  device_ID = String(ESP.getChipId());
  if (!((read_String(0, 1).equals("0")) || (read_String(0, 1).equals("1")) || (read_String(0, 1).equals("2")))) {
    Serial.println("Initialize EEPROM........: " );
    for (int j = 0; j < 255; j++) {
      write_String(j , "!", 0);
    }
  }
  // ------------------
  if (read_String(0, 10).indexOf("!") >= 0)
    for (int i = 0; i < 10; i++) {
      write_String(i, "2", 0);
    }
  //------------------
  if (read_String(10, 100).indexOf("!") >= 0)
    for (int j = 0; j < 10; j++) {
      write_String(j * 10 + 10, "Switch..." + String(j), 0);
    }
  //--------------------
  if (read_String(142, 2).indexOf("!") >= 0)
    write_String(142, "20", 0);
  //---------------------
  if (read_String(144, 2).indexOf("!") >= 0)
    write_String(144, "05", 0);
  //---------------------
  if (read_String(110, 2).indexOf("!") >= 0)
    write_String(110, "00", 0);
  //---------------------
  if (read_String(112, 15).indexOf("!") >= 0)
    write_String(112, device_ID, 1);
  //-----------------------
  if (read_String(127, 15).indexOf("!") >= 0)
    write_String(127, device_ID, 1);
  //==========================
  for (int i = 0; i < 10; i++) {
    sw_status[i] = read_String(i, 10);
  }
  device_No = read_String(110, 2);
  dev_sw_names[device_No.toInt()] = read_String(10, 100);
  refRate = read_String(142, 2);
  if (refRate.toInt() < 10)refRate = "10";
  QrefRate = refRate.toInt() / 2;
  nod = read_String(144, 2);
  device_Name = read_String(112, 15);
  device_pass = read_String(127, 15);
  //router_ssid = read_String(146, 15);
  //router_password = read_String(161, 15);
  dev_names[device_No.toInt()] = device_Name;
  dev_numbers[device_No.toInt()] = device_No;
  HostName = read_String(176, 20);
  encoded_host_username_password = read_String(197, 58);
  Serial.println("switchs for " + device_No + "..." +  dev_sw_names[device_No.toInt()]);
  Serial.println("sw status at EEPROM...: " + read_String(0, 10));
  Serial.println("sw Names at EEPROM...: " + read_String(10, 100));
  Serial.println("device No at EEPROM...: " + read_String(110, 2));
  Serial.println("device Name at EEPROM...: " + read_String(112, 15));
  Serial.println("device_password ....: " + read_String(127, 15));
  Serial.println("no of devices ....: " + read_String(144, 2));
  Serial.println("refresh rate ....: " + read_String(142, 2));
  Serial.println("Router SSID at EEPROM...: " + read_String(146, 15));
  Serial.println("Router PASSWORD at EEPROM...: " + read_String(161, 15));
  Serial.println("Host Name...: " + read_String(176, 20));
  Serial.println("encoded host username password...: " + read_String(197, 58));
  for (int count = 0; count < 10; count++) {
    pinMode(Pins[count], OUTPUT);
  }
  start_AP(AP_NAME, AP_PASS);
  start_wifi_STA();
  wep_server.begin();
  httpserver.begin();
}
void loop() {
  timePeriod = timePeriod + 1;
  if ((timePeriod > 40000000) && (WiFi.status() == WL_CONNECTED)) {
    timePeriod = 0;
    DUC_Update();
  }
  /*
    if ((xcount == 0) && (WiFi.status() == WL_CONNECTED)) {
    xcount = 1;
    sw_all_get_action = true;
    sw_all_get_done = false;
    sw_all_index = 0;
    send_sw_all_get();
    }
  */
  wep_server.handleClient();
  //------------------http server-----------------
  client = httpserver.available();
  if (client)  {                           // If a new client connects,
    header = "";
    WIFI_STATUS = WiFi_Status();
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    currentTime = millis();
    previousTime = currentTime;
    while (client.connected() && currentTime - previousTime <= timeoutTime) { // loop while the client's connected
      currentTime = millis();
      if (client.available()) {           // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        dev_no = device_No;
        if (c == '\n')  {
          // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println(" <!DOCTYPE html > <html>");
            client.println(" <meta name = \"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println();
            A = client.remoteIP().toString();
            B = deviceIP;
            if (!C.equals(A)) {
              for (int i = 0; i < 10; i++) {
                old_status[i] = "a";
              }
              C = A;
            }
            Serial.println("remote IP ..." + A );
            Serial.println("device IP ..." + B );
            if ((header.indexOf(device_pass) >= 0) || (RIP0.toInt() == 192))
            {
              if ((header.indexOf("Order_" ) >= 0) && (header.indexOf("Order_" ) < 100)) {
                if ((header.indexOf("Dev_Order_" ) >= 0) && (header.indexOf("Dev_Order_" ) < 100))
                  deviceOrder = true;
                else
                  deviceOrder = false;
                rcv_String = header.substring(header.indexOf("Order_") + 6, header.indexOf("--"));
                order = rcv_String.substring(0, 2).toInt();
                rcv_pass = rcv_String.substring(3, rcv_String.indexOf("a1a"));
                data_received = rcv_String.substring(rcv_String.indexOf("a1a") + 4);
                RIP_Divider();
                Serial.println("order received...." + String(order) );
                Serial.println("device pass received...." + rcv_pass );
                Serial.println("data received...." + data_received );
                switch (order) {
                  case 0: {//startup
                      String dsn;
                      Serial.println("startup........ ");
                      for (int i = 0; i < nod.toInt() ; i++) {
                        String ii = "";
                        dsn = dev_sw_names[i];
                        if (!dsn.equals("")) {
                          if (i < 10)ii = "0" + String(i); else ii = String(i);
                          Serial.println("My_sw_names.." + ii + "$$" + dsn + "$$" );
                          client.print( ii + "$$" + dsn + "$$");
                        }
                      }
                      client.print( "!END!" + WIFI_STATUS + "$$" + WiFi.localIP().toString() + "$$" );
                      client.println("\n");
                      client.println("");
                      break;
                    }
                  case 1: {//when drawer open(nod_get)
                      nod = read_String(144, 2);
                      device_No = read_String(110, 2);
                      Serial.println("nod..." + nod + "device_No..." + device_No);
                      client.println(nod + "$$" + device_No + "$$" );
                      client.println("\n");
                      client.println("");
                      break;
                    }
                  case 2:  {//when drawer close(nod_set,device_pass)
                      client.println("Password OK");
                      if (!data_received.equals(""))nod = data_received;
                      else nod = "01";
                      device_pass = rcv_pass;
                      Serial.println("Received nod..." + nod );
                      Serial.println("Received password..." + device_pass );
                      write_String(144, nod, 0);
                      write_String(127, device_pass, 1);
                      client.println("\n");
                      client.println("");
                      break;
                    }
                  case 3: {//all_devices_info
                      devList = data_received;
                      Serial.println("devList........ " + devList );
                      while (devList.indexOf("$") > 0) {
                        d_no = devList.substring(0, 2);
                        devList = devList.substring(3);
                        d_name = devList.substring(0, 15);
                        devList = devList.substring(16);
                        Serial.println("Received ALL device info........ " );
                        Serial.println( "unit No from client " + d_no );
                        Serial.println( "unit name from client " + d_name );
                        dev_names[d_no.toInt()] = d_name;
                        dev_numbers[d_no.toInt()] = d_no;
                      }
                      IPAddress main_IP(RIP0.toInt(), RIP1.toInt(), RIP2.toInt(), RIP3.toInt());
                      if ((RIP0.toInt() == 192) && deviceOrder) {
                        client.connect(main_IP, httpport);
                        client.println("Dev_Order_10/" + device_pass + "a1a1" + "--" + "\n");
                      }
                      client.println("");
                      client.stop();
                      break;
                    }
                  case 4: {//sw_set
                      unitNo = data_received.substring(0, 2);
                      unitNoI = unitNo.toInt();
                      dev_no = unitNo;
                      sw_state = data_received.substring(3, 4);;
                      sw_no = data_received.substring(5);
                      Serial.println( "unit No...: " + unitNo);
                      Serial.println( "Request to set sw no.." + sw_no + " sw_state" + sw_state);
                      switch_set(unitNo, sw_no, sw_state);
                      unitNo = "";
                      break;
                    }
                  case 5:  {//sw_get
                      unitNo = data_received.substring(0, 2);
                      unitNoI = unitNo.toInt();
                      Serial.println( "sw_get_unit No...: " + unitNo);
                      if ((unitNoI == device_No.toInt()) || ( WIFI_STATUS.indexOf("smart_home_") >= 0)) {
                        send_default(device_No);
                      }
                      else {
                        clientNo = unitNoI + 100;
                        IPAddress unit_IP(LIP0.toInt(), LIP1.toInt(), LIP2.toInt(), clientNo);
                        if (RIP0.equals(LIP0)) {
                          client.print("<meta http-equiv='refresh' content='" + String(1) + ";url=http://" + LIP0 + "." + LIP1 + "." );
                          client.print(LIP2 + "." + String(clientNo) + ":2000/" + "Order_05/" + device_pass + "a1a1" + unitNo + "--/" + "'/>");
                        } else {
                          sw__Status = "pending";
                          client.print("<meta http-equiv='refresh' content='" + String(1) + ";url=/" + "Order_06/" + device_pass + "a1a1" + unitNo + "--" + "/"  + "'/>");
                          send_html2("Call Other Device To Get Info");
                          client2.connect(unit_IP, httpport);             // connects to the server
                          client2.println("Dev_Order_08/" + device_pass + "a1a1" + "--" + "\n");
                          client2.println("");
                        }
                      }
                      unitNo = "";
                      break;
                    }
                  case 6: {//Sw__Status_forOther
                      Serial.println( "Request to get status for other....");
                      unitNo = data_received;
                      unitNoI = unitNo.toInt();
                      for (int i = 0; i < 10; i++) {
                        sw_status[i] = dev_sw_status[unitNoI][i] ;
                      }
                      if (sw__Status.equals("Ok")) {
                        Serial.println( "Data received from other....");
                        client.print("<meta http-equiv='refresh' content='" + refRate + ";url=/Order_05_" + device_pass + "a1a1" + unitNo + "--" + "/"  + "'/>");
                        send_html(sw_status, dev_sw_names[unitNoI], unitNo);
                      }
                      else {
                        Serial.println( "Data Not received from other....");
                        client.print("<meta http-equiv='refresh' content='" + String(1) + ";url=/" + "Order_06/" +  device_pass + "a1a1" + unitNo + "--" + "/"  + "'/>");
                      } break;
                    }
                  case 7: {//set_status
                      sw_state = data_received.substring(0, 1);
                      sw_no = data_received.substring(2);
                      Serial.println( "Request from other to set sw .." + sw_no + ".." + sw_state);
                      int swnoI = sw_no.toInt();
                      String sswstate = read_String( swnoI, 1);
                      if ((sswstate.equals("1")) && (sw_state.equals("2"))) {
                        sswstate = "0";
                        Serial.println("0");
                      }
                      else if ((sswstate.equals("0")) && (sw_state.equals("2"))) {
                        sswstate = "1";
                        Serial.println("1");
                      }
                      else if ((sswstate.equals("2")) && (sw_state.equals("2"))) {
                        sswstate = "1";
                        Serial.println("11");
                      }
                      else {
                        sswstate = sw_state;
                        Serial.println("22");
                      }
                      write_String( swnoI, sswstate, 0);
                      Serial.println("sswstate...." );
                      if (sswstate.equals("1"))
                        digitalWrite(Pins[swnoI], HIGH);
                      else
                        digitalWrite(Pins[swnoI], LOW);
                      SW_Status = read_String(0, 10);
                      SW_Names = read_String(10, 100);
                      device_Name = read_String(112, 15);
                      sw__Status = "Ok";
                      Serial.println("SW_Status settled........ " + SW_Status);
                      Serial.println("Server No........ " + RIP3);
                      IPAddress main_IP(RIP0.toInt(), RIP1.toInt(), RIP2.toInt(), RIP3.toInt());
                      client.connect(main_IP, httpport);             // connects to the server
                      client.println("Dev_Order_09/" + device_pass + "a1a1" + "0$$" + device_No + "$$"  + SW_Status + "$$" + SW_Names + "$$" + device_Name + "$$" + WIFI_STATUS + "$$" + sw__Status + "$$" + "--" );
                      client.println("\n");
                      client.println("");
                      break;
                    }
                  case 8: {//get_status
                      Serial.println( "Request to get status ...!!");
                      String mystatus =  "Dev_Order_09/" + device_pass + "a1a1" + "0$$";
                      if (header.indexOf("scan" ) >= 0)mystatus = "scan/" + mystatus ;
                      SW_Status = read_String(0, 10);
                      SW_Names = read_String(10, 100);
                      device_No = read_String(110, 2);
                      device_Name = read_String(112, 15);
                      sw__Status = "Ok";
                      if ((RIP0.toInt() == 192) && deviceOrder) {
                        IPAddress main_IP(RIP0.toInt(), RIP1.toInt(), RIP2.toInt(), RIP3.toInt());
                        client.connect(main_IP, httpport);

                      }
                      client.println( mystatus +  device_No + "$$" +  SW_Status + "$$" + SW_Names + "$$" + device_Name + "$$" + sw__Status + "$$" + "--");
                      client.println("\n");
                      client.println("");
                      break;
                    }
                  case 9: {//my_status
                      String unitName, startup;
                      //if (header.indexOf("scan" ) >= 0)scan = true; else scan = false;
                      startup = data_received.substring(0, data_received.indexOf("$$"));
                      data_received = data_received.substring(data_received.indexOf("$$") + 2);
                      unitNo = data_received.substring(0, data_received.indexOf("$$"));
                      data_received = data_received.substring(data_received.indexOf("$$") + 2);
                      SW_Status = data_received.substring(0, data_received.indexOf("$$"));
                      data_received = data_received.substring(data_received.indexOf("$$") + 2);
                      SW_Names = data_received.substring(0, data_received.indexOf("$$"));
                      data_received = data_received.substring(data_received.indexOf("$$") + 2);
                      unitName = data_received.substring(0, data_received.indexOf("$$"));
                      data_received = data_received.substring(data_received.indexOf("$$") + 2);
                      sw__Status = data_received.substring(0, data_received.indexOf("$$"));
                      dev_no = unitNo;
                      Serial.println( "unit No from client " + unitNo );
                      Serial.println( "sw status from client " + SW_Status );
                      Serial.println( "sw names from client " + SW_Names );
                      Serial.println( "unit Name from client " + unitName );
                      Serial.println( "Sw Get Status " + sw__Status );
                      dev_names[dev_no.toInt()] = unitName;
                      dev_numbers[dev_no.toInt()] = unitNo;
                      dev_sw_names[dev_no.toInt()] = SW_Names;
                      for (int i = 0; i < 10; i++) {
                        dev_sw_status[dev_no.toInt()][i] = SW_Status.substring(i, i + 1);
                      }
                      if (startup.equals("1")) {
                        for (String k : dev_numbers) {
                          Serial.println("dev_numbers info........ " + k);
                          if (!k.equals("")) {
                            if (k.toInt() > 0) {
                              clientNo = k.toInt() + 100;
                              IPAddress unit_IP(LIP0.toInt(), LIP1.toInt(), LIP2.toInt(), clientNo);
                              Serial.println("contacting device to set info........ " + k);
                              String mystatus =  "Dev_Order_09/" + device_pass + "a1a1" + "0$$" ;
                              client2.connect(unit_IP, httpport);
                              client2.println( mystatus +  device_No + "$$" +  SW_Status + "$$" + SW_Names + "$$" + device_Name + "$$" + sw__Status + "$$" + "--");
                              client2.println("\n");
                              client2.println("");
                            }
                          }
                        }
                      }
                      /*client.stop();
                        if (sw_all_get_action && scan)  {
                        scan = false;
                        if (sw_all_index < (nod.toInt() - 1)) {
                          sw_all_index = sw_all_index + 1;
                          send_sw_all_get();
                        }
                        else {
                          sw_all_get_done = true;
                          sw_all_get_action = false;
                          devList = "";
                          String ii = "";
                          for (int k = 0; k < nod.toInt(); k++) {
                            if (dev_names[k].length() > 0) {
                              if (k <= 9) ii = "0" + String(k);
                              else ii = String(k);
                              devList = devList + ii + "$" + dev_names[k] + "$";
                            }
                          }
                          devList = devList + "$$";
                          Serial.println( "sw_all_get_done " + devList );
                          sw_all_index = -1;
                          send_sw_all_set();
                        }
                        }*/
                      break;
                    }
                  case 10: {//all_devicelist_ok
                      send_sw_all_set();
                      break;
                    }
                  case 11: {//sw_all_get
                      Serial.println("sw_all_get........ ");
                      if (data_received.indexOf("132568") >= 0) {
                        unitNo = data_received.substring((data_received.indexOf("132568") + 7), (data_received.indexOf("132568") + 9));
                        nod = data_received.substring((data_received.indexOf("132568") + 10), (data_received.indexOf("*")));
                        Serial.println("received unit no........ " + unitNo);
                        Serial.println("Number of Devices........ " + nod);
                        send_default(unitNo);
                      } else {
                        nod = data_received;
                        unitNo = device_No;
                        client.println("\n");
                        client.println("");
                      }
                      sw_all_get_action = true;
                      sw_all_get_done = false;
                      sw_all_index = 0;
                      send_sw_all_get();
                      break;
                    }
                  case 12: {//sw_collect
                      if (sw_all_get_done) {
                        sw_all_get_done = false;
                        String dss, dsn;
                        Serial.println("sw_collect........ ");
                        for (int i = 0; i < nod.toInt() ; i++) {
                          String ii = "";
                          dss = "";
                          dsn = dev_sw_names[i];
                          for (int k = 0; k < 10 ; k++) {
                            dss = dss + dev_sw_status[i][k];
                          }
                          if (!dsn.equals("")) {
                            if (i < 10)ii = "0" + String(i); else ii = String(i);
                            Serial.println("My_sw_collect" + ii + "$$" + dss + "$$" + dsn + "$$" + dev_names[i] + "$$" );
                            client.print( ii + "$$" + dss + "$$" + dsn + "$$" + dev_names[i] + "$$");
                          }
                        }
                        client.print("!END!");
                        client.println("\n");
                        client.println("");
                      } else {
                        client.println("sw_all_get_not_complete");
                        client.println("\n");
                        client.println("");
                      }
                      break;
                    }
                  case 13:  {//Delete/set
                      client.println("deleted");
                      if (dev_no.equals(device_No)) {
                        if (data_received.substring(2).equals("1"))
                        {
                          for (int j = 0; j < 255; j++) write_String(j , "!", 0);
                          Serial.println("deleted All");
                        } else {
                          if (data_received.substring(0, 1).equals("1"))
                          {
                            for (int index = 0 ; index < 10 ; index++) {
                              write_String(index, "2", 0);
                            }
                            Serial.println("deleted sw status");
                          }
                          if (data_received.substring(1, 2).equals("1"))
                          {
                            for (int j = 0; j < 10; j++) {
                              write_String(j * 10 + 10, "Switch..." + String(j), 0);
                            }
                            Serial.println("deleted sw names");
                          }
                        }
                      } else {
                        clientNo = dev_no.toInt() + 100;
                        IPAddress unit_IP(LIP0.toInt(), LIP1.toInt(), LIP2.toInt(), clientNo);
                        Serial.println("device No to send to..." + String(clientNo));
                        client.connect(unit_IP, httpport);             // connects to the server
                        client.println(  "Dev_Order_13/" + device_pass + "a1a1" + data_received + "--" + "\n");
                        client.println("");
                      }
                      break;
                    }
                  case 14: {//sw Names/get
                      SW_Names = read_String(10, 100);
                      SW_Names = device_No + "$$" + SW_Names;
                      Serial.println("SW Names..." + SW_Names);
                      client.println(SW_Names);
                      break;
                    }
                  case 15: {//sw Names_all/set
                      client.println("SW Names received");
                      dev_no = data_received.substring(0, 2);
                      SW_Names = data_received.substring(2);
                      if (dev_no.equals(device_No)) {
                        Serial.println("dev_no..." + dev_no);
                        Serial.println("SW_Names..." + SW_Names);
                        write_String(10, SW_Names, 0);
                      } else {
                        clientNo = dev_no.toInt() + 100;
                        IPAddress unit_IP(LIP0.toInt(), LIP1.toInt(), LIP2.toInt(), clientNo);
                        client.connect(unit_IP, httpport);             // connects to the server
                        client.println(  "Dev_Order_15/" + device_pass + "a1a1" + dev_no + SW_Names  + "--" + "\n");
                        client.println("");
                      }
                      break;
                    }
                  case 16: {//write sw Name from Alert single/set
                      client.println("SW Name received");
                      SW_Names = data_received.substring(0, 10);
                      dev_no = data_received.substring(10, 12);
                      sw_no = data_received.substring(12);
                      if (dev_no.equals(device_No)) {
                        int locno = 10 + sw_no.toInt() * 10;
                        write_String(locno, SW_Names, 0);
                      } else {
                        clientNo = dev_no.toInt() + 100;
                        IPAddress unit_IP(LIP0.toInt(), LIP1.toInt(), LIP2.toInt(), clientNo);
                        Serial.println("device No to send to..." + String(clientNo));
                        client.connect(unit_IP, httpport);             // connects to the server
                        client.println(  "Dev_Order_16/" + device_pass + "a1a1" + SW_Names + dev_no + sw_no + "--" + "\n");
                        client.println("");
                      }
                      break;
                    }
                  case 17:  {//write sw status by voice/set
                      client.println("received SW to set");
                      unitNo = data_received.substring(8, 10);
                      sw_no = data_received.substring(20, 21);
                      sw_state = data_received.substring(30);
                      Serial.println("data voice received........." + unitNo + ".." + sw_no + ".." + sw_state);
                      switch_set(unitNo, sw_no, sw_state);
                      break;
                    }
                  case 18: {//device_setup_data/get
                      device_ID = String(ESP.getChipId());
                      device_No = read_String(110, 2);
                      device_Name = read_String(112, 15);
                      router_ssid = read_String(146, 15);
                      router_password = read_String(161, 15);
                      refRate = read_String(142, 2);
                      if (refRate.toInt() < 10)refRate = "10";
                      HostName = read_String(176, 20);
                      encoded_host_username_password = read_String(197, 58);
                      device_setup_data = device_ID + "$$" + device_No + "$$" + device_Name
                                          + "$$" + router_ssid + "$$" + router_password + "$$"
                                          + refRate + "$$" + HostName + "$$" + encoded_host_username_password + "$$";
                      Serial.println("device_setup..." + device_setup_data);
                      client.println(device_setup_data);
                      break;
                    }
                  case 19: {//device_setup_data/set
                      client.println("device setup data received");
                      device_setup_data = data_received;
                      device_No = device_setup_data.substring(0, device_setup_data.indexOf("$$"));
                      device_setup_data = device_setup_data.substring(device_setup_data.indexOf("$$") + 2);
                      device_Name = device_setup_data.substring(0, device_setup_data.indexOf("$$"));
                      device_setup_data = device_setup_data.substring(device_setup_data.indexOf("$$") + 2);
                      router_ssid = device_setup_data.substring(0, device_setup_data.indexOf("$$"));
                      device_setup_data = device_setup_data.substring(device_setup_data.indexOf("$$") + 2);
                      router_password = device_setup_data.substring(0, device_setup_data.indexOf("$$"));
                      device_setup_data = device_setup_data.substring(device_setup_data.indexOf("$$") + 2);
                      refRate = device_setup_data.substring(0, device_setup_data.indexOf("$$"));
                      device_setup_data = device_setup_data.substring(device_setup_data.indexOf("$$") + 2);
                      HostName = device_setup_data.substring(0, device_setup_data.indexOf("$$"));
                      device_setup_data = device_setup_data.substring(device_setup_data.indexOf("$$") + 2);
                      encoded_host_username_password = device_setup_data.substring(0, device_setup_data.indexOf("$$"));
                      device_setup_data = device_setup_data.substring(device_setup_data.indexOf("$$") + 2);
                      if (refRate.toInt() < 10)refRate = "10";
                      QrefRate = String(refRate.toInt() / 2);
                      write_String(110, device_No, 0);
                      write_String(112, device_Name, 1);
                      write_String(146, router_ssid, 1);
                      write_String(161, router_password, 1);
                      write_String(142, refRate, 0);
                      write_String(176, HostName, 1);
                      write_String(197, encoded_host_username_password, 1);
                      start_wifi_STA();
                      break;
                    }
                  default: break;
                }
              } else {
                Serial.println("Password Correct or inside LAN..." );
                send_default(dev_no);
              }
            } else {
              Serial.println("Received Password Wrong........ " );
              client.println("Wrong Password");
              client.println("\n");
              client.println("");
              send_login_html();
            }
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("client disconnected.");
    Serial.println("");
  }
  if (sw_all_get_action) {
    if ((millis() - sw_all_get_Time) >= 5000) {
      if (sw_all_index < (nod.toInt() - 1)) {
        sw_all_index = sw_all_index + 1;
        send_sw_all_get();
      }
      else {
        sw_all_get_done = true;
        sw_all_get_action = false;
        devList = "";
        String ii = "";
        for (int k = 0; k < nod.toInt(); k++) {
          if (dev_names[k].length() == 15) {
            if (k <= 9) ii = "0" + String(k);
            else ii = String(k);
            devList = devList + ii + "$" + dev_names[k] + "$";
          }
        }
        devList = devList + "$$";
        Serial.println( "sw_all_get_done " + devList );
        sw_all_index = -1;
        send_sw_all_set();
      }
    }
  }
  //check for update every 10 hours
  currentMillis = millis();
  if ((currentMillis - previousMillis) >= interval)
  {
    previousMillis = currentMillis;
    FirmwareUpdate();
  }
}
void send_sw_all_get() {
  sw_all_get_Time = millis();
  if (sw_all_index == device_No.toInt()) {
    String dsn = read_String(10, 100);
    dev_sw_names[device_No.toInt()] = dsn;
    dev_names[device_No.toInt()] = read_String(112, 15);
    dev_numbers[device_No.toInt()] = device_No;
    for (int j = 0; j < 10; j++) {
      dev_sw_status[device_No.toInt()][j] = read_String(j, 1);
    }
    sw_all_index = sw_all_index + 1;
  }
  clientNo = sw_all_index + 100;
  IPAddress unit_IP(LIP0.toInt(), LIP1.toInt(), LIP2.toInt(), clientNo);
  Serial.println("contacting device to get info........ " + String(sw_all_index));
  client2.connect(unit_IP, httpport);             // connects to the server
  client2.println("scan/Dev_Order_08/" + device_pass + "a1a1" + "--");
  client2.println("\n");
  client2.println("");
}
void send_sw_all_set() {
  sw_all_index = sw_all_index + 1;
  int di = sw_all_index;
  int df = di + 1;
  //------------
  for (int dd = di; dd < df; dd++) {
    if ((dev_numbers[sw_all_index].length() > 0) && (sw_all_index != device_No.toInt())) {
      clientNo = sw_all_index + 100;
      IPAddress unit_IP(LIP0.toInt(), LIP1.toInt(), LIP2.toInt(), clientNo);
      Serial.println("contacting device to set info........ " + String(sw_all_index));
      client2.connect(unit_IP, httpport);             // connects to the server
      client2.println("GET /Dev_Order_03/" + device_pass + "a1a1" + devList + "--"  + "\n" );
      client2.println("");
    }
    else if (sw_all_index < nod.toInt()) {
      sw_all_index = sw_all_index + 1;
      df = df + 1;
    }
  }
}
void start_AP(String AP_name, String AP_pass) {
  //WiFi.softAPConfig(AP_IP,gateway,subnet);
  if (WiFi.softAP(AP_name, AP_pass)) {
    LED_View(16);
    Serial.println();
    Serial.println("Starting AP: " + AP_name);
    Serial.print("IP address: ");
    Serial.println(WiFi.softAPIP());
    Serial.println("Port: " + String(localport));
  }
}
void start_wifi_STA() {
  //router_ssid = read_String(146, 15);
  //router_password = read_String(161, 15);
  Serial.println("SSID: " + router_ssid);
  Serial.println("password: " + router_password);
  WiFi.hostname("DEVICE_" + String(ESP.getChipId()));
  WiFi.begin(router_ssid, router_password);
  // Waiting for Wifi connect
  counter = 0;
  while ((WiFi.status() != WL_CONNECTED) && (counter < 10)) {
    counter++;
    delay(2000);
    Serial.print(".");
  }
  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.print("Auto IP address: ");
    Serial.println(WiFi.localIP());
    LIP_Divider();
    int No;
    for (int i = 100; i < 200 ; i++) {
      No = i;
      IPAddress IPT(LIP0.toInt(), LIP1.toInt(), LIP2.toInt(), No);
      bool ret = Ping.ping(IPT , 10);
      if (!ret) {
        AP_IP = IPT;
        break;
      }
    }
    IPAddress gateway(LIP0.toInt(), LIP1.toInt(), LIP2.toInt(), 1);
    IPAddress subnet(255, 255, 255, 0);
    IPAddress dns(LIP0.toInt(), LIP1.toInt(), LIP2.toInt(), No);
    WiFi.hostname("DEVICE_" + String(ESP.getChipId()));
    WiFi.config(AP_IP, dns, gateway, subnet);
    WiFi.begin(router_ssid, router_password);
    // Waiting for Wifi connect
    counter = 0;
    while ((WiFi.status() != WL_CONNECTED) && (counter < 10)) {
      counter++;
      delay(2000);
      Serial.print(".");
    }
    if (WiFi.status() == WL_CONNECTED)
    {
      LIP_Divider();
      LED_View(2);
      Serial.println("Connected to WiFi network ");
      Serial.print("IP address: ");
      Serial.println(WiFi.localIP());
      Serial.print("Port: ");
      Serial.println(localport);
      DUC_Update();
      if (LIP3.toInt() != 100) {
        String mystatus =  "Dev_Order_09/" + device_pass + "a1a1" + "1$$" ;
        IPAddress main_IP(LIP0.toInt(), LIP1.toInt(), LIP2.toInt(), 100);
        client2.connect(main_IP, httpport);
        client2.println( mystatus +  device_No + "$$" +  SW_Status + "$$" + SW_Names + "$$" + device_Name + "$$" + sw__Status + "$$" + "--");
        client2.println("\n");
        client2.println("");
      }
    } else
      Serial.println("Not Connected to WiFi network ");
  } else
    Serial.println("Not Connected to WiFi network ");
}
void write_String(char add, String data, int _end)
{
  int _size = data.length();
  for (int i = 0; i < _size; i++)
  {
    EEPROM.write(add + i, data[i]);
  }
  if (_end == 1)EEPROM.write(add + _size, '\0'); //Add termination null character for String Data
  EEPROM.commit();
}
String read_String(char add, int length)
{
  int i;
  char data[200]; //Max 200 Bytes
  int len = 0;
  unsigned char k;
  k = EEPROM.read(add);
  while (k != '\0' && len < length) //Read until null character
  {
    k = EEPROM.read(add + len);
    data[len] = k;
    len++;
  }
  //data[len]='\0';
  String dataRead = (String(data)).substring(0, length);
  if (dataRead.substring(0, 1) == "@")dataRead = "";
  return dataRead;
}
void LED_View(int pin) {
  for (int i = 1 ; i < 3 ; i++) {
    digitalWrite(pin, HIGH);  // Turn the LED off by making the voltage HIGH
    delay(1000);            // Wait for two seconds
    digitalWrite(pin, LOW);   // Turn the LED on by making the voltage LOW
    delay(1000);            // Wait for a second
  }
}
void send_html( String sw_status[10], String sw_names_list, String dev_no) {
  String swNamesList = sw_names_list;
  String sw_names[10];
  unitNo = dev_no;
  for (int si = 0; si < 10; si++) {
    sw_names[si] = swNamesList.substring(0, 10);
    swNamesList = swNamesList.substring(10);
  }
  // Display the HTML web page
  client.println("<head><link rel = \"icon\" href=\"data:,\">");
  client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 1px auto; text-align: center;}");
  client.println(".button {color: white;border: 2px solid black;text-align: center;width: 45%;height:100px;");
  client.println("text-decoration: none; font-size: 25px; margin: 5px; cursor: pointer;}");
  client.println(".button1 {background-color:Red;}");
  client.println(".button2 {background-color:Green;}");
  client.println(".button3 {background-color:Gray;border: 2px solid black;height:100px;}");
  client.println(".button4 {background-color:Yellow;font-size: 16px;text-align:center; width:50; height:45 ;border: 1px solid black;}");
  client.println(".button5 {background-color:Yellow;font-size: 16px;text-align:center; width:150; height:45 ;border: 1px solid black;}");
  client.println("</style></head>");
  client.println("<body style='background-color: #aacc55' ><h2><font color='black'>Smart Home System</h2>");
  //------------
  client.println("<form name = unit style = 'display: inline;'>");
  client.println("<input type=submit name='' value='GO' class = button4 formaction='/' > ");
  client.println("<select autocomplete =on class= button5 name=");
  client.println("Order_05_" + device_pass + "a1a>");
  Serial.println("device_No..........." + device_No);
  Serial.println("unitNo........." + unitNo);
  for (int i = 0; i < nod.toInt(); i++) {
    if (i == unitNo.toInt())
      client.println("<option name=" + dev_names[i] + " value=" + dev_numbers[i] + "--" + " selected>" + dev_numbers[i] + "--" + dev_names[i] + "</option>");
    else
      client.println("<option name=" + dev_names[i] + " value=" + dev_numbers[i] + "--" + ">" + dev_numbers[i] + "--" + dev_names[i] + "</option>");
  }
  client.println("</select>");
  client.println("</form>");
  //----------
  client.println("<form name = unit style = 'display: inline;'>");
  //client.println("<select class= button4 name= Order_11_" + device_pass + "a1a1132568_" + dev_no + ">");
  //for (int i = 1; i <= 100; i++) {
  //  if (i == nod.toInt())
  //    client.println("<option name=" + nod + " value=" + String(i) + "*--" + " selected>" + String(i) + "</option>");
  //  else
  //    client.println("<option name=" + nod + " value=" + String(i) + "*--" + " >" + String(i) + "</option>");
  //}
  //client.println("</select>");
  client.println("<input type=submit name=Order_11_" + device_pass + "a1a1132568_" + dev_no + "-" + nod + "*--" + " value=Scan" + " class = button4 formaction='/' > ");
  client.println("</form> </br> ");
  //-----------
  for (int j = 0; j < 5; j++) {
    x1 = sw_status[2 * j];
    x2 = sw_status[2 * j + 1];
    if (x1 == "0")
      client.println("<a href=\"/Order_04/" + device_pass + "a1a1" + dev_no + "/1/" + String(2 * j) + "--" + "\"><button class = \"button button1\">" + sw_names[2 * j] + "</button></a>");
    else if (x1 == "1")
      client.println("<a href=\"/Order_04/" + device_pass + "a1a1" + dev_no + "/0/" + String(2 * j) + "--" + "\"><button class = \"button button2\">" + sw_names[2 * j] + "</button></a>");
    else
      client.println("<a href=\"/Order_04/" + device_pass + "a1a1" + dev_no + "/1/" + String(2 * j) + "--" + "\"><button class = \"button button3\">" + sw_names[2 * j] + "</button></a>");
    if (x2 == "0")
      client.println("<a href=\"/Order_04/" + device_pass + "a1a1" + dev_no + "/1/" + String(2 * j + 1) + "--" + "\"><button class = \"button button1\">" + sw_names[2 * j + 1] + "</button></a>");
    else if (x2 == "1")
      client.println("<a href=\"/Order_04/" + device_pass + "a1a1" + dev_no + "/0/" + String(2 * j + 1) + "--" + "\"><button class = \"button button2\">" + sw_names[2 * j + 1] + "</button></a>");
    else
      client.println("<a href=\"/Order_04/" + device_pass + "a1a1" + dev_no + "/1/" + String(2 * j + 1) + "--" + "\"><button class = \"button button3\">" + sw_names[2 * j + 1] + "</button></a>");
  }
  client.println("</body></html>");
  // The HTTP response ends with another blank line
  client.println();

}
void send_html2( String message) {
  client.println("<head><link rel=\"icon\" href=\"data:,\">");
  client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}</style></head>");
  client.println("<body><h2><font color='black'> " + message + " </h2>");
  client.println("</body></html>");
  client.println();
}
void send_default(String unit) {
  client.print("<meta http-equiv='refresh' content='" + refRate + ";url=/" + "Order_05/" + device_pass + "a1a1" + unit + "--/" + "'/>");
  dev_sw_names[dev_no.toInt()] = read_String(10, 100);
  for (int i = 0; i < 10; i++) {
    sw_status[i] = read_String(i, 1);
    dev_sw_status[device_No.toInt()][i] = sw_status[i];
  }
  Serial.println("Send default unit no.." + unit);
  send_html(sw_status , dev_sw_names[dev_no.toInt()], unit);
  unitNo = "";
}
void switch_set(String unitno, String swno, String swstate) {
  int unitnoI = unitno.toInt();
  int swnoI = swno.toInt();
  if (unitnoI == device_No.toInt())
  {
    String sswstate = read_String(swnoI, 1);
    if ((sswstate.equals("1")) && (swstate.equals("2")))sswstate = "0";
    else if ((sswstate.equals("0")) && (swstate.equals("2")))sswstate = "1";
    else if ((sswstate.equals("2")) && (swstate.equals("2")))sswstate = "1";
    else sswstate = swstate;
    Serial.println("data to write........." + unitno + ".." + swno + ".." + sswstate);
    write_String(swnoI, sswstate, 0);
    if (sswstate.equals("1"))
      digitalWrite(Pins[swnoI], HIGH);
    else
      digitalWrite(Pins[swnoI], LOW);
    dev_sw_names[unitnoI] = read_String(10, 100);
    for (int i = 0; i < 10; i++) {
      dev_sw_status[unitnoI][i] = read_String( i, 1);
      sw_status[i] = dev_sw_status[unitnoI][i] ;
    }
    send_default(unitno);
  }
  else {
    if (RIP0.equals(LIP0))
      client.print("<meta http-equiv='refresh' content='" + String(1) + ";url=http://" + String(LIP0) + "." + String(LIP1) + "." + String(LIP2) + "." + String(clientNo) + ":2000/a1a1" + device_pass + "--" + "'/>");
    else {
      sw__Status = "pending";
      client.print("<meta http-equiv='refresh' content='" + String(1) + ";url=/" + "Order_06/" + device_pass + "a1a1" + unitNo + "--" + "/"  + "'/>");
      send_html2("Call Other Device To Set Info");
      Serial.println( "sending to other");
      clientNo = unitnoI + 100;
      IPAddress unit_IP(LIP0.toInt(), LIP1.toInt(), LIP2.toInt(), clientNo);
      client2.connect(unit_IP, httpport);
      client2.println("Dev_Order_07/" + device_pass + "a1a1" + swstate + "/" + swno + "--" + "\n");
      client2.println("");
    }
  }
}
void send_login_html() {

  // Display the Login HTML web page
  client.println("<head><link rel=\"icon\" href=\"data:,\">");
  client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 1px auto; text-align: center;}");
  client.println("text-decoration: none; font-size: 25px; margin: 5px; cursor: pointer;}");
  client.println(".button1 {background-color:Grey;border: 2px solid black;text-align: center;width: 70%;height:50%;}");
  client.println("</style></head>");
  client.println("<body style='background-color: #aacc55' ><h2><font color='black'>Login Password</h2>");
  client.println("<form name=login style='display: inline;'>");
  client.println("<input type=password name=password maxlength=15 maxwidth=10 ; > ");
  client.println("<input type=submit value='OK' class= button1 background-color=Gray formaction='/' > ");
  client.println("</form></br>");
  client.println("</body></html>");
  // The HTTP response ends with another blank line
  client.println();
}
void LIP_Divider() {
  String LIPString = WiFi.localIP().toString();
  LIP0 = LIPString.substring(0, LIPString.indexOf("."));
  LIPString = LIPString.substring(LIPString.indexOf(".") + 1);
  LIP1 = LIPString.substring(0, LIPString.indexOf("."));
  LIPString = LIPString.substring(LIPString.indexOf(".") + 1);
  LIP2 = LIPString.substring(0, LIPString.indexOf("."));
  LIPString = LIPString.substring(LIPString.indexOf(".") + 1);
  LIP3 = LIPString;
}
void RIP_Divider() {
  String RIPString = client.remoteIP().toString();
  RIP0 = RIPString.substring(0, RIPString.indexOf("."));
  RIPString = RIPString.substring(RIPString.indexOf(".") + 1);
  RIP1 = RIPString.substring(0, RIPString.indexOf("."));
  RIPString = RIPString.substring(RIPString.indexOf(".") + 1);
  RIP2 = RIPString.substring(0, RIPString.indexOf("."));
  RIPString = RIPString.substring(RIPString.indexOf(".") + 1);
  RIP3 = RIPString;
}
void DUC_Update() {
  //--------duc to connect to NO-IP server to update ip address
  client.connect(noipServer, 80);
  if (client.connect(noipServer, 80)) {
    Serial.println("Starting DUC.....");
    Serial.println("HostName....." + HostName);
    Serial.println("encoded_host_username_password....." + encoded_host_username_password);
    Serial.println("Connected to noip");
    client.println("GET /nic/update?hostname=" + HostName + " HTTP/1.1");
    client.println("Host: dynupdate.no-ip.com");
    client.println("Authorization:Basic " + encoded_host_username_password);
    //use Authorization: Basic base64-encoded-auth-string
    //goto https://www.opinionatedgeek.com/codecs/base64encoder to convert String authorization to Base64
    client.println("User-Agent: AlGamal Smart_home Client/0.0 goodjamal@yahoo.com");
    client.println();
    // close your end after the server closes its end
    client.stop();
  }
}
String WiFi_Status() {
  String RIPString = client.remoteIP().toString();
  RIP0 = RIPString.substring(0, RIPString.indexOf("."));
  RIPString = RIPString.substring(RIPString.indexOf(".") + 1);
  RIP1 = RIPString.substring(0, RIPString.indexOf("."));
  RIPString = RIPString.substring(RIPString.indexOf(".") + 1);
  RIP2 = RIPString.substring(0, RIPString.indexOf("."));
  RIPString = RIPString.substring(RIPString.indexOf(".") + 1);
  RIP3 = RIPString;
  if ((RIP0.toInt() == 192) && (RIP2.toInt() == 4)) {
    WIFI_STATUS = "smart_home_" + String(ESP.getChipId());
    deviceIP = WiFi.softAPIP().toString();
  }
  else if ((RIP0.toInt() == 192) && (RIP2.toInt() != 4)) {
    WIFI_STATUS = "WiFi:inner_" + String(ESP.getChipId());
    deviceIP = WiFi.localIP().toString();
  }
  else {
    WIFI_STATUS = "outer_network";
    deviceIP = WiFi.localIP().toString();
  }

  return WIFI_STATUS;
}
void FirmwareUpdate()
{
  Serial.println("FirmwareUpdate started......");
  WiFiClientSecure clientSecure;
  if (!clientSecure.connect(update_host, update_httpsPort)) {
    Serial.println("Git Connection for update failed");
    return;
  }
  clientSecure.print(String("GET ") + URL_fw_Version + " HTTP/1.1\r\n" +
                     "Host: " + update_host + "\r\n" +
                     "User-Agent: SmartHomeControlSystem\r\n" +
                     "Connection: close\r\n\r\n");
  while (clientSecure.connected()) {
    String line = clientSecure.readStringUntil('\n');
    if (line == "\r") {
      break;
    }
  }
  String payload = clientSecure.readStringUntil('\n');
  payload.trim();
  Serial.println("new FirmwareVer..." + payload);
  Serial.println("original FirmwareVer...." + FirmwareVer);
  if (payload.equals( FirmwareVer) )
  {
    Serial.println("Device already on latest firmware version");
  }
  else
  {
    Serial.println("New firmware detected");
    pinMode(LED_BUILTIN, OUTPUT);
    ESPhttpUpdate.setLedPin(LED_BUILTIN, LOW);
    t_httpUpdate_return ret = ESPhttpUpdate.update(clientSecure, URL_fw_Bin);

    switch (ret) {
      case HTTP_UPDATE_FAILED:
        Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s\n", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
        break;

      case HTTP_UPDATE_NO_UPDATES:
        Serial.println("HTTP_UPDATE_NO_UPDATES");
        break;

      case HTTP_UPDATE_OK:
        Serial.println("HTTP_UPDATE_OK");
        break;
    }
  }
}
