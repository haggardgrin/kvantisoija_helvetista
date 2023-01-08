#include <Adafruit_MCP4728.h>
#include <Wire.h>

//Where the fiery planes clash with the material

Adafruit_MCP4728 mcp; 

int Channel = 0;
int i = 0;
int x = 0;
int X = 0;

const int NextScalePin = 9;
const int PrevScalePin = 8;
const int Scale00Pin = 7;
const int ModePin = 6;

const int TrigAPin = 5;
const int TrigBPin = 4;
const int TrigCPin = 3;
const int TrigDPin = 2;

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
  Serial.println("MCP4728 Found!");

  mcp.setChannelValue(MCP4728_CHANNEL_A, 0, MCP4728_VREF_INTERNAL, MCP4728_GAIN_1X);
  mcp.setChannelValue(MCP4728_CHANNEL_B, 0, MCP4728_VREF_INTERNAL, MCP4728_GAIN_1X);
  mcp.setChannelValue(MCP4728_CHANNEL_C, 0, MCP4728_VREF_INTERNAL, MCP4728_GAIN_1X);
  mcp.setChannelValue(MCP4728_CHANNEL_D, 0, MCP4728_VREF_INTERNAL, MCP4728_GAIN_1X);

  pinMode(NextScalePin, INPUT); //D9 Next Scale 
  pinMode(PrevScalePin, INPUT); //D8 Prev Scale
  pinMode(Scale00Pin, INPUT); //D7 Default Scale
  pinMode(ModePin, INPUT); //D6 Mode
  pinMode(TrigAPin, INPUT); //D5 TrigA
  pinMode(TrigBPin, INPUT); //D4 TrigB
  pinMode(TrigCPin, INPUT); //D3 TrigC
  pinMode(TrigDPin, INPUT); //D2 TrigD 

  Serial.println("Movin and groovin");
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
      if (Channel == 0 ) {Trig = digitalRead(TrigAPin); } 
      if (Channel == 1 ) {Trig = digitalRead(TrigBPin); }
      if (Channel == 2 ) {Trig = digitalRead(TrigCPin); }
      if (Channel == 3 ) {Trig = digitalRead(TrigDPin); }
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
  if (Channel == 0) {CVin = analogRead(A0); }  
  if (Channel == 1) {CVin = analogRead(A1); }
  if (Channel == 2) {CVin = analogRead(A2); }
  if (Channel == 3) {CVin = analogRead(A3); }
  CVin = map(CVin, 0, 1023, 0, 4999); 
  x = CVin; 
  x %= 1000;  
  X = x; 
  i = 0; 
  while (X > 10) { 
    i++; 
    if (i > MaxNoteQTY) { 
      i = 0;
      CVin += 1000; //not sure why this is necessary but it fixed a glitch...
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
  if (Channel == 0) {mcp.setChannelValue(MCP4728_CHANNEL_A, DAC, MCP4728_VREF_INTERNAL, MCP4728_GAIN_1X); } 
  if (Channel == 1) {mcp.setChannelValue(MCP4728_CHANNEL_B, DAC, MCP4728_VREF_INTERNAL, MCP4728_GAIN_1X); }
  if (Channel == 2) {mcp.setChannelValue(MCP4728_CHANNEL_C, DAC, MCP4728_VREF_INTERNAL, MCP4728_GAIN_1X); }
  if (Channel == 3) {mcp.setChannelValue(MCP4728_CHANNEL_D, DAC, MCP4728_VREF_INTERNAL, MCP4728_GAIN_1X); }
}
