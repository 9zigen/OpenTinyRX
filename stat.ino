
struct saveStatRec {
   unsigned char flightNum;         // номер полета (инкрементируется при каждом включении)
   unsigned char FS;                // количество провалов связи и FS в момент сохранения (старший бит)
   unsigned char lost[HOPE_NUM];    // количество потерянных пакетов за интервал (в миинуту макс 238) FF, FE - все - нет потерь (что-бы не насиловать FLASH)
   unsigned char rssi[HOPE_NUM];    // средний уровень RSSI за интервал
   unsigned char noise[HOPE_NUM];   // средний уровень шума за интервал
} saveStat;                 // текущая статистика и сохраняемая во FLASH

unsigned char statByte,flightCntr=0;        // номер полета
unsigned long statTime;                     // время последней записи статистики
unsigned int  statAdr=STAT_EPROM_ADR;      // адрес в EEPROM

void statSave(void)    // записать очередную запись во FLASH 
{
  unsigned char i; 
  saveStat.flightNum=flightCntr;
  saveStat.FS=curStat.FS;
  if(failsafe_mode || (PWM_enable == 0)) saveStat.FS |= 0x80;       // признак, что находимся в состоянии FS
  curStat.FS=0;
  
  if(statCntr == 0) statCntr=1;  // что-бы не делить на ноль
  for(i=0; i<HOPE_NUM; i++) {    // подготовим новую статистику и сохраненние старой
    saveStat.lost[i]=curStat.lost[i];
    saveStat.rssi[i]=curStat.rssi[i]/statCntr;
    saveStat.noise[i]=curStat.noise[i]/statCntr;

    curStat.lost[i]=0;
    curStat.rssi[i]=0;
    curStat.noise[i]=0;
//    if(saveStat.lost[i] == 0) saveStat[i]=0xff;      // что-бы не писать много 0-й во FLASH  
  } 
  statByte=0;                                       // запускаем фоновую запись
  statCntr=0;
}

void statLoop(void)                                 // фоновой цикл записи статистики (максиммум 3 байта за раз
{
  unsigned char *ptr=(unsigned char *)&saveStat;
  
  if(Regs4[3] == 0) return;                             // if disabled
  
  if(statByte < sizeof(saveStat)) {                 
     EEPROM.write(statAdr+statByte,ptr[statByte]);  // пишем байт
     if(++statByte == sizeof(saveStat)) {           // если закончили, сформируем указатель на след. запись
        statAdr+=sizeof(saveStat);                  
        if(statAdr >= LAST_EEPROM_ADR) statAdr=STAT_EPROM_ADR;
     }
  } else if(statByte == sizeof(saveStat)) {
        EEPROM.write(STAT_PTR_ADR+1,statAdr/256);    // сначала старший бай, так как он важнее
        statByte++;
  } else if(statByte == sizeof(saveStat)+1) {
        EEPROM.write(STAT_PTR_ADR,statAdr&0xff);
        statByte++;
//      if(!satFlag) Serial.print("SS end "); Serial.println(statAdr);
  } else if(statByte == sizeof(saveStat)+2){
       statByte++;
       if(statMin == 0)  EEPROM.write(STAT_FLIGHT_ADR,flightCntr);  // номер полета пишем после первой заиси, что-бы не суетится
       statMin++;
  }

  if((time-statTime)/1000 >= STAT_INTERVAL) {      // интервал истек 
      statTime=time;
      statSave();                                  // запустим сохранение очередной записи
//    if(!satFlag)  Serial.print("SS beg "); Serial.println(flightCntr);
  }
}  

