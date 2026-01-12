require 'socket'

# 서버 설정
HOST = '127.0.0.1'
PORT = 9000
CLIENT_COUNT = 6 # 테스트를 위해 인원 조정

def run_mock_client(client_id)
  begin
    socket = TCPSocket.new(HOST, PORT)
    
    # 1. 환영 메시지 수신 (Welcome message)
    # 서버가 접속하자마자 보내주는 메시지를 읽어냄
    welcome_msg = socket.recv(1024)
    puts "[Client #{client_id}] Connected. Server said: #{welcome_msg.strip}"

    # 2. 방 선택 (짝수는 1번방, 홀수는 2번방)
    room_num = (client_id % 2) + 1
    sleep(0.5)

    # 3. /join 명령어 전송
    join_cmd = "/join #{room_num}"
    socket.write(join_cmd)
    puts "[Client #{client_id}] Command Sent: #{join_cmd}"

    # 4. 방 이동 확인 메시지 수신
    move_msg = socket.recv(1024)
    puts "[Client #{client_id}] Server said: #{move_msg.strip}"

    sleep(1.0)

    # 5. 채팅 메시지 전송
    chat_msg = "Hello neighbors in Room #{room_num}! I am Client #{client_id}.\n"
    socket.write(chat_msg)
    puts "[Client #{client_id}] Chat Sent: #{chat_msg}"

    # 6. 다른 사람의 메시지를 받기 위해 잠시 대기
    # 실제로는 별도 스레드로 읽어야 하지만, 테스트용으로 잠시 열어둠
    sleep(3.0)
    
    socket.close
    puts "[Client #{client_id}] Disconnected."

  rescue Errno::ECONNREFUSED
    puts "[Client #{client_id}] Error: Connection Refused."
  rescue StandardError => e
    puts "[Client #{client_id}] Error: #{e.message}"
  end
end

puts "=== Room System Test Start (Target: #{HOST}:#{PORT}) ==="
puts "Clients will split into Room 1 and Room 2.\n\n"

threads = []

(0...CLIENT_COUNT).each do |i|
  threads << Thread.new { run_mock_client(i) }
  sleep(0.2) # 순차적 접속 유도
end

threads.each(&:join)

puts "\n=== Test Complete ==="