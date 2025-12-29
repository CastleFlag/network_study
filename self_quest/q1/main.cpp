#include <arpa/inet.h> // sockaddr_in, inet_addr
#include <cstdlib>     // atoi, exit
#include <cstring>
#include <iostream>
#include <string>
#include <sys/socket.h> // socket, bind, listen, accept...
#include <unistd.h>     // close

using namespace std;

void RunTcpServer(int port);
void RunUdpServer(int port);
void RunTcpClient(const char *ip, int port);
void RunUdpClient(const char *ip, int port);

void PrintUsage() {
  cout << "Usage:" << endl;
  cout << "  Server: ./Tester server <tcp|udp> <port>" << endl;
  cout << "  Client: ./Tester client <tcp|udp> <server_ip> <port>" << endl;
}

int main(int argc, char *argv[]) {
  // 인자 개수 확인 (최소 4개 필요: 프로그램명, 모드, 프로토콜, 포트/IP)
  if (argc < 4) {
    PrintUsage();
    return 1;
  }

  string mode = argv[1];  // server or client
  string proto = argv[2]; // tcp or udp

  // 1. 서버 모드 실행
  if (mode == "server") {
    if (argc != 4) {
      cout << "[Error] 서버 실행에는 포트 번호가 필요합니다." << endl;
      PrintUsage();
      return 1;
    }

    int port = atoi(argv[3]);

    if (proto == "tcp") {
      RunTcpServer(port);
    } else if (proto == "udp") {
      RunUdpServer(port);
    } else {
      cout << "[Error] 알 수 없는 프로토콜입니다: " << proto << endl;
      return 1;
    }
  }
  // 2. 클라이언트 모드 실행
  else if (mode == "client") {
    if (argc != 5) {
      cout << "[Error] 클라이언트 실행에는 IP와 포트 번호가 필요합니다."
           << endl;
      PrintUsage();
      return 1;
    }

    const char *ip = argv[3];
    int port = atoi(argv[4]);

    if (proto == "tcp") {
      RunTcpClient(ip, port);
    } else if (proto == "udp") {
      RunUdpClient(ip, port);
    } else {
      cout << "[Error] 알 수 없는 프로토콜입니다: " << proto << endl;
      return 1;
    }
  }
  // 3. 잘못된 모드
  else {
    cout << "[Error] 알 수 없는 모드입니다: " << mode << endl;
    PrintUsage();
    return 1;
  }

  return 0;
}

// --- 아래는 추후 구현할 빈 함수들 (Stub) ---

void RunTcpServer(int port) {
  cout << "[System] TCP Server 시작 (Port: " << port << ")" << endl;
  // TODO: socket() -> bind() -> listen() -> accept() -> read()
  int sock;
  int client;

  struct sockaddr_in serv_addr;
  socklen_t clilen;

  sock = socket(AF_INET, SOCK_STREAM, 0);

  memset(&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  serv_addr.sin_port = htons(port);

  if (::bind(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1) {
    cerr << "Bind failed" << endl;
    close(sock);
    exit(1);
  }
  listen(sock, 5);
  clilen = sizeof(serv_addr);
  client = accept(sock, (struct sockaddr *)&serv_addr, &clilen);
  write(client, "Hello from server", 18);
  close(client);
  close(sock);
}

void RunUdpServer(int port) {
  cout << "[System] UDP Server 시작 (Port: " << port << ")" << endl;
  // TODO: socket() -> bind() -> recvfrom()
}

void RunTcpClient(const char *ip, int port) {
  cout << "[System] TCP Client 시작 (Target: " << ip << ":" << port << ")"
       << endl;
  // TODO: socket() -> connect() -> write()
  int sock;
  struct sockaddr_in serv_addr;
  char buffer[1024];
  sock = socket(AF_INET, SOCK_STREAM, 0);
  memset(&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = inet_addr(ip);
  serv_addr.sin_port = htons(port);

  if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
    cerr << "Connection failed" << endl;
    close(sock);
    exit(1);
  }
  int strlen = read(sock, buffer, sizeof(buffer) - 1);

  cout << "Received from server: " << buffer << endl;
  close(sock);
}

void RunUdpClient(const char *ip, int port) {
  cout << "[System] UDP Client 시작 (Target: " << ip << ":" << port << ")"
       << endl;
  // TODO: socket() -> sendto()
}
