🚀 TCP vs UDP 성능 및 신뢰성 테스트기 (Linux C++ Ver.)

이 프로젝트는 TCP의 신뢰성과 UDP의 전송 속도 및 비신뢰성을 직접 검증하기 위한 C++ 네트워크 벤치마크 툴입니다.
리눅스 환경에서 표준 소켓 프로그래밍을 익힌 뒤, 추후 Windows 및 C#으로 확장하는 것을 목표로 합니다.

🎯 프로젝트 목표

리눅스 소켓 마스터: socket, bind, listen, accept, connect, read/write 등 리눅스 시스템 콜 이해

TCP vs UDP: 연결 지향형 통신과 비연결형 통신의 실제 동작 차이 검증

성능 측정: Throughput(Mbps) 측정 및 패킷 유실률(Loss Rate) 계산

🛠️ 빌드 및 실행 방법 (Usage)

1. 환경 설정

OS: Linux (Ubuntu, CentOS 등)

Compiler: G++ (GCC)

Library: Standard Linux Socket API (<sys/socket.h>, <arpa/inet.h>)

2. 빌드 (Compile)

터미널에서 다음 명령어로 컴파일합니다.

g++ -o Tester main.cpp -std=c++11

3. 실행 커맨드 (CLI)

프로그램은 커맨드 라인 인자를 통해 모드(서버/클라이언트)와 프로토콜(TCP/UDP)을 결정합니다.

서버 실행:

# TCP 모드 (포트 12345 열기)

./Tester server tcp 12345

# UDP 모드

./Tester server udp 12345

클라이언트 실행:

# TCP 모드 (1GB 데이터 전송)

./Tester client tcp 127.0.0.1 12345

# UDP 모드 (패킷 100만 개 전송)

./Tester client udp 127.0.0.1 12345

📝 퀘스트 보드 (Quest Board)

구현해야 할 기능 목록입니다. 완료된 항목은 [x]로 표시하세요.

Lv.1 전장의 세팅 (리눅스 기초)

[ ] Task 1-1: main.cpp 생성 및 필수 헤더 포함 (<sys/socket.h>, <arpa/inet.h>, <unistd.h>)

[ ] Task 1-2: 컴파일 환경 구축 (g++ 빌드 테스트)

[ ] Task 1-3: main(int argc, char* argv[]) 파싱 로직 구현 (모드/IP/Port 분기 처리)

Lv.2 [TCP] 절대 깨지지 않는 방패 (신뢰성)

[ ] Task 2-1 (Server): socket -> bind -> listen -> accept -> read 루프 구현

Tip: 리눅스에서는 recv 대신 read를 써도 됩니다 (Everything is a file).

[ ] Task 2-2 (Client): 1GB 데이터를 4KB 버퍼로 쪼개서 write 반복 전송

[ ] Task 2-3 (Data Integrity): 데이터 검증 패턴(0, 1, 2...) 생성 및 수신 측 검사 로직 (데이터 깨짐 확인)

Lv.3 [UDP] 빗발치는 화살 (비신뢰성)

[ ] Task 3-1: UDP 패킷 구조체 설계 (struct Packet { int seq; char data[1020]; })

[ ] Task 3-2 (Client): sendto를 사용해 딜레이 없이 100만 개 패킷 폭격 (Blast)

[ ] Task 3-3 (Server): recvfrom 구현 및 시퀀스 번호(seq) 기반 유실/순서 뒤바뀜 감지

[ ] Task 3-4 (Report): 전송 완료 후 총 수신 개수 및 유실률(%) 출력

Lv.4 [심화] 속도 측정 (Benchmark)

[ ] Task 4-1: std::chrono를 이용한 정밀 시간 측정 (Start ~ End)

[ ] Task 4-2: Mbps(Megabits per second) 계산 공식 적용 및 출력

[ ] Task 4-3: 로컬호스트(Loopback) 테스트 수행

Lv.5 [보너스] 극한의 상황 (Congestion Control)

[ ] Task 5-1: 서버 측 처리 로직에 인위적인 지연(usleep) 추가

[ ] Task 5-2: TCP의 흐름 제어(Window Size Full)로 인한 송신 속도 저하 관찰

[ ] Task 5-3: UDP의 무자비한 패킷 전송과 대량 유실(Packet Drop) 관찰

🔮 Future Roadmap

Phase 1 (Current): Linux C++로 소켓 통신의 원리(OS 레벨) 파악

Phase 2: C# (.NET)으로 동일한 기능을 구현하며 언어별 차이 학습

Phase 3: Windows 환경(IOCP 등)으로 확장하여 고성능 서버 구조 학습

📚 참고 자료

윤성우의 열혈 TCP/IP 소켓 프로그래밍 (Part 01 ~ 02: 리눅스 기반)

배현직의 게임 서버 프로그래밍 교과서 (Chapter 02 ~ 03)
