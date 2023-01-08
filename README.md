# kvantisoija_helvetistä
4 Channel Pitch CV Quantizer with Sample and Hold

Comes in 2 flavors for Eurorack and other synthesizers with bipolar 12v or 15v supplies or for AE Modular and similar with unipolar 5v supplies. 
Features can be mix and matched and of course modified to fit how you plan to use it. 

The original code is set up to output 0-2v from the DAC which then must be scaled up via opamp and potentiometer.
The 5v code is set up to output 0-5v directly from the DAC. You should measure your supply voltage precisely and update the code with the correct value.

Whatever version you decide to use, check your DAC chip - the i2c address is set for MCP4728A4 by default and you may need to change it.

If you have issues or questions message me on the AE Modular forum or Mod Wiggler both @haggardgrin and I'll do what I can do!

Default scales: Chromatic, All diatonic modes, Major/minor pentatonic, Blues, Harmonic minor, Arabian, Whole Tone, Tritone, Octaves and Fifths (for tuning or one stop drone shop), 24 TET, 22 Shruti - please message me if you're familiar with ragas I'd like to add some!


Update 8 Jan '23 - Reworked code to look like 2nd year project instead of "look mom I coded a thing!" Some 200 lines of fat trimmed, big thanks joem and PexiTheBuilder and FlatulentGonad!
