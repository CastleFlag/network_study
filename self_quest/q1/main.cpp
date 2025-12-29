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

void RunTcpServer(int port) {
  cout << "[System] TCP Server 시작 (Port: " << port << ")" << endl;

  // 1. 소켓 생성 (전화기 구입)
  // 첫 번째 인자 (Domain): 프로토콜 체계
  // - PF_INET  : IPv4 인터넷 프로토콜 (가장 많이 씀)
  // - PF_INET6 : IPv6 인터넷 프로토콜
  // - PF_LOCAL (또는 PF_UNIX) : 같은 컴퓨터 내 프로세스끼리 통신할 때 (파일
  // 시스템 경로 사용)
  //
  // 두 번째 인자 (Type): 전송 방식
  // - SOCK_STREAM : 연결 지향형, 신뢰성 보장 (TCP)
  // - SOCK_DGRAM  : 비연결 지향형, 빠른 전송 (UDP)
  // - SOCK_RAW    : TCP/UDP 계층을 건너뛰고 IP 패킷 직접 조작 (해킹툴이나
  // 시스템 유틸용)
  int serverSock = socket(PF_INET, SOCK_STREAM, 0);
  if (serverSock == -1) {
    perror("socket error");
    exit(1);
  }

  // 2. 주소 정보 초기화 (전화번호 결정)
  struct sockaddr_in serverAddr;
  memset(&serverAddr, 0, sizeof(serverAddr)); // 구조체 메모리를 0으로 초기화

  // sin_family: 주소 체계 설정
  // - AF_INET  : IPv4 (sockaddr_in 구조체 사용 시 필수)
  // - AF_INET6 : IPv6 (sockaddr_in6 구조체 사용 필요)
  serverAddr.sin_family = AF_INET;

  // sin_addr.s_addr: 서버의 IP 주소 설정 (어떤 랜카드에서 들을 것인가?)
  // - htonl(INADDR_ANY)      : 컴퓨터에 있는 "모든" 랜카드의 IP로 들어오는
  // 연결을 다 받음 (0.0.0.0) -> 일반적인 서버 설정
  // - htonl(INADDR_LOOPBACK) : 외부 접속 차단, 오직 "내 컴퓨터 내부"에서만 접속
  // 가능 (127.0.0.1) -> 로컬 테스트용
  // - inet_addr("192.168.1.10") : 특정 IP(예: 사설 IP)로 들어오는 요청만 골라서
  // 받고 싶을 때
  serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

  serverAddr.sin_port =
      htons(port); // Port: 인자로 받은 포트 (Network Byte Order로 변환)

  // 3. 소켓에 주소 할당 (bind: 전화기에 번호 부여)
  if (::bind(serverSock, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) ==
      -1) {
    perror("bind error");
    exit(1);
  }

  // 4. 연결 대기 상태 진입 (listen: 개통 완료, 케이블 연결)
  // 5: 대기 큐(Backlog) 크기. 동시에 연결 요청이 몰릴 때 대기할 수 있는 수.
  if (listen(serverSock, 5) == -1) {
    perror("listen error");
    exit(1);
  }

  cout << "[System] 클라이언트 접속 대기 중..." << endl;

  // 5. 연결 수락 (accept: 수화기 들기)
  // 중요: serverSock은 '연결 대기'용이고, 실제 통신은 반환된 clientSock으로 함.
  struct sockaddr_in clientAddr;
  socklen_t clientAddrSize = sizeof(clientAddr);

  // 여기서 클라이언트가 connect 할 때까지 프로그램이 멈춰있음(Blocking)
  int clientSock =
      accept(serverSock, (struct sockaddr *)&clientAddr, &clientAddrSize);
  if (clientSock == -1) {
    perror("accept error");
    exit(1);
  }

  cout << "[System] 클라이언트 연결됨! IP: " << inet_ntoa(clientAddr.sin_addr)
       << endl;

  // 6. 데이터 수신 루프 (read)
  char buffer[4096]; // 데이터를 담을 버퍼 (4KB 단위)
  long long totalBytes =
      0; // 받은 총 데이터 크기 (1GB는 int 범위를 넘을 수 있으니 long long 추천)

  unsigned char expected_val = 0;

  while (true) {
    // read(소켓, 버퍼, 버퍼크기)
    // 반환값: 읽은 바이트 수 (>0), 연결 종료(0), 에러(-1)
    ssize_t bytesRead = read(clientSock, buffer, sizeof(buffer));

    if (bytesRead == 0) {
      // 클라이언트가 socket을 close() 하면 0이 반환됨 (EOF)
      cout << "[System] 클라이언트가 연결을 종료했습니다." << endl;
      break;
    } else if (bytesRead == -1) {
      perror("read error");
      break;
    }

    for (auto i : buffer) {
      unsigned char received = (unsigned char)i;
      if (received != expected_val) {
        cout << "diff! received : " << received << "expected: " << expected_val
             << endl;
      }
      expected_val++;
    }

    totalBytes += bytesRead;
    // 진행 상황 로그 (너무 자주 찍으면 성능 저하되므로 주석 처리 가능)
    // cout << "받은 바이트: " << bytesRead << " (누적: " << totalBytes << ")"
    // << endl;
  }

  cout << "== 결과 리포트 ==" << endl;
  cout << "총 수신 데이터: " << totalBytes << " bytes" << endl;

  // 7. 소켓 정리 (전화 끊기)
  close(clientSock); // 손님용 전화 끊기
  close(serverSock); // 대표 전화 끊기 (더 이상 연결 안 받음)
}

