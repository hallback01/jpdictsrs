#include <cstdlib>
#include "buffer.h"

Buffer::Buffer() {
    m_data = nullptr;
    m_size = 0;
    m_read_position = 0;
    m_write_position = 0;
}

Buffer::Buffer(size_t size) {
    m_data = (uint8_t*)malloc(size);
    m_size = size;
    m_read_position = 0;
    m_write_position = 0;
}

Buffer::~Buffer() {
    free(m_data);
}

void Buffer::reset_write_position() {
    m_write_position = 0;
}

void Buffer::reset_read_position() {
    m_read_position = 0;
}

void Buffer::copy_from(uint8_t* other, size_t byte_count) {
    memcpy(m_data + m_write_position, other, byte_count);
    m_write_position += byte_count;
}

void Buffer::copy_to(uint8_t* other, size_t byte_count) {
    memcpy(other, m_data + m_read_position, byte_count);
    m_read_position += byte_count;
}

uint8_t* Buffer::data() {
    return m_data;
}

size_t Buffer::size() {
    return m_size;
}

size_t Buffer::write_position() {
    return m_write_position;
}

size_t Buffer::read_position() {
    return m_read_position;
}

void Buffer::set_read_position(size_t position) {
    m_read_position = position;
}

void Buffer::set_write_position(size_t position) {
    m_write_position = position;
}

void Buffer::write_string(const std::string& string) {
    write<uint32_t>(string.length());

    for(size_t i = 0; i < string.size(); i++) {
        write<uint8_t>(string[i]);
    }
}

std::string Buffer::read_string() {
    size_t size = read<uint32_t>();
    char* str = new char[size];

    for(size_t i = 0; i < size; i++) {
        str[i] = read<uint8_t>();
    }

    std::string return_string(str);
    delete[] str;
    return return_string;
}