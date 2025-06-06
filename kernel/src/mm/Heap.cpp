#include <cstddef>
#include <cstdint>

#include <shared/memory/defs.hpp>

#include <mm/PhysicalMemory.hpp>
#include <mm/VirtualMemory.hpp>

#include <screen/Log.hpp>

namespace ShdMem = Shared::Memory;

struct malloc_block_t {
	uint64_t height;
	malloc_block_t* left;
	malloc_block_t* right;
	uint64_t regionSize;
};

namespace {
    static constexpr size_t HEAP_ALIGN_V = 16;

	static malloc_block_t* firstBlock = nullptr;
}

namespace Heap {
	void* Allocate(size_t size) {
		static_assert(sizeof(malloc_block_t) % HEAP_ALIGN_V == 0);

		// align on a natural boundary
		if (size % HEAP_ALIGN_V != 0) {
			size += HEAP_ALIGN_V - (size % HEAP_ALIGN_V);
		}

		if (firstBlock == nullptr) {
			void* pages = nullptr;

			size_t allocatedPages = 16;

			if (size + sizeof(malloc_block_t) >= allocatedPages * ShdMem::FRAME_SIZE) {
				allocatedPages = (size + sizeof(malloc_block_t) + ShdMem::FRAME_SIZE - 1) / ShdMem::FRAME_SIZE;
			}
			pages = VirtualMemory::AllocateKernelHeap(allocatedPages);

			if (pages == nullptr) {
				return nullptr;
			}

			malloc_block_t* allocated = static_cast<malloc_block_t*>(pages);
			allocated->prev = allocated;
			allocated->size = size + sizeof(malloc_block_t);

			if (allocatedPages * ShdMem::FRAME_SIZE - size - sizeof(malloc_block_t) >= sizeof(malloc_block_t) + HEAP_ALIGN_V) {
				firstBlock = reinterpret_cast<malloc_block_t*>(static_cast<uint8_t*>(pages) + size + sizeof(malloc_block_t));
				firstBlock->prev = nullptr;
				firstBlock->next = nullptr;
				firstBlock->size = allocatedPages * ShdMem::FRAME_SIZE - size - sizeof(malloc_block_t);
				allocated->next = firstBlock;
			}
			else {
				allocated->next = nullptr;
			}

			return static_cast<uint8_t*>(pages) + sizeof(malloc_block_t);
		}

		malloc_block_t* currentBlock = firstBlock;
		while (currentBlock != nullptr) {
			if (currentBlock->size < size + sizeof(malloc_block_t)) {
				currentBlock = currentBlock->next;
				continue;
			}

			malloc_block_t* allocated = currentBlock;
			malloc_block_t* prev = allocated->prev;
			allocated->prev = allocated;
			const size_t remainingSize = allocated->size - (size + sizeof(malloc_block_t));
			allocated->size -= remainingSize;

			if (remainingSize >= sizeof(malloc_block_t) + HEAP_ALIGN_V) {
				currentBlock = reinterpret_cast<malloc_block_t*>(reinterpret_cast<uint8_t*>(allocated) + allocated->size);
				currentBlock->prev = prev;
				currentBlock->next = allocated->next;
				currentBlock->size = remainingSize;
				allocated->next = currentBlock;
				if (currentBlock->next != nullptr) {
					currentBlock->next->prev = currentBlock;
				}
				if (currentBlock->prev != nullptr) {
					currentBlock->prev->next = currentBlock;
				}
				if (firstBlock == allocated) {
					firstBlock = currentBlock;
				}
			}
			else {
				if (allocated->next != nullptr) {
					allocated->next->prev = allocated->prev;
				}
				if (allocated->prev != nullptr) {
					allocated->prev->next = allocated->next;
				}
				allocated->next = nullptr;
			}

			return allocated + 1;
		}

		return nullptr;
	}

	void Free(void* ptr) {
		if (ptr == nullptr) {
			return;
		}

		malloc_block_t* blockPtr = reinterpret_cast<malloc_block_t*>(static_cast<uint8_t*>(ptr) - sizeof(malloc_block_t));
		malloc_block_t* blockEndPtr = reinterpret_cast<malloc_block_t*>(reinterpret_cast<uint8_t*>(blockPtr) + blockPtr->size);

		if (blockPtr->next != nullptr) {
			if (blockEndPtr == blockPtr->next) {
				blockPtr->size += blockPtr->next->size;
				blockPtr->prev = blockPtr->next->prev;
				blockPtr->next = blockPtr->next->next;

				if (blockPtr->next != nullptr) {
					blockPtr->next->prev = blockPtr;
				}
				if (blockPtr->prev != nullptr) {
					blockPtr->prev->next = blockPtr;
				}
			}
			else {
				blockPtr->prev = blockPtr->next->prev;
				blockPtr->next->prev = blockPtr;
				blockPtr->prev->next = blockPtr;
			}

			while (blockPtr->prev != nullptr
				&& blockEndPtr == reinterpret_cast<malloc_block_t*>(
					reinterpret_cast<uint8_t*>(blockPtr->prev + blockPtr->prev->size)
			)) {
				blockPtr->prev->size += blockPtr->size;
				blockPtr->prev->next = blockPtr->next;
				blockPtr->next->prev = blockPtr->prev;
				blockPtr = blockPtr->prev;
			}

			if (blockPtr->prev == nullptr) {
				firstBlock = blockPtr;
			}
		}
		else {
			if (firstBlock == nullptr) {
				firstBlock = blockPtr;
				blockPtr->next = nullptr;
				blockPtr->prev = nullptr;
			}
			else {
				malloc_block_t* ptr = firstBlock;
				while (ptr->next != nullptr) {
					ptr = ptr->next;
				}
				ptr->next = blockPtr;
				blockPtr->prev = ptr;
				blockPtr->next = nullptr;
			}
		}
	}
}
