FROM gcc:latest AS builder

WORKDIR /app

COPY . .

RUN make all

FROM debian:stable-slim

RUN useradd -m cuser
WORKDIR /home/cuser

COPY --from=builder /app/server /app/client ./
COPY data.txt ./

RUN chown -R cuser:cuser /home/cuser && \
    chmod +x server client

USER cuser

CMD ["./server"]