void RunUdpServer(int port) {
  cout << "[System] UDP Server 시작 (Port: " << port << ")" << endl;
  // TODO: socket() -> bind() -> recvfrom()
}

void RunTcpClient(const char *ip, int port) {
  cout << "[System] TCP Client 시작 (Target: " << ip << ":" << port << ")"
       << endl;

  // 1. 소켓 생성 (SOCK_STREAM = TCP)
  int sock = socket(PF_INET, SOCK_STREAM, 0);
  if (sock == -1) {
    perror("socket error");
    exit(1);
  }

  // 2. 서버 주소 구조체 설정
  struct sockaddr_in serverAddr;
  memset(&serverAddr, 0, sizeof(serverAddr));
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_addr.s_addr =
      inet_addr(ip); // 문자열 IP("127.0.0.1")를 네트워크 주소로 변환
  serverAddr.sin_port = htons(port);

  // 3. 연결 요청 (connect)
  // 클라이언트는 bind 과정 없이 바로 connect를 호출하면,
  // OS가 남는 포트를 자동으로 할당해서 연결을 시도함.
  if (connect(sock, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
    perror("connect error");
    exit(1);
  }

  cout << "[System] 서버에 연결되었습니다. 1GB 데이터 전송을 시작합니다..."
       << endl;

  // 4. 데이터 전송 (1GB = 1024 * 1024 * 1024 bytes)
  const long long TOTAL_SIZE =
      1LL * 1024 * 1024 * 1024; // 1GB (1LL은 long long 리터럴)
  const int BUFFER_SIZE = 4096; // 4KB 단위로 쪼개서 전송
  char buffer[BUFFER_SIZE];
  long long sentBytes = 0;

  // 데이터 검증용 패턴 생성 (0, 1, 2, ... 255 반복)
  // 수신 측에서 데이터가 깨졌는지 확인할 때 사용
  for (int i = 0; i < BUFFER_SIZE; ++i) {
    buffer[i] = (char)(i % 256);
  }

  // 전송 루프
  while (sentBytes < TOTAL_SIZE) {
    // 남은 데이터 크기 계산
    long long remaining = TOTAL_SIZE - sentBytes;
    // 이번에 보낼 크기 결정 (남은 게 버퍼보다 작으면 남은 만큼만)
    int currentChunk = (remaining < BUFFER_SIZE) ? (int)remaining : BUFFER_SIZE;

    // write(소켓, 데이터, 길이)
    ssize_t written = write(sock, buffer, currentChunk);
    if (written == -1) {
      perror("write error");
      break;
    }

    sentBytes += written;

    // 진행 상황 표시 (약 10MB 마다 로그 출력)
    if (sentBytes % (10 * 1024 * 1024) == 0) {
      cout << "\r전송 중... " << (sentBytes / (1024 * 1024)) << " MB / 1024 MB"
           << flush;
    }
  }

  cout << endl
       << "[System] 전송 완료! 총 전송량: " << sentBytes << " bytes" << endl;

  // 5. 소켓 종료
  close(sock);
}

void RunUdpClient(const char *ip, int port) {
  cout << "[System] UDP Client 시작 (Target: " << ip << ":" << port << ")"
       << endl;
  // TODO: socket() -> sendto()
}
