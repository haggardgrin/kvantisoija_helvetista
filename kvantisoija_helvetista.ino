#include <Adafruit_MCP4728.h>
#include <Wire.h>

//Where the fiery planes clash with the material

Adafruit_MCP4728 mcp; 

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

int CVinA = 0;
int CVinB = 0;
int CVinC = 0;
int CVinD = 0;

int TrigA = 0;
int TrigB = 0;
int TrigC = 0;
int TrigD = 0;

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

int ANote = 0;
int AOct = 0;
int AmV = 0;
int ADAC = 0;
int BNote = 0;
int BOct = 0;
int BmV = 0;
int BDAC = 0;
int CNote = 0;
int COct = 0;
int CmV = 0;
int CDAC = 0;
int DNote = 0;
int DOct = 0;
int DmV = 0;
int DDAC = 0;

void setup() {
  Wire.begin();
  Serial.begin(115200);
  while (!Serial) 
    delay(10); 
//  if (!mcp.begin()) {   //Use this line if you have MCP4728
  if (!mcp.begin(0x64)) {   //Use this line if you have MCP4728A4
    while (1) {
    delay(10);
   }
  }   

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

}

void loop() {
  Scale00PinRead = digitalRead(Scale00Pin); 
  switch (Scale00PinRead)  {
    case LOW:
      break;
    case HIGH:
      ScaleID = 0;
      delay(150);
      break;
  }

  NextScalePinRead = digitalRead(NextScalePin);  
  switch (NextScalePinRead) {
    case LOW:
      break;  
    case HIGH:
      ScaleID++;
      if (ScaleID > ScaleQTY) {
        ScaleID = 0;
      }
      delay(150);
      break;
  }

  PrevScalePinRead = digitalRead(PrevScalePin); 
  Serial.print("Prev Scale Request:");
  switch (PrevScalePinRead) {
    case LOW:
      Serial.println("Nope!");
      break;  
    case HIGH:
      Serial.println("Yup!");
      ScaleID--;
      if (ScaleID < 0) {
        ScaleID = ScaleQTY;
      }
      delay(150);
      break;
  }

  Mode = digitalRead(ModePin); 
  switch (Mode) {
    case LOW:
      goto ProcessCVA;
    case HIGH:
      TrigA = digitalRead(TrigAPin); 
      switch (TrigA) {
        case LOW:
          goto ADone;
        case HIGH:
          goto ProcessCVA;
      }
  }
      
  ProcessCVA:    
  CVinA = analogRead(A0);
    if(CVinA > 1023) {CVinA = 1023; }
    if(CVinA < 0) {CVinA = 0; }
  CVinA = map(CVinA, 0, 1023, 0, 4999); //convert 10 bit to mV  
  x = CVinA; 
  x %= 1000;  //remove octaves, leaving only notes
  X = x;
  while (X > (CurrentScale[ScaleID][1] / 2)) {
    i++;
    if (i > MaxNoteQTY) {
      i = 0;
      CVinA += 1000;
      break;
    }
    X = x; //dummy
    X -= CurrentScale[ScaleID][i]; //Subtract each scale note from input sequentially until result 0 or less
  }
  ANote = CurrentScale[ScaleID][i]; //save correct note for later 
  X = 0; //return variables for next channel
  i = 0;
  x = CVinA; 
  x /= 1000;
  x = abs(x); 
  x *= 1000; //remove notes, leaving only octaves
  AOct = x;
  AmV = ANote + AOct;
  ADAC = map(AmV, 0, 4999, 0, 4095);
  mcp.setChannelValue(MCP4728_CHANNEL_A, ADAC, MCP4728_VREF_INTERNAL, MCP4728_GAIN_1X);

  ADone:
  switch (Mode) {
    case LOW:
      goto ProcessCVB;
    case HIGH:
      TrigB = digitalRead(TrigBPin); 
      switch (TrigB) {
        case 0:
          goto BDone;
        case 1:
          goto ProcessCVB;
      }
  }
      
  ProcessCVB:   
  CVinB = analogRead(A1);
    if(CVinB > 1023) {CVinB = 1023; }
    if(CVinB < 0) {CVinB = 0; }
  CVinB = map(CVinB, 0, 1023, 0, 4999); 
  x = CVinB; 
  x %= 1000;  
  X = x;
  while (X > (CurrentScale[ScaleID][1] / 2)) {
    i++;
    if (i > MaxNoteQTY) {
      i = 0;
      CVinB += 1000;
      break;
    }
    X = x; 
    X -= CurrentScale[ScaleID][i]; 
  }
  BNote = CurrentScale[ScaleID][i]; 
  X = 0; 
  i = 0;
  x = CVinB; 
  x /= 1000;
  x = abs(x); 
  x *= 1000; 
  BOct = x; 
  BmV = BNote + BOct;
  BDAC = map(BmV, 0, 4999, 0, 4095);
  mcp.setChannelValue(MCP4728_CHANNEL_B, BDAC, MCP4728_VREF_INTERNAL, MCP4728_GAIN_1X);

  BDone:
  switch (Mode) {
    case LOW:
      goto ProcessCVC;
    case HIGH:
      TrigC = digitalRead(TrigCPin); 
      switch (TrigC) {
        case LOW:
          goto CDone;
        case HIGH:
          goto ProcessCVC;
      }
  }
      
  ProcessCVC:
  CVinC = analogRead(A2);
    if(CVinC > 1023) {CVinC = 1023; }
    if(CVinC < 0) {CVinC = 0; }
  CVinC = map(CVinC, 0, 1023, 0, 4999); 
  x = CVinC; 
  x %= 1000;  
  X = x;
  while (X > (CurrentScale[ScaleID][1] / 2)) {
    i++;
    if (i > MaxNoteQTY) {
      i = 0;
      CVinC += 1000;
      break;
    }
    X = x; 
    X -= CurrentScale[ScaleID][i]; 
  }
  CNote = CurrentScale[ScaleID][i]; 
  X = 0; 
  i = 0;
  x = CVinC; 
  x /= 1000;
  x = abs(x); 
  x *= 1000; 
  COct = x; 
  CmV = CNote + COct;
  CDAC = map(CmV, 0, 4999, 0, 4095);
  mcp.setChannelValue(MCP4728_CHANNEL_C, CDAC, MCP4728_VREF_INTERNAL, MCP4728_GAIN_1X);

  CDone:
  switch (Mode) {
    case LOW:
      goto ProcessCVD;
    case HIGH:
      TrigD = digitalRead(TrigDPin); 
      switch (TrigD) {
        case 0:
          goto DDone;
        case 1:
          goto ProcessCVD;
      }
  }
      
  ProcessCVD:  
  CVinD = analogRead(A3);
    if(CVinD > 1023) {CVinD = 1023; }
    if(CVinD < 0) {CVinD = 0; }
  CVinD = map(CVinD, 0, 1023, 0, 4999);
  x = CVinD; 
  x %= 1000; 
  X = x;
  while (X > (CurrentScale[ScaleID][1] / 2)) {
    i++;
    if (i > MaxNoteQTY) {
      i = 0;
      CVinD += 1000;
      break;
    }
    X = x; 
    X -= CurrentScale[ScaleID][i]; 
  }
  DNote = CurrentScale[ScaleID][i]; 
  X = 0; 
  i = 0;
  x = CVinD; 
  x /= 1000;
  x = abs(x); 
  x *= 1000; 
  DOct = x; 
  DmV = DNote + DOct;
  DDAC = map(DmV, 0, 4999, 0, 4095);
  mcp.setChannelValue(MCP4728_CHANNEL_D, DDAC, MCP4728_VREF_INTERNAL, MCP4728_GAIN_1X);

  DDone:

  Serial.print("Output A:");
  Serial.println(AmV);
  Serial.print("Output B:");
  Serial.println(BmV);
  Serial.print("Output C:");
  Serial.println(CmV);
  Serial.print("Output D:");
  Serial.println(DmV);

  delay(0);     // Whine without, damn black magic thingies

}    
