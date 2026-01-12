require 'socket'

# 서버 설정
HOST = '127.0.0.1'
PORT = 9000

# 1. 성실한 유저 (Active User)
# 1초마다 메시지를 보내서 5초 타임아웃을 피함
def run_active_client(client_id)
  begin
    socket = TCPSocket.new(HOST, PORT)
    puts "[Active Client #{client_id}] Connected."

    # 10초 동안 생존 시도
    10.times do |i|
      socket.write("Ping #{i}")
      # puts "[Active Client #{client_id}] Sent Heartbeat (#{i})"
      sleep(1.0)
    end
    
    socket.close
    puts "[Active Client #{client_id}] Finished successfully."

  rescue => e
    puts "[Active Client #{client_id}] Died unexpectedly: #{e.message}"
  end
end

# 2. 유령 유저 (Zombie User)
# 접속 후 아무 말도 안 하고 7초간 잠수 -> 강퇴당해야 함
def run_zombie_client(client_id)
  begin
    socket = TCPSocket.new(HOST, PORT)
    puts "[Zombie Client #{client_id}] Connected. Sleeping..."

    # 환영 메시지 정도는 읽어줌
    socket.recv_nonblock(1024) rescue IO::WaitReadable

    # 7초간 잠수 (서버 타임아웃은 5초)
    sleep(7.0)

    # 잠수 후 메시지 보내보기 (이미 끊겨있어야 정상)
    socket.write("I am back!")
    puts "[Zombie Client #{client_id}] Error: Still alive? (Should be disconnected)"

  rescue Errno::ECONNRESET, Errno::EPIPE
    puts "[Zombie Client #{client_id}] Success: Disconnected by server (As expected)."
  rescue => e
    puts "[Zombie Client #{client_id}] Disconnected: #{e.class} - #{e.message}"
  end
end

puts "=== Heartbeat(Zombie) Test Start ==="
puts "Active clients should survive, Zombies should die.\n\n"

t1 = Thread.new { run_active_client(1) }
sleep(0.5)
t2 = Thread.new { run_zombie_client(2) }

t1.join
t2.join

puts "\n=== Test Complete ==="