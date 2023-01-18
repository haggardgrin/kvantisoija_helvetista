#include <Adafruit_MCP4728.h>
#include <Wire.h>

//Where the fiery planes clash with the material

Adafruit_MCP4728 mcp; 

const float SupplyVoltage = 5.025;  //Measure your supply voltage precisely and input here

float vCor = 0; 

int Channel = 0;
int i = 0;
int x = 0;
int X = 0;

const int NextScalePin = 9;
const int PrevScalePin = 8;
const int Scale00Pin = 7;
const int ModePin = 6;

const int TrigPins[] = {5, 4, 3, 2};
const int CVinPins[] = {A0, A1, A2, A3};
const int MCPChannel[] = {MCP4728_CHANNEL_A, MCP4728_CHANNEL_B, MCP4728_CHANNEL_C, MCP4728_CHANNEL_D};

int Scale00PinRead = LOW;
int NextScalePinRead = LOW;
int PrevScalePinRead = LOW;

//notes in mV, first octave only
const int CurrentScale[18][24] = {
  {0, 83, 167, 250, 333, 417, 500, 583, 667, 750, 833, 917}, //Chromatic - default
  {0, 167, 333, 417, 583, 750, 917}, //Ionian/Major
  {0, 167, 250, 417, 583, 750, 833}, //Dorian
  {0, 83, 250, 417, 583, 667, 833}, //Phrygian
  {0, 167, 333, 500, 583, 750, 917}, //Lydian
  {0, 167, 333, 417, 583, 750, 833}, //Mixolydian
  {0, 167, 250, 417, 583, 667, 833}, //Aeolian/Minor
  {0, 83, 250, 417, 500, 667, 833}, //Locrian
  {0, 167, 333, 583, 750}, //Major Pentatonic
  {0, 250, 417, 583, 833}, //Minor Pentatonic
  {0, 250, 333, 417, 500, 583, 833, 917}, //Blues
  {0, 167, 250, 417, 583, 667, 833, 917}, //Harmonic Minor
  {0, 83, 333, 417, 583, 667, 917}, //Arabian
  {0, 167, 333, 500, 667, 833}, //Whole Tone
  {0, 250, 500, 750}, //Tritone
  {0, 583}, //Octaves and 5ths for tuning by ear
  {0, 42, 83, 125, 167, 209, 250, 292, 333, 375, 417, 459, 500, 542, 583, 625, 667, 709, 750, 792, 833, 875, 917, 959}, //24TET
  {0, 75, 93, 152, 170, 245, 263, 322, 340, 415, 433, 492, 510, 585, 660, 678, 737, 755, 830, 848, 907, 925}, //22 Shruti 
};
int ScaleQTY = 17;  //update these and size of the array if you add more scales!
int MaxNoteQTY = 23;  //remember to subtract one!
int ScaleID = 0; 

int Mode = 0; //low = Free Running, high = Sample + Hold
int CVin = 0;
int Trig = 0;
int Note = 0;
int Oct = 0;
int mV = 0;
int DAC = 0;

void setup() {
  Wire.begin();
  Serial.begin(115200);
  while (!Serial) 
    delay(10); 
//  if (!mcp.begin()) {   //Use this line if you have MCP4728
  if (!mcp.begin(0x64)) {   //Use this line if you have MCP4728A4
    Serial.println("Failed to find MCP4728 chip");
    while (1) {
    delay(10);
   }
  }   

  for (Channel = 0; Channel < 4; Channel++ ) {
    mcp.setChannelValue(MCPChannel[Channel], 0, MCP4728_VREF_VDD);
    pinMode(TrigPins[Channel], INPUT);
  }
  pinMode(NextScalePin, INPUT); //D9 Next Scale 
  pinMode(PrevScalePin, INPUT); //D8 Prev Scale
  pinMode(Scale00Pin, INPUT); //D7 Default Scale
  pinMode(ModePin, INPUT); //D6 Mode

  vCor = SupplyVoltage / 5;
}

void loop() {
  Scale00PinRead = digitalRead(Scale00Pin); 
  if (Scale00PinRead == HIGH) {
    ScaleID = 0;
    delay(250);
  }

  NextScalePinRead = digitalRead(NextScalePin);  
  if (NextScalePinRead == HIGH) {
    ScaleID++;
    if (ScaleID > ScaleQTY) { ScaleID = 0; }
    delay(250);
  }

  PrevScalePinRead = digitalRead(PrevScalePin); 
  if (PrevScalePinRead == HIGH) {
      ScaleID--;
      if (ScaleID < 0) {ScaleID = ScaleQTY; }
      delay(250);
  }

  Mode = digitalRead(ModePin);
  if (Mode == HIGH) {
    for (Channel = 0; Channel < 4; Channel++) {
      Trig = digitalRead(TrigPins[Channel]);
      if (Trig == HIGH) {ProcessCV(Channel); }
    }
  }
  if (Mode == LOW) {
    for (Channel = 0; Channel < 4; Channel++) {
      ProcessCV(Channel);
    }
  }
  delay(0);     // Whine without, damn black magic thingies
}

void ProcessCV(int Channel) {
  CVin = analogRead(CVinPins[Channel]);
  CVin = map(CVin, 0, 1023, 0, 4999); 
  x = CVin; 
  i = 0;
  if (x < 20) {
    mcp.setChannelValue(MCPChannel[Channel], 0, MCP4728_VREF_VDD);
    }
  if (x >= 20) {
    x %= 1000;  
    X = x; 
    while (X > 1) { 
      i++; 
      if (i > MaxNoteQTY) { 
        i = 0;
        CVin += 1000; 
        break;
      }
      X = x; 
      X -= CurrentScale[ScaleID][i]; 
    }
    Note = CurrentScale[ScaleID][i]; 
    x = CVin; 
    x /= 1000; 
    x *= 1000; 
    Oct = x; 
    mV = Note + Oct; 
    DAC = map(mV, 0, 4999, 0, 4095);
    DAC *= vCor;
    if (DAC > 4095) {DAC = 4095;}
    if (DAC < 0) {DAC = 0;}
    mcp.setChannelValue(MCPChannel[Channel], DAC, MCP4728_VREF_VDD);
  }
}
