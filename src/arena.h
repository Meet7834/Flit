#pragma once

class ArenaAllocator {
private:
    size_t m_size;
    std::byte *m_buffer; // whole memory block
    std::byte *m_offset; // pointer to the location at memory block

public:
    explicit ArenaAllocator(size_t bytes) : m_size(bytes) {
        m_buffer = static_cast<std::byte *>(malloc(m_size));
        m_offset = m_buffer;
    }

    template<typename T>
    T *alloc() {
        void *offset = m_offset;
        m_offset += sizeof(T); // increase the offset to the next free location
        return static_cast<T *> (offset);
    }

    // this deletes the copy constructor (which was being automatically generated) for this class as it would cause problem if there were
    // more than one object pointers pointing to and managing same block of memory which can cause problems.
    // For ex: if both pointers tried to call destructor it could deallocate same memory twice which can lead
    // to undefined behaviour and crashing the program
    ArenaAllocator(const ArenaAllocator &other) = delete;

    // this will do the same thing but for assignment operator
    // For ex: This code will throw an error:
    /*
        {
            ArenaAllocator allocator1(1024);  // Allocate 1024 bytes
            ArenaAllocator allocator2(512);   // Allocate 512 bytes

            allocator2 = allocator1; // this won't allow because we are deleting the assignment operator
        }
    */
    ArenaAllocator operator=(const ArenaAllocator &other) = delete;

    ~ArenaAllocator() {
        free(m_buffer);
    }
};