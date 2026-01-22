⚔️ Quest Log: 스레드 동기화 연구소 (The Sync Lab)

🎯 퀘스트 목표

Race Condition 경험: 멀티스레드 환경에서 동기화 없이 공유 자원을 건드리면 어떤 재앙이 일어나는지 눈으로 확인한다.

Mutex 정복: std::mutex와 std::lock_guard를 사용하여 임계 영역(Critical Section)을 보호한다.

성능 비용 체감: 락(Lock)을 걸었을 때와 안 걸었을 때의 속도 차이를 측정한다.

📜 Sub-Quest List

1️⃣ [입문] 카오스 (Race Condition)

목표: "분명 10,000원을 입금했는데 잔고가 부족합니다?" 상황 만들기.

Task 1-1: 전역 변수 int sum = 0; 선언.

Task 1-2: 스레드 10개를 생성하여, 각자 sum을 100만 번씩 ++ (1증가) 시키는 함수 실행.

Task 1-3: 모든 스레드가 종료(join)된 후 sum 출력.

기대 결과: 10,000,000 (천만)

실제 결과: ??? (매번 다름)

2️⃣ [구현] 질서의 수호자 (Mutex)

목표: 자물쇠(Mutex)를 채워서 데이터 무결성 지키기.

Task 2-1: std::mutex 전역 객체 선언.

Task 2-2: sum++ 하는 부분을 mtx.lock()과 mtx.unlock()으로 감싸기 (임계 영역 설정).

Task 2-3: 다시 실행하여 결과가 정확히 10,000,000이 나오는지 확인.

3️⃣ [심화] 비용 계산 (Performance Cost)

목표: 안전에는 대가가 따른다. 락이 얼마나 비싼 비용인지 측정.

Task 3-1: Task 1-3(락 없음)과 Task 2-3(락 있음)의 실행 시간을 std::chrono로 정밀 측정.

Task 3-2: "Atomic 연산" (std::atomic)을 사용하여 Mutex보다 얼마나 빠른지 비교. (게임 서버에서 자주 쓰는 최적화 기법)

4️⃣ [보너스] 죽음의 포옹 (Deadlock)

목표: 서로가 서로의 자물쇠를 기다리며 영원히 멈추는 상황 재현.

Task 4-1: 두 개의 자원(Mutex A, Mutex B) 준비.

Task 4-2: 스레드 1은 A를 잠그고 B를 기다림.

Task 4-3: 스레드 2는 B를 잠그고 A를 기다림.

Task 4-4: 프로그램이 영원히 멈추는 현상 관찰.

🛠️ 기술적 포인트

Atomic성: sum++은 한 줄짜리 코드 같지만, 기계어로는 Load -> Add -> Store 3단계입니다. 이 중간에 다른 스레드가 끼어들면 값이 덮어씌워집니다.

Context Switching: 스레드가 많다고 무조건 좋은 게 아닙니다.

📚 참고 자료

배현직 게임 서버: Chapter 1 (멀티스레딩, 임계 영역, 교착 상태)

윤성우 TCP/IP: Chapter 18 (멀티쓰레드 기반의 서버구현)