#include "pch.h"
#include "StackAllocator.h"

StackAllocator* StackAllocator::pInstance{ nullptr };
std::mutex StackAllocator::m_Lock;
Stack* StackAllocator::m_pMemoryStack{ nullptr };
std::byte* StackAllocator::m_pByteWalker{ nullptr };
ObjectHeader* StackAllocator::m_pTop{ nullptr };

//---------------------------------------------------------------------

void StackAllocator::CreateAllocator(unsigned long long stackSize)
{
    //Only create it if it does not exist.
    if (!pInstance)
    {
        pInstance = DBG_NEW StackAllocator(stackSize);
    }
    else
    {
        std::cerr << "Error! You are trying to create another Stack Allocator!" << std::endl;
        assert(false);
    }
}

void StackAllocator::ToggleEnabled() noexcept
{
    m_Enabled = !m_Enabled;
}

const bool StackAllocator::IsEnabled() const noexcept
{
    return m_Enabled;
}

void StackAllocator::FreeAllMemory()
{
    delete pInstance;
    pInstance = nullptr;
}

StackAllocator* StackAllocator::GetInstance()
{
    return pInstance;
}

size_t StackAllocator::GetStackMaxSize()
{
    return m_pMemoryStack->m_stackSize;
}

size_t StackAllocator::GetStackCurrentSize()
{
    return m_pMemoryStack->m_currentSize;
}

StackAllocator::StackAllocator(unsigned long long stackSize)
{
    //Allocate the "header" of the stack.
    m_pMemoryStack = DBG_NEW Stack(stackSize);

    //Set the bytewalker to be the start of the allocated memory.
    m_pByteWalker = m_pMemoryStack->m_pData;

    m_pTop = nullptr;

    m_Enabled = false;
}

StackAllocator::~StackAllocator()
{
    delete m_pMemoryStack;
    m_pMemoryStack = nullptr;
    m_pByteWalker = nullptr;
    m_pTop = nullptr;
}

void StackAllocator::CleanUp()
{
    std::lock_guard<std::mutex> lock(m_Lock);
    //Reset pointer & currentsize & call destructor of all allocated objects.
    //Set the bytewalker to be the start of the allocated memory.
    m_pByteWalker = m_pMemoryStack->m_pData;
    //Set current stacksize to 0.
    m_pMemoryStack->m_currentSize = 0;

    //Until all objects' destructors have been called.
    while (m_pTop)
    {
        //Calls the destructor of the object.
        m_pTop->m_pDestructor(m_pTop->m_pObject);

        //Move the top pointer to the next object in the stack.
        m_pTop = m_pTop->m_pObjectHeaderUnder;

        //Do not need to call destructor of the headers, they do not allocate anything.
        //Simply overwrite them next frame.
        //Same with stack variables inside the objects.
    }
}