#-------------------------------------------------
#
# Project created by QtCreator 2018-04-11T12:57:49
#
#-------------------------------------------------

QT += core gui multimedia svg

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Mu
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000 # disables all the APIs deprecated before Qt 6.0.0

# OS 5 support is mandatory for the QT port
CONFIG += support_palm_os5

windows{
    RC_ICONS = windows/Mu.ico
    *msvc*{
        QMAKE_CFLAGS += -openmp
        QMAKE_CXXFLAGS += -openmp
        DEFINES += "_Pragma=__pragma" EMU_MULTITHREADED
    }
    *-g++{
        QMAKE_CFLAGS += -fopenmp
        QMAKE_CXXFLAGS += -fopenmp
        QMAKE_LFLAGS += -fopenmp
        DEFINES += EMU_MULTITHREADED EMU_MANAGE_HOST_CPU_PIPELINE
    }
    CONFIG += cpu_x86_32 # TODO:this should be auto detected in the future
}

macx{
    # QMAKE_CFLAGS += -std=c89 -D__STDBOOL_H -Dinline= -Dbool=char -Dtrue=1 -Dfalse=0 # tests C89 mode
    ICON = macos/Mu.icns
    QMAKE_INFO_PLIST = macos/Info.plist
    DEFINES += EMU_MULTITHREADED EMU_MANAGE_HOST_CPU_PIPELINE
    CONFIG += cpu_x86_64 # Mac OS is only x86_64
}

linux-g++{
    QMAKE_CFLAGS += -fopenmp
    QMAKE_CXXFLAGS += -fopenmp
    QMAKE_LFLAGS += -fopenmp
    DEFINES += EMU_MULTITHREADED EMU_MANAGE_HOST_CPU_PIPELINE
    CONFIG += cpu_x86_64 # TODO:this should be auto detected in the future
}

android{
    QMAKE_CFLAGS += -fopenmp
    QMAKE_CXXFLAGS += -fopenmp
    QMAKE_LFLAGS += -fopenmp
    DEFINES += EMU_MULTITHREADED EMU_MANAGE_HOST_CPU_PIPELINE
    CONFIG += cpu_armv7 # TODO:this should be auto detected in the future
}


CONFIG(debug, debug|release){
    # debug build, be accurate, fail hard, and add logging
    DEFINES += EMU_DEBUG EMU_CUSTOM_DEBUG_LOG_HANDLER
    macx|linux-g++{
        # also check for any buffer overflows and memory leaks
        # -fsanitize=undefined,leak
        QMAKE_CFLAGS += -fstack-protector-strong -fsanitize=address -Werror=array-bounds
        QMAKE_CXXFLAGS += -fstack-protector-strong -fsanitize=address -Werror=array-bounds
        QMAKE_LFLAGS += -fsanitize=address
    }
}else{
    # release build, go fast
    CONFIG += EMU_NO_SAFETY
}

EMU_NO_SAFETY{
    DEFINES += EMU_NO_SAFETY
}

