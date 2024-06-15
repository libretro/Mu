//
// Created by stephanie on 6/14/24.
//

#include <stdio.h>
#include <string.h>

#if defined(__LIBRETRO__)
#elif defined(_WIN32)
  #include <windows.h>
#else
  #include <errno.h>
  #include <fcntl.h>
  #include <termios.h>
  #include <unistd.h>
#endif

#include "emulator.h"

#define MU_SERIAL_DEBUG

#define MU_SERIAL_BUF_SIZE 4096

static const char* mu_serial_path = NULL;

#if defined(__LIBRETRO__)
#elif defined(_WIN32)
#else
static int mu_serial_fd = -1;
#endif

static uint8_t mu_serial_buf[MU_SERIAL_BUF_SIZE];
static int32_t mu_serial_buf_len;
static serial_port_properties_t mu_serial_properties;

void mu_serial_palmSerialSetPortProperties(serial_port_properties_t* properties)
{
#if defined(__LIBRETRO__)
#elif defined(_WIN32)
#else
  struct termios tty;
#endif

  // Always use base properties
  if (properties == NULL) {
    properties = &mu_serial_properties;
  }

  // Did the properties not actually change?
  if (0 == memcmp(properties, &mu_serial_properties,
    sizeof(mu_serial_properties))) {
    return;
  }

#if defined(MU_SERIAL_DEBUG)
  // Debug
  fprintf(stderr, "Serial PROPERTIES\n");
#endif

  // Write over new properties
  memmove(&mu_serial_properties, properties,
    sizeof(mu_serial_properties));

#if defined(__LIBRETRO__)
#elif defined(_WIN32)
#else
  if (properties->enable && mu_serial_fd < 0) {
#if defined(MU_SERIAL_DEBUG)
    // Debug
    fprintf(stderr, "Serial OPEN %s\n", mu_serial_path);
#endif

    mu_serial_fd = open(mu_serial_path,
      O_RDWR | O_EXCL | O_NONBLOCK);

    if (mu_serial_fd < 0) {
      fprintf(stderr, "Serial FAIL %s\n",
        strerror(errno));

      return;
    }

    mu_serial_buf_len = 0;
  } else if (!properties->enable && mu_serial_fd >= 0) {
#if defined(MU_SERIAL_DEBUG)
    // Debug
    fprintf(stderr, "Serial CLOSE %s\n", mu_serial_path);
#endif

    close(mu_serial_fd);

    mu_serial_fd = -1;
    mu_serial_buf_len = 0;
  }

  if (mu_serial_fd >= 0) {
    memset(&tty, 0, sizeof(tty));

    cfsetspeed(&tty, properties->baudRate);

    if (properties->enableParity) {
      tty.c_cflag |= PARENB;
    }

    if (properties->oddParity) {
      tty.c_cflag |= PARODD;
    }

    if (properties->stopBits > 1) {
      tty.c_cflag |= CSTOPB;
    } else {
      tty.c_cflag &= (~CSTOPB);
    }

    if (properties->use8BitMode) {
      tty.c_cflag |= CS8;
    } else {
      tty.c_lflag &= (~CSIZE);
      tty.c_cflag |= CS7;
    }
  }
#endif
}

uint32_t mu_serial_palmSerialDataSize(void)
{
  int32_t read_count;

  // Do not read any more data if there is any
  if (mu_serial_buf_len > 0) {
    return mu_serial_buf_len;
  }

#if defined(__LIBRETRO__)
#elif defined(_WIN32)
#else
  if (mu_serial_fd >= 0) {
    // Try reading in as much as possible
    if (mu_serial_buf_len < MU_SERIAL_BUF_SIZE) {
      read_count = read(mu_serial_fd,
        &mu_serial_buf[mu_serial_buf_len],
        MU_SERIAL_BUF_SIZE - mu_serial_buf_len);

      if (read_count > 0) {
        mu_serial_buf_len += read_count;

#if defined(MU_SERIAL_DEBUG)
        // Debug
        fprintf(stderr, "Serial FLOW %d < %d\n",
          mu_serial_buf_len, read_count);
#endif
      }
    }
  }
#endif

  return mu_serial_buf_len;
}

uint16_t mu_serial_palmSerialDataReceive(void)
{
  uint8_t data;
  uint32_t buf_len;

  // Get size of current buffer
  buf_len = mu_serial_palmSerialDataSize();

#if defined(MU_SERIAL_DEBUG)
  // Debug
  fprintf(stderr, "Serial RECV TRY %d\n", mu_serial_buf_len);
#endif

  // No data to read?
  if (buf_len == 0 || mu_serial_buf_len == 0) {
    return 0;
  }

  // Read in next byte
  data = mu_serial_buf[0];

  // Shift over
  memmove(&mu_serial_buf[0], &mu_serial_buf[1],
    mu_serial_buf_len - 1);
  mu_serial_buf_len -= 1;

#if defined(MU_SERIAL_DEBUG)
  // Debug
  fprintf(stderr, "Serial RECV %02x %c\n", data, data);
#endif

  // Give the byte we read
  return data;
}

void mu_serial_palmSerialDataSend(uint16_t data)
{
  uint8_t part;

  // Only need to send the lower bit
  part = data & 0xFF;

#if defined(__LIBRETRO__)
#elif defined(_WIN32)
#else
  if (mu_serial_fd >= 0) {
#if defined(MU_SERIAL_DEBUG)
    // Debug
    fprintf(stderr, "Serial SEND %02x %c\n", part, part);
#endif

    write(mu_serial_fd, &part, 1);
    fsync(mu_serial_fd);
  }
#endif
}

void mu_serial_palmSerialDataFlush(void)
{

#if defined(__LIBRETRO__)
#elif defined(_WIN32)
#else
  if (mu_serial_fd >= 0) {
#if defined(MU_SERIAL_DEBUG)
    // Debug
    fprintf(stderr, "Serial DROP\n");
#endif

    tcflush(mu_serial_fd, TCIFLUSH);
  }
#endif
}

void mu_serial_open_and_init(const char* path)
{
  mu_serial_path = strdup(path);

#if defined(__LIBRETRO__)
#elif defined(_WIN32)
#else
  mu_serial_fd = -1;
#endif

#if defined(MU_SERIAL_DEBUG)
  // Debug
  fprintf(stderr, "Serial PORT %s\n", mu_serial_path);
#endif

  // Set functions to use
  palmSerialSetPortProperties = mu_serial_palmSerialSetPortProperties;
  palmSerialDataSize = mu_serial_palmSerialDataSize;
  palmSerialDataReceive = mu_serial_palmSerialDataReceive;
  palmSerialDataSend = mu_serial_palmSerialDataSend;
  palmSerialDataFlush = mu_serial_palmSerialDataFlush;
}
