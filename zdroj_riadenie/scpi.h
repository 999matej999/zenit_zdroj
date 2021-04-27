#ifndef __SCPI_H__
#define __SCPI_H__

#define DEVICE_NAME "Zdroj ZENIT"
#define SCPI_VERSION "1999.0"

enum class RECEIVER { NONE, UART, LAN };

void cmd_arrived(RECEIVER r);

#endif // __SCPI_H__ //
