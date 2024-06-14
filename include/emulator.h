#ifndef EMULATOR_H
#define EMULATOR_H
//this is the only header a frontend needs to include from the emulator

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>

#include "audio/blip_buf.h"
#include "m5XXBus.h"//for size macros

//DEFINE INFO!!!
//define EMU_SUPPORT_PALM_OS5 to compile in Tungsten T3 support(not reccomended for low power devices)
//define EMU_MULTITHREADED to speed up long loops
//define EMU_MANAGE_HOST_CPU_PIPELINE to optimize the CPU pipeline for the most common cases
//define EMU_NO_SAFETY to remove all safety checks
//define EMU_BIG_ENDIAN on big endian systems
//define EMU_HAVE_FILE_LAUNCHER to enable launching files from the host system
//to enable degguging define EMU_DEBUG, all options below do nothing unless EMU_DEBUG is defined
//to enable memory access logging define EMU_SANDBOX_LOG_MEMORY_ACCESSES
//to enable opcode level debugging define EMU_SANDBOX_OPCODE_LEVEL_DEBUG
//to enable flow control logging define EMU_SANDBOX_LOG_JUMPS, EMU_SANDBOX_OPCODE_LEVEL_DEBUG must also be defined for this to work
//to log all API calls define EMU_SANDBOX_LOG_APIS, EMU_SANDBOX_OPCODE_LEVEL_DEBUG must also be defined for this to work

//debug
#if defined(EMU_DEBUG)
#if defined(EMU_CUSTOM_DEBUG_LOG_HANDLER)
extern uint32_t frontendDebugStringSize;
extern char*    frontendDebugString;
void frontendHandleDebugPrint();
#define debugLog(...) (snprintf(frontendDebugString, frontendDebugStringSize, __VA_ARGS__), frontendHandleDebugPrint())
#else
#define debugLog(...) printf(__VA_ARGS__)
#endif
#else
//msvc2003 doesnt support variadic macros, so just use an empty variadic function instead, EMU_DEBUG is not supported at all on msvc2003
static void debugLog(char* str, ...){};
#endif

//config options
#define EMU_FPS 60
#define DBVZ_SYSCLK_PRECISION 2000000//the amount of cycles to run before adding SYSCLKs, higher = faster, higher values may skip timer events and lower audio accuracy

#define AUDIO_SAMPLE_RATE 48000
#define AUDIO_SAMPLES_PER_FRAME (AUDIO_SAMPLE_RATE / EMU_FPS)

#define AUDIO_SPEAKER_RANGE 0x6000//prevent hitting the top or bottom of the speaker when switching direction rapidly
#define SD_CARD_NCR_BYTES 1//how many 0xFF bytes come before the R1 response
#define SAVE_STATE_VERSION 0x00000001

//shared constants
#define SD_CARD_BLOCK_SIZE 512//all newer SDSC cards have this fixed at 512
#define SD_CARD_BLOCK_DATA_PACKET_SIZE (1 + SD_CARD_BLOCK_SIZE + 2)
#define SD_CARD_RESPONSE_FIFO_SIZE (SD_CARD_BLOCK_DATA_PACKET_SIZE * 3)

//system constants
#define DBVZ_CPU_PERCENT_WAITING 0.30//account for wait states when reading memory, tested with SysInfo.prc
#define DBVZ_AUDIO_MAX_CLOCK_RATE 235929600//smallest amount of time a second can be split into:(2.0 * (14.0 * (255 + 1.0) + 15 + 1.0)) * 32768 == 235929600, used to convert the variable timing of SYSCLK and CLK32 to a fixed location in the current frame 0<->AUDIO_END_OF_FRAME
#define DBVZ_AUDIO_END_OF_FRAME (DBVZ_AUDIO_MAX_CLOCK_RATE / EMU_FPS)
#define M5XX_CRYSTAL_FREQUENCY 32768
#define SAVE_STATE_FOR_M500 0x40000000
#if defined(EMU_SUPPORT_PALM_OS5)
#define TUNGSTEN_T3_CPU_PERCENT_WAITING 0.30//TODO: dont know ARM CPU speeds yet
#define TUNGSTEN_T3_CPU_CRYSTAL_FREQUENCY 3686400
#define TUNGSTEN_T3_CPU_PLL_FREQUENCY 368640000//TODO: dont know ARM CPU speeds yet
#define TUNGSTEN_T3_RTC_CRYSTAL_FREQUENCY 32768
#define SAVE_STATE_FOR_TUNGSTEN_T3 0x80000000
#endif

