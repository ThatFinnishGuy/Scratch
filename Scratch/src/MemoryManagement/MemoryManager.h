#pragma once
#include "StackAllocator.h"

//Memory Manager Singleton class which is responsible for initializing and de-initializing custom allocators.
class MemoryManager {
	
	static MemoryManager *instance;
	MemoryManager() {};

public:
	static MemoryManager *getInstance() {
		if (!instance)
			instance = new MemoryManager;
		return instance;
	}

	

};