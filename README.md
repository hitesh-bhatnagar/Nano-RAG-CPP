# Jarvis C++ Engine: Local RAG & Agentic AI

![C++](https://img.shields.io/badge/Language-C++17-blue?style=for-the-badge&logo=c%2B%2B)
![Docker](https://img.shields.io/badge/Deployment-Docker-2496ED?style=for-the-badge&logo=docker)
![Status](https://img.shields.io/badge/Status-Production_Ready-success?style=for-the-badge)

A custom-built **Inference Engine** for Large Language Models (LLMs) written efficiently in C++. 
Unlike Python wrappers, this project manages memory, sampling, and file I/O directly, featuring a built-in **RAG (Retrieval Augmented Generation)** system and **Agentic Tool Use**.

Here's the demo video showing the proof of work for the project [DEMO VIDEO](https://github.com/hitesh-bhatnagar/Nano-RAG-CPP/blob/main/POW.mp4) 
---

## Features

### 1. High-Performance C++ Runtime
Built directly on top of `llama.cpp` libraries. It handles tokenization, sampling (Top-K/Temp), and context management without the overhead of Python interpreters.

### 2. Native RAG System
The engine dynamically scans a local `data/` folder, ingests `.txt` files, and injects them into the model's system prompt before inference.
* **Result:** The AI "knows" information (like your bio) that wasn't in its training data.

### 3. Agentic File System Tools
The AI can manipulate the host operating system using a strict syntax protocol.
* **Capability:** It can write code, save logs, or create scripts on your actual computer (via Docker Volume mounting).
* **Safety:** Confined to a specific `output/` directory.

### 4. Production Deployment
Fully containerized with **Static Linking**. The image creates a consistent environment with:
* **Timezone Sync:** Matches the host's real time (e.g., IST).
* **Volume Mapping:** "Wormhole" between Container and Host storage.

---

## How to Run (One Command)

You do not need to compile the code. I have published the pre-built engine to Docker Hub.

**Prerequisites:**
1. [Docker Installed](https://www.docker.com/)
2. A `.gguf` model (e.g., [Llama-3-8B-Instruct](https://huggingface.co/Meta-Llama-3.1-8B-Instruct-GGUF))

**Run this command:**
```bash
docker run -it --rm \
  -e TZ="Asia/Kolkata" \
  -v ~/Desktop/Jarvis_Output:/app/output \
  -v /PATH/TO/YOUR/MODEL.gguf:/app/models/model.gguf \
  hitesh917/jarvis-cpp:latest
```
*NOTE : Replace /PATH/TO/YOUR/MODEL.gguf with the actual location*

---

## Build from Source

If you want to modify the C++ engine with a new model like mixtral or bert etc. 

```bash
# 1. Clone the repo
git clone [https://github.com/your_username/Jarvis-CPP-Engine.git](https://github.com/your_username/Jarvis-CPP-Engine.git)
cd Jarvis-CPP-Engine

# 2. Build (Requires CMake & G++)
mkdir build && cd build
cmake ..
make -j4

# 3. Run
./nano_rag ../models/model.gguf
```

## Project Structure

 => main.cpp          # Core Inference Loop & Tool Logic
 => Dockerfile        # Multi-stage build (Builder -> Static Binary -> Runtime)
 => CMakeLists.txt    # Build configuration
 => data/             # RAG Knowledge Base (Text files go here)
 => register_model.py # Utility for MLflow tracking

### References
https://www.datacamp.com/tutorial/llama-cpp-tutorial
https://medium.com/data-science/a-complete-guide-to-write-your-own-transformers-29e23f371ddd
https://medium.com/data-science/llama-cpp-writing-a-simple-c-inference-program-for-gguf-llm-models-12bc5f58505f