//emu errors
enum{
   EMU_ERROR_NONE = 0,
   EMU_ERROR_UNKNOWN,
   EMU_ERROR_NOT_IMPLEMENTED,
   EMU_ERROR_CALLBACKS_NOT_SET,
   EMU_ERROR_OUT_OF_MEMORY,
   EMU_ERROR_INVALID_PARAMETER,
   EMU_ERROR_RESOURCE_LOCKED
};

//port types
enum{
   EMU_PORT_NONE = 0,
   EMU_PORT_USB_CRADLE,
   EMU_PORT_SERIAL_CRADLE,
   EMU_PORT_USB_PERIPHERAL,
   EMU_PORT_SERIAL_PERIPHERAL,
   EMU_PORT_END
};

//serial codes, behaviors of a serial connection other then the raw bytes, the data bytes are stored as uint16_t's with the top 8 bits being the special codes
#define EMU_SERIAL_USE_EXTERNAL_CLOCK_SOURCE 0//check baud rate against this when a serial port is reconfigured, if its equal use the clock from the other device
#define EMU_SERIAL_PARITY_ERROR 0x100
#define EMU_SERIAL_FRAME_ERROR 0x200
#define EMU_SERIAL_BREAK (EMU_SERIAL_PARITY_ERROR | 0x00)

//emulated devices
enum{
   EMU_DEVICE_PALM_M500 = 0,
   EMU_DEVICE_PALM_M515
#if defined(EMU_SUPPORT_PALM_OS5)
   ,EMU_DEVICE_TUNGSTEN_T3
#endif
};

//types
typedef struct{
   bool     enable;
   bool     enableParity;
   bool     oddParity;
   uint8_t  stopBits;
   bool     use8BitMode;
   uint32_t baudRate;
}serial_port_properties_t;

typedef struct{
   bool  buttonUp;
   bool  buttonDown;
#if defined(EMU_SUPPORT_PALM_OS5)
   bool  buttonLeft;
   bool  buttonRight;
   bool  buttonCenter;
#endif
   
   bool  buttonCalendar;//hw button 1
   bool  buttonAddress;//hw button 2
   bool  buttonTodo;//hw button 3
   bool  buttonNotes;//hw button 4
#if defined(EMU_SUPPORT_PALM_OS5)
   bool  buttonVoiceMemo;
#endif
   
   bool  buttonPower;
   
   float touchscreenX;//0.0 = left, 1.0 = right
   float touchscreenY;//0.0 = top, 1.0 = bottom
   bool  touchscreenTouched;
}input_t;

typedef struct{
   uint8_t  csd[16];
   uint8_t  cid[16];
   uint8_t  scr[8];
   uint32_t ocr;
   bool     writeProtectSwitch;
}sd_card_info_t;

typedef struct{
   uint64_t       command;
   uint8_t        commandBitsRemaining;
   uint8_t        runningCommand;
   uint32_t       runningCommandVars[3];
   uint8_t        runningCommandPacket[SD_CARD_BLOCK_DATA_PACKET_SIZE];
   uint8_t        responseFifo[SD_CARD_RESPONSE_FIFO_SIZE];
   uint16_t       responseReadPosition;
   int8_t         responseReadPositionBit;
   uint16_t       responseWritePosition;
   bool           commandIsAcmd;
   bool           allowInvalidCrc;
   bool           chipSelect;
   bool           receivingCommand;
   bool           inIdleState;
   sd_card_info_t sdInfo;
   uint8_t*       flashChipData;
   uint32_t       flashChipSize;
}sd_card_t;

