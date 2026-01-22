#!/usr/bin/env ruby
# ⚔️ Quest 4: Stress Test Script
#
# 다수의 동시 접속 클라이언트를 생성하여 서버 성능 테스트
#
# 사용법: ruby stress_test.rb [host] [port] [num_clients]
# 예시: ruby stress_test.rb 127.0.0.1 9000 100

require 'socket'
require 'timeout'

HOST = ARGV[0] || '127.0.0.1'
PORT = (ARGV[1] || 9000).to_i
NUM_CLIENTS = (ARGV[2] || 100).to_i
MESSAGES_PER_CLIENT = 10

puts "=========================================="
puts "  Quest 4: Stress Test"
puts "=========================================="
puts "[*] Target: #{HOST}:#{PORT}"
puts "[*] Clients: #{NUM_CLIENTS}"
puts "[*] Messages per client: #{MESSAGES_PER_CLIENT}"
puts ""

# 결과 저장
results = {
  connected: 0,
  failed_connect: 0,
  messages_sent: 0,
  messages_received: 0,
  errors: 0
}
mutex = Mutex.new

# 시작 시간
start_time = Time.now

# 클라이언트 스레드 생성
threads = NUM_CLIENTS.times.map do |i|
  Thread.new do
    begin
      # 연결
      socket = TCPSocket.new(HOST, PORT)
      mutex.synchronize { results[:connected] += 1 }
      puts "[*] Client #{i} connected."

      # 메시지 송수신
      MESSAGES_PER_CLIENT.times do |j|
        message = "Client#{i}_Message#{j}\n"
        socket.write(message)
        mutex.synchronize { results[:messages_sent] += 1 }

        begin
          Timeout.timeout(5) do
        # Echo 응답 대기
            response = socket.gets
            if response
              mutex.synchronize { results[:messages_received] += 1 }
            else
              mutex.synchronize { results[:errors] += 1 }
            end
          end
        end
      end


      puts "[*] Client #{i} completed."
      socket.close
    rescue => e
      mutex.synchronize do
        results[:failed_connect] += 1
        results[:errors] += 1
      end
      # puts "[!] Client #{i} error: #{e.message}"
    end
  end
end

# 모든 스레드 완료 대기
threads.each(&:join)

# 종료 시간
end_time = Time.now
elapsed = end_time - start_time

puts "=========================================="
puts "  Results"
puts "=========================================="
puts "[*] Time elapsed: #{elapsed.round(2)} seconds"
puts "[*] Connections successful: #{results[:connected]}/#{NUM_CLIENTS}"
puts "[*] Connection failures: #{results[:failed_connect]}"
puts "[*] Messages sent: #{results[:messages_sent]}"
puts "[*] Messages received: #{results[:messages_received]}"
puts "[*] Errors: #{results[:errors]}"

if results[:messages_sent] > 0
  throughput = results[:messages_sent] / elapsed
  puts "[*] Throughput: #{throughput.round(2)} msg/sec"
end

# 성공률 계산
success_rate = (results[:messages_received].to_f / results[:messages_sent] * 100).round(2) rescue 0
puts "[*] Success rate: #{success_rate}%"
