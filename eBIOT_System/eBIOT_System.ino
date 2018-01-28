#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266mDNS.h>
#include <DNSServer.h>
#include <EEPROM.h>
#include <FirebaseArduino.h>

//#define FIREBASE_HOST "ebiot-b1c8d.firebaseio.com"
//#define FIREBASE_AUTH "jJtxrvYeoQcgC14pvUwbT7fdN0Dv8thUzEHWM5Jd"
//#define FIREBASE_KEY  "AIzaSyBXdtHLY1JdwlYq-lgi-5fY8hekLiTB7YQ"
//#define WIFI_SSID "@blacxcode"
//#define WIFI_PASS "th3bl4ckh0l3"

//#define _DEBUG
//#define _DNSHOSTNAME

ESP8266WebServer webServer(80);
HTTPClient http;
DNSServer dnsServer;

IPAddress apIP(192, 168, 10, 1);
IPAddress apGateway(192, 168, 10, 1);
IPAddress apSubmask(255, 255, 255, 0);

const byte DNS_PORT = 53;
const char* dnsHostname = "ebiotsystem.iot";
const char* ssid = "eBIOT-System";
const char* pass = "ebiotsystem";
const int ch = 11; //11

const int emgbtn = A0; //A0
const int swrst = 16; //D0
const int swd1 = 5; //D1
const int swd2 = 4; //D2
const int swd3 = 0; //D3
const int swd4 = 2; //D4
const int swd5 = 14; //D5
const int swd6 = 12; //D6
const int swd7 = 13; //D7
const int swd8 = 15; //D8
const int bzrp = 3; //RX

const String ebiotclient = "/ebiotdb/ebiotclient/";
const String ebiotdata = "/ebiotdb/ebiotdata/";

String st = "", opt = "", content = "";
String esid = "", epass = "", ehost = "", eauth = "", eskey = "", ename = "", epbtn = "", emsg = "", esnd = "", ehacs = "", edata = "";

int statusCode = 0, errorCode = 0, dataSwitch = 0, remgbtn = 0;

bool data[8] = {0};
bool dscan = false;

void setup() {
  Serial.begin(115200);
  EEPROM.begin(512);
  delay(10);

#ifdef _DEBUG
  Serial.println();
  Serial.println("Startup - eBIOT System");
  Serial.println();
  Serial.println("[System Information]");
  Serial.print("> Chip ID : 0x");
  Serial.println(ESP.getChipId(), HEX);
  Serial.print("> Flash Chip ID : 0x");
  Serial.println(ESP.getFlashChipId(), HEX);
  Serial.print("> Free Memory : ");
  Serial.print(ESP.getFreeHeap());
  Serial.println(" byte");
  Serial.print("> Using Memory : ");
  Serial.print(ESP.getFlashChipSize());
  Serial.println(" byte");
  Serial.print("> Chip Speed : ");
  Serial.print(ESP.getFlashChipSpeed());
  Serial.println(" Hz");
#endif

  pinMode(emgbtn, INPUT);

  pinMode(swrst, OUTPUT);
  digitalWrite(swrst, HIGH);
  pinMode(swd1, OUTPUT);
  digitalWrite(swd1, HIGH);
  pinMode(swd2, OUTPUT);
  digitalWrite(swd2, HIGH);
  pinMode(swd3, OUTPUT);
  digitalWrite(swd3, HIGH);
  pinMode(swd4, OUTPUT);
  digitalWrite(swd4, HIGH);
  pinMode(swd5, OUTPUT);
  digitalWrite(swd5, HIGH);
  pinMode(swd6, OUTPUT);
  digitalWrite(swd6, HIGH);
  pinMode(swd7, OUTPUT);
  digitalWrite(swd7, HIGH);
  pinMode(swd8, OUTPUT);
  digitalWrite(swd8, HIGH);
  pinMode(bzrp, OUTPUT);
  digitalWrite(bzrp, LOW);

  readEEPROMData();
  delay(10);
  scanNetwork();
  delay(10);
  setupAP();
  delay(10);
  startup();
}

void readEEPROMData() {
#ifdef _DEBUG
  Serial.println();
  Serial.println("[Data Configuration]");
#endif

  esid = "";
  for (int i = 0; i < 32; ++i) {
    if (char(EEPROM.read(i)) != '\0') {
      esid += char(EEPROM.read(i));
    }
  }
#ifdef _DEBUG
  Serial.print("> SSID : ");
  Serial.println(esid);
#endif

  epass = "";
  for (int i = 32; i < 64; ++i) {
    if (char(EEPROM.read(i)) != '\0') {
      epass += char(EEPROM.read(i));
    }
  }
#ifdef _DEBUG
  Serial.print("> Password : ");
  Serial.println(epass);
#endif

  ehost = "";
  for (int i = 64; i < 128; ++i) {
    if (char(EEPROM.read(i)) != '\0') {
      ehost += char(EEPROM.read(i));
    }
  }
#ifdef _DEBUG
  Serial.print("> Database Hostname : ");
  Serial.println(ehost);
#endif

  eauth = "";
  for (int i = 128; i < 192; ++i) {
    if (char(EEPROM.read(i)) != '\0') {
      eauth += char(EEPROM.read(i));
    }
  }
#ifdef _DEBUG
  Serial.print("> Database Auth : ");
  Serial.println(eauth);
#endif

  eskey = "";
  for (int i = 192; i < 242; ++i) {
    if (char(EEPROM.read(i)) != '\0') {
      eskey += char(EEPROM.read(i));
    }
  }
#ifdef _DEBUG
  Serial.print("> Server key : ");
  Serial.println(eskey);
#endif

  ename = "";
  for (int i = 242; i < 267; ++i) {
    if (char(EEPROM.read(i)) != '\0') {
      ename += char(EEPROM.read(i));
    }
  }
#ifdef _DEBUG
  Serial.print("> Device name : ");
  Serial.println(ename);
#endif
}

void setupAP(void) {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(10);
  WiFi.softAPConfig(apIP, apGateway, apSubmask);
  delay(10);
  WiFi.softAP(ssid, pass, ch);

#ifdef _DEBUG
  Serial.println();
  Serial.println("[Access Point Configuration]");
  Serial.print("> SSID : ");
  Serial.println(ssid);
  Serial.print("> Password : ");
  Serial.println(pass);
  Serial.print("> Channel : ");
  Serial.println(ch);
#endif
}

void scanNetwork() {
  int n = WiFi.scanNetworks();
#ifdef _DEBUG
  Serial.println();
  Serial.println("[Scanning Network]");
  Serial.println("> Scanning Done");
#endif

  if (n == 0) {
    errorCode = 800;
#ifdef _DEBUG
    Serial.println("> No Networks Found!");
#endif
  } else {
#ifdef _DEBUG
    Serial.print("> ");
    Serial.print(n);
    Serial.println(" Networks Found :");
#endif

    for (int i = 0; i < n; ++i) {
#ifdef _DEBUG
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" => ");
      Serial.print(SignalStrength((int)WiFi.RSSI(i)));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(" dBm)");
      Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*");
#endif
    }
  }

#ifdef _DEBUG
  Serial.println("*) WiFi using password or encryption");
  Serial.println();
#endif

  st = "<ol>";
  opt = "";
  for (int i = 0; i < n; ++i) {
    st += "<li>";
    st += WiFi.SSID(i);
    st += " => ";
    st += SignalStrength((int)WiFi.RSSI(i));
    st += " (";
    st += WiFi.RSSI(i);
    st += " dBm)";
    st += (WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*";
    st += "</li>";
    opt += "<option value=";
    opt += WiFi.SSID(i);
    opt += ">";
    opt += WiFi.SSID(i);
    opt += "</option>";
  }
  st += "</ol>";
  st += "<label>*) WiFi using password or encryption</label><br>";
}

void startup() {
  if (esid.length() > 1) {
#ifdef _DEBUG
    Serial.println();
    Serial.print("> SSID : ");
    Serial.println(esid.c_str());
    Serial.print("> Password : ");
    Serial.println(epass.c_str());
#endif
    WiFi.begin(esid.c_str(), epass.c_str());
    //WiFi.begin("@blacxcode", "th3bl4ckh0l3");
    if (testWifi()) {
      errorCode = 0; //Succeed

#ifdef _DEBUG
      Serial.println();
      Serial.print(">> Succeed to connect to the internet");
#endif
      delay(10);
      if (setupDB()) {
        errorCode = 0; //Succeed

        //#ifdef _DNSHOSTNAME
        //        setupDNS();
        //#endif //_DNSHOSTNAME

#ifdef _DEBUG
        Serial.println();
        Serial.print(">> Succeed to connect to the database");
#endif

        launchWeb(2);
        //delay(10);
        beepReady();
      } else {
        //resetDBConf();
        errorCode = 500; //Error DB

        //#ifdef _DNSHOSTNAME
        //        setupDNS();
        //#endif //_DNSHOSTNAME

#ifdef _DEBUG
        Serial.println();
        Serial.print(">> Failed to connect to the database");
#endif
        launchWeb(1);
        //delay(10);
        beepAuthFail();
      }
    } else {
      //resetWifiConf();
      errorCode = 600; //Error Internet

      //#ifdef _DNSHOSTNAME
      //      setupDNS();
      //#endif //_DNSHOSTNAME

#ifdef _DEBUG
      Serial.println();
      Serial.print(">> Failed to connect to the internet");
#endif
      launchWeb(1);
      //delay(10);
      beepDisconnect();
    }
  } else {
    //#ifdef _DNSHOSTNAME
    //    setupDNS();
    //#endif //_DNSHOSTNAME

    launchWeb(1);
  }
}

