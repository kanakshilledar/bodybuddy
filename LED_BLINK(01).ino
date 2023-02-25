float body_temperature= 104;
int heart_rate=89;
int SpO2_level=80;
void setup()
{
  pinMode(6,OUTPUT);
  pinMode(7,OUTPUT);
  pinMode(8,OUTPUT);
  Serial.begin(9600);
}
void BODY_TEMPERATURE();
void HEART_RATE();
void SpO2_LEVEL();
void loop()
{
  BODY_TEMPERATURE();
  HEART_RATE();
  SpO2_LEVEL();
}
void BODY_TEMPERATURE()
{
  if (body_temperature<97)
  {
    digitalWrite(6,HIGH);
    delay(1000);
    Serial.println("Low Temperature");
  }
else if(body_temperature>=97 && body_temperature<=99)
{
  digitalWrite(6,LOW);
  delay(1000);
  Serial.println("Normal Temperature");
}
else if(body_temperature>=100)
  {
  digitalWrite(6,HIGH);
  delay(1000);
  Serial.println("High Temperature");
  }
}
void HEART_RATE()
{
if(heart_rate>=60 && heart_rate<=100)
{
  Serial.println("Normal Heart Rate");
}
else if (heart_rate < 60)
  {
  digitalWrite(7,HIGH);
  delay(1000);
  Serial.println("Slow Heart Rate");
  }
else if (heart_rate > 100)
  {
  digitalWrite(7,HIGH);
  delay(1000);
  Serial.println("High Heart Rate"); 
  }
}
void SpO2_LEVEL()
{
if(SpO2_level>90 && SpO2_level<100)
{
  Serial.println("Normal Oxygen Level");
}
else if(SpO2_level<=90)
  {
  digitalWrite(8,HIGH);
  delay(1000);
  Serial.println("Low Oxygen Level");
  }
}