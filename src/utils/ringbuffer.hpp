//brief Simple SPSC ring buffer implementation
#pragma once

#include <stdint.h>
#include <limits>
#include <atomic>


//brief Lock free, with no wasted slots ringbuffer implementation

//tparam T Type of buffered elements
//tparam buffer_size Size of the buffer. Must be a power of 2.
//tparam cacheline_size Size of the cache line, to insert appropriate padding in between indexes and buffer
	  
template<typename T, size_t buffer_size = 16, size_t cacheline_size = 64>
class ringbuffer
{
public:
	//brief Default constructor, will initialize head and tail indexes	 
	ringbuffer() : head(0), tail(0) {}

	//brief Special case constructor to premature out unnecessary initialization contract when object is
	//instatiated in .bss section
	//warning If object is instantiated on stack, heap or inside noinit section then the contents have to be
	//explicitly cleared before use
	//param dummy Ignored	 
	ringbuffer(int dummy) { (void)(dummy); }

	//brief Clear buffer from producer side
	//warning function may return without performing any action if consumer tries to read data at the same time
	void producerClear()
	{
		// head modification will lead to underflow if cleared during consumer read
		// doing this properly with CAS is not possible without modifying the consumer contract
		consumerClear();
	}

	//brief Clear buffer from consumer side
	void consumerClear()
	{
		tail.store(head.load(std::memory_order_relaxed), std::memory_order_relaxed);
	}

	//brief Check if buffer is empty
	//return True if buffer is empty
	bool isEmpty() const
	{
		return readAvailable() == 0;
	}

	//brief Check how many elements can be read from the buffer
	//return Number of elements that can be read
	size_t readAvailable() const
	{
		return head.load(std::memory_order_acquire) - tail.load(std::memory_order_relaxed);
	}

	//brief Check if buffer is full
	//return True if buffer is full
	bool isFull() const
	{
		return writeAvailable() == 0;
	}
	
	//brief Check how many elements can be written into the buffer
	//return Number of free slots that can be be written
	size_t writeAvailable() const
	{
		return buffer_size - (head.load(std::memory_order_relaxed) - tail.load(std::memory_order_acquire));
	}

	//brief Inserts data into internal buffer, without blocking
	//param data element to be inserted into internal buffer
	//return True if data was inserted
	bool insert(T data)
	{
		size_t tmp_head = head.load(std::memory_order_relaxed);
		if ((tmp_head - tail.load(std::memory_order_acquire)) == buffer_size)
			return false;
		else
		{
			data_buff[tmp_head++ & buffer_mask] = data;
			std::atomic_signal_fence(std::memory_order_release);
			head.store(tmp_head, std::memory_order_release);
		}
		return true;
	}

	//brief Inserts data into internal buffer, without blocking
	//param[in] data Pointer to memory location where element, to be inserted into internal buffer, is located
	//return True if data was inserted
	bool insert(const T* data)
	{
		size_t tmp_head = head.load(std::memory_order_relaxed);
		if ((tmp_head - tail.load(std::memory_order_acquire)) == buffer_size)
			return false;
		else
		{
			data_buff[tmp_head++ & buffer_mask] = *data;
			std::atomic_signal_fence(std::memory_order_release);
			head.store(tmp_head, std::memory_order_release);
		}
		return true;
	}
	
	//brief Removes single element without reading
	//return True if one element was removed
	bool remove()
	{
		size_t tmp_tail = tail.load(std::memory_order_relaxed);
		if (tmp_tail == head.load(std::memory_order_relaxed))
			return false;
		else
			tail.store(++tmp_tail, std::memory_order_release); // release in case data was loaded/used before

		return true;
	}
	
	//brief Reads one element from internal buffer without blocking
	//param[out] data Reference to memory location where removed element will be stored
	//return True if data was fetched from the internal buffer
	bool remove(T& data)
	{
		return remove(&data); // references are anyway implemented as pointers
	}

	//brief Reads one element from internal buffer without blocking
	//param[out] data Pointer to memory location where removed element will be stored
	//return True if data was fetched from the internal buffer
	bool remove(T* data)
	{
		size_t tmp_tail = tail.load(std::memory_order_relaxed);
		if (tmp_tail == head.load(std::memory_order_acquire))
			return false;
		else
		{
			*data = data_buff[tmp_tail++ & buffer_mask];
			std::atomic_signal_fence(std::memory_order_release);
			tail.store(tmp_tail, std::memory_order_release);
		}
		return true;
	}


private:
	constexpr static size_t buffer_mask = buffer_size - 1; //!< bitwise mask for a given buffer size

	alignas(cacheline_size) std::atomic<size_t> head; //!< head index
	alignas(cacheline_size) std::atomic<size_t> tail; //!< tail index

	// put buffer after variables so everything can be reached with short offsets
	alignas(cacheline_size) T data_buff[buffer_size]; //!< actual buffer

	// let's assert that no UB will be compiled in
	static_assert((buffer_size != 0), "buffer cannot be of zero size");
	static_assert((buffer_size & buffer_mask) == 0, "buffer size is not a power of 2");

	static_assert(std::numeric_limits<size_t>::is_integer, "indexing type is not integral type");
	static_assert(!(std::numeric_limits<size_t>::is_signed), "indexing type shall not be signed");
	static_assert(buffer_mask <= ((std::numeric_limits<size_t>::max)() >> 1),
		"buffer size is too large for a given indexing type (maximum size for n-bit type is 2^(n-1))");
};