void statInit(void)                            // инициализация статистики в начале работы
{
   unsigned char i;
   statAdr=EEPROM.read(STAT_PTR_ADR);          // читаем указатель на очередную запись
   statAdr += EEPROM.read(STAT_PTR_ADR+1)*256; 
   if(statAdr < STAT_EPROM_ADR || statAdr >=LAST_EEPROM_ADR) { // некорректный указатель
      statAdr=STAT_EPROM_ADR;  
      EEPROM.write(STAT_PTR_ADR,statAdr&0xff);
      EEPROM.write(STAT_PTR_ADR+1,statAdr/256);
      flightCntr=0;      
   } else {
     i=(statAdr-STAT_EPROM_ADR + sizeof(saveStat))%sizeof(saveStat);   // защищаемся от непопадания на границу записи
     if(i != 0)  statAdr-=i;
     flightCntr=EEPROM.read(STAT_FLIGHT_ADR);    // читаем номер полета
   }
   flightCntr++;                                 // ставим новый полет 
   if(flightCntr >= 100) flightCntr=0;           // меняется в интервале 1-100
//   EEPROM.write(STAT_FLIGHT_ADR,flightCntr);

  for(i=0; i<HOPE_NUM; i++) {    // подготовим новую статистику
    curStat.lost[i]=0;
    curStat.rssi[i]=0;
    curStat.noise[i]=0;
  }
  curStat.FS=0;
  statByte=sizeof(saveStat);      // не будем писать, пока не накопим первую запись
  statCntr=statMin=0;
  statTime=millis();
}

void print3(unsigned char val)  // печать 3-цифр с выравниваем пробелами
{
  if(val < 10) Serial.print("  ");
  else if(val <100) Serial.print(" ");
  Serial.print(val);
  Serial.print(" ");
}  

void statShow(unsigned char mode)                  // вывести статистику на экран
{
   int i,j,n=0;
   unsigned char tmp,fn=0;
   unsigned char fs= (mode == 's') || (mode ==  'S');   // признак вывода только статистики пакетов
   unsigned char fl= (mode == 'l') || (mode ==  'L');   // признак вывода только уровней
   
   if(!fs && !fl) fs=fl=1;                              // полный вывод
   Serial.print("Last statisics: "); Serial.println(statAdr);
   Serial.print("FN  cnt ");
   if(fs) Serial.print("FSn InFS  Drops:1   2   3   4   5   6   7   8  ");
   if(fl) Serial.println("RSSI:1   2   3   4   5   6   7   8 Noise:1   2   3   4   5   6   7   8");
   else Serial.println("");

   i=statAdr;
   while(1) {
     j=EEPROM.read(i++);               // номер полета
     if(j > 100) {                     //  стертая запись или бракованная
       i+=(sizeof(saveStat)-1);        // пропускаем стертые записи
     } else {
       if(j != fn) { n=0; fn=j; }         // счетчик записей внутри полета
       print3(fn);                        // печатаем номер полета
       print3(++n);                       // печатаем счетчик записей
    
       if(fs) {            
         tmp=EEPROM.read(i++);              // кол-во FS
         print3(tmp&0x7f);                  // перчатаем FS
         Serial.print(" ");                 // выравнивание  
         print3((tmp&0x80) == 0x80);        // печатаем признак FS в конце записи
     
         Serial.print("     ");              // выравнивание  
         for(j=0; j<HOPE_NUM; j++) print3(EEPROM.read(i++));  // печатаем потери пакетов
       } else i+=9;

       if(fl) {
         Serial.print("   ");              // выравнивание  
         if(fs) Serial.print(" ");         // маленькая хитрость, чтобы вписаться в 80 символов
         for(j=0; j<HOPE_NUM; j++) print3(EEPROM.read(i++));  // печатаем RSSI

         Serial.print("    ");              // выравнивание  
         for(j=0; j<HOPE_NUM; j++) print3(EEPROM.read(i++));  // печатаем шум
       } else i+=16;
       
       Serial.println("");
     }
     if(i >= LAST_EEPROM_ADR) i=STAT_EPROM_ADR;
     if(i == statAdr) break;           // круг замкнулся, значит мы достигли конца
   }   
}  

void statErase(void)                  // стирание статистики
{
  Serial.print("Statistics in EEPROM erase ");
  for(int i=STAT_EPROM_ADR; i<LAST_EEPROM_ADR; i++) {
          EEPROM.write(i,0xff);
          if((i&0x7f) == 0) Serial.print(".");
  }
  statAdr=STAT_EPROM_ADR;
  flightCntr=0;
  EEPROM.write(STAT_PTR_ADR,statAdr&0xff);
  EEPROM.write(STAT_PTR_ADR+1,statAdr/256);
  EEPROM.write(STAT_FLIGHT_ADR,flightCntr);
  
  statInit();
  Serial.println(" done");
}  
