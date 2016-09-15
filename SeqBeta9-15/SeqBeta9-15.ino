/*  MIDI SEQ BETA
 *   github:  embankment/midiseq
 *   A non-quantized multitrack MIDI sequencer
 *   using the Teensy 3.X 
 */

int ledPin = 13;
unsigned int i;
byte BPM = 120;
byte roll = 0;
const byte max_tracks = 4;  //for now
unsigned long track_ticks[max_tracks];
const unsigned int max_notes = 2000;  // RAM is the limit; 6000 notes uses 81% of memory and gets stability warning
unsigned int num_notes = 4;  //there are 4 starting note events seeded

elapsedMicros ticks;

typedef struct track_specs  //if does not work as expected, move to .cpp
  {
      unsigned long track_length;
      float speed_mult;
      byte track_chan;
  }   track_specs;

  track_specs track[max_tracks];  //

typedef struct note_type  //if does not work as expected, move to .cpp
  {
      unsigned long timestamp;
      byte played;
      byte channel;      
      byte note;
      byte velocity;
  }  note_type;

  note_type notes[max_notes];  //there's more RAM, but 9 bytes X 2000 events - 18K

void setup() {
  // put your setup code here, to run once:
  
  for (i = 0; i < max_tracks; i = i + 1) {        //  tracks can have completely different speeds (reverse maybe in the future)
     track[i] = (track_specs) {10000000,1.0,1};   //  initializing tracks (length in micros, speed mult, MIDI channel)
     }
  
  for (i = 0; i < max_notes; i = i + 1) {     //blanking notes (time of play, not played, chan/note/vel)
     notes[i] = (note_type) {0,0,255,0,0};    //channel of 255 is basically a "not used" flag
     }

  notes[0] = {2000000,0,8,44,110};  // seeding some notes
  notes[1] = {5000000,0,8,44,0};
  notes[2] = {7000000,0,8,46,112};
  notes[3] = {4000000,0,8,47,113};

  usbMIDI.setHandleNoteOff(OnNoteOff);       // launch callback handlers
  usbMIDI.setHandleNoteOn(OnNoteOn);
  
  pinMode(ledPin, OUTPUT);                  // blink
  digitalWrite(ledPin, HIGH);
  delay(400);
  digitalWrite(ledPin, LOW);
  
  Serial.begin(9600); // USB is always 12 Mbit/sec
  ticks = 0;          // reset the elapsedMillis clock going into the loop
}

void loop() {

usbMIDI.read();  //get and process notes via callbacks

for (i = 0; i < max_tracks; i = i + 1) {            //  tracks can have completely different speeds (reverse maybe in the future)
     track_ticks[i] = ticks * track[i].speed_mult;  //  position accounting for speed mult
     } //tracktime multipliers go here 

  for (i = 0; i < num_notes; i = i + 1) {
    if (notes[i].channel != 255) {
     if (ticks >= notes[i].timestamp && notes[i].played != roll) {  //note is past time to play and hasn't played this roll
      usbMIDI.sendNoteOn(notes[i].note, notes[i].velocity, notes[i].channel);
      Serial.print("notes #: ");
      Serial.println(i);    
      notes[i].played = roll;
      }
     }
   }

// update track 
if (ticks >= track[0].track_length) {
  Serial.print("tick rollover at: ");
  Serial.println(ticks);  
  ticks=ticks-track[0].track_length;
  Serial.print("roll is at: ");
  Serial.println(roll);   
  Serial.print("num_notes is at: ");
  Serial.println(num_notes);   
  roll++;
  }
}


void OnNoteOn(byte channel, byte note, byte velocity)
{
      notes[num_notes] = {ticks, roll, channel, note, velocity};
      Serial.print("new note #:" );
      Serial.println(note); 
      num_notes++;
}

void OnNoteOff(byte channel, byte note, byte velocity)
{
      notes[num_notes] = {ticks, roll, channel, note, 0};
      Serial.print("new note (off) #:" );
      Serial.println(note); 
      num_notes++;
}
