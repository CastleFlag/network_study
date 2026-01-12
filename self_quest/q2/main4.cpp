#include <algorithm>
#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <sys/select.h> // select 함수와 fd_set 매크로 사용
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#include <vector>

using namespace std;

const int PORT = 9000;
const int BUF_SIZE = 1024;
const int HEARTBEAT_TIMEOUT = 5; // [Task 4] 5초 동안 말 없으면 강퇴

// [Task 3] 유저 정보를 담는 구조체
struct User {
  int socket;
  int roomId;           // 0: 로비, 1~N: 채팅방
  time_t lastHeartbeat; // [Task 4] 마지막 생존 신고 시간
};

// 접속자 관리 (int 대신 User 구조체 저장)
vector<User> users;

// 유저 찾기 헬퍼 함수
User *findUser(int sock) {
  for (auto &user : users) {
    if (user.socket == sock)
      return &user;
  }
  return nullptr;
}

// [Task 3] 같은 방에 있는 사람들에게만 전송
void sendToRoom(int senderSock, char *msg, int len) {
  User *sender = findUser(senderSock);
  if (!sender)
    return; // 유저를 못 찾으면 중단

  for (const auto &user : users) {
    // 1. 나 자신에게는 보내지 않음
    // 2. 나와 같은 방(roomId)에 있는 사람에게만 전송
    if (user.socket != senderSock && user.roomId == sender->roomId) {
      write(user.socket, msg, len);
    }
  }
}

