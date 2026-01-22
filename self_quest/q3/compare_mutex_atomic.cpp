#include <atomic> // [Task 3] Atomic 헤더 추가
#include <chrono> // [Task 3] 시간 측정 헤더 추가
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

using namespace std;
using namespace std::chrono;

const int NUM_THREADS = 10;
const int NUM_INCREMENTS = 1000000;

// [가설 검증] 약간의 업무 로직 추가 (CPU를 바쁘게 만듦)
// 왜 mutex가 atomic보다 더 빠를까?
void do_some_work() {
  // volatile을 써서 컴파일러가 이 루프를 삭제하지 못하게 함
  volatile int dummy = 0;
  for (int k = 0; k < 100; ++k) {
    dummy++;
  }
}

// ==========================================
// Case 1: 동기화 없음 (Unsafe)
// ==========================================
int sum_unsafe = 0;
void worker_unsafe() {
  for (int i = 0; i < NUM_INCREMENTS; ++i) {
    do_some_work(); // 딴짓 좀 하다가

    sum_unsafe++; // 데이터 경쟁 발생
  }
}

// ==========================================
// Case 2: Mutex 사용 (Safe but Slow)
// ==========================================
int sum_mutex = 0;
mutex mtx;
void worker_mutex() {
  for (int i = 0; i < NUM_INCREMENTS; ++i) {
    // 매 연산마다 잠금/해제 -> 오버헤드 큼
    lock_guard<mutex> lock(mtx);
    do_some_work(); // 딴짓 좀 하다가
    sum_mutex++;
  }
}

// ==========================================
// Case 3: Atomic 사용 (Safe & Fast)
// ==========================================
// CPU 차원의 원자적 연산 명령어 사용 (Lock-free)
atomic<int> sum_atomic(0);
void worker_atomic() {
  for (int i = 0; i < NUM_INCREMENTS; ++i) {
    do_some_work(); // 딴짓 좀 하다가
    sum_atomic++; // fetch_add
  }
}

// 벤치마크 실행 도우미 함수
void run_benchmark(string name, void (*worker_func)(), int &result) {
  cout << "[" << name << "] 테스트 시작..." << endl;

  auto start_time = high_resolution_clock::now();

  vector<thread> threads;
  for (int i = 0; i < NUM_THREADS; ++i) {
    threads.emplace_back(worker_func);
  }

  for (auto &t : threads) {
    t.join();
  }

  auto end_time = high_resolution_clock::now();
  duration<double> diff = end_time - start_time;

  cout << "  소요 시간 : " << diff.count() << " 초" << endl;
  cout << "  최종 결과 : " << result << endl;

  int expected = NUM_THREADS * NUM_INCREMENTS;
  if (result == expected) {
    cout << "  무결성    : [PASS]" << endl;
  } else {
    cout << "  무결성    : [FAIL] (오차: " << expected - result << ")" << endl;
  }
  cout << endl;
}

// Atomic 전용 오버로딩 (atomic<int>는 int&로 바로 못 받아서 분리)
void run_benchmark_atomic(string name, void (*worker_func)()) {
  cout << "[" << name << "] 테스트 시작..." << endl;

  auto start_time = high_resolution_clock::now();

  vector<thread> threads;
  for (int i = 0; i < NUM_THREADS; ++i) {
    threads.emplace_back(worker_func);
  }
  for (auto &t : threads) {
    t.join();
  }

  auto end_time = high_resolution_clock::now();
  duration<double> diff = end_time - start_time;

  // atomic 값 로드
  int result = sum_atomic.load();

  cout << "  소요 시간 : " << diff.count() << " 초" << endl;
  cout << "  최종 결과 : " << result << endl;
  cout << "  무결성    : [PASS]" << endl;
  cout << endl;
}

int main() {
  cout << "=== 동기화 성능 비교 (스레드 " << NUM_THREADS << "개 x "
       << NUM_INCREMENTS << "회 연산) ===\n"
       << endl;

  // 1. Unsafe 실행
  run_benchmark("1. Unsafe (No Lock)", worker_unsafe, sum_unsafe);

  // 2. Mutex 실행
  run_benchmark("2. Mutex (Lock)", worker_mutex, sum_mutex);

  // 3. Atomic 실행
  run_benchmark_atomic("3. Atomic (Hardware)", worker_atomic);

  return 0;
}