void resetWifiConf() {
  esid = "";
  for (int i = 0; i < 32; ++i) {
    EEPROM.write(i, 0);
  }
#ifdef _DEBUG
  Serial.print("> reset SSID : ");
  Serial.println(esid);
#endif

  epass = "";
  for (int i = 32; i < 64; ++i) {
    EEPROM.write(i, 0);
  }

#ifdef _DEBUG
  Serial.print("> reset Password : ");
  Serial.println(epass);
#endif

  EEPROM.commit();
}

void resetDBConf() {
  ehost = "";
  for (int i = 64; i < 128; ++i) {
    EEPROM.write(i, 0);
  }
#ifdef _DEBUG
  Serial.print("> reset Database Hostname : ");
  Serial.println(ehost);
#endif

  eauth = "";
  for (int i = 128; i < 192; ++i) {
    EEPROM.write(i, 0);
  }
#ifdef _DEBUG
  Serial.print("> reset Database Auth : ");
  Serial.println(eauth);
#endif

  eskey = "";
  for (int i = 192; i < 242; ++i) {
    EEPROM.write(i, 0);
  }
#ifdef _DEBUG
  Serial.print("> Server key : ");
  Serial.println(eskey);
#endif

  EEPROM.commit();
}

String SignalStrength(int rssi) {
  String status = "";

  if (rssi > -50) {
    status = "Excellent";
  } else if (-50 < rssi > -60) {
    status = "Good";
  } else if (-60 < rssi > -70) {
    status = "Fair";
  } else if (rssi < -70) {
    status = "Weak";
  }
  return status;
}

bool testWifi(void) {
  int i = 0;

#ifdef _DEBUG
  Serial.println();
  Serial.println("> Waiting for Router WiFi to internet connection");
#endif

  while ( i < 50 ) {
    if (WiFi.status() == WL_CONNECTED) {

      //#ifdef _DNSHOSTNAME
      //      dnsResponder();
      //#endif //_DNSHOSTNAME

      return true;
    }
    delay(200);
#ifdef _DEBUG
    Serial.print('>');
#endif
    i++;
  }

#ifdef _DEBUG
  Serial.println();
  Serial.println("> Connection timed out, check your Router WiFi");
  Serial.println("> No internet connection");
#endif
  return false;
}

//#ifdef _DNSHOSTNAME
//void dnsResponder() {
//  if (!MDNS.begin(dnsHostname)) {
//#ifdef _DEBUG
//    Serial.println("Error setting up MDNS responder!");
//#endif
//  } else {
//#ifdef _DEBUG
//    Serial.println("mDNS responder started");
//#endif
//    MDNS.addService("http", "tcp", 80);
//  }
//}
//#endif //_DNSHOSTNAME

bool setupDB() {
#ifdef _DEBUG
  Serial.println();
  Serial.println();
  Serial.println("[Cloud Database Configuration]");
  Serial.print("> Database Hostname info : ");
  Serial.println(ehost.c_str());
  Serial.print("> Database Auth. info : ");
  Serial.println(eauth.c_str());
  Serial.print("> Server Key : ");
  Serial.println(eskey.c_str());
  Serial.print("> Device name : ");
  Serial.println(ename.c_str());
#endif
  if (!ehost.equals("") || !eauth.equals("")) {
    Firebase.begin(ehost.c_str(), eauth.c_str());

    if (Firebase.failed()) {
#ifdef _DEBUG
      Serial.print(">> create/update database failed : ");
      Serial.println(Firebase.error());
#endif
      return false;
    } else {
      delay(10);
      Firebase.setString(ebiotdata + "iotrst", "OFF");
      delay(10);
      FirebaseObject iotdata = Firebase.get(ebiotdata);

      delay(10);
      emsg = iotdata.getString("iotmsg");
      if (emsg.equals("")) {
        delay(10);
        Firebase.setString(ebiotdata + "iotmsg", "Silahkan ganti pesan ini sesuai dengan pesan yang anda inginkan!");
        delay(10);
        emsg = iotdata.getString("iotmsg");
      }
#ifdef _DEBUG
      Serial.print(">> retrieve data iotmsg : ");
      Serial.println(emsg);
#endif

      delay(10);
      esnd = iotdata.getString("iotsnd");
      if (esnd.equals("")) {
        delay(10);
        Firebase.setString(ebiotdata + "iotsnd", "OFF");
        delay(10);
        esnd = iotdata.getString("iotsnd");
      }
#ifdef _DEBUG
      Serial.print(">> retrieve data iotsnd : ");
      Serial.println(esnd);
#endif

      delay(10);
      ehacs = iotdata.getString("iothacs");
      if (ehacs.equals("")) {
        delay(10);
        Firebase.setString(ebiotdata + "iothacs", "OFF");
        delay(10);
        ehacs = iotdata.getString("iothacs");
      }
#ifdef _DEBUG
      Serial.print(">> retrieve data iothacs : ");
      Serial.println(ehacs);
#endif

      delay(10);
      epbtn = iotdata.getString("iotebtn");
      if (epbtn.equals("")) {
        delay(10);
        Firebase.setString(ebiotdata + "iotebtn", "OFF");
        delay(10);
        epbtn = iotdata.getString("iotebtn");
      }
#ifdef _DEBUG
      Serial.print(">> retrieve data iotebtn : ");
      Serial.println(epbtn);
#endif

      delay(10);
      dataSwitch = iotdata.getString("iotdacs").toInt();
      if (epbtn.equals("")) {
        delay(10);
        Firebase.setString(ebiotdata + "iotdacs", "0");
        delay(10);
        dataSwitch = iotdata.getString("iotdacs").toInt();
      }
#ifdef _DEBUG
      Serial.print(">> retrieve data iotdacs : ");
      Serial.println(dataSwitch);
      Serial.println(">> create/update database succeed!");
#endif
    }
    return true;
  } else {
    return false;
  }
}

//#ifdef _DNSHOSTNAME
//void setupDNS() {
//#ifdef _DEBUG
//  Serial.println();
//  Serial.println("[DNS Configuration]");
//  Serial.println("> Hostname : http://ebiotsystem.iot");
//#endif
//
//  // modify TTL associated  with the domain name (in seconds)
//  // default is 60 seconds
//  dnsServer.setTTL(300);
//
//  // set which return code will be used for all other domains (e.g. sending
//  // ServerFailure instead of NonExistentDomain will reduce number of queries
//  // sent by clients)
//  // default is DNSReplyCode::NonExistentDomain
//  dnsServer.setErrorReplyCode(DNSReplyCode::ServerFailure);
//
//  // start DNS server for a specific domain name
//  dnsServer.start(DNS_PORT, dnsHostname, apIP);
//#ifdef _DEBUG
//  Serial.println(">> DNSserver  service started");
//#endif
//}
//#endif //_DNSHOSTNAME

void launchWeb(int webtype) {
#ifdef _DEBUG
  Serial.println();
  Serial.println("[Network Information]");
  Serial.print("> Public IP Addr : ");
  Serial.print(WiFi.localIP());
  Serial.println(" (for Internet connection)");
  Serial.print("> Local IP Addr : ");
  Serial.print(WiFi.softAPIP());
  Serial.println(" (for Access Point)");
#endif

  createWebServer(webtype);
  //delay(10);
  webServer.begin();

#ifdef _DEBUG
  Serial.println();
  Serial.println(">> Webserver service started");
#endif
}

