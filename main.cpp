#include <iostream>
#include "llama.h"
#include <vector>
#include <string>
#include <ctime>
#include <sstream>
#include <regex>
#include <filesystem>	
#include <fstream> 

namespace fs = std::filesystem;

using std::cout; 
using std::cerr;
using std::cin;
using std::endl;
using std::flush;

std::string read_sys_files(const std::string& folder){
	std::string knowledge;

	if(!fs::exists(folder)) return knowledge;	

	for (const auto& entry : fs::directory_iterator(folder)) {

		if(!entry.is_regular_file()) continue;
		if(entry.path().extension() != ".txt") continue;

		std::ifstream file(entry.path());	// opens file ; closes automatically
		if(!file) continue;

		// reads entire file into mempry; no loop; no buffer overflwo ; no manual size tracking;
		std::string content(
			(std::istreambuf_iterator<char> (file)),
			std::istreambuf_iterator<char>()
		);

		knowledge += "Context (" + entry.path().filename().string() +"):\n" + content + "\n\n";
		
	}



	return knowledge;
}

std::string system_prompt(const std::string &data_folder = "data") {

    std::string knowledge = read_sys_files(data_folder);

	return std::string("<|begin_of_text|>\n")
         + "<|start_header_id|>system<|end_header_id|>\n\n"
         + "You are J.A.R.V.I.S, a local AI assistant.\n"
         + "Hitesh Bhatnagar is your creator.\n\n"
         // Inject the Background Knowledge
         + "BACKGROUND KNOWLEDGE:\n"
         + knowledge + "\n\n"
         + "RULES & TOOLS:\n"
         + "1. Use BACKGROUND KNOWLEDGE to answer questions about Hitesh.\n"
         + "2. If asked about the current DATE or TIME, output EXACTLY this tag: @@TIME@@\n"
         + "   (Example: 'The current time is @@TIME@@')\n"
         + "3. To create a file, use EXACTLY this format:\n"
         + "   @@FILE: path/to/file.ext@@\n"
         + "   FILE CONTENT\n"
         + "   @@END@@\n"
         + "   (Do not write paths like /app/ or C:/, just the filename)\n"
         + "4. For general questions (math, coding, physics), answer normally.\n"
         + "<|eot_id|>";
}



std::string user_prompt(const std::string& input){
	return std::string("<|start_header_id|>user<|end_header_id|>\n\n")+ 
	input +
	std::string("\n<|eot_id|>\n<|start_header_id|>assistant<|end_header_id|>\n\n");
}

// taking fill LLM output , searching for tool and executing 
void execute_tools(const std::string& response){

	std::regex file_regex(
		R"(@@FILE:\s*(.*?)@@([\s\S]*?)@@END@@)"
	/*	
		@@FILE: 	->		Tool start
		\s*			->		ignore spaces
		(.*?)		-> 		Capture filename
		@@			-->		end filename
		([\s\S]*?)	->		capture all content including new lines
		@@END@@		-> 	tool end
	*/
	);

	std::smatch match;	// match[1] => filename ; match[2] => file content see line 51 & 52
	std::string text = response;

	if(!fs::exists("output")){
        try {
            fs::create_directory("output");
        } catch (...) {
            std::cerr << "[TOOL] Error: Could not create 'output' directory.\n";
            return;
        }
    }
	

	while(std::regex_search(text, match, file_regex))	{		// multiple tools in one response

		std::string org_filename = match[1];
		std::string content  = match[2];

		org_filename.erase(0, org_filename.find_first_not_of(" \t\n\r"));
		org_filename.erase(org_filename.find_last_not_of(" \t\n\r") + 1);

		std::string clean_file = fs::path(org_filename).filename().string();

		std::string full_path = "output/" + clean_file;
		// removes leading new line from content
		if(!content.empty() && content[0] == '\n') content.erase(0,1);
		cout << "\n [TOOL] Saving to host => : "<< full_path << " ...";
		
// 
// 		cout <<"\n[TOOL] Creating File : " << filename << endl;
// 
// 		if(filename.find("..") != std::string::npos){
// 			cerr<< "[TOOL] Path traversal blocked\n";
// 
// 			text = match.suffix().str();
// 			continue;
// 			
// 		}
// 
// 		fs::path p(filename);	// creating object that represents addr of file
// 		
// 		// creating parent direc if they don't exist
// 		if(p.has_parent_path()){
// 			std::error_code error;
// 			fs::create_directories(p.parent_path(), error);
// 
// 			if(error){
// 				cerr << "[TOOL] Failed to create parent directories :( " << error.message() << endl;
// 
// 				text = match.suffix().str();
// 				continue;
// 			}
// 		}

		std::ofstream file(full_path, std::ios::binary); 

		
		if(!file) cerr << "[TOOL] Failed to create file :( " << endl;
		else {
			file <<content;
			file.close();
			cout << "[TOOL] File written successfully :) " << endl;
			
		}

		text = match.suffix().str();
	}
}

