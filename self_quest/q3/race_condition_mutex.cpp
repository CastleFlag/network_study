#include <iostream>
#include <thread>
#include <vector>
#include <mutex> // [Task 2] Mutex 헤더 추가

using namespace std;

// [공유 자원] 모든 스레드가 함께 사용하는 전역 변수
int sum = 0;

// [Task 2] 공유 자원을 보호할 자물쇠
mutex mtx;

// 각 스레드가 실행할 함수
void worker_deposit() {
    for (int i = 0; i < 1000000; ++i) {
        // [임계 영역] Critical Section 보호 시작
        
        // 방법 1: 직접 lock/unlock (실수하기 쉬움)
        // mtx.lock();
        // sum++;
        // mtx.unlock();

        // 방법 2: RAII 패턴 (권장)
        // lock_guard 객체가 생성될 때 lock(), 범위를 벗어나면 자동으로 unlock()
        // 예외가 발생해도 안전하게 해제됨.
        lock_guard<mutex> lock(mtx);
        sum++; 
        
        // [임계 영역] 끝 (lock_guard 소멸 시 자동 unlock)
    }
}

int main() {
    cout << "=== Mutex 동기화 테스트 시작 ===" << endl;
    cout << "목표: 10개의 스레드가 각각 100만 번씩 sum을 증가시킴 (Mutex 적용)" << endl;

    vector<thread> threads;

    // 1. 스레드 10개 생성 및 실행
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back(worker_deposit);
    }

    // 2. 모든 스레드가 일을 마칠 때까지 대기 (Join)
    for (auto& t : threads) {
        t.join();
    }

    // 3. 결과 확인
    cout << "--------------------------------" << endl;
    cout << "기대값: 10000000 (천만)" << endl;
    cout << "실제값: " << sum << endl;
    cout << "--------------------------------" << endl;

    if (sum == 10000000) {
        cout << "[성공] Mutex 덕분에 정확한 값이 나왔습니다!" << endl;
    } else {
        cout << "[실패] 여전히 Race Condition이 발생했습니다." << endl;
    }

    return 0;
}