void createWebServer(int webtype) {
  if (webtype == 1) {
    /*==============================================================================
       HOME TYPE-1
      ==============================================================================*/
    webServer.on("/", []() {
      dscan = webServer.arg("dscan");

      content = "<!DOCTYPE HTML>\r\n";
      content += "<html>";
      content += "<head>";
      content += "<style>";
      content += "td, tr, th { border: 1px solid #DBDBDB; padding: 5px; }";
      content += ".view {margin auto; padding: 10px; width: 100%; height: 100%; border 3px solid #7B1FA2}";
      content += "</style>";
      content += "</head>";
      content += "<body bgcolor='#263238'>";
      content += "<div class='view'>";
      content += "<center>";

      content += "<font color='#F5F5F5' size='4'>";
      content += "<b><p>eBIOT System Configuration</p></b>";
      content += "</font>";

      if (dscan == true) {
        scanNetwork();
        dscan = false;
        content += "<input type='hidden' name='dscan' value='false'>";
      }

      content += "<br><font color='#DBDBDB' size='1'>";
      content += "<table cellspacing='1'>";
      if (errorCode != 500) {
        content += "<th bgcolor='#455A64' align='center' colspan='2'>";
        content += "<font color='#F5F5F5' size='2'>";
        content += "<label><b>Network Information</b></label>";
        content += "</font>";
        content += "</th>";
        content += "<tr>";
        content += "<td bgcolor='#607D8B' align='left'>";
        content += "<label>Local IP</label>";
        content += "</td>";
        content += "<td bgcolor='#607D8B' align='left'>";
        content += "<label>";
        content += getLocalIP();
        content += "</label>";
        content += "</td>";
        content += "</tr>";

        content += "<tr>";
        content += "<td bgcolor='#607D8B' align='left'>";
        content += "<label>Available Network</label>";
        content += "</td>";
        content += "<td bgcolor='#607D8B' align='left'>";
        content += st;
        content += "</td>";
        content += "</tr>";

        content += "<form method='get' action=''>";
        content += "<tr>";
        content += "<td bgcolor='#607D8B' align='left'>";
        content += "<label>Scan Network</label>";
        content += "<input type='hidden' name='dscan' value='true'>";
        content += "</td>";
        content += "<td bgcolor='#607D8B' align='right'>";
        //content += "<input type='submit' value='Scan' style='height:20px; width:60px'>";
        content += "<button onclick='scan()' style='height:20px; width:60px'>Scan</button>";
        content += "<script>";
        content += "function scan() {";
        content += "alert('Network will be scan ...');";
        content += "window.location.href = 'http://";
        content += getLocalIP();
        content += "/';";
        content += "}";
        content += "</script>";
        content += "</td>";
        content += "</tr>";
        content += "</form>";
      }

      content += "<form method='get' action='setting' id='config'>";
      content += "<th bgcolor='#455A64' align='center' colspan='2'>";
      content += "<font color='#F5F5F5' size='2'>";
      content += "<label><b>Configuration Setting</b></label>";
      content += "</font>";
      content += "</th>";
      content += "<tr>";
      content += "<td bgcolor='#607D8B' align='left'>";
      content += "<label>SSID</label>";
      content += "</td>";
      content += "<td bgcolor='#607D8B' align='left'>";
      if ((errorCode == 500 || errorCode == 800) && !esid.equals("")) {
        content += "<select disabled name='ssid' form='config' style='height:20px; width:150px'>";
        content += "<option value='";
        content += esid.c_str();
        content += "'>";
        content += esid.c_str();
        content += "</option>";
        content += "</select>";
      } else {
        content += "<select name='ssid' form='config' style='height:20px; width:150px'>";
        content += opt.c_str();
        content += "</select>";
      }
      content += "</td>";
      content += "</tr>";

      content += "<tr>";
      content += "<td bgcolor='#607D8B' align='left'>";
      content += "<label>Password</label>";
      content += "</td>";
      content += "<td bgcolor='#607D8B' align='left'>";
      if ((errorCode == 500  || errorCode == 800) && !epass.equals("")) {
        content += "<input disabled type='password' name='pass' maxlength='32' style='height:12px; width:140px' value ='";
        content += epass.c_str();
        content += "'>";
      } else {
        content += "<input type='password' name='pass' maxlength='32' style='height:12px; width:140px'>";
      }
      content += "</td>";
      content += "</tr>";

      content += "<tr>";
      content += "<td bgcolor='#607D8B' align='left'>";
      content += "<label>Database hostname</label>";
      content += "</td>";
      content += "<td bgcolor='#607D8B' align='left'>";
      if ((errorCode == 600 || errorCode == 700 || errorCode == 800) && !ehost.equals("")) {
        content += "<textarea disabled name='host' maxlength='64' cols='30' rows='1'>";
        content += ehost.c_str();
        content += "</textarea>";
      } else {
        content += "<textarea name='host' maxlength='64' cols='30' rows='1'></textarea>";
      }
      content += "</td>";
      content += "</tr>";

      content += "<tr>";
      content += "<td bgcolor='#607D8B' align='left'>";
      content += "<label>Database authentication</label>";
      content += "</td>";
      content += "<td bgcolor='#607D8B' align='left'>";
      if ((errorCode == 600 || errorCode == 700 || errorCode == 800) && !eauth.equals("")) {
        content += "<textarea disabled name='auth' maxlength='64' cols='30' rows='2'>";
        content += eauth.c_str();
        content += "</textarea>";
      } else {
        content += "<textarea name='auth' maxlength='64' cols='30' rows='2'></textarea>";
      }
      content += "</td>";
      content += "</tr>";

      content += "<tr>";
      content += "<td bgcolor='#607D8B' align='left'>";
      content += "<label>Server key</label>";
      content += "</td>";
      content += "<td bgcolor='#607D8B' align='left'>";
      if ((errorCode == 600 || errorCode == 700 || errorCode == 800) && !eskey.equals("")) {
        content += "<textarea disabled name='skey' maxlength='50' cols='30' rows='2'>";
        content += eskey.c_str();
        content += "</textarea>";
      } else {
        content += "<textarea name='skey' maxlength='50' cols='30' rows='2'></textarea>";
      }
      content += "</td>";
      content += "</tr>";

      content += "<tr>";
      content += "<td bgcolor='#607D8B' align='left'>";
      content += "<label>Device name</label>";
      content += "</td>";
      content += "<td bgcolor='#607D8B' align='left'>";
      if ((errorCode == 600 || errorCode == 700 || errorCode == 800) && !eskey.equals("")) {
        content += "<textarea disabled name='name' maxlength='25' cols='30' rows='1'>";
        content += ename.c_str();
        content += "</textarea>";
      } else {
        content += "<textarea name='name' maxlength='25' cols='30' rows='1'></textarea>";
      }
      content += "</td>";
      content += "</tr>";

      content += "<tr>";
      content += "<td bgcolor='#607D8B' align='left'>";
      content += "<label>Save configuration</label>";
      content += "</td>";
      content += "<td bgcolor='#607D8B' align='right'>";
      if (errorCode == 800) {
        content += "<input disabled type='submit' value='Save' style='height:20px; width:60px'>";
      } else {
        content += "<input type='submit' value='Save' style='height:20px; width:60px'>";
      }
      content += "</td>";
      content += "</tr>";
      content += "</form>";

      if (errorCode == 500) {
        content += "<tr>";
        content += "<th bgcolor='#D32F2F' align='left' colspan='2'>";
        content += "<label><b>Error :</b> Database hostname or Database auth. is wrong ! </label>";
        content += "</th>";
        content += "</tr>";

#ifdef _DEBUG
        Serial.println();
        Serial.print(">> errorCode : ");
        Serial.println(errorCode);
#endif
        errorCode = 500;
      }

      if (errorCode == 600 && !esid.equals("") && !epass.equals("")) {
        content += "<tr>";
        content += "<th bgcolor='#D32F2F' align='left' colspan='2'>";
        content += "<label><b>Error :</b> SSID or Password is wrong!</label>";
        content += "</th>";
        content += "</tr>";

#ifdef _DEBUG
        Serial.println();
        Serial.print(">> errorCode : ");
        Serial.println(errorCode);
#endif
        errorCode = 600;
      }

      if (errorCode == 700) {
        content += "<tr>";
        content += "<th bgcolor='#D32F2F' align='left' colspan='2'>";
        content += "<label><b>Error :</b> No internet connection !</label>";
        content += "</th>";
        content += "</tr>";

#ifdef _DEBUG
        Serial.println();
        Serial.print(">> errorCode : ");
        Serial.println(errorCode);
#endif
        errorCode = 700;
      }

      if (errorCode == 800) {
        content += "<tr>";
        content += "<th bgcolor='#D32F2F' align='left' colspan='2'>";
        content += "<label><b>Error :</b> No network found !</label>";
        content += "</th>";
        content += "</tr>";

#ifdef _DEBUG
        Serial.println();
        Serial.print(">> errorCode : ");
        Serial.println(errorCode);
#endif
        errorCode = 800;
      }

      if (errorCode == 900) {
        content += "<tr>";
        content += "<th bgcolor='#D32F2F' align='left' colspan='2'>";
        content += "<label><b>Error :</label> Router WiFi [";
        content += esid.c_str();
        content += "] not founded! Please change another router WiFi</p>";
        content += "</th>";
        content += "</tr>";

#ifdef _DEBUG
        Serial.println();
        Serial.print(">> errorCode : ");
        Serial.println(errorCode);
#endif
        errorCode = 900;
      }

      content += "<tr>";
      content += "<th align='right' colspan='2'>";
      content += "<label>copyright&copy;2017<a href='mailto:crack.codec@gmail.com?Subject=Bug%20Report' target='_top' style='color: #F5F5F5'> blacXcode</a></label>";
      content += "</th>";
      content += "</tr>";

      content += "</table>";
      content += "</font>";
      content += "</center>";
      content += "</div>";
      content += "</body>";
      content += "</html>";

#ifdef _DEBUG
      Serial.println(">> / Sending 200");
      Serial.print(">> Webtype : 1");
#endif
      webServer.send(200, "text/html", content);
    });
    /*------------------------------------------------------------------------------
      ==============================================================================*/

    /*------------------------------------------------------------------------------
       SETTING TYPE-1
      ------------------------------------------------------------------------------*/
    webServer.on("/setting", []() {
      String qsid = "", qpass = "", qhost = "", qauth = "", qskey = "", qname = "";

      if (errorCode == 500 && (!esid.equals("")) && (!epass.equals(""))) {
        qsid = esid.c_str();
        qpass = epass.c_str();

#ifdef _DEBUG
        Serial.println();
        Serial.print(">> esid : ");
        Serial.println(esid.c_str());
        Serial.print(">> epass : ");
        Serial.println(eauth.c_str());
        Serial.print(">> errorCode : ");
        Serial.println(errorCode);
#endif
      } else {
        qsid = webServer.arg("ssid");
        qpass = webServer.arg("pass");
      }

      if ((errorCode == 600) && (!ehost.equals("")) && (!eauth.equals(""))
          && (!eskey.equals("")) && (!ename.equals(""))) {
        qhost = ehost.c_str();
        qauth = eauth.c_str();
        qskey = eskey.c_str();
        qname = ename.c_str();

#ifdef _DEBUG
        Serial.println();
        Serial.print(">> ehost : ");
        Serial.println(ehost.c_str());
        Serial.print(">> eauth : ");
        Serial.println(eauth.c_str());
        Serial.print(">> eskey : ");
        Serial.println(eskey.c_str());
        Serial.print(">> ename : ");
        Serial.println(ename.c_str());
        Serial.print(">> errorCode : ");
        Serial.println(errorCode);
#endif
      } else {
        qhost = webServer.arg("host");
        qauth = webServer.arg("auth");
        qskey = webServer.arg("skey");
        qname = webServer.arg("name");
      }

#ifdef _DEBUG
      Serial.print(">> qsid : ");
      Serial.println(qsid);
      Serial.print(">> qpass : ");
      Serial.println(qpass);
      Serial.print(">> qhost : ");
      Serial.println(qhost);
      Serial.print(">> qauth : ");
      Serial.println(qauth);
      Serial.print(">> qskey : ");
      Serial.println(qskey);
      Serial.print(">> qname : ");
      Serial.println(qname);
#endif

      if (qsid.length() > 0 && qpass.length() > 0 && qhost.length() > 0
          && qauth.length() > 0 && qskey.length() > 0 && qname.length() > 0) {

        resetWifiConf();

#ifdef _DEBUG
        Serial.println(qsid);
        Serial.println(">> saving ssid to memory : ");
#endif

        for (int i = 0; i < qsid.length(); ++i) {
          EEPROM.write(i, qsid[i]);
#ifdef _DEBUG
          Serial.print(qsid[i]);
#endif
        }

#ifdef _DEBUG
        Serial.println();
        Serial.println(qpass);
        Serial.println(">> saving pass to memory : ");
#endif

        for (int i = 0; i < qpass.length(); ++i) {
          EEPROM.write(32 + i, qpass[i]);
#ifdef _DEBUG
          Serial.print(qpass[i]);
#endif
        }

        resetDBConf();

#ifdef _DEBUG
        Serial.println();
        Serial.println(qhost);
        Serial.println(">> saving host to memory : ");
#endif

        for (int i = 0; i < qhost.length(); ++i) {
          EEPROM.write(64 + i, qhost[i]);
#ifdef _DEBUG
          Serial.print(qhost[i]);
#endif
        }

#ifdef _DEBUG
        Serial.println();
        Serial.println(qauth);
        Serial.println(">> saving auth to memory : ");
#endif

        for (int i = 0; i < qauth.length(); ++i) {
          EEPROM.write(128 + i, qauth[i]);
#ifdef _DEBUG
          Serial.print(qauth[i]);
#endif
        }

#ifdef _DEBUG
        Serial.println();
        Serial.println(qskey);
        Serial.println(">> saving server key to memory : ");
#endif

        for (int i = 0; i < qskey.length(); ++i) {
          EEPROM.write(192 + i, qskey[i]);
#ifdef _DEBUG
          Serial.print(qskey[i]);
#endif
        }

#ifdef _DEBUG
        Serial.println();
        Serial.println(qname);
        Serial.println(">> saving username to memory : ");
#endif

        for (int i = 0; i < qname.length(); ++i) {
          EEPROM.write(242 + i, qname[i]);
#ifdef _DEBUG
          Serial.print(qname[i]);
#endif
        }

#ifdef _DEBUG
        Serial.println();
#endif
        EEPROM.commit();

        content = "<!DOCTYPE HTML>\r\n";
        content += "<html>";
        content += "<head>";
        content += "<style>";
        content += ".view {margin auto; padding: 10px; width: 100%; height: 100%; border 3px solid #7B1FA2}";
        content += "</style>";
        content += "</head>";
        content += "<body bgcolor='#263238'>";
        content += "<div class='view'>";
        content += "<center>";

        content += "<form method='get' action='reboot'>";
        content += "<font color='#9E9E9E' size='3'>";
        content += "<br><p>Saving Configuration ... Succeed !</p><br>";
        content += "</font>";
        content += "<button onclick='reboot()' style='height:20px; width:60px'>Reboot</button>";
        content += "<script>";
        content += "function reboot() {";
        content += "alert('The device will be reboot ...\\n\\nNote: If after reboot\\nWiFi AP [";
        content += ssid;
        content += "] does not exist,\\nPlease [Hard Reset] the device !!!');";
        content += "}";
        content += "</script>";
        content += "</form>";

        content += "</center>";
        content += "</div>";
        content += "</body>";
        content += "</html>";

        statusCode = 200;
#ifdef _DEBUG
        Serial.println(">> /setting? Sending 200");
        Serial.print(">> Webtype : 1");
#endif
      } else {
        content = "<!DOCTYPE HTML>\r\n";
        content += "<html>";
        content += "<head>";
        content += "<style>";
        content += ".view {margin auto; padding: 10px; width: 100%; height: 100%; border 3px solid #7B1FA2}";
        content += "</style>";
        content += "</head>";
        content += "<body bgcolor='#263238'>";
        content += "<div class='view'>";
        content += "<center>";

#ifdef _DEBUG
        Serial.println();
        Serial.print(">> errorCode : ");
        Serial.println(errorCode);
#endif

        content += "<font color='#D32F2F' size='3'>";
        content += "<br><p>Error Parameter : 404 not found</p><br>";
        content += "</font>";
        content += "<button onclick='refresh()' style='height:20px; width:60px'>Refresh</button>";
        content += "<script>";
        content += "function refresh() {";
        content += "window.location.href = 'http://";
        content += getLocalIP();
        content += "/';";
        content += "}";
        content += "</script>";

        content += "</center>";
        content += "</div>";
        content += "</body>";
        content += "</html>";

        statusCode = 404;
#ifdef _DEBUG
        Serial.println(">> /setting? Sending 404");
        Serial.print(">> Webtype : 1");
#endif
      }
      webServer.send(statusCode, "text/html", content);
    });
    /*------------------------------------------------------------------------------
      ==============================================================================*/

    /*------------------------------------------------------------------------------
       SETTING TYPE-1
      ------------------------------------------------------------------------------*/
    webServer.on("/reboot", []() {
      content = "<!DOCTYPE HTML>\r\n";
      content += "<html>";
      content += "<body bgcolor='#263238'>";
      content += "<script>";
      content += "window.onload = refresh() {";
      content += "window.location.href = 'http://";
      content += getLocalIP();
      content += "/';";
      content += "}";
      content += "</script>";
      content += "</body>";
      content += "</html>";

      statusCode = 200;
#ifdef _DEBUG
      Serial.println(">> /reboot? Sending 200");
      Serial.print(">> Webtype : 1");
#endif
      webServer.send(statusCode, "text/html", content);
      reboot(1);
    });
    /*------------------------------------------------------------------------------
      ==============================================================================*/

  } else if (webtype == 2) {
    /*==============================================================================
       HOME TYPE-2
      ==============================================================================*/
    webServer.on("/", []() {
      if (!Firebase.failed()) {
        delay(10);
        FirebaseObject iotdata = Firebase.get(ebiotdata);
        delay(10);
        emsg = iotdata.getString("iotmsg");
        delay(10);
        esnd = iotdata.getString("iotsnd");
        delay(10);
        ehacs = iotdata.getString("iothacs");
      }

      content = "<!DOCTYPE HTML>\r\n";
      content += "<html>";
      content += "<head>";
      content += "<style>";
      content += "td, tr, th { border: 1px solid #DBDBDB; padding: 5px; }";
      content += ".view {margin auto; padding: 10px; width: 100%; height: 100%; border 3px solid #7B1FA2}";
      content += "</style>";
      content += "</head>";
      content += "<body bgcolor='#263238'>";
      content += "<div class='view'>";
      content += "<center>";

      content += "<font color='#F5F5F5' size='4'>";
      content += "<b><p>eBIOT System Configuration</p></b>";
      content += "</font>";

      content += "<br><font color='#DBDBDB' size='1'>";
      content += "<table cellspacing='1'>";

      content += "<th bgcolor='#455A64' align='center' colspan='2'>";
      content += "<font color='#F5F5F5' size='2'>";
      content += "<label><b>Network Information</b></label>";
      content += "</font>";
      content += "</th>";

      content += "<tr>";
      content += "<td bgcolor='#607D8B' align='left'>";
      content += "<label>Local IP</label>";
      content += "</td>";
      content += "<td bgcolor='#607D8B' align='left'>";
      content += "<label>";
      content += getLocalIP();
      content += "</label>";
      content += "</td>";
      content += "</tr>";

      content += "<tr>";
      content += "<td bgcolor='#607D8B' align='left'>";
      content += "<label>Public IP</label>";
      content += "</td>";
      content += "<td bgcolor='#607D8B' align='left'>";
      content += "<label>";
      content += getPublicIP();
      content += "</label>";
      content += "</td>";
      content += "</tr>";

      content += "<th bgcolor='#455A64' align='center' colspan='2'>";
      content += "<font color='#F5F5F5' size='2'>";
      content += "<label><b>Configuration Setting</b></label>";
      content += "</font>";
      content += "</th>";

      content += "<form method='get' action='setting'>";

      content += "<tr>";
      content += "<td bgcolor='#607D8B' align='left'>";
      content += "<label>SSID</label>";
      content += "</td>";
      content += "<td bgcolor='#607D8B' align='left'>";
      content += "<label>";
      content += esid.c_str();
      content += "</label>";
      content += "</td>";
      content += "</tr>";

      content += "<tr>";
      content += "<td bgcolor='#607D8B' align='left'>";
      content += "<label>Password</label>";
      content += "</td>";
      content += "<td bgcolor='#607D8B' align='left'>";
      content += "<input type='password' name='pass' maxlength='32' style='height:12px; width:140px' value ='";
      content += epass.c_str();
      content += "'>";
      content += "</td>";
      content += "</tr>";

      content += "<tr>";
      content += "<td bgcolor='#607D8B' align='left'>";
      content += "<label>Database hostname</label>";
      content += "</td>";
      content += "<td bgcolor='#607D8B' align='left'>";
      content += "<label>";
      content += ehost.c_str();
      content += "</label>";
      content += "</td>";
      content += "</tr>";

      content += "<tr>";
      content += "<td bgcolor='#607D8B' align='left'>";
      content += "<label>Database auth.</label>";
      content += "</td>";
      content += "<td bgcolor='#607D8B' align='left'>";
      content += "<label>";
      content += eauth.c_str();
      content += "</label>";
      content += "</td>";
      content += "</tr>";

      content += "<tr>";
      content += "<td bgcolor='#607D8B' align='left'>";
      content += "<label>Server key</label>";
      content += "</td>";
      content += "<td bgcolor='#607D8B' align='left'>";
      content += "<label>";
      content += eskey.c_str();
      content += "</label>";
      content += "</td>";
      content += "</tr>";

      content += "<tr>";
      content += "<td bgcolor='#607D8B' align='left'>";
      content += "<label>Device name</label>";
      content += "</td>";
      content += "<td bgcolor='#607D8B' align='left'>";
      content += "<label>";
      content += ename.c_str();
      content += "</label>";
      content += "</td>";
      content += "</tr>";

      content += "<tr>";
      content += "<td bgcolor='#607D8B' align='left'>";
      content += "<label>Current message notif.</label>";
      content += "</td>";
      content += "<td bgcolor='#607D8B' align='left'>";
      content += "<textarea disabled maxlength='300' cols='30' rows='4'>";
      content += emsg.c_str();
      content += "</textarea>";
      content += "</td>";
      content += "</tr>";

      content += "<tr>";
      content += "<td bgcolor='#607D8B' align='left'>";
      content += "<label>Update message notif.</label>";
      content += "</td>";
      content += "<td bgcolor='#607D8B' align='left'>";
      content += "<textarea name='msg' maxlength='300' cols='30' rows='4'></textarea>";
      content += "</td>";
      content += "</tr>";

      content += "<tr>";
      content += "<td bgcolor='#607D8B' align='left'>";
      content += "<label>Sound notification</label>";
      content += "</td>";
      content += "<td bgcolor='#607D8B' align='left' form='config'>";
      content += "<select name='snd' style='height:20px; width:60px'>";
      if (esnd.equals("ON")) {
        content += "<option value='ON'>ON</option>";
        content += "<option value='OFF'>OFF</option>";
      } else {
        content += "<option value='OFF'>OFF</option>";
        content += "<option value='ON'>ON</option>";
      }
      content += "</select>";
      content += "</td>";
      content += "</tr>";

      content += "<tr>";
      content += "<td bgcolor='#607D8B' align='left'>";
      content += "<label>Home access control</label>";
      content += "</td>";
      content += "<td bgcolor='#607D8B' align='left' form='config'>";
      content += "<select name='hacs' style='height:20px; width:60px'>";
      if (ehacs.equals("ON")) {
        content += "<option value='ON'>ON</option>";
        content += "<option value='OFF'>OFF</option>";
      } else {
        content += "<option value='OFF'>OFF</option>";
        content += "<option value='ON'>ON</option>";
      }
      content += "</select>";
      content += "</td>";
      content += "</tr>";

      content += "<tr>";
      content += "<td bgcolor='#607D8B' align='left'>";
      content += "<label>Save configuration</label>";
      content += "</td>";
      content += "<td bgcolor='#607D8B' align='right'>";
      content += "<input type='submit' value='Update' style='height:20px; width:60px'>";
      content += "</td>";
      content += "</tr>";
      content += "</form>";

      content += "<form method='get' action='userlist'>";
      content += "<tr>";
      content += "<td bgcolor='#607D8B' align='left'>";
      content += "<label>User list access control</label>";
      content += "</td>";
      content += "<td bgcolor='#607D8B' align='right'>";
      content += "<input type='submit' value='Check' style='height:20px; width:60px'>";
      content += "</td>";
      content += "</tr>";

      content += "</form>";

      content += "<form method='get' action='controlstatus'>";

      content += "<tr>";
      content += "<td bgcolor='#607D8B' align='left'>";
      content += "<label>Monitoring access control</label>";
      content += "</td>";
      content += "<td bgcolor='#607D8B' align='right'>";
      content += "<input type='submit' value='Check' style='height:20px; width:60px'>";
      content += "</td>";
      content += "</tr>";

      content += "</form>";

      content += "<form method='get' action='cleareeprom'>";

      content += "<tr>";
      content += "<td bgcolor='#CDDC39' align='left'>";
      content += "<font color='#424242' size='1'>";
      content += "<label>Clear configuration</label>";
      content += "</font>";
      content += "</td>";
      content += "<td bgcolor='#CDDC39' align='right'>";
      content += "<input type='submit' value='Clear' style='height:20px; width:60px'>";
      content += "</td>";
      content += "</tr>";

      content += "</form>";

      content += "<tr>";
      content += "<th align='right' colspan='2'>";
      content += "<label>copyright&copy;2017<a href='mailto:crack.codec@gmail.com?Subject=Bug%20Report' target='top' style='color: #F5F5F5'> blacXcode</a></label>";
      content += "</th>";
      content += "</tr>";

      content += "</table>";
      content += "</font>";

      content += "</center>";
      content += "</div>";
      content += "</body>";
      content += "</html>";

      statusCode = 200;
#ifdef _DEBUG
      Serial.println(">> / Sending 200");
      Serial.print(">> Webtype : 2");
#endif
      webServer.send(statusCode, "text/html", content);
    });
    /*------------------------------------------------------------------------------
      ==============================================================================*/

    /*------------------------------------------------------------------------------
       SETTINGS TYPE-2
      ------------------------------------------------------------------------------*/
    webServer.on("/setting", []() {
      String qpass = webServer.arg("pass");
      String qmsg = webServer.arg("msg");
      String qsnd = webServer.arg("snd");
      String qhacs = webServer.arg("hacs");

      if (qpass.equals("")) qpass = epass;
      if (qmsg.equals("")) qmsg = emsg;

      if (qpass.length() > 0 && qmsg.length() > 0 && qsnd.length() > 0 && qhacs.length() > 0) {
#ifdef _DEBUG
        Serial.println();
        Serial.println(qpass);
        Serial.println(">> saving pass to memory : ");
#endif

        for (int i = 0; i < qpass.length(); ++i) {
          EEPROM.write(32 + i, qpass[i]);
#ifdef _DEBUG
          Serial.print(qpass[i]);
#endif
        }
        EEPROM.commit();

        if (!Firebase.failed()) {
          delay(10);
          FirebaseObject iotdata = Firebase.get(ebiotdata);
          delay(10);
          Firebase.setString(ebiotdata + "iotmsg", qmsg);
          delay(10);
          emsg = iotdata.getString("iotmsg");

#ifdef _DEBUG
          Serial.print(">> update message to database : ");
          Serial.println(qmsg);
          Serial.println();
#endif

          delay(10);
          Firebase.setString(ebiotdata + "iotsnd", qsnd);
          delay(10);
          esnd = iotdata.getString("iotsnd");

#ifdef _DEBUG
          Serial.print(">> update sound alert to database : ");
          Serial.println(qsnd);
          Serial.println();
#endif
          delay(10);
          Firebase.setString(ebiotdata + "iothacs", qhacs);
          delay(10);
          qhacs = iotdata.getString("iothacs");

#ifdef _DEBUG
          Serial.print(">> update access control systems to database : ");
          Serial.println(qhacs);
          Serial.println();
#endif
        }

        content = "<!DOCTYPE HTML>\r\n";
        content += "<html>";
        content += "<head>";
        content += "<style>";
        content += ".view {margin auto; padding: 10px; width: 100%; height: 100%; border 3px solid #7B1FA2}";
        content += "</style>";
        content += "</head>";
        content += "<body bgcolor='#263238'>";
        content += "<div class='view'>";
        content += "<center>";

        content += "<font color='#D32F2F' size='3'>";
        content += "<br><p>Updating Configuration ... Succeed !</p><br>";
        content += "</font>";
        content += "<button onclick='refresh()' style='height:20px; width:60px'>Refresh</button>";
        content += "<script>";
        content += "function refresh() {";
        content += "window.location.href = 'http://";
        content += getLocalIP();
        content += "/';";
        content += "}";
        content += "</script>";

        content += "</center>";
        content += "</div>";
        content += "</body>";
        content += "</html>";

        statusCode = 200;
#ifdef _DEBUG
        Serial.println(">> /setting? Sending 200");
        Serial.print(">> Webtype : 2");
#endif
      } else {
        content = "<!DOCTYPE HTML>\r\n";
        content += "<html>";
        content += "<head>";
        content += "<style>";
        content += ".view {margin auto; padding: 10px; width: 100%; height: 100%; border 3px solid #7B1FA2}";
        content += "</style>";
        content += "</head>";
        content += "<body bgcolor='#263238'>";
        content += "<div class='view'>";
        content += "<center>";

        content += "<font color='#D32F2F' size='3'>";
        content += "<br><p>Error Paramater : 404 not found</p><br>";
        content += "</font>";
        content += "<button onclick='refresh()' style='height:20px; width:60px'>Refresh</button>";
        content += "<script>";
        content += "function refresh() {";
        content += "window.location.href = 'http://";
        content += getLocalIP();
        content += "/';";
        content += "}";
        content += "</script>";

        content += "</center>";
        content += "</div>";
        content += "</body>";
        content += "</html>";

        statusCode = 404;
#ifdef _DEBUG
        Serial.println(">> /setting? Sending 404");
        Serial.print(">> Webtype : 2");
#endif
      }
      webServer.send(statusCode, "text/html", content);
    });
    /*------------------------------------------------------------------------------
      ==============================================================================*/

    /*-----------------------------------------------------------------------------
       CONTROL STATUS TYPE-2
      ------------------------------------------------------------------------------*/
    webServer.on("/controlstatus", []() {
      content = "<!DOCTYPE HTML>\r\n";
      content += "<html>";
      content += "<head>";
      content += "<style>";
      content += "td, tr, th { border: 1px solid #DBDBDB; padding: 5px; }";
      content += ".view {margin auto; padding: 10px; width: 100%; height: 100%; border 3px solid #7B1FA2}";
      content += "</style>";
      content += "<meta http-equiv='refresh' content='5'>";
      content += "</head>";
      content += "<body bgcolor='#263238'>";
      content += "<div class='view'>";
      content += "<center>";

      content += "<font color='#F5F5F5' size='4'>";
      content += "<b><p>eBIOT System Configuration</p></b>";
      content += "</font>";

      content += "<br><font color='#DBDBDB' size='1'>";
      content += "<table cellspacing='1'>";

      content += "<th bgcolor='#455A64' align='center' colspan='2'>";
      content += "<font color='#F5F5F5' size='2'>";
      content += "<label><b>Monitoring Access Control</b></label>";
      content += "</font>";
      content += "</th>";

      content += "<tr>";
      content += "<td bgcolor='#607D8B' align='left'>";
      content += "<label>Switch 1</label>";
      content += "</td>";
      if (data[0] == false) {
        content += "<td bgcolor='#00FF00' align='center'>";
        content += "<font color='#424242' size='1'>";
        content += "<label>ON</label>";
        content += "</font>";
        content += "</td>";
      } else {
        content += "<td bgcolor='#FF0000' align='center'>";
        content += "<font color='#424242' size='1'>";
        content += "<label>OFF</label>";
        content += "</font>";
        content += "</td>";
      }
      content += "</tr>";

      content += "<tr>";
      content += "<td bgcolor='#607D8B' align='left'>";
      content += "<label>Switch 2</label>";
      content += "</td>";
      if (data[1] == false) {
        content += "<td bgcolor='#00FF00' align='center'>";
        content += "<font color='#424242' size='1'>";
        content += "<label>ON</label>";
        content += "</font>";
        content += "</td>";
      } else {
        content += "<td bgcolor='#FF0000' align='center'>";
        content += "<font color='#424242' size='1'>";
        content += "<label>OFF</label>";
        content += "</font>";
        content += "</td>";
      }
      content += "</tr>";

      content += "<tr>";
      content += "<td bgcolor='#607D8B' align='left'>";
      content += "<label>Switch 3</label>";
      content += "</td>";
      if (data[2] == false) {
        content += "<td bgcolor='#00FF00' align='center'>";
        content += "<font color='#424242' size='1'>";
        content += "<label>ON</label>";
        content += "</font>";
        content += "</td>";
      } else {
        content += "<td bgcolor='#FF0000' align='center'>";
        content += "<font color='#424242' size='1'>";
        content += "<label>OFF</label>";
        content += "</font>";
        content += "</td>";
      }
      content += "</tr>";

      content += "<tr>";
      content += "<td bgcolor='#607D8B' align='left'>";
      content += "<label>Switch 4</label>";
      content += "</td>";
      if (data[3] == false) {
        content += "<td bgcolor='#00FF00' align='center'>";
        content += "<font color='#424242' size='1'>";
        content += "<label>ON</label>";
        content += "</font>";
        content += "</td>";
      } else {
        content += "<td bgcolor='#FF0000' align='center'>";
        content += "<font color='#424242' size='1'>";
        content += "<label>OFF</label>";
        content += "</font>";
        content += "</td>";
      }
      content += "</tr>";

      content += "<tr>";
      content += "<td bgcolor='#607D8B' align='left'>";
      content += "<label>Switch 5</label>";
      content += "</td>";
      if (data[4] == false) {
        content += "<td bgcolor='#00FF00' align='center'>";
        content += "<font color='#424242' size='1'>";
        content += "<label>ON</label>";
        content += "</font>";
        content += "</td>";
      } else {
        content += "<td bgcolor='#FF0000' align='center'>";
        content += "<font color='#424242' size='1'>";
        content += "<label>OFF</label>";
        content += "</font>";
        content += "</td>";
      }
      content += "</tr>";

      content += "<tr>";
      content += "<td bgcolor='#607D8B' align='left'>";
      content += "<label>Switch 6</label>";
      content += "</td>";
      if (data[5] == false) {
        content += "<td bgcolor='#00FF00' align='center'>";
        content += "<font color='#424242' size='1'>";
        content += "<label>ON</label>";
        content += "</font>";
        content += "</td>";
      } else {
        content += "<td bgcolor='#FF0000' align='center'>";
        content += "<font color='#424242' size='1'>";
        content += "<label>OFF</label>";
        content += "</font>";
        content += "</td>";
      }
      content += "</tr>";

      content += "<tr>";
      content += "<td bgcolor='#607D8B' align='left'>";
      content += "<label>Switch 7</label>";
      content += "</td>";
      if (data[6] == false) {
        content += "<td bgcolor='#00FF00' align='center'>";
        content += "<font color='#424242' size='1'>";
        content += "<label>ON</label>";
        content += "</font>";
        content += "</td>";
      } else {
        content += "<td bgcolor='#FF0000' align='center'>";
        content += "<font color='#424242' size='1'>";
        content += "<label>OFF</label>";
        content += "</font>";
        content += "</td>";
      }
      content += "</tr>";

      content += "<tr>";
      content += "<td bgcolor='#607D8B' align='left'>";
      content += "<label>Switch 8</label>";
      content += "</td>";
      if (data[7] == false) {
        content += "<td bgcolor='#00FF00' align='center'>";
        content += "<font color='#424242' size='1'>";
        content += "<label>ON</label>";
        content += "</font>";
        content += "</td>";
      } else {
        content += "<td bgcolor='#FF0000' align='center'>";
        content += "<font color='#424242' size='1'>";
        content += "<label>OFF</label>";
        content += "</font>";
        content += "</td>";
      }
      content += "</tr>";

      content += "<th bgcolor='#455A64' align='right' colspan='2'>";
      content += "<button onclick='back()' style='height:20px; width:60px'>Back</button>";
      content += "<script>";
      content += "function back() {";
      content += "window.location.href = 'http://";
      content += getLocalIP();
      content += "/';";
      content += "}";
      content += "</script>";
      content += "</th>";

      content += "<tr>";
      content += "<th align='right' colspan='2'>";
      content += "<label>copyright&copy;2017<a href='mailto:crack.codec@gmail.com?Subject=Bug%20Report' target='top' style='color: #F5F5F5'> blacXcode</a></label>";
      content += "</th>";
      content += "</tr>";

      content += "</table>";
      content += "</font>";
      content += "</center>";
      content += "</div>";
      content += "</body>";
      content += "</html>";

      statusCode = 200;
#ifdef _DEBUG
      Serial.println(">> /controlstatus? Sending 200");
      Serial.print(">> Webtype : 2");
#endif
      webServer.send(statusCode, "text/html", content);
    });
    /*------------------------------------------------------------------------------
      ==============================================================================*/

    /*------------------------------------------------------------------------------
       REMOTE USERLIST TYPE-2
      ------------------------------------------------------------------------------*/
    webServer.on("/userlist", []() {
      content = "<!DOCTYPE HTML>\r\n";
      content += "<html>";
      content += "<head>";
      content += "<style>";
      content += "td, tr, th { border: 1px solid #DBDBDB; padding: 5px; }";
      content += ".view {margin auto; padding: 10px; width: 100%; height: 100%; border 3px solid #7B1FA2}";
      content += "</style>";
      content += "</head>";
      content += "<body bgcolor='#263238'>";
      content += "<div class='view'>";
      content += "<center>";

      content += "<font color='#F5F5F5' size='4'>";
      content += "<b><p>eBIOT System Configuration</p></b>";
      content += "</font>";

      content += "<br><font color='#DBDBDB' size='1'>";
      content += "<table cellspacing='1'>";

      content += "<th bgcolor='#455A64' align='center' colspan='2'>";
      content += "<font color='#F5F5F5' size='2'>";
      content += "<label><b>List User Remote Access Request</b></label>";
      content += "</font>";
      content += "</th>";

      content += "<tr>";
      content += "<td bgcolor='#607D8B' align='left'>";
      content += "<label>Name</label>";
      content += "</td>";
      content += "<td bgcolor='#607D8B' align='left'>";
      content += "<label>Remote Access Permission</label>";
      content += "</td>";
      content += "</tr>";

      content += "<form method='get' action='upduserlist'>";

      if (!Firebase.failed()) {
        String getdataclientid = "";
        String deviceid = "";
        String remoteallow = "";
        String id = "";
        String username = "";

        for (int i = 0; i < 100; i++) {
          getdataclientid = ebiotclient + String(i);
          delay(10);
          FirebaseObject iotdataclientid = Firebase.get(getdataclientid);
          delay(10);
          deviceid = iotdataclientid.getString("deviceid");
          delay(10);
          remoteallow = iotdataclientid.getString("remoteallow");

#ifdef _DEBUG
          Serial.print("> Device ID : ");
          Serial.println(deviceid);
          Serial.print("> Remote Allow : ");
          Serial.println(remoteallow);
          Serial.print("> Username : ");
          Serial.println(username);
#endif

          if (!deviceid.equals("") && remoteallow.equals("request")) {
            id = String(i);
            delay(10);
            username = iotdataclientid.getString("username");

            content += "<tr>";
            content += "<td bgcolor='#607D8B' align='left'>";
            content += "<label >";
            content += username;
            content += "</label>";
            content += "</td>";
            content += "<td bgcolor='#607D8B' align='left'>";
            content += "<input type='checkbox' name='";
            content += id;
            content += "' value='true'> Allow<br>";
            content += "</td>";
            content += "</tr>";
          } else if (!deviceid.equals("") && remoteallow.equals("true")) {
            id = String(i);
            delay(10);
            username = iotdataclientid.getString("username");

            content += "<tr>";
            content += "<td bgcolor='#607D8B' align='left'>";
            content += "<label>";
            content += username;
            content += "</label>";
            content += "</td>";
            content += "<td bgcolor='#607D8B' align='left'>";
            content += "<input type='checkbox' name='";
            content += id;
            content += "' value='true' checked> Allow<br>";
            content += "</td>";
            content += "</tr>";
          } else if (deviceid.equals("")) {
            break;
          }
        }
      }

      content += "<th bgcolor='#455A64' align='center' colspan='2'>";
      content += "</th>";
      content += "<tr>";
      content += "<td bgcolor='#607D8B' align='left'>";
      content += "<label>Update configuration</label>";
      content += "</td>";
      content += "<td bgcolor='#607D8B' align='right'>";
      content += "<input type='submit' value='Update' style='height:20px; width:60px'>";
      content += "</td>";
      content += "</tr>";
      content += "</form>";

      content += "<th bgcolor='#455A64' align='right' colspan='2'>";
      content += "<button onclick='back()' style='height:20px; width:60px'>Back</button>";
      content += "<script>";
      content += "function back() {";
      content += "window.location.href = 'http://";
      content += getLocalIP();
      content += "/';";
      content += "}";
      content += "</script>";
      content += "</th>";

      content += "<tr>";
      content += "<th align='right' colspan='2'>";
      content += "<label>copyright&copy;2017<a href='mailto:crack.codec@gmail.com?Subject=Bug%20Report' target='_top' style='color: #F5F5F5'> blacXcode</a></label>";
      content += "</th>";
      content += "</tr>";

      content += "</table>";
      content += "</font>";
      content += "</center>";
      content += "</div>";
      content += "</body>";
      content += "</html>";

      statusCode = 200;
#ifdef _DEBUG
      Serial.println(">> /userlist? Sending 200");
      Serial.print(">> Webtype : 2");
#endif
      webServer.send(statusCode, "text/html", content);
    });
    /*------------------------------------------------------------------------------
      ==============================================================================*/

    /*-----------------------------------------------------------------------------
       UPDATE USERLIST TYPE-2
      ------------------------------------------------------------------------------*/
    webServer.on("/upduserlist", []() {
      if (!Firebase.failed()) {
        String getdataclientid = "";
        String deviceid = "";
        String remoteallow = "";
        String allow = "";

        for (int i = 0; i < 100; i++) {
          getdataclientid = ebiotclient + String(i);
          delay(10);
          FirebaseObject iotdataclientid = Firebase.get(getdataclientid);
          delay(10);
          deviceid = iotdataclientid.getString("deviceid");
          delay(10);
          remoteallow = iotdataclientid.getString("remoteallow");

          if (!deviceid.equals("") && remoteallow.equals("request")) {
            allow = webServer.arg(String(i));
            delay(10);
            Firebase.setString(ebiotclient + String(i) + "/remoteallow", allow);
          } else if (!deviceid.equals("") && remoteallow.equals("true")) {
            allow = webServer.arg(String(i));

            if (allow.equals("")) {
              allow = "false";
            }
            delay(10);
            Firebase.setString(ebiotclient + String(i)  + "/remoteallow", allow);
          } else if (deviceid.equals("")) {
            break;
          }
        }
      }

      content = "<!DOCTYPE HTML>\r\n";
      content += "<html>";
      content += "<head>";
      content += "<style>";
      content += ".view {margin auto; padding: 10px; width: 100%; height: 100%; border 3px solid #7B1FA2}";
      content += "</style>";
      content += "</head>";
      content += "<body bgcolor='#263238'>";
      content += "<div class='view'>";
      content += "<center>";

      content += "<font color='#D32F2F' size='3'>";
      content += "<br><p>Updating Configuration ... Succeed !</p><br>";
      content += "</font>";
      content += "<button onclick='refresh()' style='height:20px; width:60px'>Refresh</button>";
      content += "<script>";
      content += "function refresh() {";
      content += "window.location.href = 'http://";
      content += getLocalIP();
      content += "/';";
      content += "}";
      content += "</script>";

      content += "</center>";
      content += "</div>";
      content += "</body>";
      content += "</html>";

      statusCode = 200;
#ifdef _DEBUG
      Serial.println(">> /upduserlist? Sending 200");
      Serial.print(">> Webtype : 2");
#endif
      webServer.send(statusCode, "text/html", content);
    });
    /*------------------------------------------------------------------------------
      ==============================================================================*/

    /*------------------------------------------------------------------------------
       CLEAR EEPROM TYPE-2
      ------------------------------------------------------------------------------*/
    webServer.on("/cleareeprom", []() {
      MemClear();
      content = "<!DOCTYPE HTML>\r\n";
      content += "<html>";
      content += "<head>";
      content += "<style>";
      content += ".view {margin auto; padding: 10px; width: 100%; height: 100%; border 3px solid #7B1FA2}";
      content += "</style>";
      content += "</head>";
      content += "<body bgcolor='#263238'>";
      content += "<div class='view'>";
      content += "<center>";

      content += "<form method='get' action='reboot'>";
      content += "<font color='#D32F2F' size='3'>";

      content += "<br><p>Clearing Configuration ... Succeed !</p><br>";
      content += "</font>";
      content += "<button onclick='reboot()' style='height:20px; width:60px'>Reboot</button>";

      content += "<script>";
      content += "function reboot() {";
      content += "alert('The device will be reboot ...\\n\\nNote: If after reboot\\nWiFi AP [";
      content += ssid;
      content += "] does not exist,\\nPlease [Hard Reset] the device !!!');";
      content += "}";
      content += "</script>";

      content += "</form>";

      content += "</center>";
      content += "</div>";
      content += "</body>";
      content += "</html>";

      statusCode = 200;
#ifdef _DEBUG
      Serial.println(">> /cleareeprom? Sending 200");
      Serial.print(">> Webtype : 2");
#endif
      webServer.send(statusCode, "text/html", content);
    });
    /*------------------------------------------------------------------------------
      ==============================================================================*/

    /*------------------------------------------------------------------------------
       REBOOT TYPE-2
      ------------------------------------------------------------------------------*/
    webServer.on("/reboot", []() {
      content = "<!DOCTYPE HTML>\r\n";
      content += "<html>";
      content += "<body bgcolor='#263238'>";
      content += "<script>";
      content += "window.onload = refresh() {";
      content += "window.location.href = 'http://";
      content += getLocalIP();
      content += "/';";
      content += "}";
      content += "</script>";
      content += "</body>";
      content += "</html>";

      statusCode = 200;
#ifdef _DEBUG
      Serial.println(">> /reboot? Sending 200");
      Serial.print(">> Webtype : 2");
#endif
      webServer.send(statusCode, "text/html", content);
      reboot(1);
    });
    /*------------------------------------------------------------------------------
      ==============================================================================*/
  }
}

