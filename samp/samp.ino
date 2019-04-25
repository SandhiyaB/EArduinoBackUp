void setup(){
//start the Serial connection
Serial.begin(115200);
pinMode(8,OUTPUT);
pinMode(9,OUTPUT);
digitalWrite(8,LOW);
digitalWrite(9,LOW);
}

void loop(){
if (Serial.available()){
char a=Serial.read();
if(a=='a')
{
digitalWrite(9,HIGH);
}
else if(a=='b')
{
digitalWrite(9,LOW);
}
}
}
