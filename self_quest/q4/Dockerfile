# Alpine 기반 최소 이미지 (약 50MB)
FROM alpine:3.19

# 빌드 도구 설치
RUN apk add --no-cache \
    g++ \
    musl-dev \
    ruby

WORKDIR /app

# 소스 복사
COPY epoll_echo_server.cpp .
COPY stress_test.rb .

# 컴파일
RUN g++ -o epoll_server epoll_echo_server.cpp -std=c++11 -O2

EXPOSE 9000

CMD ["./epoll_server", "9000"]
