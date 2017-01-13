#include <Servo.h>
#include <MIDIUSB.h>

Servo esc;

int idleSpeed=990;

int noteToPeriod[127]=
//0,    1,     2,    3,    4,   5,    6,    7,    8,    9,    10,   11
{1015 ,1015 ,1015 ,1015 ,1015 ,1015 ,1015 ,1015 ,1015 ,1015 ,1015 ,1015 ,
1015 ,1015 ,1015 ,1015 ,1015 ,1015 ,1015 ,1015 ,1015 ,1015 ,1015 ,1015 ,
1015 ,1015 ,1015 ,1015 ,1015 ,1015 ,1016 ,1017 ,1018 ,1019 ,1020 ,1021 , 
1022 ,1023 ,1025 ,1026 ,1028 ,1029 ,1031 ,1033 ,1035 ,1039 ,1042 ,1045 , //first good octave
1048 ,1052 ,1056 ,1060 ,1066 ,1070 ,1075 ,1081 ,1087 ,1094 ,1100 ,1107 ,
1115 ,1124 ,1132 ,1142 ,1150 ,1162 ,1172 ,1184 ,1198 ,1211 ,1226 ,1240 ,
1257 ,1273 ,1290 ,1313 ,1330 ,1355 ,1380 ,1404 ,1430 ,1457 ,1485 ,1500 , //last good octave
1015 ,1015 ,1015 ,1015 ,1015 ,1015 ,1015 ,1015 ,1015 ,1015 ,1015 ,1015 ,
1015 ,1015 ,1015 ,1015 ,1015 ,1015 ,1015 ,1015 ,1015 ,1015 ,1015 ,1015 ,
1015 ,1015 ,1015 ,1015 ,1015 ,1015 ,1015 ,1015 ,1015 ,1015 ,1015 ,1015 ,
1015 ,1015 ,1015 ,1015 ,1015 ,1015 ,1015};

int curNote=0;
int nextNote=0;
int curSpeed=idleSpeed;
void setup() {
  esc.attach(9);
  esc.writeMicroseconds(idleSpeed);
}

void loop() {
  midiEventPacket_t rx = MidiUSB.read(); //read midi packet
  
  if(rx.header == 0) return; // if no midi packet do nothing
  //byte chan = (rx.byte1 & 0x0f);
  byte note = rx.byte2;


//Note off
  if((rx.header == 0x08 || (rx.header == 0x09 && rx.byte3 == 0x00) || (rx.byte2 == 0xB0 && rx.byte3 == 0x00))) {
    if(nextNote==note)
      nextNote=0;
      
    if(curNote == note){
      curNote = 0;
      
      if(nextNote>0){ //play buffered note
        curNote=nextNote;
        nextNote=0;
      }
      curSpeed = noteToPeriod[curNote];
    }
    
  }else if(rx.header == 0x09) { //note on
    if(curNote>0){
      nextNote=curNote;
    }
    curNote = note; //save note
    curSpeed = noteToPeriod[curNote];
  
  } else if(rx.header==0x0B) { //control change
    if(rx.byte1 == 0xB0 && (rx.byte2==0x78 || rx.byte2==0x7B)){ //reset
      curSpeed=idleSpeed;
      curNote=0;
      nextNote=0;
    }
  } else if(rx.header==0x0E) { //pitch bend
    int pb = ((rx.byte3 & 0x7f) << 7) + (rx.byte2 & 0x7f) - 8192;
    if(pb!=0){
      int pbAdd= pb/50;
      curSpeed = min(2000,max(noteToPeriod[0],noteToPeriod[curNote]+pbAdd));
    }
  }

  //----------------------------
  esc.writeMicroseconds(curSpeed);
}

