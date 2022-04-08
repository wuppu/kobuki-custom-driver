// C library headers
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <signal.h>
#include <unistd.h>
#include <stdbool.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

// Linux headers
#include <fcntl.h> // Contains file controls like O_RDWR
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h> // write(), read(), close()

// User headers
#include "kobuki.h"

struct MIB g_mib;

/**
 * @brief 어플리케이션 종료 시에 호출되는 시그널 함수
 * @param[in] signum 시그널 번호
 * @details 종료 시에 반드시 close() 함수가 호출되어야 한다.
 * */
static void TerminateEvent(int signum)
{
  (void)signum;

  PrintLog(kMessageType_Info, "Application terminating\n");
	KOBUKI_ControlSpeed(g_mib.device, 0, 0);
  if (g_mib.device < 0) {
    close(g_mib.device);
  }
  PrintLog(kMessageType_Pass, "Success to terminate\n");
  exit(0);
}


/**
 * @brief input parameter 파싱
 * @param[in] argc 파라미터 개수
 * @param[in] argv 파라미터
 * @retval 0: 성공
 * @retval -1: 실패
 * */
int ParseInputParameter(int argc, char *argv[])
{
  g_mib.log_level = kMessageType_Error;
  strcpy(g_mib.script_file_name, "script.txt");
  strcpy(g_mib.server_ip_addr, "192.168.240.1");
  g_mib.server_port_num = 5555;
  strcpy(g_mib.baud_rate, "115200");
  memset(g_mib.device_name, 0x00, sizeof(g_mib.device_name));

  for (int i = 0; i < argc; i++) {
    if (strcmp(argv[i], "--help") == 0) {
      return -1;
    }
    if (strcmp(argv[i], "--ip") == 0) {
      if (i + 1 < argc) {
        strcpy(g_mib.server_ip_addr, argv[i + 1]);
      }
      else {
        PrintLog(kMessageType_Error, "Fail to parse input parameters - server_ip_addr\n");
        return -1;
      }
    }
    if (strcmp(argv[i], "--port") == 0) {
      if (i + 1 < argc) {
        g_mib.server_port_num = atoi(argv[i + 1]);
      }
      else {
        PrintLog(kMessageType_Error, "Fail to parse input parameters - server_port_num\n");
        return -1;
      }
    }
#if 0
    if (strcmp(argv[i], "--dev") == 0) {
      if (i + 1 < argc) {
        strcpy(g_mib.device_name, argv[i + 1]);
      }
      else {
        PrintLog(kMessageType_Error, "Fail to parse input parameters - device_name\n");
        return -1;
      }
    }
#endif
    if (strcmp(argv[i], "--baud") == 0) {
      if (i + 1 < argc) {
        strcpy(g_mib.baud_rate, argv[i + 1]);
      }
      else {
        PrintLog(kMessageType_Error, "Fail to parse input parameters - baud_rate\n");
        return - 1;
      }
    }

    if (strcmp(argv[i], "--script") == 0) {
      if (i + 1 < argc) {
        strcpy(g_mib.script_file_name, argv[i + 1]);
      }
      else {
        PrintLog(kMessageType_Error, "Fail to parse input parameters - script_file_name\n");
        return -1;
      }
    }

    if (strcmp(argv[i], "--dbg") == 0) {
      if (i + 1 < argc) {
        g_mib.log_level = atoi(argv[i + 1]);
      }
      else {
        PrintLog(kMessageType_Error, "Fail to parse input parameters - log_level\n");
        return -1;
      }
    }
  }

  // if (g_mib.baud_rate[0] == '\0') {
  //   PrintLog(kMessageType_Error, "Fail to parse input parameters - baud_rate\n");
  //   return - 1;
  // }
  // if (g_mib.device_name[0] == '\0') {
  //   PrintLog(kMessageType_Error, "Fail to parse input parameters - device_name\n");
  //   return -1;
  // }
  PrintLog(kMessageType_Pass, "Success to parse input parameters\n");
  PrintLog(kMessageType_Debug, "server_ip_addr: %s\n", g_mib.server_ip_addr);
  PrintLog(kMessageType_Debug, "server_port_num: %d\n", g_mib.server_port_num);
  PrintLog(kMessageType_Debug, "device_name: %s\n", g_mib.device_name);
  PrintLog(kMessageType_Debug, "baud_rate: %s\n", g_mib.baud_rate);
  PrintLog(kMessageType_Debug, "script_file_name: %s\n", g_mib.script_file_name);
  PrintLog(kMessageType_Debug, "log_level: %d\n", g_mib.log_level);
  return 0;
}


/**
 * @brief print usage
 * */
void Usage(char *app_name)
{
  printf("\n\n");
  printf(" Description: Communicate with kobuki device using serial communication\n");
  printf(" Version: %s\n", _VERSION_);
  printf(" Author: wuppu\n");
  printf(" Email: wndud1640@gmail.com\n");

  printf("\n");
  printf(" [USAGE]\n");
  printf(" %s <OPTIONS>\n", app_name);
  printf(" --ip <ip_address>         Arduino IPv4 address. If not specified, set to 192.168.240.1\n");
  printf(" --port <port_number>      Arduino UDP port number. If not specified, set to 5555\n");
  // printf(" --dev <device_name>       Device name in \"/dev/\" directory\n");
  printf(" --baud <baud_rate>        Serial port baud rate. if not specified, set to 115200\n");
  printf("     1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200 bits per seconds\n");
  printf(" --script <script_file>    Script file name. If not specified, set to ./script.txt\n");
  printf(" --dbg <dbg_level>         Print log level. If not specified, set to 1\n");
  printf("     0: None, 1: Error, 2: Event, 3: Info, 4: Debug\n");
  printf("\n\n");
}