void reboot(int hw) {
  beepReboot();

  if (hw == 0) {
    digitalWrite(swrst, LOW);
    delay(1000);
    digitalWrite(swrst, HIGH);
  } else {
    ESP.reset();
  }
}

void MemClear() {
#ifdef _DEBUG
  Serial.println(">> Clearing Network Configuration");
#endif
  WiFi.disconnect(true);
#ifdef _DEBUG
  Serial.println(">> Clearing EEPROM");
#endif
  for (int i = 0 ; i < 512 ; i++) {
    EEPROM.write(i, 0);
  }
  EEPROM.commit();
}

String getLocalIP() {
  IPAddress ip = WiFi.softAPIP();
  String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
  return ipStr;
}

String getPublicIP() {
  IPAddress ip = WiFi.localIP();
  String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
  return ipStr;
}

//String urldecode(String str) {
//  String encodedString = "";
//  char c;
//  char code0;
//  char code1;
//
//  for (int i = 0; i < str.length(); i++) {
//    c = str.charAt(i);
//
//    if (c == '+') {
//      encodedString += ' ';
//    } else if (c == '%') {
//      i++;
//      code0 = str.charAt(i);
//      i++;
//      code1 = str.charAt(i);
//      c = (h2int(code0) << 4) | h2int(code1);
//      encodedString += c;
//    } else {
//      encodedString += c;
//    }
//    yield();
//  }
//  return encodedString;
//}

