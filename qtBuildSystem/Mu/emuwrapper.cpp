#include <QByteArray>
#include <QDate>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QString>
#include <QTime>
#include <QVector>

#include <chrono>
#include <new>
#include <stdint.h>
#include <string.h>
#include <string>
#include <thread>
#include <time.h>

#include "emuwrapper.h"

#if defined(BUILT_FROM_CMAKE)
#include "emulator.h"
#include "fileLauncher/launcher.h"
#else
#include "../../include/emulator.h"
#include "../../include/fileLauncher/launcher.h"
#endif

extern "C" {
#if defined(BUILT_FROM_CMAKE)
#include "flx68000.h"
#include "m68k/m68k.h"
#include "pxa260/pxa260.h"
#include "armv5te/disasm.h"
#else
#include "../../include/flx68000.h"
#include "../../include/m68k/m68k.h"
#include "../../include/pxa260/pxa260.h"
#include "../../include/armv5te/disasm.h"
#endif
}

#define MAX_LOG_ENTRYS 2000
#define MAX_LOG_ENTRY_LENGTH 200

static bool alreadyExists =
    false; // there can only be one of this class since it wrappers C code

static QVector<QString> debugStrings;
static uint64_t debugDeletedStrings;
static QVector<uint64_t> debugDuplicateCallCount;
static bool debugAbort = false;
static QString debugAbortString = "";
uint32_t frontendDebugStringSize;
char *frontendDebugString;

void frontendHandleDebugPrint() {
  QString newDebugString = frontendDebugString;

  // lock out logs to capture a specific section
  if (debugAbort)
    return;

  if (!debugAbortString.isEmpty() && newDebugString.contains(debugAbortString))
    debugAbort = true;

  // this debug handler doesnt need the \n
  if (newDebugString.back() == '\n')
    newDebugString.remove(newDebugString.length() - 1, 1);
  else
    newDebugString.append("MISSING \"\\n\"");

  if (!debugStrings.empty() && newDebugString == debugStrings.back()) {
    debugDuplicateCallCount.back()++;
  } else {
    debugStrings.push_back(newDebugString);
    debugDuplicateCallCount.push_back(1);
  }

  while (debugStrings.size() > MAX_LOG_ENTRYS) {
    debugStrings.remove(0);
    debugDuplicateCallCount.remove(0);
    debugDeletedStrings++;
  }
}

static void frontendGetCurrentTime(uint8_t *writeBack) {
  time_t rawTime;
  struct tm *timeInfo;

  time(&rawTime);
  timeInfo = localtime(&rawTime);

  writeBack[0] = timeInfo->tm_hour;
  writeBack[1] = timeInfo->tm_min;
  writeBack[2] = timeInfo->tm_sec;
}

EmuWrapper::EmuWrapper() {
  if (alreadyExists == true)
    throw std::bad_alloc();
  alreadyExists = true;

  emuInited = false;
  emuThreadJoin = false;
  emuRunning = false;
  emuPaused = false;
  emuNewFrameReady = false;

  frontendDebugString = new char[MAX_LOG_ENTRY_LENGTH];
  frontendDebugStringSize = MAX_LOG_ENTRY_LENGTH;
}

EmuWrapper::~EmuWrapper() {
  if (emuInited)
    exit();

  delete[] frontendDebugString;
  frontendDebugStringSize = 0;
  debugStrings.clear();
  debugDuplicateCallCount.clear();

  // allow creating a new emu class after the old one is closed
  alreadyExists = false;
}

void EmuWrapper::emuThreadRun() {
  while (!emuThreadJoin) {
    if (emuRunning) {
      emuPaused = false;
      if (!emuNewFrameReady) {
        palmInput = emuInput;
        emulatorRunFrame();
        emuNewFrameReady = true;
      }
    } else {
      emuPaused = true;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(5));
  }
}

