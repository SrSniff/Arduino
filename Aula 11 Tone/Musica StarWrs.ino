
const int d = 294;
const int dS = 311;
const int e = 329;
const int f = 349;
const int fS = 370;
const int g = 391;
const int gS = 415;
const int a = 440;
const int aS = 466;
const int aSL = 233;
const int b = 494;
const int bH = 932;
const int cH = 523;
const int cSH = 554;
const int dH = 587;
const int dSH = 622;
const int eH = 659;
const int fH = 698;
const int fSH = 740;
const int fSL = 185;
const int gH = 784;
const int gSH = 830;
const int aH = 880;

const int buzzerPin = 5;
const int ledPin1 = 12;
const int ledPin2 = 13;

int counter = 0;

void setup()
{
//Setup pin modes
pinMode(buzzerPin, OUTPUT);
pinMode(ledPin1, OUTPUT);
pinMode(ledPin2, OUTPUT);
}

void loop()
{

//Play first section
firstSection();



}

void beep(int note, int duration)
{
//Play tone on buzzerPin
tone(buzzerPin, note, duration);

//Play different LED depending on value of 'counter'
if(counter % 2 == 0)
{
digitalWrite(ledPin1, HIGH);
delay(duration);
digitalWrite(ledPin1, LOW);
}else
{
digitalWrite(ledPin2, HIGH);
delay(duration);
digitalWrite(ledPin2, LOW);
}

//Stop tone on buzzerPin
noTone(buzzerPin);

delay(50);

//Increment counter
counter++;
}

void firstSection()
{

beep(g, 500);
beep(g, 500);
beep(g, 500);
beep(dS, 350);
beep(aS, 150);
beep(g, 500);
beep(dS, 350);
beep(aS, 150);
beep(g, 650);

delay(500);

beep(dH, 500);
beep(dH, 500);
beep(dH, 500);
beep(dSH, 350);
beep(aS, 150);
beep(fS, 500);
beep(dS, 350);
beep(aS, 150);
beep(g, 650);

delay(500);

beep(gH, 500);
beep(g, 300);
beep(g, 150);
beep(gH, 500);
beep(fSH, 325);
beep(fH, 175);
beep(eH, 125);
beep(dSH, 125);
beep(eH, 250);

delay(325);

beep(gS, 250);
beep(cSH, 500);
beep(cH, 325);
beep(b, 175);
beep(aS, 125);
beep(a, 125);
beep(aS, 250);

delay(350);

beep(dS, 250);
beep(fS, 500);
beep(dS, 350);
beep(fS, 125);
beep(aS, 500);
beep(g, 375);
beep(aS, 125);
beep(dH, 650);

delay(500);

beep(gH, 500);
beep(g, 300);
beep(g, 150);
beep(gH, 500);
beep(fSH, 325);
beep(fH, 175);
beep(eH, 125);
beep(dSH, 125);
beep(eH, 250);

delay(325);

beep(gS, 250);
beep(cSH, 500);
beep(cH, 325);
beep(b, 175);
beep(aS, 125);
beep(a, 125);
beep(aS, 250);

delay(350);

beep(dS, 250);
beep(fS, 500);
beep(dS, 350);
beep(aS, 150);
beep(g, 500);
beep(dS, 350);
beep(aS, 150);
beep(g, 650);

delay(500);

}