//String urlencode(String str) {
//  String encodedString = "";
//  char c;
//  char code0;
//  char code1;
//  char code2;
//
//  for (int i = 0; i < str.length(); i++) {
//    c = str.charAt(i);
//
//    if (c == ' ') {
//      encodedString += '+';
//    } else if (isalnum(c)) {
//      encodedString += c;
//    } else {
//      code1 = (c & 0xf) + '0';
//      if ((c & 0xf) > 9) {
//        code1 = (c & 0xf) - 10 + 'A';
//      }
//      c = (c >> 4) & 0xf;
//      code0 = c + '0';
//      if (c > 9) {
//        code0 = c - 10 + 'A';
//      }
//      code2 = '\0';
//      encodedString += '%';
//      encodedString += code0;
//      encodedString += code1;
//    }
//    yield();
//  }
//  return encodedString;
//}

//unsigned char h2int(char c) {
//  if (c >= '0' && c <= '9') {
//    return ((unsigned char)c - '0');
//  }
//  if (c >= 'a' && c <= 'f') {
//    return ((unsigned char)c - 'a' + 10);
//  }
//  if (c >= 'A' && c <= 'F') {
//    return ((unsigned char)c - 'A' + 10);
//  }
//  return (0);
//}

void HomeAccesControl() {
  if (WiFi.status() == WL_CONNECTED) {
    if (!Firebase.failed()) {
      int tdata[8];
      String qdata = "";
      String qrst = "";
      dataSwitch = 0;

      delay(10);
      FirebaseObject iotdata = Firebase.get(ebiotdata);
      delay(10);
      qdata = iotdata.getString("iotdacs");
      dataSwitch = qdata.toInt();
      delay(10);
      qrst = iotdata.getString("iotrst");

      if (qrst.equals("ON")) {
        reboot(1);
      }

      if (!(edata.equals(qdata))) {
#ifdef _DEBUG
        Serial.println();
        Serial.println("[Switch Data Information]");
        Serial.println("<--> Before Sync ");
        Serial.print("> qData : ");
        Serial.println(qdata);
        Serial.print("> eData : ");
        Serial.println(edata);
#endif

        edata = qdata;
#ifdef _DEBUG
        Serial.println("-><- After Sync ");
        Serial.print("> qData : ");
        Serial.println(qdata);
        Serial.print("> eData : ");
        Serial.println(edata);
        Serial.println();
#endif

        for (int i = 0; i <= 7; ++i, dataSwitch >>= 1) {
          tdata[i] = (dataSwitch & 1) + '0';
          if (tdata[i] == 48) {
            data[i] = true;
          } else if (tdata[i] == 49) {
            data[i] = false;
          } else {
            data[i] = false;
          }
        }

        digitalWrite(swd1, data[0]);
#ifdef _DEBUG
        Serial.print("> switch 1 : ");
        Serial.println(!data[0]);
#endif

        digitalWrite(swd2, data[1]);
#ifdef _DEBUG
        Serial.print("> switch 2 : ");
        Serial.println(!data[1]);
#endif

        digitalWrite(swd3, data[2]);
#ifdef _DEBUG
        Serial.print("> switch 3 : ");
        Serial.println(!data[2]);
#endif

        digitalWrite(swd4, data[3]);
#ifdef _DEBUG
        Serial.print("> switch 4 : ");
        Serial.println(!data[3]);
#endif

        digitalWrite(swd5, data[4]);
#ifdef _DEBUG
        Serial.print("> switch 5 : ");
        Serial.println(!data[4]);
#endif

        digitalWrite(swd6, data[5]);
#ifdef _DEBUG
        Serial.print("> switch 6 : ");
        Serial.println(!data[5]);
#endif

        digitalWrite(swd7, data[6]);
#ifdef _DEBUG
        Serial.print("> switch 7 : ");
        Serial.println(!data[6]);
#endif

        digitalWrite(swd8, data[7]);
#ifdef _DEBUG
        Serial.print("> switch 8 : ");
        Serial.println(!data[7]);
#endif
      }
    }
  }
}

