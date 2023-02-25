#include "twilio.hpp"

// Set these - but DON'T push them to GitHub!
static const char *ssid = "1172 wifi sharks";
static const char *password = "Arni1309";

// Values from Twilio (find them on the dashboard)
static const char *account_sid = "Axx2";
static const char *auth_token = "1cxxx86";
// Phone number should start with "+<countrycode>"
static const char *from_number = "+13xxx45";

// You choose!
// Phone number should start with "+<countrycode>"
static const char *to_number = "+918xx1";
static char *critical = "Alert! Health is critical. Please consult a doctor!\n - ";
static char *greet = "Welcome on board! Your device UID is ";
static const char *uid = "l33tc0de";
Twilio *twilio;

void init_message() {
  // remove the code for wifi setup in the main file
  Serial.begin(115200);
  Serial.print("Connecting to WiFi network ;");
  Serial.print(ssid);
  Serial.println("'...");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Connecting...");
    delay(500);
  }
  Serial.println("Connected!");

  twilio = new Twilio(account_sid, auth_token);

  delay(1000);
  String response;
  strcat(greet, uid);
  bool success = twilio->send_message(to_number, from_number, greet, response);
  if (success) {
    Serial.println("Sent message successfully!");
    free(greet);
  } else {
    Serial.println(response);
  }
}

void alert_message() {
  int alert = 1;
  strcat(critical, uid);
  if (alert) {
    String response;
    bool success = twilio->send_message(to_number, from_number, critical, response);
    if (success) {
      Serial.println("Sent message successfully!");
      // free(critical);
    } else {
      Serial.println(response);
    }
  } else {
    Serial.println("everything is normal!!!");
  }
}
