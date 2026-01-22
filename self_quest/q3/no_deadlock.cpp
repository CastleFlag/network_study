#include <iostream>
#include <thread>
#include <mutex>
#include <chrono> // sleep_for 사용

using namespace std;

// 두 개의 자원(자물쇠) 준비
mutex mtxA;
mutex mtxB;

// [Worker 1] A와 B가 모두 필요함
void worker1() {
    cout << "[Worker1] 자원 A, B 획득 시도..." << endl;
    
    // [Fix] std::lock을 사용하여 두 뮤텍스를 한 번에 안전하게 잠금
    // 내부적으로 데드락 방지 알고리즘이 적용되어 순서 꼬임을 막아줌
    std::lock(mtxA, mtxB);

    // 이미 잠긴 뮤텍스의 소유권만 lock_guard에게 넘겨줌 (adopt_lock)
    // 이렇게 해야 함수가 끝날 때 자동으로 unlock 됨
    lock_guard<mutex> lockA(mtxA, adopt_lock);
    lock_guard<mutex> lockB(mtxB, adopt_lock);

    cout << "[Worker1] 자원 A, B 획득 성공! (작업 중...)" << endl;

    // 작업 시뮬레이션
    this_thread::sleep_for(chrono::milliseconds(100));

    cout << "[Worker1] 작업 완료 및 자원 해제" << endl;
}

// [Worker 2] A와 B가 모두 필요함
void worker2() {
    cout << "[Worker2] 자원 A, B 획득 시도..." << endl;

    // [Fix] Worker1과 마찬가지로 std::lock 사용
    // Worker1과 인자 순서를 다르게 넣어도(mtxB, mtxA) std::lock이 알아서 안전하게 처리해줌
    std::lock(mtxA, mtxB);

    lock_guard<mutex> lockB(mtxB, adopt_lock);
    lock_guard<mutex> lockA(mtxA, adopt_lock);

    cout << "[Worker2] 자원 A, B 획득 성공! (작업 중...)" << endl;

    // 작업 시뮬레이션
    this_thread::sleep_for(chrono::milliseconds(100));

    cout << "[Worker2] 작업 완료 및 자원 해제" << endl;
}

int main() {
    cout << "=== 데드락(Deadlock) 해결 테스트 ===" << endl;
    cout << "std::lock을 사용하여 교착 상태 없이 두 스레드가 모두 완료되는지 확인하세요.\n" << endl;

    thread t1(worker1);
    thread t2(worker2);

    // 스레드가 끝날 때까지 대기
    t1.join();
    t2.join();

    cout << "=== 모든 작업 완료 (정상 종료) ===" << endl;
    return 0;
}