int main() {
  // 1. [Task 1-1] 소켓 초기화 (대표 전화 개설)
  int serverSock = socket(PF_INET, SOCK_STREAM, 0);
  if (serverSock == -1) {
    perror("socket error");
    return 1;
  }

  struct sockaddr_in serverAddr;
  memset(&serverAddr, 0, sizeof(serverAddr));
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  serverAddr.sin_port = htons(PORT);

  // SO_REUSEADDR 옵션: 서버 재시작 시 "Address already in use" 에러 방지 (필수
  // 옵션)
  int opt = 1;
  setsockopt(serverSock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

  if (::bind(serverSock, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) ==
      -1) {
    perror("bind error");
    return 1;
  }

  if (listen(serverSock, 5) == -1) {
    perror("listen error");
    return 1;
  }

  cout << "[System] 채팅 서버가 시작되었습니다 (Port: " << PORT << ")" << endl;

  // 2. [Task 1-2] fd_set 초기화 (관제탑 세팅)
  fd_set reads;      // 감시 대상 목록 (원본)
  fd_set copy_reads; // 감시 대상 목록 (복사본 - select가 내용을 바꾸기 때문)

  FD_ZERO(&reads);            // 1) 목록을 깨끗이 비운다.
  FD_SET(serverSock, &reads); // 2) 대표 전화(리스닝 소켓)를 목록에 추가한다.

  int maxFd = serverSock; // 감시 대상 중 가장 높은 번호 (select 함수에 필요)

  cout << "[System] 클라이언트 접속 대기 중 (Select Model)..." << endl;

  while (true) {
    // [중요] 원본(reads)을 복사해서 사용해야 함!
    // select 함수가 호출되고 나면, 변화가 *없는* 소켓들은 목록에서 지워버리기
    // 때문.
    copy_reads = reads;

    // [Task 4] 타임아웃을 1초로 줄임 (자주 깨어나서 유령 검사하려고)
    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    // 3. [Task 1-2] select 함수 호출 (감시 시작)
    // 첫 번째 인자: 감시할 소켓 번호의 최대값 + 1 (이유: 파일 디스크립터는
    // 0부터 시작하니까 개수는 +1) 두 번째 인자: 수신(Read) 이벤트를 감시할 목록
    // 반환값: 변화가 생긴 소켓의 개수 (-1: 오류, 0: 타임아웃)
    int fdNum = select(maxFd + 1, &copy_reads, 0, 0, &timeout);

    if (fdNum == -1) {
      perror("select error");
      break;
    }
    if (fdNum == 0) {
      // (타임아웃(0)이어도 아래 루프를 지나서 좀비 검사 로직으로 감)
    }

    // 4. [Task 1-3] 변화가 생긴 소켓 찾기
    // 0번부터 maxFd번까지 모든 소켓을 전수 조사 (Loop)
    for (int i = 0; i < maxFd + 1; ++i) {

      // i번 소켓에 변화가 생겼는가? (체크박스가 켜져있는가?)
      if (FD_ISSET(i, &copy_reads)) {

        // Case A: 대표 전화(serverSock)에 신호가 옴 -> "새 손님 입장!"
        if (i == serverSock) {
          struct sockaddr_in clientAddr;
          socklen_t clientAddrSize = sizeof(clientAddr);
          int clientSock = accept(serverSock, (struct sockaddr *)&clientAddr,
                                  &clientAddrSize);

          if (clientSock == -1) {
            perror("accept error");
            continue;
          }

          // [중요] 새 손님도 감시 목록(reads)에 추가해야 다음 루프부터 말하는
          // 걸 들을 수 있음
          FD_SET(clientSock, &reads);

          // maxFd 갱신 (더 큰 번호의 소켓이 생겼다면 갱신)
          if (maxFd < clientSock) {
            maxFd = clientSock;
          }

          // [Task 4] 입장 시 현재 시간 기록
          User newUser;
          newUser.socket = clientSock;
          newUser.roomId = 0;
          newUser.lastHeartbeat = time(NULL);
          users.push_back(newUser);

          // 입장 메시지 알림 (옵션)
          const char *welcomeMsg =
              "[System] Welcome! Use '/join <number>' to enter a room.\n";
          write(clientSock, welcomeMsg, strlen(welcomeMsg));
        }
        // Case B: 일반 손님(clientSock)에 신호가 옴 -> "메시지 수신!"
        else {
          char buf[BUF_SIZE];
          int strLen = read(i, buf, BUF_SIZE); // i가 곧 소켓 번호

          // 1) 연결 종료 (EOF)
          if (strLen == 0) {
            // 감시 목록에서 제거 (더 이상 이 소켓은 안 봄)
            FD_CLR(i, &reads);
            close(i);
            cout << "[System] 클라이언트 종료 (Socket: " << i << ")" << endl;

            // 벡터에서 삭제
            for (auto it = users.begin(); it != users.end(); ++it) {
              if (it->socket == i) {
                users.erase(it);
                break;
              }
            }
          }
          // 2) 데이터 수신
          else {
            // [Task 4] 생존 신고! 시간 갱신

            User *u = findUser(i);
            if (u) {
              u->lastHeartbeat = time(NULL);
            }

            buf[strLen] = 0; // 문자열 끝 처리
            // [Task 3] 명령어 파싱
            // 메시지가 '/'로 시작하면 명령어로 처리
            if (buf[0] == '/') {
              // "/join " 명령어 확인
              if (strncmp(buf, "/join ", 6) == 0) {
                int newRoomId = atoi(buf + 6); // 숫자 부분 파싱
                User *u = findUser(i);
                if (u) {
                  int oldRoom = u->roomId;
                  u->roomId = newRoomId;

                  // 변경 알림
                  char sysMsg[128];
                  sprintf(sysMsg, "[System] Moved from Room %d to Room %d\n",
                          oldRoom, newRoomId);
                  write(i, sysMsg, strlen(sysMsg));

                  cout << "[Log] User " << i << " moved to Room " << newRoomId
                       << endl;
                }
              } else {
                const char *errMsg = "[System] Unknown command.\n";
                write(i, errMsg, strlen(errMsg));
              }
            }
            // 일반 채팅
            else {
              // [Task 3] 같은 방 유저들에게만 전송
              sendToRoom(i, buf, strLen);
            }
          }
        }
      }
    }

    // 2. [Task 4] 유령 잡기 (좀비 프로세스 정리)
    time_t now = time(NULL);
    // 반복자(iterator)를 직접 제어하며 삭제
    for (auto it = users.begin(); it != users.end();) {
      double gap = difftime(now, it->lastHeartbeat);

      if (gap > HEARTBEAT_TIMEOUT) {
        // 타임아웃 발생! 강제 퇴장
        int targetSock = it->socket;

        cout << "[System] 유령 유저 감지! (Socket: " << targetSock << ", "
             << (int)gap << "초간 무응답) -> 강제 종료" << endl;

        // 소켓 닫고 감시 목록에서 제외
        close(targetSock);
        FD_CLR(targetSock, &reads);

        // 벡터에서 삭제하고, 반복자는 다음 요소를 가리키게 함
        it = users.erase(it);
      } else {
        // 생존자는 다음으로 넘어감
        ++it;
      }
    }
  }

  close(serverSock);
  return 0;
}