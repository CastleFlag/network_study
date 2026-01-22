/**
 * ⚔️ Quest 4: Epoll Echo Server (Linux)
 *
 * 고성능 I/O 멀티플렉싱 서버 - Epoll 기반
 * Select의 한계를 넘어, 수천 개의 동시 접속을 처리하는 서버
 *
 * 컴파일: g++ -o epoll_server epoll_echo_server.cpp -std=c++11
 * 실행: ./epoll_server [port]
 */

#include <arpa/inet.h>
#include <cstring>
#include <errno.h>
#include <fcntl.h>
#include <iostream>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#define MAX_EVENTS 1024
#define BUFFER_SIZE 1024
#define DEFAULT_PORT 9000

// ============================================================
// TODO: 유틸리티 함수 구현
// ============================================================

/**
 * Task 3-2: 소켓을 Non-blocking으로 설정
 * 힌트: fcntl(fd, F_GETFL, 0) -> fcntl(fd, F_SETFL, flags | O_NONBLOCK)
 */
void setNonBlocking(int fd) {
  int flags = fcntl(fd, F_GETFL, 0);
  fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

/**
 * 에러 출력 후 종료
 */
void errorExit(const char *msg) {
  perror(msg);
  exit(1);
}

// ============================================================
// TODO: 서버 초기화
// ============================================================

/**
 * Task 1-1: 서버 소켓 생성 및 바인딩
 */
int createServerSocket(int port) {
  int serverSock = socket(AF_INET, SOCK_STREAM, 0);
  if (serverSock < 0) {
    errorExit("socket() failed");
  }

  // SO_REUSEADDR 옵션 설정 (재시작 시 바인딩 에러 방지)
  int optval = 1;
  setsockopt(serverSock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

  struct sockaddr_in serverAddr;
  memset(&serverAddr, 0, sizeof(serverAddr));
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_addr.s_addr = INADDR_ANY;
  serverAddr.sin_port = htons(port);

  if (bind(serverSock, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) <
      0) {
    errorExit("bind() failed");
  }

  if (listen(serverSock, SOMAXCONN) < 0) {
    errorExit("listen() failed");
  }

  return serverSock;
}

/**
 * Task 1-1: Epoll 인스턴스 생성
 * 힌트: epoll_create() 또는 epoll_create1()
 */
int createEpoll() {
  int epollFd = -1;

  epollFd = epoll_create1(0);

  if (epollFd < 0) {
    errorExit("epoll_create1() failed");
  }

  return epollFd;
}

/**
 * Task 1-2: Epoll에 소켓 등록
 * @param epollFd: epoll 인스턴스
 * @param fd: 등록할 소켓
 * @param events: 감시할 이벤트 (EPOLLIN, EPOLLOUT, EPOLLET 등)
 */
void epollAdd(int epollFd, int fd, uint32_t events) {
  struct epoll_event ev;
  ev.events = events;
  ev.data.fd = fd;
  if (epoll_ctl(epollFd, EPOLL_CTL_ADD, fd, &ev) < 0) {
    errorExit("epoll_ctl(ADD) failed");
  }
}

/**
 * Epoll에서 소켓 제거
 */
void epollRemove(int epollFd, int fd) {
  epoll_ctl(epollFd, EPOLL_CTL_DEL, fd, nullptr);
}

// ============================================================
// TODO: 이벤트 핸들러
// ============================================================

/**
 * Task 1-3, 2-1: 새 클라이언트 연결 처리
 */
void handleAccept(int serverSock, int epollFd, bool useEdgeTrigger) {
  while (true) {
    struct sockaddr_in clientAddr;
    socklen_t clientLen = sizeof(clientAddr);

    int clientSock =
        accept(serverSock, (struct sockaddr *)&clientAddr, &clientLen);
    if (clientSock < 0) {
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        break; // 더 이상 대기 중인 연결 없음 (ET 모드 정상)
      }
      perror("accept() failed");
      return;
    }

    std::cout << "[+] Client connected: " << inet_ntoa(clientAddr.sin_addr)
              << ":" << ntohs(clientAddr.sin_port) << " (fd=" << clientSock
              << ")" << std::endl;

    if (useEdgeTrigger) {
      setNonBlocking(clientSock);
      epollAdd(epollFd, clientSock, EPOLLIN | EPOLLET);
    } else {
      epollAdd(epollFd, clientSock, EPOLLIN);
    }

    // LT 모드면 한 번만 accept
    if (!useEdgeTrigger) {
      break;
    }
  }
}

/**
 * Task 2-2, 2-3: 클라이언트 메시지 처리 (Level Triggered)
 */
void handleClientLT(int clientSock, int epollFd) {
  char buffer[BUFFER_SIZE];

  int bytesRead = recv(clientSock, buffer, sizeof(buffer), 0);

  if (bytesRead <= 0) {
    // 연결 종료 또는 에러
    std::cout << "[-] Client disconnected (fd=" << clientSock << ")"
              << std::endl;
    epollRemove(epollFd, clientSock);
    close(clientSock);
    return;
  }

  // Echo: 받은 데이터를 그대로 전송
  send(clientSock, buffer, bytesRead, 0);
}

/**
 * Task 3-3: 클라이언트 메시지 처리 (Edge Triggered)
 * 주의: EAGAIN이 나올 때까지 반복해서 읽어야 함!
 */
void handleClientET(int clientSock, int epollFd) {
  char buffer[BUFFER_SIZE];

  // TODO: 구현하세요
  while (true) {
    int bytesRead = recv(clientSock, buffer, sizeof(buffer), 0);

    if (bytesRead < 0) {
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        // 더 이상 읽을 데이터 없음 (정상)
        break;
      }
      // 실제 에러
      perror("recv() failed");
      epollRemove(epollFd, clientSock);
      close(clientSock);
      return;
    }

    if (bytesRead == 0) {
      // 연결 종료
      std::cout << "[-] Client disconnected (fd=" << clientSock << ")"
                << std::endl;
      epollRemove(epollFd, clientSock);
      close(clientSock);
      return;
    }

    // Echo
    send(clientSock, buffer, bytesRead, 0);
  }
}

