#	NANO-LLAMA-EDGE

## What am I building ?
	A local AI assistant in C++ using llama.cpp

	Final capabilities :
		* Load an LLM from disk
		* Accept user input
		* Generate text token by token
		* Detect structured commands in output
		* Execute real actions (create files, write programs, NLP tasks ...)

### CMakeLists.txt 

	Using CMake FetchContent to vendor `llama.cpp` at build time, compile it as a static dependency, and link it cleanly using modern target-based CMake without manual include paths.

## CONTEXT in LLMs

	Only loading a model is not enough you need context.

### What is a CONTEXT 

	In transformer LLMs :
		* The model = weights (static)
		* The context = runtime state

	The context holds :
		* KV cahce (attention memory)
		* token history
		* positional information

	ANALOGY : 

		Model 		-> 		Brain
		Context 	->		Short Term Memory	 


	Without context :
	
		* No inference
		* No text generation

## How to manage memory in llama.cpp ?

	llama.cpp separates static model weights and runtime context. Using `llama_free()` and `llama_model_free()`. This allows reuse of a single model across multiple contexts and avoids uncessary reloads.

## TOKENIZATION	

	A token is :
		* Integer ID
		* Representing a piece of text

	NOTE : They are not words or characters they come from a vocabulary.

### VOCABULARY

	Each model has a vocabulary : mapping `token_id -> text_piece`


## DECODING

	Till now the model has loaded weights ; created the context; converted text -> tokens

	but the model has not processed anything yet.

	Decoding => tokens are fed into transformer -> KV cache is built -> logits are produced

###	In transformer based LLMs :

	* You give the model tokens
	
	* The model runs :
		* Vector Embeddings	
		* Attention
		* Feed fwd layers

	* The model updates :
		* KV cache (memory)

	* The model produces : 
		* logits (probs for nxt token)

###	Important

	1. Model	-->		static weights
	2. Context	-->		Runtime memory (KV cache)
	3. Decode 	-->		Forward pass
	4. Logits 	-->		Raw probabilities
	5. Sampling	-->		Choosing nxt token

## SAMPLING

###	What are logits ?

	* one floating pt score per token in vocab.
	* higher score = more likely nxt token
	* not probs yet

*NOTE : DECODING computes probabilities; SAMPLING makes decisions* 

## CHAT LOOP + ROLE BASED PROMPTS

	* Intoduce roles (system/user/assistant);
	* A chat loop
	* preserve conservation memory