std::string date_time() {
	auto now = std::chrono::system_clock::now();

	std::time_t now_time = std::chrono::system_clock::to_time_t(now);

	char buffer[100];
	std::strftime(buffer, sizeof(buffer), "%A, %B %d, %Y | %H:%M:%S", std::localtime(&now_time));
	return std::string(buffer);
}

int main(int argc, char** argv){

	if(argc < 2){
		cerr << "Usage : ./nano_rag <model_path> \n";
		return 1; 
	}

	cout << "Starting J.A.R.V.I.S \n";

	std::error_code ec;
	auto curr = fs::current_path(ec);
	if(!ec) cerr << "[RAG DEBUG] current working directory: " << curr << "\n";
	else cerr << "[RAG DEBUG] Failed to get current path: " << ec.message() << "\n";
	cerr << "[RAG DEBUG] data folder exists? " << (fs::exists("data") ? "YES" : "NO") << "\n";

	// loading model

	// creates context configuration
	// which contains : context window size ; batching behaviour ; threading 
	llama_model_params model_params = llama_model_default_params();

	// loading model into RAM/VRAM ; parses weights ; prepares tensors
	llama_model* model = llama_model_load_from_file(argv[1], model_params);

	if(!model){
		cerr << "Failed to load model !!! >..< \n";
		return 1;
	}

	cout << "Model loaded successfully :3 \n";

	// create context
	llama_context_params context_params = llama_context_default_params();

	context_params.n_ctx = 8192;	// max number of tokens model can remember;

	// creating runtime state : allocates KV cache ; initializes attention buffers 
	llama_context* context = llama_init_from_model(model, context_params);

	if(!context)  {		// check  RAM is enough 
		cerr <<"Failed to create context \n";
		llama_model_free(model);
		return 1;

	}

	// get model's  voabulary object from the model
	const llama_vocab* vocab = llama_model_get_vocab(model);

	// SAMPLER
	llama_sampler* sampler = llama_sampler_chain_init(llama_sampler_chain_default_params());

	llama_sampler_chain_add(sampler, llama_sampler_init_top_k(40));
	llama_sampler_chain_add(sampler, llama_sampler_init_temp(0.7f));

	llama_sampler_chain_add(sampler, llama_sampler_init_dist(std::time(nullptr)));
	

	std::string text = system_prompt();

	// allocate token buffer
	std::vector<llama_token> tokens(text.length() + 2);

	// tokenize
	// NOTE : Vocabulary is C-based not C++ 
	int n_tokens = llama_tokenize(
		vocab,
		text.c_str(),	// C-type string pointer
		text.length(),	// number of bytes to read
		
		tokens.data(), // pointer to raw token buffer
		tokens.size(),	// max tokens allowed
		false,	// add special tokens
		true		// parse special tokens
	);

	if(n_tokens < 0){
		cout  << "Token buffer too small \n";
		tokens.resize(-1 * n_tokens);
		n_tokens = llama_tokenize(
			vocab,
			text.c_str(),
			text.length(),
			tokens.data(),
			tokens.size(),
			true,
			true
		);
	}


	// shrinks vector to actual token count; avoids garbage tokens ; avoids decoding errors later
	tokens.resize(n_tokens);


	// create batch for decoding 
	llama_batch batch = llama_batch_init(context_params.n_ctx, 0, 1);

	for(int i = 0; i < n_tokens; i++){
		batch.token[i] = tokens[i];
		batch.pos[i] = i;				// positional embeddings like Hello -> pos 0 and world -> pos 1
		batch.n_seq_id[i] = 1;	// number of sequences this token belongs to 
		batch.seq_id[i][0] = 0;
		batch.logits[i] = false;		
	}

	batch.logits[n_tokens  -1] = true;	// tells model to o/p the logits (raw pred scores) only for very last token in the batch.
	// there are n_tokens valid entried in this batch
	batch.n_tokens = n_tokens;

	// decoding
	if(llama_decode(context, batch ) != 0){
		cerr << "Decoding failed!!!\n";
		llama_batch_free(batch);
		llama_free(context);
		llama_model_free(model);

		return 1;
	}

	int n_past = n_tokens;
	batch.n_tokens = 0;	// reusing the batch 

	// reset sampler internal state after feeding system prompt
	llama_sampler_reset(sampler);

	// CHAT loop
	std::string input;

	while(1){
		cout << "\n>> ";
		std::getline(std::cin, input);

		if(input == "exit") break;
		if(input.empty()) continue;

		std ::string prompt= user_prompt(input);

		std::vector<llama_token> user_tokens(prompt.size() + 2);

		int n_user = llama_tokenize(
			vocab, 
			prompt.c_str(),
			prompt.size(),
			user_tokens.data(),
			user_tokens.size(),
			false,
			true
		);

		if(n_user < 0){
			user_tokens.resize(-1*n_user);
			
			n_user = llama_tokenize(
				vocab, 
				prompt.c_str(),
				prompt.size(),
				user_tokens.data(),
				user_tokens.size(),
				false,
				true
			);
		}
		
		user_tokens.resize(n_user);

		// feed user prompt
		for(int i = 0; i < n_user;i++){
			batch.token[batch.n_tokens] = user_tokens[i];
			batch.pos[batch.n_tokens] = n_past;
			batch.n_seq_id[batch.n_tokens ] = 1;

			batch.seq_id[batch.n_tokens][0] = 0;

			bool logit_checker_for_last_user_token = (i == n_user - 1);
			batch.logits[batch.n_tokens] = logit_checker_for_last_user_token;

			batch.n_tokens++;
			n_past++;

			if(n_past >= context_params.n_ctx - 1){
				cerr << "\n [SYSTEM] Context limit reached. Program restart required.\n";
				goto clean;
			}
		}

		if(llama_decode(context, batch) != 0) goto clean;

		batch.n_tokens =  0;

		// generation loop 

		std::string full_response;
		bool hide_stuff = false;
		std::string tool_buffer;
		
		static std::string visible_printed;
		visible_printed.clear();		// reset whatever printed so far for this resp
		
		for(int i = 0; i < 256; i++){
		
			llama_token token = llama_sampler_sample(sampler, context, -1);

			if(llama_vocab_is_eog(vocab, token)) break;

			char buffer[256];
			int length = llama_token_to_piece(vocab, token , buffer, sizeof(buffer), 0, true);
// 
// 			if(length > 0){
// 				std::string piece(buffer, length);
// 				
// 				full_response += piece;	
// 				if(piece.find("@@FILE:") != std::string::npos) hide_stuff = true; // std::string::npos is a static const representing largest value for size_t if searching failed or no position found
// 
// 				if(!hide_stuff) cout << piece << flush;
// 				else tool_buffer += piece;
// 
// 				if(piece.find("@@END@@") != std::string::npos) {
// 					hide_stuff= false;
// 					tool_buffer.clear();
// 				}
// 				
// 			}

			if(length > 0){
				std::string piece(buffer, length);

				full_response += piece; 

				if(piece.find("@@FILE:") != std::string::npos) hide_stuff = true;
				if(piece.find("@@END@@") != std::string::npos) {
					hide_stuff = false;
					tool_buffer.clear();
				}

				// print logic
				if(!hide_stuff) cout << piece << flush;
				else tool_buffer += piece;

				if(full_response.length() >= 8) {
					std::string suffix = full_response.substr(full_response.length() - 8);
					
					if(suffix == "@@TIME@@"){
						std::string real_time = date_time();						
						for(int k=0; k<8; k++) cout << "\b \b" << flush;
						cout << real_time << flush;
						full_response.resize(full_response.length() - 8);
						full_response += real_time;
					}
				}
			}


			llama_sampler_accept(sampler, token);

			batch.token[0] = token;
			batch.pos[0] = n_past;
			batch.n_seq_id[0] = 1;
			batch.seq_id[0][0] = 0;

			batch.logits[0] = true;
			batch.n_tokens = 1;
			n_past++;

			if(llama_decode(context, batch) != 0) break;
			batch.n_tokens = 0;
	

			
		}
		cout << endl;
		
		if(!full_response.empty()) execute_tools(full_response);

	}	

	clean:
	llama_sampler_free(sampler);
	llama_batch_free(batch);
	llama_free(context);
	llama_model_free(model);
	return 0;
}
