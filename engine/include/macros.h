#pragma once

#define LOAD_SHADER_MODULE(file, shader_ref) (!load_shader_module(file, &shader_ref)) ? spdlog::error("Error when building shader file: {}", file) :  spdlog::info("Shader ({}) successfully loaded", file);

using namespace std;
#define VK_CHECK(x)                                                 	\
	do                                                              	\
	{                                                               	\
		VkResult err = x;                                           	\
		if (err)                                                    	\
		{                                                           	\
			spdlog::error("Detected Vulkan error: {}", err);			\
			abort();                                                	\
		}                                                           	\
	} while (0) 
