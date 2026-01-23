#pragma once
#include "includes.cuh"
#include "strict.h"

namespace Bocari
{
    /**
     * @brief A simple RAII wrapper for a CUDA device memory buffer.
     * Manages allocation, deallocation, and basic operations.
     * @tparam T The type of data to be stored in the buffer.
     */
    template <typename T>
    class DeviceBuffer
    {
    public:
        DeviceBuffer() : m_data(nullptr), m_size(0) {}

        explicit DeviceBuffer(size_t size) : m_data(nullptr), m_size(0)
        {
            allocate(size);
        }

        ~DeviceBuffer()
        {
            free();
        }

        // Disable copy constructor and assignment to prevent shallow copies
        DeviceBuffer(const DeviceBuffer&) = delete;
        DeviceBuffer& operator=(const DeviceBuffer&) = delete;

        // Enable move constructor and assignment
        DeviceBuffer(DeviceBuffer&& other) noexcept : m_data(other.m_data), m_size(other.m_size)
        {
            other.m_data = nullptr;
            other.m_size = 0;
        }

        DeviceBuffer& operator=(DeviceBuffer&& other) noexcept
        {
            if (this != &other)
            {
                free();
                m_data = other.m_data;
                m_size = other.m_size;
                other.m_data = nullptr;
                other.m_size = 0;
            }
            return *this;
        }

        void allocate(size_t newSize)
        {
            if (m_size == newSize) return;

            free();
            if (newSize > 0)
            {
                cudaError_t err = cudaMalloc(&m_data, newSize * sizeof(T));
                if (err != cudaSuccess)
                {
                    throw std::runtime_error("Failed to allocate device memory.");
                }
                m_size = newSize;
            }
        }

        void free()
        {
            if (m_data != nullptr)
            {
                cudaFree(m_data);
                m_data = nullptr;
                m_size = 0;
            }
        }

        void memset(int value = 0)
        {
            if (m_data != nullptr)
            {
                cudaMemset(m_data, value, m_size * sizeof(T));
            }
        }

        T* data() { return m_data; }
        const T* data() const { return m_data; }
        size_t size() const { return m_size; }

    private:
        T* m_data;
        size_t m_size;
    };

} // namespace Bocari
