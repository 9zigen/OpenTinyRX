// **********************************************************
// Baychi soft 2013
// **      RFM22B/23BP/Si4432 Reciever with Expert protocol **
// **      This Source code licensed under GPL            **
// **********************************************************
// Latest Code Update : 2013-10-22
// Supported Hardware : Expert Tiny/2G RX, Orange/OpenLRS Rx boards (store.flytron.com)
// Project page       : https://github.com/baychi/OpenTinyRX
// **********************************************************

// Функции меню терминала
//

void printlnPGM(char *adr, char ln)   // печать строки из памяти программы
{
  byte b;
  while(1) {
    b=pgm_read_byte(adr++);
    if(!b) break;
    Serial.write(b);
  }

  if(ln) Serial.println();  
}

static unsigned char regs[] = {1, 2, 3, 4, 11,12,13,14,15,16,17,18,19,20,24,25,26,28,40,41,42 } ;

// Держим текст в программной памяти
//
static char help1[] PROGMEM =  "Bind N";
static char help2[] PROGMEM =  "Freq Corr";
static char help3[] PROGMEM =  "Servo 150% strech num (1-12)";
static char help4[] PROGMEM =  "Statistics enable";
static char help5[] PROGMEM =  "Hope F1";
static char help6[] PROGMEM =  "Hope F2";
static char help7[] PROGMEM =  "Hope F3";
static char help8[] PROGMEM =  "Hope F4";
static char help9[] PROGMEM =  "Hope F5";
static char help10[] PROGMEM =  "Hope F6";
static char help11[] PROGMEM =  "Hope F7";
static char help12[] PROGMEM =  "Hope F8";
static char help13[] PROGMEM =  "Beacon F (FF=disable)";
static char help14[] PROGMEM =  "Beacon Pmax (mWt): 0-1.2; 1-2; 2-3; 3-6; 4-12; 5-25; 6-50; 7-100";  
static char help15[] PROGMEM =  "Beacon start time (sec)";
static char help16[] PROGMEM =  "SAW Fmin";
static char help17[] PROGMEM =  "SAW Fmax";
static char help18[] PROGMEM =  "PPM mode 1st PWM chnl (1-8) [4]"; 
static char help19[] PROGMEM =  "RSSI type: sound(0)/level(1-99=average)";
static char help20[] PROGMEM =  "RSSI mode: level(0)/SN ratio(1)";
static char help21[] PROGMEM =  "RSSI over PWM(chan:1-12) 0-not use";
static char *menuAdr[] = {      // массив адресов строк 
   help1, help2, help3, help4, help5, help6, help7, help8, help9, help10, 
   help11, help12, help13, help14, help15, help16, help17, help18, help19, help20, 
   help21
};  


byte checkMenu(void)   // проверка на вход в меню
{
   int in; 
   
   if (Serial.available() > 0) {
      in= Serial.read();             // все, что пришло, отображаем
      if(in == 'm' || in == 'M') return 1; // есть вход в меню
   } 
   return 0;                        // не дождались 
}


void getStr(char str[])             // получение строки, завершающейся Enter от пользователя
{
  int in,sn=0;
  str[0]=0;
  while(1) {
    if (Serial.available() > 0) {
      if(Serial.peek() == SAT_PACK_HEADER) {  // если обнаружили заголовок от саттелита
        str[0]='q'; str[1]=0;      // иммитируем Quit
        menuFlag=0;              // запрещаем меню
        return;
      }
       in=Serial.read();             // все, что пришло, отображаем
       if(in > 0) {
          Serial.write(in);
          if(in == 0xd || in == 0xa) {
            Serial.println();
            return;                     // нажали Enter
          }
          if(in == 8) {                 // backspace, удаляем последний символ
            if(sn) sn--;
            continue;
          } 
          str[sn]=in; str[sn+1]=0;
          if(sn < 10) sn++;             // не более 10 символов
        }
     } else delay(1);
      wdt_reset();               //  поддержка сторожевого таймера
   }
}

#define R_AVR 199               // усреднение RSSI

byte margin(byte v)
{
   if(v < 10) return 0; 
   else if(v>71) return 61;

   return  v-10;
}

char ntxt1[] PROGMEM = "FHn: Min Avr Max";

