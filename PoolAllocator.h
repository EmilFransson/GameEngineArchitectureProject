#pragma once
template<typename T>
struct PoolChunk
{
	alignas(alignof(T))
	std::byte data[sizeof(T)];
	PoolChunk<T>* nextPoolChunk;
};

template<class T>
class PoolAllocator
{
public:
	PoolAllocator(const char* tag, const uint64_t entityCapacity = 1000000u);
	~PoolAllocator();
	template<typename... Arguments>
	T* New(Arguments&&... args);
	void Delete(T* pData);
	[[nodiscard]] const char* GetTag() const noexcept;
	[[nodiscard]] const uint64_t GetByteUsage() const noexcept;
	[[nodiscard]] const uint64_t GetByteCapacity() const noexcept;
	[[nodiscard]] const uint64_t GetEntityUsage() const noexcept;
	[[nodiscard]] const uint64_t GetEntityCapacity() const noexcept;
	void FreeAllMemory(const std::vector<T*>& objects) noexcept;
private:
	PoolChunk<T>* m_pMemoryPool;
	PoolChunk<T>* m_pHead;
	const char* m_Tag;
	uint64_t m_MaxEntities;
	uint64_t m_BytesCapacity;
	uint64_t m_UsedBytes;
	uint64_t m_NrOfEntities;
	std::mutex m_Lock;
};

template<class T>
PoolAllocator<T>::PoolAllocator(const char* tag, const uint64_t entityCapacity)
	: m_Tag{tag}, 
	  m_MaxEntities{ entityCapacity }, 
	  m_BytesCapacity{ sizeof(T) * m_MaxEntities },
	  m_UsedBytes{0u},
	  m_NrOfEntities{0u}
{
	m_pMemoryPool = DBG_NEW PoolChunk<T>[m_MaxEntities];
	m_pHead = m_pMemoryPool;

	for (uint64_t i{ 0u }; i < m_MaxEntities - 1; i++)
	{
		m_pMemoryPool[i].nextPoolChunk = std::addressof(m_pMemoryPool[i + 1]);
	}
	m_pMemoryPool[m_MaxEntities - 1].nextPoolChunk = nullptr;
}

template<class T>
PoolAllocator<T>::~PoolAllocator()
{
	delete[] m_pMemoryPool;
	m_pMemoryPool = nullptr;
	m_pHead = nullptr;
}

template<class T>
template<typename ...Arguments>
T* PoolAllocator<T>::New(Arguments&&... args)
{
	std::lock_guard<std::mutex> lock(m_Lock);
	if (m_pHead == nullptr)
		return nullptr;

	PoolChunk<T>* pPoolChunk = m_pHead;
	m_pHead = m_pHead->nextPoolChunk;

	m_UsedBytes += sizeof(T);
	m_NrOfEntities++;

	return new(std::addressof(pPoolChunk->data))T(std::forward<Arguments>(args)...);
}

template<class T>
void PoolAllocator<T>::Delete(T* pData)
{
	using pointer = T*;
	std::lock_guard<std::mutex> lock(m_Lock);
	m_UsedBytes -= sizeof(T);
	pData->~T();
	PoolChunk<T>* poolChunk = reinterpret_cast<PoolChunk<T>*>(pData);
	poolChunk->nextPoolChunk = m_pHead;
	m_pHead = poolChunk;
	m_NrOfEntities--;
}

template<typename T>
const char* PoolAllocator<T>::GetTag() const noexcept
{
	return m_Tag;
}

template<class T>
const uint64_t PoolAllocator<T>::GetByteUsage() const noexcept
{
	return m_UsedBytes;
}

template<class T>
const uint64_t PoolAllocator<T>::GetByteCapacity() const noexcept
{
	return m_BytesCapacity;
}

template<class T>
const uint64_t PoolAllocator<T>::GetEntityUsage() const noexcept
{
	return m_NrOfEntities;
}

template<class T>
const uint64_t PoolAllocator<T>::GetEntityCapacity() const noexcept
{
	return m_MaxEntities;
}

/*The vector and pool allocator must "match"!*/
template<class T>
void PoolAllocator<T>::FreeAllMemory(const std::vector<T*>& objects) noexcept
{
	std::lock_guard<std::mutex> lock(m_Lock);
	for (uint64_t i{ 0u }; i < m_NrOfEntities; i++)
	{
		Delete(objects[i]);
	}
}