typedef struct{
   bool    greenLed;
#if defined(EMU_SUPPORT_PALM_OS5)
   bool    redLed;
#endif
   bool    lcdOn;
   uint8_t backlightLevel;
   bool    vibratorOn;
   bool    batteryCharging;
   uint8_t batteryLevel;
   uint8_t dataPort;
}misc_hw_t;

//emulator data, some are GUI interface variables, some should be left alone
#if defined(EMU_SUPPORT_PALM_OS5)
extern bool      palmEmulatingTungstenT3;//read allowed, but not advised
#endif
extern bool      palmEmulatingM500;//dont touch
extern uint8_t*  palmRom;//dont touch
extern uint8_t*  palmRam;//access allowed to read save RAM without allocating a giant buffer, but endianness must be taken into account
extern input_t   palmInput;//write allowed
extern sd_card_t palmSdCard;//access allowed to read flash chip data without allocating a giant buffer
extern misc_hw_t palmMisc;//read/write allowed
extern uint16_t* palmFramebuffer;//read allowed
extern uint16_t  palmFramebufferWidth;//read allowed
extern uint16_t  palmFramebufferHeight;//read allowed
extern int16_t*  palmAudio;//read allowed, 2 channel signed 16 bit audio
extern blip_t*   palmAudioResampler;//dont touch
extern double    palmCycleCounter;//dont touch
extern double    palmClockMultiplier;//dont touch
extern bool      palmSyncRtc;//dont touch
extern bool      palmAllowInvalidBehavior;//dont touch
extern void      (*palmIrSetPortProperties)(serial_port_properties_t* properties);//configure port I/O behavior, used for proxyed native I/R connections
extern uint32_t  (*palmIrDataSize)(void);//returns the current number of bytes in the hosts IR receive FIFO
extern uint16_t  (*palmIrDataReceive)(void);//called by the emulator to read the hosts IR receive FIFO
extern void      (*palmIrDataSend)(uint16_t data);//called by the emulator to send IR data
extern void      (*palmIrDataFlush)(void);//called by the emulator to delete all data in the hosts IR receive FIFO
extern void      (*palmSerialSetPortProperties)(serial_port_properties_t* properties);//configure port I/O behavior, used for proxyed native serial connections
extern uint32_t  (*palmSerialDataSize)(void);//returns the current number of bytes in the hosts serial receive FIFO
extern uint16_t  (*palmSerialDataReceive)(void);//called by the emulator to read the hosts serial receive FIFO
extern void      (*palmSerialDataSend)(uint16_t data);//called by the emulator to send serial data
extern void      (*palmSerialDataFlush)(void);//called by the emulator to delete all data in the hosts serial receive FIFO
extern void      (*palmGetRtcFromHost)(uint8_t* writeBack);//[0] = hours, [1] = minutes, [2] = seconds

//functions
uint32_t emulatorInit(uint8_t emulatedDevice, uint8_t* palmRomData, uint32_t palmRomSize, uint8_t* palmBootloaderData, uint32_t palmBootloaderSize, bool syncRtc, bool allowInvalidBehavior);
void emulatorDeinit(void);
void emulatorHardReset(void);
void emulatorSoftReset(void);
void emulatorSetRtc(uint16_t days, uint8_t hours, uint8_t minutes, uint8_t seconds);
void emulatorSetCpuSpeed(double speed);
uint32_t emulatorGetStateSize(void);
bool emulatorSaveState(uint8_t* data, uint32_t size);//true = success
bool emulatorLoadState(uint8_t* data, uint32_t size);//true = success
uint32_t emulatorGetRamSize(void);
bool emulatorSaveRam(uint8_t* data, uint32_t size);//true = success
bool emulatorLoadRam(uint8_t* data, uint32_t size);//true = success
uint32_t emulatorInsertSdCard(uint8_t* data, uint32_t size, sd_card_info_t* sdInfo);//use (NULL, desired size) to create a new empty SD card, pass NULL for sdInfo to use defaults
uint32_t emulatorGetSdCardSize(void);
uint32_t emulatorGetSdCardData(uint8_t* data, uint32_t size);
void emulatorEjectSdCard(void);
void emulatorRunFrame(void);
void emulatorSkipFrame(void);
   
#ifdef __cplusplus
}
#endif

#endif