void EmuWrapper::writeOutSaves() {
  if (emuRamFilePath != "") {
    QFile ramFile(emuRamFilePath);
    uint32_t emuRamSize = emulatorGetRamSize();
    uint8_t *emuRamData = new uint8_t[emuRamSize];

    emulatorSaveRam(emuRamData, emuRamSize);

    // save out RAM before exit
    if (ramFile.open(QFile::WriteOnly | QFile::Truncate)) {
      ramFile.write((const char *)emuRamData, emuRamSize);
      ramFile.close();
    }

    delete[] emuRamData;
  }
  if (emuSdCardFilePath != "") {
    uint32_t emuSdCardSize = emulatorGetSdCardSize();
    uint8_t *emuSdCardData = new uint8_t[emuSdCardSize];

    if (emulatorGetSdCardData(emuSdCardData, emuSdCardSize) == EMU_ERROR_NONE) {
      QFile sdCardFile(emuSdCardFilePath);

      // save out SD card before exit
      if (sdCardFile.open(QFile::WriteOnly | QFile::Truncate)) {
        sdCardFile.write((const char *)emuSdCardData, emuSdCardSize);
        sdCardFile.close();
      }
    }
  }
}

uint32_t EmuWrapper::init(const QString &assetPath, const QString &osVersion,
                          bool syncRtc, bool allowInvalidBehavior,
                          bool fastBoot, const QString &serialPortDev) {
  if (!emuRunning && !emuInited) {
    // start emu
    uint32_t error;
    uint8_t deviceModel;
    bool hasBootloader = true;

    if (osVersion == "Palm m500/Palm OS 4.0")
      emuOsName = "palmos40-en-m500";
    else if (osVersion == "Palm m515/Palm OS 4.1")
      emuOsName = "palmos41-en-m515";
    else if (osVersion == "Tungsten T3/Palm OS 5.2.1")
      emuOsName = "palmos52-en-t3";
    else if (osVersion == "Tungsten T3/Palm OS 5.2.1")
      emuOsName = "palmos60-en-t3";
    else
      return EMU_ERROR_INVALID_PARAMETER;

    if (osVersion.contains("Palm m500"))
      deviceModel = EMU_DEVICE_PALM_M500;
    else if (osVersion.contains("Palm m515"))
      deviceModel = EMU_DEVICE_PALM_M515;
    else if (osVersion.contains("Tungsten T3"))
      deviceModel = EMU_DEVICE_TUNGSTEN_T3;
    else
      return EMU_ERROR_INVALID_PARAMETER;

    QFile romFile(assetPath + "/" + emuOsName + ".rom");
    QFile bootloaderFile(assetPath + "/bootloader-dbvz.rom");
    QFile ramFile(assetPath + "/userdata-" + emuOsName + ".ram");
    QFile sdCardFile(assetPath + "/sd-" + emuOsName + ".img");

    if (!romFile.open(QFile::ReadOnly | QFile::ExistingOnly))
      return EMU_ERROR_INVALID_PARAMETER;

    if (deviceModel == EMU_DEVICE_TUNGSTEN_T3 ||
        !bootloaderFile.open(QFile::ReadOnly | QFile::ExistingOnly))
      hasBootloader = false;

    error = emulatorInit(
        deviceModel,
        (uint8_t *)romFile.readAll().data(), romFile.size(),
        hasBootloader ? (uint8_t *)bootloaderFile.readAll().data() : NULL,
        hasBootloader ? bootloaderFile.size() : 0, syncRtc,
        allowInvalidBehavior,
        (serialPortDev.length() == 0 ? NULL :
          serialPortDev.toLocal8Bit().data()));

    if (error == EMU_ERROR_NONE) {
      QTime now = QTime::currentTime();

      palmGetRtcFromHost = frontendGetCurrentTime;
      emulatorSetRtc(QDate::currentDate().day(), now.hour(), now.minute(),
                     now.second());

      if (ramFile.open(QFile::ReadOnly | QFile::ExistingOnly)) {
        emulatorLoadRam((uint8_t *)ramFile.readAll().data(), ramFile.size());
        ramFile.close();
      }

      if (sdCardFile.open(QFile::ReadOnly | QFile::ExistingOnly)) {
        emulatorInsertSdCard((uint8_t *)sdCardFile.readAll().data(),
                             sdCardFile.size(), NULL);
        sdCardFile.close();
      }

      emuInput = palmInput;
      emuRamFilePath = assetPath + "/userdata-" + emuOsName + ".ram";
      emuSdCardFilePath = assetPath + "/sd-" + emuOsName + ".img";
      emuSaveStatePath = assetPath + "/states-" + emuOsName + ".states";

      // make the place to store the saves
      QDir(emuSaveStatePath).mkdir(".");

      // skip the boot screen
      if (fastBoot)
        launcherBootInstantly(ramFile.exists());

      // start the thread
      emuThreadJoin = false;
      emuInited = true;
      emuRunning = true;
      emuPaused = false;
      emuNewFrameReady = false;
      emuThread = std::thread(&EmuWrapper::emuThreadRun, this);
    } else {
      return error;
    }

    romFile.close();
    bootloaderFile.close();
  }

  return EMU_ERROR_NONE;
}

