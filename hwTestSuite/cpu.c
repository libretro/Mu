#include <PalmOS.h>
#include <stdint.h>

#include "testSuite.h"
#include "emuFunctions.h"
#include "specs/hardwareRegisterNames.h"
#include "viewer.h"
#include "cpu.h"


static char cpuStringBuffer[100];
static Boolean interruptsEnabled = true;
static uint32_t oldImr;
static uint8_t oldScr;


void turnInterruptsOff(){
   if(interruptsEnabled){
      oldImr = readArbitraryMemory32(HW_REG_ADDR(IMR));
      oldScr = readArbitraryMemory8(HW_REG_ADDR(SCR));
      writeArbitraryMemory32(HW_REG_ADDR(IMR), 0xFFFFFFFF);
      writeArbitraryMemory8(HW_REG_ADDR(SCR), oldScr & 0xEF);
      interruptsEnabled = false;
   }
}

void turnInterruptsOn(){
   if(!interruptsEnabled){
      writeArbitraryMemory32(HW_REG_ADDR(IMR), oldImr);
      writeArbitraryMemory8(HW_REG_ADDR(SCR), oldScr);
      interruptsEnabled = true;
   }
}

uint8_t getPhysicalCpuType(){
   int32_t osVer;
   
   FtrGet(sysFtrCreator, sysFtrNumROMVersion, &osVer);
   if(isEmulator())
      return CPU_NONE;
   else if(osVer >= PalmOS50)
      return CPU_ARM;
   
   /*get dragonball CPU subtype*/
   
   return CPU_M68K;
}

uint8_t getSupportedInstructionSets(){
   if((getPhysicalCpuType() & CPU_ARM) || isEmulator()){
      /*emulated m68k on physical ARM or running both CPUs from emulator*/
      return CPU_BOTH;
   }
   return CPU_M68K;
}

const char* getCpuString(){
   uint8_t cpuType = getPhysicalCpuType() & CPU_TYPES;
   
   if(cpuType == CPU_NONE){
      StrPrintF(cpuStringBuffer, "CPU:Emulator, InstructionSets:%s", isEmulatorFeatureEnabled(FEATURE_HYBRID_CPU) ? "68k|arm" : "68k");
   }
   else if(cpuType == CPU_ARM){
      StrPrintF(cpuStringBuffer, "CPU:ARM(Type Unknown), InstructionSets:68k|arm");
   }
   else if(cpuType == CPU_M68K){
      uint8_t dragonballSubtype = getPhysicalCpuType() & CPU_M68K_TYPES;
      char* dragonballTypeName;
      
      switch(dragonballSubtype){
         case CPU_M68K_328:
            dragonballTypeName = "328";
            break;
            
         case CPU_M68K_EZ:
            dragonballTypeName = "EZ";
            break;
            
         case CPU_M68K_VZ:
            dragonballTypeName = "VZ";
            break;
            
         case CPU_M68K_SZ:
            dragonballTypeName = "SZ";
            break;
            
         default:
            dragonballTypeName = "Unknown";
            break;
      }
      
      /*add die revision here*/
      
      StrPrintF(cpuStringBuffer, "CPU:Dragonball %s, InstructionSets:68k", dragonballTypeName);
   }
   
   return cpuStringBuffer;
}
