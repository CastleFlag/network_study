#include <iostream>
#include <thread>
#include <mutex>
#include <chrono> // sleep_for 사용

using namespace std;

// 두 개의 자원(자물쇠) 준비
mutex mtxA;
mutex mtxB;

// [Worker 1] A를 먼저 잡고, B를 원함
void worker1() {
    cout << "[Worker1] 자원 A 획득 시도..." << endl;
    lock_guard<mutex> lockA(mtxA); // A 잠금
    cout << "[Worker1] 자원 A 획득 성공! (작업 중...)" << endl;

    // [중요] 데드락을 유발하기 위한 강제 지연
    // Worker1이 A를 잡고 있는 동안, Worker2가 B를 잡을 시간을 벌어줌
    this_thread::sleep_for(chrono::milliseconds(100));

    cout << "[Worker1] 자원 B 획득 대기 중..." << endl;
    // 여기서 멈춤! (Worker2가 B를 놓아줄 때까지)
    lock_guard<mutex> lockB(mtxB); 
    
    cout << "[Worker1] 자원 B 획득 성공! (이 메시지는 영원히 볼 수 없을 것입니다)" << endl;
}

// [Worker 2] B를 먼저 잡고, A를 원함 (잠금 순서가 Worker1과 반대!)
void worker2() {
    cout << "[Worker2] 자원 B 획득 시도..." << endl;
    lock_guard<mutex> lockB(mtxB); // B 잠금
    cout << "[Worker2] 자원 B 획득 성공! (작업 중...)" << endl;

    // Worker1이 A를 잡을 시간을 줌
    this_thread::sleep_for(chrono::milliseconds(100));

    cout << "[Worker2] 자원 A 획득 대기 중..." << endl;
    // 여기서 멈춤! (Worker1이 A를 놓아줄 때까지)
    lock_guard<mutex> lockA(mtxA);

    cout << "[Worker2] 자원 A 획득 성공! (이 메시지는 영원히 볼 수 없을 것입니다)" << endl;
}

int main() {
    cout << "=== 데드락(Deadlock) 시뮬레이션 시작 ===" << endl;
    cout << "두 스레드가 서로를 기다리며 영원히 멈추는지 확인하세요.\n" << endl;

    thread t1(worker1);
    thread t2(worker2);

    // 스레드가 끝날 때까지 대기 (데드락이 걸리면 여기서 영원히 대기함)
    t1.join();
    t2.join();

    cout << "=== 모든 작업 완료 (정상 종료) ===" << endl;
    return 0;
}