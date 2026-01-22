#include <iostream>
#include <thread>
#include <vector>

using namespace std;

// [공유 자원] 모든 스레드가 함께 사용하는 전역 변수
int sum = 0;

// 각 스레드가 실행할 함수
void worker_deposit() {
    for (int i = 0; i < 1000000; ++i) {
        // [임계 영역] Critical Section
        // 여기서 문제가 발생합니다!
        sum++; 
    }
}

int main() {
    cout << "=== Race Condition 테스트 시작 ===" << endl;
    cout << "목표: 10개의 스레드가 각각 100만 번씩 sum을 증가시킴" << endl;

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
        cout << "[성공] 기적적으로 정확합니다!" << endl;
    } else {
        cout << "[실패] 돈이 어디로 사라졌습니다! (Race Condition 발생)" << endl;
    }

    return 0;
}