void sendFCMNotif(String userid, String title, String message) {
  String data = "{";
  data += "\"to\": \"" + userid + "\",";
  data += "\"priority\" : \"high\",";
  data += "\"notification\" : {";
  data += "\"title\" : \"" + title + "\",";
  data += "\"body\" : \"" + message + "\" ";
  data += "\"sound\" : \"notification\",";
  data += "\"click_action\" : \"EBIOTMainActivity\",";
  data += "},";
  data += "\"data\" : {";
  data += "\"title\" : \"" + title + "\",";
  data += "\"message\" : \"" + message + "\",";
  data += "\"isbackground\" : \"true\",";
  data += "\"image\" : \"\",";
  data += "\"timestamp\" : \"\",";
  data += "\"payload\" : \"\" ";
  data += "} }";

  http.begin("http://fcm.googleapis.com/fcm/send");
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", "key=" + eskey);
  http.addHeader("Host", "fcm.googleapis.com");
  http.addHeader("Content-Length", String(data.length()));
  http.POST(data);
  http.writeToStream(&Serial);
  http.end();
  Serial.println();
}

//void sendFCMData(String userid, String title, String message, bool isBackground, String imageUrl, String timestamp, String payload) {
//  String data = "{";
//  data = data + "\"to\": \"" + userid + "\",";
//  data = data + "\"data\": {";
//  data = data + "\"title\": \"" + title + "\",";
//  data = data + "\"message\" : \"" + message + "\",";
//  data = data + "\"isbackground\" : \"" + isBackground + "\",";
//  data = data + "\"image\" : \"" + imageUrl + "\",";
//  data = data + "\"timestamp\" : \"" + timestamp + "\",";
//  data = data + "\"payload\" : \"" + payload + "\" ";
//  data = data + "} }";
//
//  http.begin("http://fcm.googleapis.com/fcm/send");
//  http.addHeader("Content-Type", "application/json");
//  http.addHeader("Authorization", "key=" + eskey);
//  http.addHeader("Host", "fcm.googleapis.com");
//  http.addHeader("Content-Length", String(content.length()));
//  http.POST(data);
//  http.writeToStream(&Serial);
//  http.end();
//  Serial.println();
//}