support_palm_os5{
    DEFINES += EMU_SUPPORT_PALM_OS5 # the Qt build will not be supporting anything too slow to run OS 5
    DEFINES += SUPPORT_LINUX # forces the dynarec to use accurate mode and disable Nspire OS hacks

    EMU_NO_SAFETY{
        # Windows is only supported in 32 bit mode right now(this is a limitation of the dynarec)
        # iOS needs IS_IOS_BUILD set, but the Qt port does not support iOS currently

        cpu_x86_32{
            SOURCES += \
                ../../src/armv5te/translate_x86.c \
                ../../src/armv5te/asmcode_x86.S
        }
        else{
            # x86 has this implemented in asmcode_x86.S
            SOURCES += \
                ../../src/armv5te/asmcode.c
        }

        cpu_x86_64{
            SOURCES += \
                ../../src/armv5te/translate_x86_64.c \
                ../../src/armv5te/asmcode_x86_64.S
        }

        cpu_armv7{
            SOURCES += \
                ../../src/armv5te/translate_arm.cpp \
                ../../src/armv5te/asmcode_arm.S
        }

        cpu_armv8{
            SOURCES += \
                ../../src/armv5te/translate_aarch64.cpp \
                ../../src/armv5te/asmcode_aarch64.S
        }
    }
    else{
        # use platform independant C with no dynarec
        SOURCES += \
            ../../src/armv5te/asmcode.c \
            ../../src/armv5te/uArm/CPU_2.c \
            ../../src/armv5te/uArm/icache.c \
            ../../src/armv5te/uArm/uArmGlue.cpp
        DEFINES += NO_TRANSLATION
    }

    windows{
        SOURCES += \
            ../../src/armv5te/os/os-win32.c
    }

    macx|linux-g++|android{
        SOURCES += \
            ../../src/armv5te/os/os-linux.c
    }

    SOURCES += \
        ../../src/pxa260/pxa260_DMA.c \
        ../../src/pxa260/pxa260_DSP.c \
        ../../src/pxa260/pxa260_GPIO.c \
        ../../src/pxa260/pxa260_IC.c \
        ../../src/pxa260/pxa260_LCD.c \
        ../../src/pxa260/pxa260_PwrClk.c \
        ../../src/pxa260/pxa260_RTC.c \
        ../../src/pxa260/pxa260_TIMR.c \
        ../../src/pxa260/pxa260_UART.c \
        ../../src/pxa260/pxa260I2c.c \
        ../../src/pxa260/pxa260Memctrl.c \
        ../../src/pxa260/pxa260Timing.c \
        ../../src/pxa260/pxa260Ssp.c \
        ../../src/pxa260/pxa260Udc.c \
        ../../src/pxa260/pxa260.c \
        ../../src/armv5te/arm_interpreter.cpp \
        ../../src/armv5te/cpu.cpp \
        ../../src/armv5te/coproc.cpp \
        ../../src/armv5te/disasm.c \
        ../../src/armv5te/emuVarPool.c \
        ../../src/armv5te/thumb_interpreter.cpp \
        ../../src/armv5te/mem.c \
        ../../src/armv5te/mmu.c \
        ../../src/tps65010.c \
        ../../src/pxa260/tsc2101.c \
        ../../src/w86l488.c

    HEADERS += \
        ../../include/pxa260/pxa260_CPU.h \
        ../../include/pxa260/pxa260_DMA.h \
        ../../include/pxa260/pxa260_DSP.h \
        ../../include/pxa260/pxa260_GPIO.h \
        ../../include/pxa260/pxa260_IC.h \
        ../../include/pxa260/pxa260_LCD.h \
        ../../include/pxa260/pxa260_PwrClk.h \
        ../../include/pxa260/pxa260_RTC.h \
        ../../include/pxa260/pxa260_TIMR.h \
        ../../include/pxa260/pxa260_UART.h \
        ../../include/pxa260/pxa260I2c.h \
        ../../include/pxa260/pxa260Memctrl.h \
        ../../include/pxa260/pxa260Timing.h \
        ../../include/pxa260/pxa260Ssp.h \
        ../../include/pxa260/pxa260Udc.h \
        ../../include/pxa260/pxa260_types.h \
        ../../include/pxa260/pxa260_math64.h \
        ../../include/pxa260/pxa260Accessors.c.h \
        ../../include/pxa260/pxa260.h \
        ../../include/armv5te/os/os.h \
        ../../include/armv5te/uArm/CPU_2.h \
        ../../include/armv5te/uArm/icache.h \
        ../../include/armv5te/uArm/uArmGlue.h \
        ../../include/armv5te/asmcode.h \
        ../../include/armv5te/bitfield.h \
        ../../include/armv5te/cpu.h \
        ../../include/armv5te/disasm.h \
        ../../include/armv5te/emu.h \
        ../../include/armv5te/mem.h \
        ../../include/armv5te/translate.h \
        ../../include/armv5te/cpudefs.h \
        ../../include/armv5te/debug.h \
        ../../include/armv5te/mmu.h \
        ../../include/armv5te/armsnippets.h \
        ../../include/armv5te/literalpool.h \
        ../../include/tungstenT3Bus.h \
        ../../include/tps65010.h \
        ../../include/tsc2101.h \
        ../../include/w86l488.h
}

