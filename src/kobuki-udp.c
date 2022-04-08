#include "kobuki.h"

/**
 * @brief Initialize UDP socket
 * @param[in] ip_addr 서버 IPv4 주소
 * @param[in] port_num 서버 포트 번호
 * @param[out] server_addr 서버 주소 정보 구조체
 * @param[out] socket 서버 소켓 정보
 * @retval 0: 성공
 * @retval 음수: 실패
 */
int InitUDP(const char *ip_addr, const int port_num, struct sockaddr_in *server_addr, int *m_socket)
{
  *m_socket = socket(PF_INET, SOCK_DGRAM, 0);
  if (*m_socket < 0) {
    PrintLog(kMessageType_Error, "Fail to create socket - socket: %d\n", *m_socket);
    return -1;
  }

  memset(server_addr, 0x00, sizeof(struct sockaddr_in));
  server_addr->sin_family = AF_INET;
  server_addr->sin_addr.s_addr = inet_addr(ip_addr);
  server_addr->sin_port = htons(port_num);

  PrintLog(kMessageType_Pass, "Success to create socket\n");
  return 0;
}

/**
 * @brief Send UDP message
 * @param[in] m_socket 서버 소켓 정보
 * @param[in] server_addr 서버 주소 정보 구조체
 * @param[in] payload 전송할 메시지
 * @param[in] payload_size 전송할 메시지 길이
 * @retval 0: 성공
 * @retval 음수: 실패
 */
int SendUDPMessage(int m_socket, struct sockaddr_in server_addr, char *payload, size_t payload_size)
{
  int ret;

  ret = sendto(m_socket, payload, payload_size, 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
  if (ret < 0) {
    PrintLog(kMessageType_Error, "Fail to send UDP message - ret: %d\n", ret);
    perror("sendto fail");
    return -1;
  }

  PrintLog(kMessageType_Pass, "Success to send UDP message\n");
  return 0;
}