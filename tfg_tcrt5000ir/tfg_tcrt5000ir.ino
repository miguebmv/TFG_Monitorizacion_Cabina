const int pinIRd = 10;
int IRvalueD = 0;

void setup()
{
  Serial.begin(115200);
  pinMode(pinIRd,INPUT);
}

void loop()
{

  IRvalueD = digitalRead(pinIRd);
  
  Serial.print("\t Digital Reading=");
  Serial.println(IRvalueD);
  delay(5000);
 
}
