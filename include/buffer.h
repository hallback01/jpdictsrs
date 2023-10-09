#pragma once
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>

class Buffer {

    public:
        Buffer();
        Buffer(size_t size);
        ~Buffer();
        void copy_from(uint8_t* other, size_t byte_count);
        void copy_to(uint8_t* other, size_t byte_count);
        uint8_t* data();
        size_t size();
        size_t write_position();
        size_t read_position();
        void reset_write_position();
        void reset_read_position();
        void set_read_position(size_t position);
        void set_write_position(size_t position);
        void write_string(const std::string& string);
        std::string read_string();

        template<typename T>
        void write(T data) {
            memcpy(m_data + m_write_position, &data, sizeof(T));
            m_write_position += sizeof(T);
        }

        template<typename T>
        T read() {
            T data;
            memcpy(&data, m_data + m_read_position, sizeof(T));
            m_read_position += sizeof(T);
            return data;
        }

    private:
        uint8_t* m_data;
        size_t m_size;
        size_t m_read_position;
        size_t m_write_position;
};