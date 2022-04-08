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

// Linux headers
#include <fcntl.h> // Contains file controls like O_RDWR
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h> // write(), read(), close()

// User headers
#include "kobuki.h"

/**
 * @brief hex dump 출력
 * @param[in] msg_type 출력 메시지의 타입
 * @param[in] command_type 커맨드의 타입
 * @param[in] format 출력 메시지 prefix
 * @param[in] command 커맨드의 구조체 포인터
 * */
void PrintHexDump(MessageType msg_type, CommandType command_type, const char *format, void *command)
{
  struct LEDMessageFormat *led_msg;
  struct SpeedMessageFormat *speed_msg;

  printf(">> ");
  switch (msg_type) {
    case kMessageType_Error:
      printf("\x1b[1;31m");
      break;
    case kMessageType_Pass:
      printf("\x1b[1;32m");
      break;
    case kMessageType_Debug:
      printf("\x1b[1;35m");
      break;
    case kMessageType_Info:
      printf("\x1b[0m");
      break;
  }

  unsigned char *ptr;
  printf("%s: ", format);
  switch (command_type) {
    case kCommandType_LED:
      led_msg = (struct LEDMessageFormat *)command;
      ptr = (unsigned char *)led_msg;
      for (size_t i = 0; i < sizeof(struct LEDMessageFormat); i++) {
        printf("%02X ", *(ptr + i));
      }
      break;
    case kCommandType_Speed:
      speed_msg = (struct SpeedMessageFormat *)command;
      ptr = (unsigned char *)speed_msg;
      for (size_t i = 0; i < sizeof(struct SpeedMessageFormat); i++) {
        printf("%02X ", *(ptr + i));
      }
      break;
    default:
      break;
  }
  printf("\n\x1b[0m");
}

/**
 * @brief 로그 출력
 * @param[in] msg_type 출력 메시지의 타입
 * @param[in] format 출력 메시지
 * @param[in] ...
 * */
void PrintLog(MessageType msg_type, const char *format, ...)
{
  va_list arg;

  if (g_mib.log_level < msg_type) {
    return;
  } 

  printf(">> ");
  switch (msg_type) {
    case kMessageType_Error:
      printf("\x1b[1;31m");
      break;
    case kMessageType_Pass:
      printf("\x1b[1;32m");
      break;
    case kMessageType_Debug:
      printf("\x1b[1;35m");
      break;
    case kMessageType_Info:
      printf("\x1b[0m");
      break;
  }
  va_start(arg, format);
  vprintf(format, arg);
  va_end(arg);
  printf("\x1b[0m");
}

/**
 * @brief KOBUKI의 LED를 조작한다.
 * @param[in] device tty
 * @param[in] led_num LED 숫자
 * @param[in] color 0: all off, 1: green, 2: red
 * @retval 0: 성공
 * @retval 음수: 실패
 * */
int KOBUKI_ControlLED(int device, int led_num, int color)
{

  switch (color) {
    case 0: PrintLog(kMessageType_Info, "Start to write led control message - led_num: %d, color: %s\n", led_num, "None"); break;
    case 1: PrintLog(kMessageType_Info, "Start to write led control message - led_num: %d, color: %s\n", led_num, "Green"); break;
    case 2: PrintLog(kMessageType_Info, "Start to write led control message - led_num: %d, color: %s\n", led_num, "Red"); break;
  }

  struct LEDMessageFormat msg;
  msg.header_0 = HEADER_0;
  msg.header_1 = HEADER_1;
  msg.payload_len = PAYLOAD_LED_LEN;
  msg.sub_payload_id = LED_CONTROL_ID;
  msg.sub_payload_len = LED_CONTROL_LEN;

  if (led_num == 1) {
    if (color == kLEDColor_Green) {
      g_mib.led_status = g_mib.led_status | 1 << 9;
      msg.led = g_mib.led_status;
    }
    else if (color == kLEDColor_Red) {
      g_mib.led_status = g_mib.led_status | 1 << 8;
      msg.led = g_mib.led_status;
    }
    else if (color == kLEDColor_None) {
      g_mib.led_status = g_mib.led_status & 0 << 8;
      g_mib.led_status = g_mib.led_status & 0 << 9;
      msg.led = g_mib.led_status;
    }
    else {
      PrintLog(kMessageType_Error, "Fila to write led control message - not support the color: %d\n", color);
      return -1;
    }
  }
  else if (led_num == 2) {
    if (color == kLEDColor_Green) {
      g_mib.led_status = g_mib.led_status | 1 << 11;
      msg.led = g_mib.led_status;
    }
    else if (color == kLEDColor_Red) {
      g_mib.led_status = g_mib.led_status | 1 << 10;
      msg.led = g_mib.led_status;
    }
    else if (color == kLEDColor_None) {
      g_mib.led_status = g_mib.led_status & 0 << 10;
      g_mib.led_status = g_mib.led_status & 0 << 11;
      msg.led = g_mib.led_status;
    }
    else {
      PrintLog(kMessageType_Error, "Fila to write led control message - not support the color: %d\n", color);
      return -1;
    }
  }
  else {
    PrintLog(kMessageType_Error, "Fail to write led control message - not support the led_num: %d\n", led_num);
    return -1;
  }

  // crc 때문에 + 1
  int ret = SendUDPMessage(g_mib.socket, g_mib.server_addr, (char *)&msg, sizeof(struct LEDMessageFormat) + 1);
  if (ret < 0) {
    PrintLog(kMessageType_Error, "Fail to send led control message - ret: %d\n", ret);
    return -1;
  }
  else {
    PrintLog(kMessageType_Pass, "Success to send led control message\n");
  }
#if 0
  int ret = write(device, &msg, sizeof(struct LEDMessageFormat) + 1);
  if (ret < 0) {
    PrintLog(kMessageType_Error, "Fail to write led control message - ret: %d\n", ret);
    return -1;
  }
  else {
    PrintLog(kMessageType_Pass, "Success to write led control message\n");
  }
#endif

  PrintHexDump(kMessageType_Debug, kCommandType_LED, "led_msg", &msg);
  return 0;
}