int main(int argc, char* argv[])
{
  g_mib.log_level = kMessageType_Error;

  /* application terminate handler 등록 */
  struct sigaction sig_action;
  memset(&sig_action, 0x00, sizeof(sig_action));
  sig_action.sa_handler = TerminateEvent;
  sigemptyset(&sig_action.sa_mask);
  sig_action.sa_flags = 0;
  if (sigaction(SIGINT, &sig_action, NULL) != 0) {
    PrintLog(kMessageType_Error, "Fail to sigaction - SIGINT\n");
  }
  if (sigaction(SIGHUP, &sig_action, NULL) != 0) {
    PrintLog(kMessageType_Error, "Fail to sigaction - SIGHUP\n");
  }
  if (sigaction(SIGTERM, &sig_action, NULL) != 0) {
    PrintLog(kMessageType_Error, "Fail to sigaction - SIGTERM\n");
  }
  if (sigaction(SIGSEGV, &sig_action, NULL) != 0) {
    PrintLog(kMessageType_Error, "Fail to sigaction - SIGSEGV\n");
  }

  /* help 출력 */
  // if (argc == 1) {
  //   Usage(argv[0]);
  //   TerminateEvent(-1);
  // }

  /* input parameters 처리 */
  int ret = ParseInputParameter(argc, argv);
  if (ret < 0) {
    Usage(argv[0]);
    TerminateEvent(-1);
  }
  /* script file 처리 */

  ret = ParseScriptCommand(g_mib.script_file_name, &g_mib);
  if (ret < 0) {
    TerminateEvent(-1);
  }

  ret = InitUDP(g_mib.server_ip_addr, g_mib.server_port_num, &g_mib.server_addr, &g_mib.socket);
  if (ret < 0) {
    TerminateEvent(-1);
  }

  /* 초기 동작 LED 점등 (3초) */
	KOBUKI_ControlSpeed(g_mib.device, 0, 0);
  KOBUKI_ControlLED(g_mib.device, 1, kLEDColor_None);
  KOBUKI_ControlLED(g_mib.device, 2, kLEDColor_None);
  sleep(1);  
  KOBUKI_ControlLED(g_mib.device, 1, kLEDColor_Red);
  sleep(1);  
  KOBUKI_ControlLED(g_mib.device, 2, kLEDColor_Red);
  sleep(1);  
  KOBUKI_ControlLED(g_mib.device, 1, kLEDColor_Green);
  KOBUKI_ControlLED(g_mib.device, 2, kLEDColor_Green);
  sleep(1);

  /* script 내용 순차 처리 */
  for (int i = 0; i < g_mib.script_lines_size; i++) {
    switch (g_mib.script_lines[i].type) {
      case kCommandType_None:
        continue;
      case kCommandType_LED:
        KOBUKI_ControlLED(g_mib.device, g_mib.script_lines[i].led_num, g_mib.script_lines[i].color);
        break;
      case kCommandType_Sleep:
        usleep(g_mib.script_lines[i].delay * 1000);
        break;
      case kCommandType_Speed:
        KOBUKI_ControlSpeed(g_mib.device, g_mib.script_lines[i].speed, g_mib.script_lines[i].radius);

				usleep(g_mib.script_lines[i].move_time * 1000);

				KOBUKI_ControlSpeed(g_mib.device, 0, 0);
        break;
    }
  }

  /* script 내용 처리 - LED off */
  KOBUKI_ControlLED(g_mib.device, 1, kLEDColor_None);
  KOBUKI_ControlLED(g_mib.device, 2, kLEDColor_None);  
  KOBUKI_ControlSpeed(g_mib.device, 0, 0);

#if 0
  unsigned char buf[1000];
  memset(buf, 0x00, sizeof(buf));


  while (true) {
    int recv_len = read(g_mib.device, buf, sizeof(buf));
    if (recv_len < 0) {
      continue;
      PrintLog(kMessageType_Error, "Fail to read %s serial port\n", g_mib.device_name);
      // TerminateEvent(-1);
    }

    PrintLog(kMessageType_Debug, "recv_len: %d\n", recv_len);
    PrintLog(kMessageType_Debug, "recv: \n");
    for (int i = 0; i < recv_len; i++) {

      if (i != 0 && i % 8 == 0) {
        if (i != 0 && i % 16 == 0) {
          printf("\n");
        }
        else {
          printf(" ");
        }
      }
      if (i % 16 == 0) {
        printf("%06X ", i);
      }
      printf("%02X ", buf[i]);
    }
    printf("\n");
  }

  PrintLog(kMessageType_Pass, "Done\n");
  close(g_mib.device); //close serial port
#endif
  return 0;
}