// ============================================================
// TODO: 메인 이벤트 루프
// ============================================================

/**
 * Task 1-3, 1-4: Epoll 이벤트 루프
 */
void eventLoop(int serverSock, int epollFd, bool useEdgeTrigger) {
  struct epoll_event events[MAX_EVENTS];

  std::cout << "[*] Server running... (Mode: "
            << (useEdgeTrigger ? "Edge Trigger" : "Level Trigger") << ")"
            << std::endl;

  while (true) {
    int numEvents = epoll_wait(epollFd, events, MAX_EVENTS, -1);

    if (numEvents < 0) {
      perror("epoll_wait() failed");
      break;
    }

    // 핵심: 준비된 소켓만 순회 (O(활성 소켓))
    for (int i = 0; i < numEvents; i++) {
      int fd = events[i].data.fd;

      if (fd == serverSock) {
        // 새 연결 요청
        handleAccept(serverSock, epollFd, useEdgeTrigger);
      } else {
        // 클라이언트 데이터
        if (useEdgeTrigger) {
          handleClientET(fd, epollFd);
        } else {
          handleClientLT(fd, epollFd);
        }
      }
    }
  }
}

// ============================================================
// Main
// ============================================================

int main(int argc, char *argv[]) {
  int port = DEFAULT_PORT;
  bool useEdgeTrigger = false; // true로 바꾸면 ET 모드

  if (argc >= 2) {
    port = atoi(argv[1]);
  }
  if (argc >= 3 && strcmp(argv[2], "et") == 0) {
    useEdgeTrigger = true;
  }

  std::cout << "========================================" << std::endl;
  std::cout << "  Quest 4: Epoll Echo Server (Linux)" << std::endl;
  std::cout << "========================================" << std::endl;
  std::cout << "[*] Starting server on port " << port << std::endl;

  // 1. 서버 소켓 생성
  int serverSock = createServerSocket(port);
  std::cout << "[*] Server socket created (fd=" << serverSock << ")"
            << std::endl;

  // 2. Epoll 인스턴스 생성
  int epollFd = createEpoll();
  if (epollFd < 0) {
    std::cerr << "[!] TODO: createEpoll() 구현 필요!" << std::endl;
    close(serverSock);
    return 1;
  }
  std::cout << "[*] Epoll instance created (fd=" << epollFd << ")" << std::endl;

  // 3. 서버 소켓을 Epoll에 등록
  uint32_t serverEvents = EPOLLIN;
  if (useEdgeTrigger) {
    serverEvents |= EPOLLET;
    setNonBlocking(serverSock);
  }
  epollAdd(epollFd, serverSock, serverEvents);

  // 4. 이벤트 루프 시작
  eventLoop(serverSock, epollFd, useEdgeTrigger);

  // 정리
  close(epollFd);
  close(serverSock);

  return 0;
}