/**
 * @brief KOBUKI의 speed를 조작한다.
 * @param[in] device tty
 * @param[in] speed 속도 mm/s 단위
 * @param[in] radius mm 단위
 * @retval 0: 성공
 * @retval 음수: 실패
 * */
int KOBUKI_ControlSpeed(int device, int speed, int radius)
{
  PrintLog(kMessageType_Info, "Start to write speed control message - speed: %d, radius: %d\n", speed, radius);

  struct SpeedMessageFormat msg;
  msg.header_0 = HEADER_0;
  msg.header_1 = HEADER_1;
  msg.payload_len = PAYLOAD_SPEED_LEN;
  msg.sub_payload_id = BASE_CONTROL_ID;
  msg.sub_payload_len = BASE_CONTROL_LEN;
  msg.speed = speed;
  msg.radius = radius;
  
  // crc 때문에 + 1
  int ret = SendUDPMessage(g_mib.socket, g_mib.server_addr, (char *)&msg, sizeof(struct SpeedMessageFormat) + 1);
  if (ret < 0) {
    PrintLog(kMessageType_Error, "Fail to send speed control message - ret: %d\n", ret);
    return -1;
  }
  else {
    PrintLog(kMessageType_Pass, "Success to send speed control message\n");
  }
#if 0
  int ret = write(device, &msg, sizeof(struct SpeedMessageFormat) + 1);
  if (ret < 0) {
    PrintLog(kMessageType_Error, "Fail to write speed control message - ret: %d\n", ret);
    return -1;
  }
  else {
    PrintLog(kMessageType_Pass, "Success to write speed control message\n");
  }
#endif
  PrintHexDump(kMessageType_Debug, kCommandType_Speed, "speed", &msg);

  return 0;
}

/**
 * @brief 스크립트 파일을 읽어서 저장한다.
 * @param[in] script_file 스크립트 파일 이름(경로)
 * @param[out] command_buf 저장할 커맨드 버퍼
 * @retval 0: 성공
 * @retval -1: 실패
 * */
