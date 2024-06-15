//
// Created by stephanie on 6/14/24.
//

#ifndef MU_SERIAL_H
#define MU_SERIAL_H

#include "emulator.h"

/**
 * Opens and initializes the serial port.
 *
 * @param path The path of the serial port.
 * @since 2024/06/14
 */
void mu_serial_open_and_init(const char* path);

#endif // MU_SERIAL_H
