/*  Arduino Racing Game
 *  Programm is written by Mikhail 'Mautoz' Kuznetsov
 *  Mail: mikemautos@gmail.com
 *  Youtube channel with a video about this program - https://youtube.com/@espdev
 *  
 *  I2C Display pins:
 *    SDA - A4
 *    SCL - A5
 *  Joystick pins:
 *    VRX - A0
 *    VRY - A1
 *    SW - D3
 *    
 *    This program is free software: you can redistribute it and/or modify 
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *    This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *    You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <EEPROM.h> // We will save highest scores, using EEPROM
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);
uint8_t b[8]={0x1f, 0x19, 0x15, 0x13, 0x19, 0x15, 0x13, 0x1f}; // It contains graphical model of a barrier. 
int Coords[16]; // It contains positions of the barrier objects for each column on the display (0 to 15), possible values: 0 - no barrier, 1 - barrier on the left side, 2 - barrier on the right side
int last=0; // It must prevent situations in which player has no place to go, it means last value of Coords
int last2=0;
unsigned long int points=0; // Player's points.
int i,j,c; // i,j - using in "for" construction, c - random value, defines where next barrier will spawn (0 - Nowhere on this column,1 - Left, 2 - Right)
int pos=0; // Position of the car (0 - left, 1 - right)
uint8_t car1[8]={0x18,0x1e,0x5,0x19,0x19,0x5,0x1e,0x18}; // Front side of the car model.
uint8_t car2[8]={0xc,0xf,0x14,0x13,0x13,0x14,0xf,0xc}; // Rear side of the car model.
unsigned long int e; // It contains last millis() value.
int velocity,sectors; // Difficulty defines this parametres. Velocity is a time of a frame, "sectors" defines how much space must be between 2 barriers on different sides.
int dif; // Difficulty

void printcar(){ // Prints the model of the car on the display, it's called when player turns right or left.
  if (pos==0){
    lcd.setCursor(1,0);
    lcd.write(1);
    lcd.setCursor(0,0);
    lcd.write(2);
    lcd.setCursor(0,1);
    lcd.print(" ");
    lcd.setCursor(1,1);
    lcd.print(" ");
  }
  else{
    lcd.setCursor(1,1);
    lcd.write(1);
    lcd.setCursor(0,1);
    lcd.write(2);
    lcd.setCursor(0,0);
    lcd.print(" ");
    lcd.setCursor(1,0);
    lcd.print(" ");
  }
}  

int turn(){ // It checks the position of the joystick, returns 0 if car crashed into barrier, changes car position
  if (analogRead(A1)>640){
    if (pos==0){
        if (pos==0){
          if ((Coords[0]==2)||(Coords[1]==2)){
            return 0;
          }
        }
        else{
          if ((Coords[0]==1)||(Coords[1]==1)){
            return 0;
          }
        }
        pos=1;
        printcar();  
    }      
  }  
  else if(analogRead(A1)<384){
    if (pos==1){
        if (pos==0){
          if ((Coords[0]==2)||(Coords[1]==2)){
            return 0;
          }
        }
        else{
          if ((Coords[0]==1)||(Coords[1]==1)){
            return 0;
          }
        }
        pos=0;
        printcar();
    }      
  }
  return 1;
}

void moving(){ // This procedure moves all the objects on the screen on 1 
  for (i=0;i<3;i++){
    if (pos==0){
      if (Coords[i]!=Coords[i+1]){
        Coords[i]=Coords[i+1];
        if (Coords[i]==1){
          lcd.setCursor(i,1);
          lcd.print(" ");
        }
        else if (Coords[i]==2){
          lcd.setCursor(i,1);
          lcd.write(0);
        }
        else{
          lcd.setCursor(i,1);
          lcd.print(" ");
        }
      }
        
    }
    else{
      if (Coords[i]!=Coords[i+1]){
        Coords[i]=Coords[i+1];
        if (Coords[i]==1){
          lcd.setCursor(i,0);
          lcd.write(0);
        }
        else if (Coords[i]==2){
          lcd.setCursor(i,0);
          lcd.print(" ");
        }
        else{
          lcd.setCursor(i,0);
          lcd.print(" ");
        }
      }  
    }
  }
  for (i=2;i<15;i++){
    if (Coords[i]!=Coords[i+1]){
      Coords[i]=Coords[i+1];
      if (Coords[i]==1){
        lcd.setCursor(i,0);
        lcd.write(0);
        lcd.setCursor(i,1);
        lcd.print(" ");
      }
      else if (Coords[i]==2){
        lcd.setCursor(i,1);
        lcd.write(0);
        lcd.setCursor(i,0);
        lcd.print(" ");
      }
      else{
        lcd.setCursor(i,0);
        lcd.print(" ");
        lcd.setCursor(i,1);
        lcd.print(" ");
      }
    }  
  }
}

int movingwithoutrand(){ // It's called when barrier is near to the next barrier which will be spawned on another side, so player will has no place to go, 
                         //it updates the image on the screen 2-3 times without spawning the next barrier
   if (pos+1==Coords[2]){
    return 0;
   }
   moving();
   if (Coords[14]==1){
      Coords[15]=1;
      lcd.setCursor(15,0);
      lcd.print(0);
      lcd.setCursor(15,1);
      lcd.print(" ");
   } 
   else if (Coords[14]==2){
      Coords[15]=2;
      lcd.setCursor(15,0);
      lcd.print(" ");
      lcd.setCursor(15,1);
      lcd.print(0);
   }
   if (turn()==0){
     return 0;
   }
   points++;
   e=millis();
   while(millis()-e<velocity){
     if (turn()==0){
        return 0;
     }  
   }
  for (j=0;j<sectors;j++){
      if (pos+1==Coords[2]){
        return 0;
      }
      moving();
      Coords[15]=0;
      lcd.setCursor(15,0);
      lcd.print(" ");
      lcd.setCursor(15,1);
      lcd.print(" ");
      if (turn()==0){
        return 0;
      }
      points++;
      e=millis();
      while(millis()-e<velocity){
        if (turn()==0){
          return 0;
        }  
      }
      
  } 
  return 1; 
}

void dispscore(){ // This procedure displays your score on chosen difficulty level, called by "maxscores" procedure only
  byte val1 = EEPROM.read((dif-1)*4+1);
  byte val2 = EEPROM.read((dif-1)*4+2);
  byte val3 = EEPROM.read((dif-1)*4+3);
  byte val4 = EEPROM.read((dif-1)*4+4);
  unsigned long int val = val1*pow(2,24)+val2*pow(2,16)+val3*(pow(2,8))+val4;
  switch (dif){
    case 1:
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("    TripMode >  ");
      break;
    case 2:
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("    < Easy >    ");
      break;
    case 3:
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("   < Normal >   ");
      break;    
    case 4:
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("    < Hard >    ");
      break;
    case 5:
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("< ImpossibleFast");
      break;
  }   
  lcd.setCursor(0,1);
  lcd.print(val);
}

void maxscores(){ // "difficulty" procedure calls it when user picks up "Max Score" option on the menu.
  dif=3;
  lcd.clear();
  lcd.setCursor(3,0);
  lcd.print("< Normal >");
  lcd.setCursor(0,1);
  byte val1 = EEPROM.read((dif-1)*4+1);
  byte val2 = EEPROM.read((dif-1)*4+2);
  byte val3 = EEPROM.read((dif-1)*4+3);
  byte val4 = EEPROM.read((dif-1)*4+4);
  unsigned long int val = val1*pow(2,24)+val2*pow(2,16)+val3*(pow(2,8))+val4;
  lcd.print(val);
  while (1){
    if ((analogRead(A1)>640) && (dif<5)){
      dif++;
      dispscore();
      while(analogRead(A1)>640){
        delay(5);
      }
    }
    else if ((analogRead(A1)<384) && (dif>1)){
      dif--;
      dispscore();
      while(analogRead(A1)<384){
        delay(5);
      }
    }
    else if (digitalRead(3)==0){
      while(digitalRead(3)==0){
        delay(5);
      }
      break;
    }
    
  }
  difficulty();
}

void dispdif(){ // It's called by "difficulty" procedure to set velocity and sectors parametres, also displays difficulty mode
  switch (dif){
    case 0:
      lcd.setCursor(0,1);
      lcd.print("  Max Scores >  ");
      break;
    case 1:
      lcd.setCursor(0,1);
      lcd.print("  < TripMode >  ");
      velocity=200;
      sectors=3;
      break;
    case 2:
      lcd.setCursor(0,1);
      lcd.print("    < Easy >    ");
      velocity=150;
      sectors=3;
      break;
    case 3:
      lcd.setCursor(0,1);
      lcd.print("   < Normal >   ");
      velocity=100;
      sectors=3;
      break;    
    case 4:
      lcd.setCursor(0,1);
      lcd.print("    < Hard >    ");
      velocity=150;
      sectors=2;
      break;
    case 5:
      lcd.setCursor(0,1);
      lcd.print("< ImpossibleFast");
      velocity=100;
      sectors=2;
      break;
  }
}

void difficulty(){ // It is called before race is started to let user set the difficulty
  dif=3;
  velocity=100;
  sectors=3;
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Difficulty");
  lcd.setCursor(3,1);
  lcd.print("< Normal >");
  while (1){
    if ((analogRead(A1)>640) && (dif<5)){
      dif++;
      dispdif();
      while(analogRead(A1)>640){
        delay(5);
      }
    }
    else if ((analogRead(A1)<384) && (dif>0)){
      dif--;
      dispdif();
      while(analogRead(A1)<384){
        delay(5);
      }
    }
    else if (digitalRead(3)==0){
      while(digitalRead(3)==0){
        delay(5);
      }
      if (dif>0){
        break;
      }
      else{
        maxscores();
        break;
      }
    }
    
  }
}

void setup() {
  lcd.begin();
  lcd.backlight();
  Serial.begin(9600);
  lcd.createChar(0, b);
  lcd.createChar(1,car1);
  lcd.createChar(2,car2);
  //lcd.createChar(1, c);
  
  lcd.setCursor(0,0);
  lcd.print("Racing game");
  lcd.setCursor(0,1);
  lcd.print("Press the button");
  pinMode(3,INPUT_PULLUP);
}

void loop() {
  if (digitalRead(3)==0){
    while (1){
      if (digitalRead(3)==1) break;
      delay(5);
    }
    points=0;
    difficulty();
    randomSeed(millis() % 1000);
    lcd.clear();
    for (i=0;i<15;i++){
      Coords[i]=0;
    }
    printcar();
    while(1){
      
      c=random(3);
      if (c!=0 & c!=last & last!=0 & last2<4){
        if (movingwithoutrand()==0){
          break;
        }
      }
      if (pos+1==Coords[2]){
        break;
      }
      moving();
      Coords[15]=c;
      if (Coords[15]==1){
            lcd.setCursor(15,0);
            lcd.write(0);
            lcd.setCursor(15,1);
            lcd.print(" ");
      }
      else if (Coords[15]==2){
            lcd.setCursor(15,1);
            lcd.write(0);
            lcd.setCursor(15,0);
            lcd.print(" ");
      }
      else{
            lcd.setCursor(15,0);
            lcd.print(" ");
            lcd.setCursor(15,1);
            lcd.print(" ");
      }
      if (Coords[15]!=0){
        last=Coords[15];
        last2=0;
      }
      else{
        last2++;
      }
      if (turn()==0){
        break;
      }
      points++;
      e=millis();
      while(millis()-e<velocity){
        if (turn()==0){
          break;
        }
      }
    }
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Game over");
    if (points<10000){
      lcd.setCursor(0,1);
      lcd.print("Your points:");
      lcd.setCursor(12,1);
      lcd.print(points);
    }
    else if (points<1000000){
      lcd.setCursor(0,1);
      lcd.print("Ur points:");
      lcd.setCursor(10,1);
      lcd.print(points);
    }
    else if (points<1000000000){
      lcd.setCursor(0,1);
      lcd.print("Points:");
      lcd.setCursor(7,1);
      lcd.print(points);
    }
    else{
      lcd.setCursor(0,1);
      lcd.print("Find a girlfriend");  
    }
    byte val1 = EEPROM.read((dif-1)*4+1);
    byte val2 = EEPROM.read((dif-1)*4+2);
    byte val3 = EEPROM.read((dif-1)*4+3);
    byte val4 = EEPROM.read((dif-1)*4+4);
    unsigned long int power = 2;
    unsigned long int val = val1*pow(2,24)+val2*pow(2,16)+val3*(pow(2,8))+val4;
    if (points>val){
      if (points>=1000000000){
        points=999999999;
      }
      val1=0;
      val2=0;
      val3=0;
      val4=0;
      val4=val4+bitRead(points, 0);
      for (i=1;i<8;i++){
        val4=val4+power*bitRead(points, i);
        power*=2;
      }
      val3=val3+bitRead(points, 8);
      power=2;
      for (i=9;i<16;i++){
        val3=val3+power*bitRead(points, i);
        power*=2;
      }
      power=2;
      val2=val2+bitRead(points, 16);
      for (i=17;i<24;i++){
        val2=val2+power*bitRead(points, i);
        power*=2;
      }
      power=2;
      val1=val1+bitRead(points, 24);
      for (i=25;i<29;i++){
        val1=val1+power*bitRead(points, i);
        power*=2;
      }
      EEPROM.write((dif-1)*4+1, val1);
      EEPROM.write((dif-1)*4+2, val2);
      EEPROM.write((dif-1)*4+3, val3); 
      EEPROM.write((dif-1)*4+4, val4);
    }
    while (1){
      if (digitalRead(3)==0){
        while(digitalRead(3)==0){
          delay(5);
        }
        break;
      }
    }
    lcd.setCursor(0,0);
    lcd.print("Racing game");
    lcd.setCursor(0,1);
    lcd.print("Press the button");
    
  }     
}