void showNoise(char str[])             // отображаем уровень шумов по каналам
{
  byte fBeg=0, fMax=254;
  byte rMin, rMax;
  word rAvr;
  byte i,j,k;

  rAvr=atoi(str+1);           // считаем параметры, если есть в виде Nbeg-end
  if(rAvr > 0 && rAvr < 255) {
     fBeg=rAvr;
     for(i=2; i<10; i++) {
      if(str[i] == 0) break;
      if(str[i] == '-') {
        rAvr=atoi(str+i+1);
        if(rAvr > fBeg && rAvr < 255) fMax=rAvr;
        break;
      }
    }
  }
  
  RF22B_init_parameter();      // подготовим RFMку 
  to_rx_mode(); 
  SAW_FILT_OFF                 
 
  printlnPGM(ntxt1);
  
  for(i=fBeg; i<=fMax; i++) {    // цикл по каналам
     _spi_write(0x79, i);       // ставим канал
     delayMicroseconds(749);
     rMin=255; rMax=0; rAvr=0;
     for(j=0; j<R_AVR; j++) {   // по каждому каналу 
       delayMicroseconds(99);
       k=_spi_read(0x26);         // Read the RSSI value
       rAvr+=k;
       if(k<rMin) rMin=k;         // min/max calc
       if(k>rMax) rMax=k;
     }
     if(i < 10) Serial.print("  ");
     else if(i <100) Serial.write(' ');
     Serial.print(i);
     k=':';
     for(j=0; j<HOPE_NUM; j++) {   // отметим свои частоты
        if(hop_list[j] == i) {
          k='#';
        }
     }
     Serial.write(k); Serial.write(' ');
     print3(rMin);   
     k=rAvr/R_AVR;  print3(k);
     print3(rMax);

     if(str[0] == 'N') {         // если надо, печатаем псевдографику 
       rMin=margin(rMin); 
       rMax=margin(rMax); 
       k=margin(k); 

       for(j=0; j<=rMax; j++) {                         // нарисуем псевдографик
         if(j == k) Serial.write('*');
         else if(j == rMin) Serial.write('<');
         else if(j == rMax) Serial.write('>');
         else if(j>rMin && j <rMax) Serial.write('.');
         else Serial.write(' ');
       }
     }
     
     Serial.println();
     wdt_reset();               //  поддержка сторожевого таймера
  }
}

// Перенесем текст меню в память программ
char mtxt1[] PROGMEM = "To Enter MENU Press ENTER";
char mtxt2[] PROGMEM = "Type Reg and press ENTER, type Value and press ENTER (q=Quit; ss/sl/sa=Stat)";
char mtxt3[] PROGMEM = "Rg=Val \tComments -----------------------";

void showRegs(void)         // показать значения регистров
{
  unsigned char i,j=0,k;
  
  printlnPGM(mtxt3);
  for(int i=1; i<=REGS_NUM; i++) {
    if(regs[j] == i) {
      Serial.print(i);
      Serial.write('=');
      Serial.print(read_eeprom_uchar(i));
      Serial.write('\t');
      printlnPGM(menuAdr[j]);   // читаем строки из программной памяти
      j++;
    }
  }
}


void doMenu()                       // работаем с меню
{
  char str[12];
  int reg,val;

  printlnPGM(mtxt1);
  getStr(str);
  if(str[0] == 'q' || str[0] == 'Q') return;     // Q - то quit
  
  while(1) {
    showRegs();
    printlnPGM(mtxt2);

rep:  
    getStr(str);
    if(str[0] == 'n' || str[0] == 'N') {  // отсканировать и отобразить уровень шума 
       showNoise(str);
       goto rep;
    }
    
    if(str[0] == 's' || str[0] == 'S') {
      if(str[1] == 'e') statErase();
      else statShow(str[1]);  // Печать статистики
      goto rep;
    }
    
    if(str[0] == 'q' || str[0] == 'Q') return;     // Q - то quit
    reg=atoi(str);
    if(reg<0 || reg>REGS_NUM) continue; 

    getStr(str);
    if(str[0] == 'q' || str[0] == 'Q') return;     // Q - то quit
    val=atoi(str);
    if(val<0 || val>255) continue; 
    if(reg == 0 && val ==0) continue;              // избегаем потери s/n

    Serial.print(reg); Serial.write('=');   Serial.println(val);  // Отобразим полученное
    
     write_eeprom_uchar(reg,val);  // пишем регистр
     read_eeprom();                // читаем из EEPROM    
     write_eeprom();               // и тут-же пишем, что-бы сформировать КС 
  }    
}  

char htxt1[] PROGMEM = "\r\nBaychi soft 2013";
char htxt3[] PROGMEM = "RX Open Expert V2 F";
void printHeader(void)
{
  printlnPGM(htxt1);
  printlnPGM(htxt3,0); Serial.println(version[0]);
}  