void EmuWrapper::exit() {
  emuThreadJoin = true;
  emuRunning = false;
  if (emuThread.joinable())
    emuThread.join();
  if (emuInited) {
    writeOutSaves();
    emulatorDeinit();
  }
}

void EmuWrapper::pause() {
  if (emuInited) {
    emuRunning = false;
    while (!emuPaused)
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
}

void EmuWrapper::resume() {
  if (emuInited) {
    emuRunning = true;
    while (emuPaused)
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
}

void EmuWrapper::reset(bool hard) {
  if (emuInited) {
    bool wasPaused = isPaused();

    if (!wasPaused)
      pause();

    if (hard)
      emulatorHardReset();
    else
      emulatorSoftReset();

    if (!wasPaused)
      resume();
  }
}

void EmuWrapper::setCpuSpeed(double speed) {
  if (emuInited) {
    bool wasPaused = isPaused();

    if (!wasPaused)
      pause();

    emulatorSetCpuSpeed(speed);

    if (!wasPaused)
      resume();
  }
}

uint32_t EmuWrapper::bootFromFile(const QString &mainPath) {
  bool wasPaused = isPaused();
  uint32_t error = EMU_ERROR_NONE;
  QFileInfo pathInfo(mainPath);
  QFile appFile(mainPath);
  QFile ramFile(mainPath + "." + emuOsName + ".ram");
  QFile sdCardFile(mainPath + "." + emuOsName + ".sd.img");
  QString suffix = QFileInfo(mainPath).suffix().toLower();
  bool hasSaveRam;
  bool hasSaveSdCard;

  if (!wasPaused)
    pause();

  // save the current data for the last program launched, or the standard device
  // image if none where launched
  writeOutSaves();

  // its OK if these fail, the buffer will just be NULL, 0 if they do
  hasSaveRam = ramFile.open(QFile::ReadOnly | QFile::ExistingOnly);
  hasSaveSdCard = suffix != "img"
                      ? sdCardFile.open(QFile::ReadOnly | QFile::ExistingOnly)
                      : false;

  // fully clear the emu
  emulatorEjectSdCard();
  emulatorHardReset();

  if (hasSaveRam)
    emulatorLoadRam((uint8_t *)ramFile.readAll().data(), ramFile.size());
  if (hasSaveSdCard)
    emulatorInsertSdCard((uint8_t *)sdCardFile.readAll().data(),
                         sdCardFile.size(), NULL);

  // its OK if these fail
  if (hasSaveRam)
    ramFile.close();
  if (hasSaveSdCard)
    sdCardFile.close();

  launcherBootInstantly(hasSaveRam);

  if (appFile.open(QFile::ReadOnly | QFile::ExistingOnly)) {
    QByteArray fileBuffer = appFile.readAll();

    appFile.close();

    if (suffix == "img") {
      QFile infoFile(mainPath.mid(0, mainPath.length() - 3) +
                     "info"); // swap "img" for "info"
      sd_card_info_t sdInfo;

      memset(&sdInfo, 0x00, sizeof(sdInfo));

      if (infoFile.open(QFile::ReadOnly | QFile::ExistingOnly)) {
        launcherGetSdCardInfoFromInfoFile((uint8_t *)infoFile.readAll().data(),
                                          infoFile.size(), &sdInfo);
        infoFile.close();
      }

      error = emulatorInsertSdCard((uint8_t *)fileBuffer.data(),
                                   fileBuffer.size(), &sdInfo);
      if (error != EMU_ERROR_NONE)
        goto errorOccurred;
    } else {
      if (!hasSaveRam) {
        error = launcherInstallFile((uint8_t *)fileBuffer.data(),
                                    fileBuffer.size());
        if (error != EMU_ERROR_NONE)
          goto errorOccurred;
      }
      error = launcherExecute(
          launcherGetAppId((uint8_t *)fileBuffer.data(), fileBuffer.size()));
      if (error != EMU_ERROR_NONE)
        goto errorOccurred;
    }
  }

  // everything worked, set output save files
  emuRamFilePath = mainPath + "." + emuOsName + ".ram";
  emuSdCardFilePath = suffix != "img"
                          ? mainPath + "." + emuOsName + ".sd.img"
                          : ""; // dont duplicate booted SD card images
  emuSaveStatePath = mainPath + "." + emuOsName + ".states";

  // make the place to store the saves
  QDir(emuSaveStatePath).mkdir(".");

// need this goto because the emulator must be released before returning
errorOccurred:
  if (error != EMU_ERROR_NONE) {
    // try and recover from error
    emulatorEjectSdCard();
    emulatorHardReset();
  }

  if (!wasPaused)
    resume();

  return error;
}

uint32_t EmuWrapper::installApplication(const QString &path) {
  bool wasPaused = isPaused();
  uint32_t error = EMU_ERROR_INVALID_PARAMETER;
  QFile appFile(path);

  if (!wasPaused)
    pause();

  if (appFile.open(QFile::ReadOnly | QFile::ExistingOnly)) {
    error = launcherInstallFile((uint8_t *)appFile.readAll().data(),
                                appFile.size());
    appFile.close();
  }

  if (!wasPaused)
    resume();

  return error;
}

uint32_t EmuWrapper::saveState(const QString &name) {
  bool wasPaused = isPaused();
  uint32_t error = EMU_ERROR_INVALID_PARAMETER;
  QFile stateFile(emuSaveStatePath + "/" + name + ".state");

  if (!wasPaused)
    pause();

  // save here
  if (stateFile.open(QFile::WriteOnly)) {
    uint32_t stateSize = emulatorGetStateSize();
    uint8_t *stateData = new uint8_t[stateSize];

    emulatorSaveState(stateData,
                      stateSize); // no need to check for errors since the
                                  // buffer is always the right size
    stateFile.write((const char *)stateData, stateSize);
    stateFile.close();

    error = EMU_ERROR_NONE;
  }

  if (!wasPaused)
    resume();

  return error;
}

uint32_t EmuWrapper::loadState(const QString &name) {
  bool wasPaused = isPaused();
  uint32_t error = EMU_ERROR_INVALID_PARAMETER;
  QFile stateFile(emuSaveStatePath + "/" + name + ".state");

  if (!wasPaused)
    pause();

  if (stateFile.open(QFile::ReadOnly | QFile::ExistingOnly)) {
    if (emulatorLoadState((uint8_t *)stateFile.readAll().data(),
                          stateFile.size()))
      error = EMU_ERROR_NONE;
    stateFile.close();
  }

  if (!wasPaused)
    resume();

  return error;
}

void EmuWrapper::setPenValue(float x, float y, bool touched) {
  emuInput.touchscreenX = x;
  emuInput.touchscreenY = y;
  emuInput.touchscreenTouched = touched;
}

void EmuWrapper::setKeyValue(uint8_t key, bool pressed) {
  switch (key) {
  case BUTTON_UP:
    emuInput.buttonUp = pressed;
    break;

  case BUTTON_DOWN:
    emuInput.buttonDown = pressed;
    break;

  case BUTTON_LEFT:
    emuInput.buttonLeft = pressed;
    break;

  case BUTTON_RIGHT:
    emuInput.buttonRight = pressed;
    break;

  case BUTTON_CENTER:
    emuInput.buttonCenter = pressed;
    break;

  case BUTTON_CALENDAR:
    emuInput.buttonCalendar = pressed;
    break;

  case BUTTON_ADDRESS:
    emuInput.buttonAddress = pressed;
    break;

  case BUTTON_TODO:
    emuInput.buttonTodo = pressed;
    break;

  case BUTTON_NOTES:
    emuInput.buttonNotes = pressed;
    break;

  case BUTTON_VOICE_MEMO:
    emuInput.buttonVoiceMemo = pressed;
    break;

  case BUTTON_POWER:
    emuInput.buttonPower = pressed;
    break;

  default:
    break;
  }
}

QVector<QString> &EmuWrapper::debugLogEntrys() { return debugStrings; }

QVector<uint64_t> &EmuWrapper::debugDuplicateLogEntryCount() {
  return debugDuplicateCallCount;
}

uint64_t &EmuWrapper::debugDeletedLogEntryCount() {
  return debugDeletedStrings;
}

QString EmuWrapper::debugGetCpuRegisterString() {
  QString regString = "";

  if (palmEmulatingTungstenT3) {
    for (uint8_t regs = 0; regs < 16; regs++)
      regString +=
          QString::asprintf("R%d:0x%08X\n", regs, pxa260GetRegister(regs));
    regString += QString::asprintf("SP:0x%08X\n", pxa260GetRegister(13));
    regString += QString::asprintf("LR:0x%08X\n", pxa260GetRegister(14));
    regString += QString::asprintf("PC:0x%08X\n", pxa260GetPc());
    regString += QString::asprintf("CPSR:0x%08X\n", pxa260GetCpsr());
    regString += QString::asprintf("SPSR:0x%08X", pxa260GetSpsr());
  } else {
    for (uint8_t dRegs = 0; dRegs < 8; dRegs++)
      regString +=
          QString::asprintf("D%d:0x%08X\n", dRegs, flx68000GetRegister(dRegs));
    for (uint8_t aRegs = 0; aRegs < 8; aRegs++)
      regString += QString::asprintf("A%d:0x%08X\n", aRegs,
                                     flx68000GetRegister(8 + aRegs));
    regString += QString::asprintf("SP:0x%08X\n", flx68000GetRegister(15));
    regString += QString::asprintf("PC:0x%08X\n", flx68000GetPc());
    regString += QString::asprintf("SR:0x%04X", flx68000GetStatusRegister());
  }

  return regString;
}

uint64_t EmuWrapper::debugGetEmulatorMemory(uint32_t address, uint8_t size) {
  if (palmEmulatingTungstenT3)
    return pxa260ReadArbitraryMemory(address, size);
  return flx68000ReadArbitraryMemory(address, size);
}

QString EmuWrapper::debugDisassemble(uint32_t address, uint32_t opcodes) {
  QString output = "";

  if (palmEmulatingTungstenT3) {
    for (uint32_t index = 0; index < opcodes; index++) {
      address += disasm_arm_insn(address);
      output += disasmReturnBuf;
      output += '\n';
    }
  } else {
    char temp[100];

    for (uint32_t index = 0; index < opcodes; index++) {
      uint8_t opcodeSize = m68k_disassemble(temp, address, M68K_CPU_TYPE_DBVZ);
      QString opcodeHex = "";

      for (uint8_t opcodeByteIndex = 0; opcodeByteIndex < opcodeSize;
           opcodeByteIndex++)
        opcodeHex += QString::asprintf(
            "%02X", flx68000ReadArbitraryMemory(address + opcodeByteIndex, 8));

      output += QString::asprintf("0x%08X: 0x%s\t%s \n", address,
                                  opcodeHex.toStdString().c_str(), temp);

      address += opcodeSize;
    }
  }

  return output;
}
