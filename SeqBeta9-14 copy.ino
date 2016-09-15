int ledPin = 13;
unsigned int i;
byte BPM = 120;
byte roll = 0;
const byte max_tracks = 4;  //for now
const unsigned int max_notes = 2000;  //for now
unsigned int num_notes = 5;  //however far we are going to run (eventually dynamic, but fixed for now)
unsigned long temp_ticks;

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
  
  for (i = 0; i < max_tracks; i = i + 1) {
     track[i] = (track_specs) {10000000,1.0,1}; 
     }
  
  for (i = 0; i < max_notes; i = i + 1) {
     notes[i] = (note_type) {0,0,255,0,0};    //channel of 255 is basically a "not used" flag
     }

  notes[0] = {2000000,0,8,44,110};  //seeding some notes
  notes[1] = {5000000,0,8,45,111};
  notes[2] = {7000000,0,8,46,112};
  notes[3] = {4000000,0,8,47,113};

  usbMIDI.setHandleNoteOff(OnNoteOff);
  usbMIDI.setHandleNoteOn(OnNoteOn);
  
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH);
  delay(400);
  digitalWrite(ledPin, LOW);
  
  Serial.begin(9600); // USB is always 12 Mbit/sec
  ticks = 0;
}

void loop() {
  // put your main code here, to run repeatedly:

usbMIDI.read();  //get and process notes
//tracktime multipliers here 

  for (i = 0; i < num_notes; i = i + 1) {
     if (ticks >= notes[i].timestamp && notes[i].played != roll && notes[i].channel != 255) {  //note is past time to play and hasn't played this roll
      usbMIDI.sendNoteOn(notes[i].note, notes[i].velocity, notes[i].channel);
      Serial.print("notes #: ");
      Serial.println(i);    
      notes[i].played = roll;
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
