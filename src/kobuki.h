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

#define STR_FAIL            "\x1b[1;31mFAIL\x1b[0m"
#define STR_ERRO            "\x1b[1;31mERRO\x1b[0m"
#define STR_PASS            "\x1b[1;32mPASS\x1b[0m"
#define STR_INFO            "\x1b[0mINFO\x1b[0m"
#define STR_WARN            "\x1b[1;33mWARN\x1b[0m"
#define STR_DEBUG           "\x1b[1;35mDBUG\x1b[0m"

/* KOBUKI DFINES */
#define HEADER_0 0xAA
#define HEADER_1 0x55
#define PAYLOAD_LED_LEN 4
#define PAYLOAD_SPEED_LEN 6
#define LED_CONTROL_ID 0x0C
#define LED_CONTROL_LEN 2
#define BASE_CONTROL_ID 0x01
#define BASE_CONTROL_LEN 4
#define SCRIPT_COMMAND_MAX_LEN 100

/* UDP DEFINES */
#define UDP_PORT_NUM 5555
#define UDP_PACKET_MAX_SIZE 1024

/**
 * @brief Log message type
 */
enum eMessageType
{
  kMessageType_None = 0,
  kMessageType_Error = 1,
  kMessageType_Pass = 2,
  kMessageType_Info = 3,
  kMessageType_Debug = 4,
};
typedef int MessageType;

/**
 * @brief KOBUKI LED color type
 */
enum eLEDColor
{
  kLEDColor_None = 0,
  kLEDColor_Green = 1,
  kLEDColor_Red = 2,
};
typedef int LEDColor;

/**
 * @brief KOBUKI command tpye in script file
 * 
 */
enum eCommandType
{
  kCommandType_None = 0,
  kCommandType_LED = 1,
  kCommandType_Speed = 2,
  kCommandType_Sleep = 3,
};
typedef int CommandType;

/**
 * @brief KOBUKI LED command message format
 * 
 */
struct LEDMessageFormat
{
  uint8_t header_0;
  uint8_t header_1;
  uint8_t payload_len;
  uint8_t sub_payload_id;
  uint8_t sub_payload_len;
  uint16_t led;
} __attribute__((__packed__));

/**
 * @brief KOBUKI speed command message format
 * 
 */
struct SpeedMessageFormat
{
  uint8_t header_0;
  uint8_t header_1;
  uint8_t payload_len;
  uint8_t sub_payload_id;
  uint8_t sub_payload_len;
  uint16_t speed;
  uint16_t radius;
} __attribute__((__packed__));


/**
 * @brief Command line in script file
 * 
 */
struct ScriptLine
{
  CommandType type;
  int speed; ///< 이동 속도 mm/s 단위
  int radius; ///< 회전 반경 mm 단위
	float radian; ///< 회전 각도
	int distance; ///< 이동 거리 mm 단위
	int move_time; ///< 속도, 이동 거리로 이동 시간 계산 ms 단위

  int delay; ///< ms 단위
  
  int led_num;
  int color;
};

/**
 * @brief Global MIB
 * 
 */
struct MIB
{
  char server_ip_addr[16];
  int server_port_num;

  int device;
  struct termios newtio;
  uint16_t led_status;
  uint16_t speed_status;
  uint16_t radius_status;

  char device_name[SCRIPT_COMMAND_MAX_LEN];
  char baud_rate[SCRIPT_COMMAND_MAX_LEN];
  int log_level;
  char script_file_name[SCRIPT_COMMAND_MAX_LEN];
  int script_lines_size;
  struct ScriptLine script_lines[SCRIPT_COMMAND_MAX_LEN];

  struct sockaddr_in server_addr;
  int socket;
};

extern struct MIB g_mib;

/* kobuki-fun.c */
void PrintLog(MessageType msg_type, const char *format, ...);
void PrintHexDump(MessageType msg_Type, CommandType command_type, const char *format, void *command);
int KOBUKI_ControlLED(int device, int led_num, int color);
int KOBUKI_ControlSpeed(int device, int speed, int radius);
int ParseScriptCommand(char *script_file, struct MIB *mib);

/* kobuki-udp.c */
int InitUDP(const char *ip_addr, const int port_num, struct sockaddr_in *server_addr, int *socket);
int SendUDPMessage(int m_socket, struct sockaddr_in server_addr, char *payload, size_t payload_size);