void beepReady() {
#ifdef _DEBUG
  Serial.println(">> Notification device ready ...");
#endif

  beep(500);
  beep(50);
  beep(50);
  beep(50);
}

void beepPushNotif() {
#ifdef _DEBUG
  Serial.println(">> Notification Push ...");
#endif

  beep(500);
}

void beepAuthFail() {
#ifdef _DEBUG
  Serial.println(">> Notification auth. failed ...");
#endif

  beep(500);
  beep(500);
}

void beepDisconnect() {
#ifdef _DEBUG
  Serial.println(">> Notification network disconnected ...");
#endif

  beep(500);
  beep(500);
  beep(500);
}

void beepReboot() {
#ifdef _DEBUG
  Serial.println(">> Notification device rebooted ...");
#endif

  beep(500);
  beep(500);
  beep(500);
  beep(500);
}

void beep(unsigned long delayms) {
  digitalWrite(bzrp, HIGH); // (+) buzzer set low
  delay(delayms);
  digitalWrite(bzrp, LOW); // (+) buzzer set high
  delay(delayms);
}

void loop() {
  //ESP.wdtDisable();
  //#ifdef _DNSHOSTNAME
  //  dnsServer.processNextRequest();
  //#endif //_DNSHOSTNAME
  //remgbtn = analogRead(emgbtn);

  if (analogRead(emgbtn) > 100) {
    if (WiFi.status() == WL_CONNECTED) {
      if (Firebase.failed()) {
#ifdef _DEBUG
        Serial.print(">> create/update database failed : ");
        Serial.println(Firebase.error());
#endif
        //return;
      } else {
        delay(10);
        Firebase.setString(ebiotdata + "iotebtn", "ON");
        delay(10);
        beepPushNotif();
        delay(10);
        Firebase.setString(ebiotdata + "iotmsg", emsg);
        delay(10);
        Firebase.setString(ebiotdata + "iotsnd", esnd);
        delay(10);
        Firebase.setString(ebiotdata + "iothacs", ehacs);

#ifdef _DEBUG
        Serial.println(">> create/update database succeed!");
        Serial.println();
        Serial.print("[Client Data Information]");
#endif
        String getdataclientid = "";
        String deviceid = "";
        String devicekey = "";

        for (int i = 0; i < 100; i++) {
          getdataclientid = ebiotclient + String(i);
#ifdef _DEBUG
          Serial.println();
          Serial.print(">> Retrieve JSON data : ");
          Serial.println(getdataclientid);
#endif
          delay(10);
          FirebaseObject iotdataclientid = Firebase.get(getdataclientid);
          delay(10);
          deviceid = iotdataclientid.getString("deviceid");
          delay(10);
          devicekey = iotdataclientid.getString("devicekey");
#ifdef _DEBUG
          Serial.print("> Device ID : ");
          Serial.println(deviceid);
          Serial.print("> Device Key : ");
          Serial.println(devicekey);
#endif
          if (!devicekey.equals("")) {
            sendFCMNotif(devicekey, ename.c_str(), emsg.c_str());
            //sendFCMData(devicekey, ename.c_str(), emsg.c_str(), true, "", "", "");
#ifdef _DEBUG
            Serial.println(">> Sending notification ... ");
#endif
          } else {
#ifdef _DEBUG
            Serial.println(">> No data to sending notification ... ");
#endif
            break;
          }
        }
      }
      delay(10);
      Firebase.setString(ebiotdata + "iotebtn", "OFF");
    }
  }

  HomeAccesControl();
  webServer.handleClient();
}
