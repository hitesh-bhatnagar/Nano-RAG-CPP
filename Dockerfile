FROM ubuntu:22.04 AS builder
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y build-essential cmake git wget && rm -rf /var/lib/apt/lists/*
WORKDIR /app

COPY CMakeLists.txt .
COPY main.cpp .

RUN mkdir build && cd build && \
    cmake -DBUILD_SHARED_LIBS=OFF -DLLAMA_SHARED=OFF .. && \
    make -j$(nproc)

FROM ubuntu:22.04

# install timezone data AND libgomp1 
RUN apt-get update && apt-get install -y \
    tzdata \
    libgomp1 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY --from=builder /app/build/nano_rag .

RUN mkdir data models output
COPY data/ ./data/

ENTRYPOINT ["./nano_rag"]
CMD ["/app/models/model.gguf"]
