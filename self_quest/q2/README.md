⚔️ Quest Log: I/O 멀티플렉싱 채팅 서버 (The Chatter)

🎯 퀘스트 목표

Multiplexing 이해: select 함수를 사용하여 단일 스레드로 여러 클라이언트의 접속과 통신을 동시에 처리한다.

프로토콜 설계: 단순한 문자열이 아닌, 명령어(/join, /msg) 기반의 텍스트 프로토콜을 처리한다.

세션 관리: 누가 어느 방에 있는지 관리하고, 특정 그룹에게만 메시지를 뿌린다(Broadcast).

📜 Sub-Quest List

1️⃣ [입문] 관제탑 구축 (Select 모델 기초)

목표: "손님이 왔나요?" 아니면 "손님이 말을 했나요?"를 구분하는 select 기본 구조 잡기.

Task 1-1: 소켓 초기화 (socket, bind, listen).

Task 1-2: fd_set 초기화 및 select 함수 호출 루프 구현.

Task 1-3: select 반환값 처리.

serverSock에 변화가 생김 -> accept() (새 손님 입장).

clientSock에 변화가 생김 -> recv() (메시지 수신).

2️⃣ [구현] 확성기 (Broadcasting)

목표: 한 명이 말하면 접속한 모든 사람에게 들리게 하기.

Task 2-1: 접속된 클라이언트 소켓들을 저장할 컨테이너(std::vector<int> clients) 만들기.

Task 2-2: 클라이언트가 입장/퇴장할 때 컨테이너에 추가/삭제.

Task 2-3: A가 메시지를 보내면, 컨테이너를 순회하며 A를 제외한 나머지 모두에게 send() 하기.

3️⃣ [심화] 방 만들기 (Room System & Packet Parsing)

목표: 채팅방 기능을 위한 명령어 파싱 및 로직 구현.

Task 3-1: 명령어 처리기 구현.

/join <방번호>: 해당 방으로 이동.

/exit: 로비로 복귀.

Task 3-2: 유저 구조체 설계 (struct User { int socket; int roomId; }).

Task 3-3: 멀티방 브로드캐스트 (같은 roomId를 가진 유저에게만 전송).

4️⃣ [보너스] 유령 잡기 (Heartbeat)

목표: 랜선을 쑥 뽑고 사라진 유저(Zombie Connection) 정리하기.

Task 4-1: User 구조체에 lastHeartbeatTime 추가.

Task 4-2: 클라이언트가 아무 말도 안 해도 5초마다 주기적으로 핑(Ping)을 보내게 함(혹은 서버가 보냄).

Task 4-3: select의 timeout을 활용하여 주기적으로 접속자 리스트를 검사, 오랫동안 조용한 유저 강제 퇴장(close).

🛠️ 기술적 포인트 (Why Select?)

스레드를 100개 만들면(1 client = 1 thread) 컨텍스트 스위칭 비용 때문에 서버가 느려집니다.
**Multiplexing(Select)**은 "웨이터 한 명(Thread)이 여러 테이블(Socket)을 계속 주시하면서, 벨을 누른 테이블에만 가서 주문을 받는 방식"이라 매우 효율적입니다.

📚 참고 자료

윤성우 TCP/IP: Chapter 12 (I/O 멀티플렉싱, select 함수)

게임 서버 교과서: Chapter 3.6 (논블록 소켓), 3.7 (비동기 I/O 개념)