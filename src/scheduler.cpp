#include <algorithm>
#include <fstream>
#include <iostream>
#include <iterator>
#include "scheduler.h"
#include "jikan.h"
#include "buffer.h"

#define VOCAB_FILE "vocab.dat"

Scheduler::Scheduler() {
    std::ifstream cache(VOCAB_FILE, std::ios::binary);
    if(cache.is_open()) {

        cache.unsetf(std::ios::skipws);
        std::streampos file_size;
        cache.seekg(0, std::ios::end);
        file_size = cache.tellg();
        cache.seekg(0, std::ios::beg);

        Buffer buffer(file_size);
        {
            std::vector<uint8_t> b;
            b.reserve(file_size);
            b.insert(b.begin(), std::istream_iterator<uint8_t>(cache), std::istream_iterator<uint8_t>());
            buffer.copy_from(b.data(), b.size());
        }

        if(buffer.size() != 0) {
            uint32_t vocab_count = buffer.read<uint32_t>();
            for(size_t i = 0; i < vocab_count; i++) {
                Vocabulary* vocab = Vocabulary::from_buffer(buffer);
                m_vocabulary.push_back(vocab);
                m_sequence_to_vocab.insert({vocab->sequence_id(), vocab});
            }
        }
        cache.close();
    }
}

Scheduler::~Scheduler() {
    std::ofstream cache(VOCAB_FILE, std::ios::binary);
    if(cache.is_open()) {

        //calculate size
        size_t buffer_size = 4;
        for(auto& vocab : m_vocabulary) {
            buffer_size += 17;
        }
        //buffer_size += 10000;

        Buffer buffer(buffer_size);
        buffer.write<uint32_t>(m_vocabulary.size());
        for(auto& vocab : m_vocabulary) {
            buffer.write<uint8_t>(vocab->level());
            buffer.write<int64_t>(vocab->next_level_time());
            buffer.write<uint64_t>(vocab->sequence_id());
        }
        cache.write((char*)buffer.data(), buffer.size());
        cache.close();
    }

    for(Vocabulary* vocab : m_vocabulary) {
        delete vocab;
    }
}

bool Scheduler::add(uint64_t seq_id) {

    if(vocabulary(seq_id)) {
        return false;
    }

    Vocabulary* new_vocab = new Vocabulary(seq_id);
    m_vocabulary.push_back(new_vocab);
    m_sequence_to_vocab.insert({seq_id, new_vocab});
    sort();
    return true;
}

std::vector<Vocabulary*> Scheduler::reviewable_vocabulary() {
    int64_t epoch = Jikan::epoch_time_seconds();
    static std::vector<Vocabulary*> vocab_vec;
    vocab_vec.clear();
    for(size_t i = 0; i < m_vocabulary.size(); i++) {
        if(epoch > m_vocabulary[i]->next_level_time()) {
            if(m_vocabulary[i]->is_completed()) {
                continue;
            }
            vocab_vec.push_back(m_vocabulary[i]);
        } else {
            break;
        }
    }
    return vocab_vec;
}

size_t Scheduler::seconds_until_next_review() {
    int64_t epoch = Jikan::epoch_time_seconds();
    for(size_t i = 0; i < m_vocabulary.size(); i++) {
        if(!m_vocabulary[i]->is_completed()) {
            return m_vocabulary[i]->next_level_time() - epoch;
        } else {
            continue;
        }
    }
    return 0;
}

size_t Scheduler::reviewable_vocabulary_count() {
    size_t count = 0;
    int64_t epoch = Jikan::epoch_time_seconds();
    for(size_t i = 0; i < m_vocabulary.size(); i++) {
        if(epoch > m_vocabulary[i]->next_level_time()) {
            if(m_vocabulary[i]->is_completed()) {
                continue;
            }
            count++;
        } else {
            break;
        }
    }
    return count;
}

std::vector<Vocabulary*> Scheduler::vocabulary_inside_time_interval(int64_t seconds) {
    static std::vector<Vocabulary*> vocab;
    int64_t epoch = Jikan::epoch_time_seconds();
    
    vocab.clear();
    for(size_t i = 0; i < m_vocabulary.size(); i++) {
        if((epoch + seconds) > m_vocabulary[i]->next_level_time()) {
            if(m_vocabulary[i]->is_completed()) {
                continue;
            }
            vocab.push_back(m_vocabulary[i]);
        } else {
            break;
        }
    }

    return vocab;
}

size_t Scheduler::total_vocabulary_count() {
    return m_vocabulary.size();
}

size_t Scheduler::vocabulary_inside_time_interval_count(int64_t seconds) {
    size_t count = 0;
    int64_t epoch = Jikan::epoch_time_seconds();
    for(size_t i = 0; i < m_vocabulary.size(); i++) {
        if((epoch + seconds) > m_vocabulary[i]->next_level_time()) {
            if(m_vocabulary[i]->is_completed()) {
                continue;
            }
            count++;
        } else {
            break;
        }
    }
    return count;
}

bool compare_by_time_until_review(Vocabulary* a, Vocabulary* b) {
    return a->next_level_time() < b->next_level_time();
}

void Scheduler::sort() {
    std::sort(m_vocabulary.begin(), m_vocabulary.end(), compare_by_time_until_review);
}

Vocabulary* Scheduler::vocabulary(uint64_t sequence_id) {
    auto it = m_sequence_to_vocab.find(sequence_id);
    if(it != m_sequence_to_vocab.end()) {
        return it->second;
    } else {
        return nullptr;
    }
}

void Scheduler::remove(uint64_t seq_id) {
    Vocabulary* vocab = vocabulary(seq_id);
    if(vocab) {
        delete vocab;
        m_sequence_to_vocab.erase(seq_id);

        for(size_t i = 0; i < m_vocabulary.size(); i++) {
            if(vocab == m_vocabulary[i]) {
                m_vocabulary.erase(m_vocabulary.begin() + i);
                break;
            }
        }
    }
}