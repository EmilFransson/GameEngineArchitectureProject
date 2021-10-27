#pragma once
#include "Stack.h"
#include <stdint.h>
#include <memory>
#include <cstddef>
#include <iostream>
#include <string>
#include <assert.h>

class StackAllocator
{
    //Protected so that no one else can create StackAllocators.
protected:
    //Constructor is called only by CreateAllocator.
    StackAllocator(unsigned long long);
    //Called when we delete the allocator in the FreeMemory-function.
    ~StackAllocator();

public:
    //Remove the copy and assign.
    StackAllocator(StackAllocator& other) = delete;
    void operator=(const StackAllocator&) = delete;

    //Used to create the allocator if it does not exist.
    static void CreateAllocator(unsigned long long);
    static void FreeAllMemory();
    //Returns the instance of the singleton.
    static StackAllocator* GetInstance();
    
    //Get stack size and current stack size.
    size_t GetStackMaxSize();
    size_t GetStackCurrentSize();
    //Create a new object.
    template<typename T, typename... Arguments>
    T* New(Arguments&&... args);

    //Called at the end of our scope each frame to clear the stack.
    void CleanUp();

    void ToggleEnabled() noexcept;
    const bool IsEnabled() const noexcept;
private:
    bool m_Enabled;
    //Singleton instance.
    static StackAllocator* pInstance;
    static std::mutex m_Lock;

    //Our stack header.
    static Stack* m_pMemoryStack;
    //Walks across our stack to assign addresses to created objects and their headers.
    static std::byte* m_pByteWalker;
    //Pointer to top object in the stack.
    static ObjectHeader* m_pTop;
};

template<typename T, typename... Arguments>
T* StackAllocator::New(Arguments&&... args)
{
    std::lock_guard<std::mutex> lock(m_Lock);
    //Check if we have enough space on the stack for the object.
    //If not, return nullptr and nothing happens.
    size_t newSize = m_pMemoryStack->m_currentSize + sizeof(T) + sizeof(ObjectHeader);
    if (m_pMemoryStack->m_stackSize < newSize)
    {
        std::cout << "Error, not enough memory free on the Stack." << std::endl;
        return nullptr;
    }
    //If less than 10% memory remain.
    else if (m_pMemoryStack->m_stackSize * (9.0f / 10.0f) < (float)newSize)
    {
        std::cout << "Warning! Less than 10% memory left on the stack." << std::endl;
    }

    //Get an address for the new object and then move the walker by the size of the object.
    std::byte* pObjectDataChunk = m_pByteWalker;
    m_pByteWalker += sizeof(T);

    //Create the object at the assigned address. Increase the currentsize of the stack.
    T* newObject = new(pObjectDataChunk)T(std::forward<Arguments>(args)...);
    m_pMemoryStack->m_currentSize += sizeof(T);

    //Create a header for the new object at the current position of the walker and then move the walker.
    std::byte* pHeaderDataChunk = m_pByteWalker;
    m_pByteWalker += sizeof(ObjectHeader);

    //"Create" or "fill in" the ObjectHeader "object". At the assigned address. Increase the current size of the stack.
    ObjectHeader* header = new(pHeaderDataChunk)ObjectHeader(
        m_pTop,
        newObject,
        [](const void* x) {static_cast<const T*>(x)->~T(); }
    );
    m_pMemoryStack->m_currentSize += sizeof(ObjectHeader);

    //Set the new top element to the new header.
    m_pTop = header;

    return newObject;
}