import mlflow
import os

mlflow.set_experiment("J.A.R.V.I.S_Cpp_LLM")

model_path = os.path.expanduser("~/Desktop/Meta-Llama-3.1-8B-Instruct-Q4_K_M.gguf")
model_name = "Llama-3-Quantized-CPP"

print(f"MLOps: Registring {model_name}")

with mlflow.start_run() as run:
    mlflow.log_param("engine", "llama.cpp-custom-build")
    mlflow.log_param("quantization", "q4_k_m")
    mlflow.log_param("context_window", 8192)
    mlflow.log_param("system_prompt_version", "v2-tools-enabled")

    if os.path.exists(model_path):
        mlflow.log_artifact(model_path, artifact_path = "model")

        print(f"Model uploaded to registry")
    else : print(f"Errr: Model not found at {model_path}")

    print(f"Experiment ID: {run.info.run_id}")
    print("To view: Run 'mlflow ui' in terminal")
    
