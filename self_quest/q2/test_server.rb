require 'socket'

# 서버 설정
HOST = '127.0.0.1'
PORT = 9000
CLIENT_COUNT = 10 # 동시에 접속할 클라이언트 수

def run_mock_client(client_id)
  begin
    # 1. 소켓 생성 및 연결
    socket = TCPSocket.new(HOST, PORT)
    puts "[Client #{client_id}] 연결 성공!"

    # 2. 랜덤한 시간 대기 (사람처럼 행동)
    # Ruby에서 float 범위 랜덤: rand * (max - min) + min
    sleep(rand * (2.0 - 0.5) + 0.5)

    # 3. 메시지 전송
    msg = "Hello from Client #{client_id}"
    socket.write(msg)
    puts "[Client #{client_id}] 메시지 전송: #{msg}"

    # 4. 잠시 유지 (서버가 다중 접속을 유지하는지 확인)
    sleep(rand * (3.0 - 1.0) + 1.0)

    # 5. 연결 종료
    socket.close
    puts "[Client #{client_id}] 연결 종료"

  rescue Errno::ECONNREFUSED
    puts "[Client #{client_id}] Error: 서버에 연결할 수 없습니다. 서버가 켜져 있나요?"
  rescue StandardError => e
    puts "[Client #{client_id}] Error: #{e.message}"
  end
end

puts "=== 자동화 테스트 시작 (Target: #{HOST}:#{PORT}) ==="
puts "총 #{CLIENT_COUNT}명의 클라이언트를 생성합니다.\n\n"

threads = []

# 여러 클라이언트를 동시에 실행 (멀티스레딩)
(0...CLIENT_COUNT).each do |i|
  threads << Thread.new { run_mock_client(i) }
  # 너무 동시에 붙으면 OS가 거부할 수 있으니 아주 살짝 텀을 줌
  sleep(0.1)
end

# 모든 스레드가 끝날 때까지 대기
threads.each(&:join)

puts "\n=== 테스트 완료 ==="