int ParseScriptCommand(char *script_file, struct MIB *mib)
{
  PrintLog(kMessageType_Info, "Start to parse script file\n");
  char buf[1000];
  int line = 0;
  int file_line = 0;

  g_mib.script_lines_size = 0;

  FILE *fp = fopen(script_file, "r");
  if (fp == NULL) {
    PrintLog(kMessageType_Error, "Fail to open script file\n");
    return -1;
  }

  while (!feof(fp)) {
    file_line++;
    fgets(buf, sizeof(buf), fp);

    /* 파일 끝 확인 후, 종료 */
    if (feof(fp)) {
      break;
    }

    /* 개행문자 삭제 */
    if (strstr(buf, "\r") != NULL) { *strstr(buf, "\r") = '\0'; }
    if (strstr(buf, "\n") != NULL) { *strstr(buf, "\n") = '\0'; }
    
    /* 공백 줄 예외처리 */
    if (buf[0] == '\0') {
      continue;
    }

    /* 초기화 */
    g_mib.script_lines[line].type = kCommandType_None;

    /* 문자열 분리 */
    char *ptr = strtok(buf, " ");

    /* 주석 문자열 예외처리 */
    if (strcmp(ptr, "#") == 0) {
      continue;
    }
    if (ptr == NULL) {
      continue;
    }
    
    /* 속도(이동) 처리 */
    if (strcmp(buf, "speed") == 0) {
      mib->script_lines[line].type = kCommandType_Speed;

      /**
       * speed
       * input: km/s 단위
       * output: mm/s 단위 
       */
      ptr = strtok(NULL, " ");
      if (ptr == NULL) {
        PrintLog(kMessageType_Error, "Fail to load script command line - line: %d\n", file_line);
        fclose(fp);
        return -1;
      }
      mib->script_lines[line].speed = (int)((atof(ptr) * 1000000) / 3600);

      /**
       * radius
       * input: m 단위
       * output: mm 단위
       */
      ptr = strtok(NULL, " ");
      if (ptr == NULL) {
        PrintLog(kMessageType_Error, "Fail to load script command line - line: %d\n", file_line);
        fclose(fp);
        return -1;
      }
      mib->script_lines[line].radius = (int)(atof(ptr) * 1000);

      /**
       * distance
       * input: m 단위
       * output: mm 단위
       */
      ptr = strtok(NULL, " ");
      if (ptr == NULL) {
        PrintLog(kMessageType_Error, "Fail to load script command line - line: %d\n", file_line);
        fclose(fp);
        return -1;
      }
      mib->script_lines[line].distance = (int)(atof(ptr) * 1000);

      // move_time 이동 시간 ms 단위
      float move_time = ((float)mib->script_lines[line].distance / (float)mib->script_lines[line].speed);
      move_time = (int)(move_time * 1000);
      mib->script_lines[line].move_time = move_time;
    }

    /* 딜레이 처리 */
    else if (strcmp(buf, "sleep") == 0) {
      mib->script_lines[line].type = kCommandType_Sleep;

      /* 딜레이 시간 ms 단위 */
      ptr = strtok(NULL, " ");
      if (ptr == NULL) {
        PrintLog(kMessageType_Error, "Fail to load script command line - line: %d\n", file_line);
        fclose(fp);
        return -1;
      }
      mib->script_lines[line].delay = atoi(ptr);
    }

    /* LED 처리 */
    else if (strcmp(buf, "led") == 0) {
      mib->script_lines[line].type = kCommandType_LED;

      /* LED 번호(0, 1) */
      ptr = strtok(NULL, " ");
      if (ptr == NULL) {
        PrintLog(kMessageType_Error, "Fail to load script command line - line: %d\n", file_line);
        fclose(fp);
        return -1;
      }
      mib->script_lines[line].led_num = atoi(ptr);

      /* LED 색상(0: off, 1: green, 2: red) */
      ptr = strtok(NULL, " ");
      if (ptr == NULL) {
        PrintLog(kMessageType_Error, "Fail to load script command line - line: %d\n", file_line);
        fclose(fp);
        return -1;
      }
      mib->script_lines[line].color = atoi(ptr);
    }
    else {
      continue;
    } 
    line++;
  }
  fclose(fp);

  mib->script_lines_size = line;
  PrintLog(kMessageType_Pass, "Success to parse script file - script_lines_size: %d\n", mib->script_lines_size);
  for (int i = 0; i < mib->script_lines_size; i++) {
    switch (mib->script_lines[i].type) {
      case kCommandType_None: 
        PrintLog(kMessageType_Debug, "#%d: None\n", i); 
        break;
      case kCommandType_Speed: 
        PrintLog(kMessageType_Debug, "#%d: Speed - speed: %dmm/s, radius: %dmm, distance: %dmm, move_time: %dms\n", 
                i, mib->script_lines[i].speed, mib->script_lines[i].radius, mib->script_lines[i].distance, mib->script_lines[i].move_time); 
        break;
      case kCommandType_Sleep: 
        PrintLog(kMessageType_Debug, "#%d: Sleep - time: %dms\n", i, mib->script_lines[i].delay); 
        break;
      case kCommandType_LED: 
        PrintLog(kMessageType_Debug, "#%d: LED - led_num: %d, led_color: %d\n", 
                i, mib->script_lines[i].led_num, mib->script_lines[i].color); 
        break;
    }
  }
  return 0;
}