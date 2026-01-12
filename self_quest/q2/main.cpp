#include <iostream>
#include <vector>
#include <algorithm>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/select.h> // select 함수와 fd_set 매크로 사용

using namespace std;

const int PORT = 9000;
const int BUF_SIZE = 1024;

// [Task 2] 접속자 소켓을 관리할 벡터
vector<int> clientSockets;

// [Task 2] 메시지 브로드캐스트 함수
// senderSock: 메시지를 보낸 사람 (이 사람에게는 다시 안 보냄)
// msg: 보낼 메시지
// len: 메시지 길이
void sendToAll(int senderSock, char* msg, int len) {
    for (int i = 0; i < clientSockets.size(); i++) {
        // 보낸 사람 본인을 제외하고 전송 (원하면 본인 포함 가능)
        if (clientSockets[i] != senderSock) {
            write(clientSockets[i], msg, len);
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

    // SO_REUSEADDR 옵션: 서버 재시작 시 "Address already in use" 에러 방지 (필수 옵션)
    int opt = 1;
    setsockopt(serverSock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if (::bind(serverSock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        perror("bind error");
        return 1;
    }

    if (listen(serverSock, 5) == -1) {
        perror("listen error");
        return 1;
    }

    cout << "[System] 채팅 서버가 시작되었습니다 (Port: " << PORT << ")" << endl;

    // 2. [Task 1-2] fd_set 초기화 (관제탑 세팅)
    fd_set reads;       // 감시 대상 목록 (원본)
    fd_set copy_reads;  // 감시 대상 목록 (복사본 - select가 내용을 바꾸기 때문)
    
    FD_ZERO(&reads);            // 1) 목록을 깨끗이 비운다.
    FD_SET(serverSock, &reads); // 2) 대표 전화(리스닝 소켓)를 목록에 추가한다.
    
    int maxFd = serverSock;     // 감시 대상 중 가장 높은 번호 (select 함수에 필요)

    cout << "[System] 클라이언트 접속 대기 중 (Select Model)..." << endl;

    while (true) {
        // [중요] 원본(reads)을 복사해서 사용해야 함!
        // select 함수가 호출되고 나면, 변화가 *없는* 소켓들은 목록에서 지워버리기 때문.
        copy_reads = reads;

        // 타임아웃 설정 (5초)
        // 설정하지 않으면(NULL), 변화가 생길 때까지 영원히 잠듦(Blocking).
        struct timeval timeout;
        timeout.tv_sec = 5;
        timeout.tv_usec = 5000;

        // 3. [Task 1-2] select 함수 호출 (감시 시작)
        // 첫 번째 인자: 감시할 소켓 번호의 최대값 + 1 (이유: 파일 디스크립터는 0부터 시작하니까 개수는 +1)
        // 두 번째 인자: 수신(Read) 이벤트를 감시할 목록
        // 반환값: 변화가 생긴 소켓의 개수 (-1: 오류, 0: 타임아웃)
        int fdNum = select(maxFd + 1, &copy_reads, 0, 0, &timeout);

        if (fdNum == -1) {
            perror("select error");
            break;
        }
        if (fdNum == 0) {
            // 타임아웃 발생 (5초 동안 아무 일도 없었음)
            // 나중에 여기서 좀비 유저 청소(Heartbeat 체크)를 하면 됨
            // cout << "."; 
            // cout.flush();
            continue;
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
                    int clientSock = accept(serverSock, (struct sockaddr*)&clientAddr, &clientAddrSize);

                    if (clientSock == -1) {
                        perror("accept error");
                        continue;
                    }

                    // [중요] 새 손님도 감시 목록(reads)에 추가해야 다음 루프부터 말하는 걸 들을 수 있음
                    FD_SET(clientSock, &reads);
                    
                    // maxFd 갱신 (더 큰 번호의 소켓이 생겼다면 갱신)
                    if (maxFd < clientSock) {
                        maxFd = clientSock;
                    }

                    cout << "\n[System] 새 클라이언트 접속! (Socket: " << clientSock 
                         << ", IP: " << inet_ntoa(clientAddr.sin_addr) << ")" << endl;
                    // [Task 2] 명단에 추가
                    clientSockets.push_back(clientSock);
                    
                    // 입장 메시지 알림 (옵션)
                    string msg = "[System] New user joined!\n";
                    // sendToAll(clientSock, (char*)msg.c_str(), msg.length());


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
                        
                        // [Task 2] 명단에서 삭제
                        // 벡터에서 해당 소켓 번호를 찾아 지움
                        for(int j=0; j<clientSockets.size(); j++) {
                            if(clientSockets[j] == i) {
                                clientSockets.erase(clientSockets.begin() + j);
                                break;
                            }
                        }

                    }
                    // 2) 데이터 수신
                    else {
                        buf[strLen] = 0; // 문자열 끝 처리
                        cout << "[Msg from " << i << "] " << buf << endl;
                        
                        // [Task 2] 확성기 발사! (나를 제외한 모두에게 전송)
                        // 받은 메시지를 그대로 뿌려줌 (Echo Broadcast)
                        sendToAll(i, buf, strLen);

                    }
                }
            }
        }
    }

    close(serverSock);
    return 0;
}