CONFIG += c++11

INCLUDEPATH += $$PWD/../../include

SOURCES += \
    ../../src/ads7846.c \
    ../../src/audio/blip_buf.c \
    ../../src/dbvz.c \
    ../../src/emulator.c \
    ../../src/fileLauncher/launcher.c \
    ../../src/flx68000.c \
    ../../src/m5XXBus.c \
    ../../src/m68k/m68kcpu.c \
    ../../src/m68k/m68kdasm.c \
    ../../src/m68k/m68kopac.c \
    ../../src/m68k/m68kopdm.c \
    ../../src/m68k/m68kopnz.c \
    ../../src/m68k/m68kops.c \
    ../../src/pdiUsbD12.c \
    ../../src/sdCard.c \
    ../../src/sed1376.c \
    ../../src/silkscreen.c \
    debugviewer.cpp \
    emuwrapper.cpp \
    main.cpp \
    mainwindow.cpp \
    statemanager.cpp \
    touchscreen.cpp \
    settingsmanager.cpp

HEADERS += \
    ../../include/ads7846.h \
    ../../include/audio/blip_buf.h \
    ../../include/dbvz.h \
    ../../include/dbvzRegisterAccessors.c.h \
    ../../include/dbvzRegisterNames.c.h \
    ../../include/dbvzTiming.c.h \
    ../../include/emulator.h \
    ../../include/fileLauncher/launcher.h \
    ../../include/flx68000.h \
    ../../include/m5XXBus.h \
    ../../include/m68k/m68k.h \
    ../../include/m68k/m68kconf.h \
    ../../include/m68k/m68kcpu.h \
    ../../include/m68k/m68kexternal.h \
    ../../include/m68k/m68kops.h \
    ../../include/pdiUsbD12.h \
    ../../include/pdiUsbD12CommandNames.c.h \
    ../../include/portability.h \
    ../../include/sdCard.h \
    ../../include/sdCardAccessors.c.h \
    ../../include/sdCardCommandNames.c.h \
    ../../include/sdCardCrcTables.c.h \
    ../../include/sed1376.h \
    ../../include/sed1376Accessors.c.h \
    ../../include/sed1376RegisterNames.c.h \
    ../../include/silkscreen.h \
    debugviewer.h \
    emuwrapper.h \
    mainwindow.h \
    statemanager.h \
    touchscreen.h \
    settingsmanager.h


FORMS += \
    mainwindow.ui \
    debugviewer.ui \
    statemanager.ui \
    settingsmanager.ui

CONFIG += mobility
MOBILITY = 

DISTFILES += \
    ../../src/fileLauncher/readme.md \
    android/AndroidManifest.xml \
    android/gradle/wrapper/gradle-wrapper.jar \
    android/gradlew \
    android/res/values/libs.xml \
    android/build.gradle \
    android/gradle/wrapper/gradle-wrapper.properties \
    android/gradlew.bat \
    android/res/drawable-hdpi/icon.png \
    android/res/drawable-ldpi/icon.png \
    android/res/drawable-mdpi/icon.png \
    images/boot.svg \
    images/addressBook.svg \
    images/calendar.svg \
    images/center.svg \
    images/center.svg \
    images/debugger.svg \
    images/down.svg \
    images/install.svg \
    images/left.svg \
    images/left.svg \
    images/notes.svg \
    images/pause.svg \
    images/play.svg \
    images/power.svg \
    images/right.svg \
    images/right.svg \
    images/screenshot.svg \
    images/settingsManager.svg \
    images/stateManager.svg \
    images/stop.svg \
    images/todo.svg \
    images/up.svg

RESOURCES += \
    mainwindow.qrc

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android
