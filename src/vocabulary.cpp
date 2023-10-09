#include <algorithm>
#include <iostream>
#include "vocabulary.h"
#include "jikan.h"

Vocabulary::Vocabulary() {
    m_level = 0;
    m_next_level_time = 0;
    m_seq_id = -1;
}

Vocabulary::Vocabulary(uint64_t sequence_id) {
    //new vocab
    m_level = 0;
    m_next_level_time = Jikan::epoch_time_seconds() + level_time(m_level);
    m_seq_id = sequence_id;
}

Vocabulary* Vocabulary::from_buffer(Buffer& buffer) {
    Vocabulary* v = new Vocabulary();

    //deserialize from buffer
    v->m_level = buffer.read<uint8_t>();
    v->m_next_level_time = buffer.read<int64_t>();
    v->m_seq_id = buffer.read<uint64_t>();
    return v;
}

void Vocabulary::decrease_level() {
    if(m_level > 0) {
        m_level--;
    }
    m_level = std::clamp<uint8_t>(m_level, 0, 9);
    m_next_level_time = Jikan::epoch_time_seconds() + level_time(m_level);
}

void Vocabulary::increase_level() {
    m_level++;
    m_level = std::clamp<uint8_t>(m_level, 0, 9);
    m_next_level_time = Jikan::epoch_time_seconds() + level_time(m_level);
}

int64_t Vocabulary::next_level_time() {
    return m_next_level_time;
}

uint8_t Vocabulary::level() {
    return m_level;
}

bool Vocabulary::is_completed() {
    return m_level == 9;
}

uint64_t Vocabulary::sequence_id() {
    return m_seq_id;
}

int64_t Vocabulary::level_time(uint8_t level) {
    switch(level) {
        case 0: return LEVEL_0_TIME;
        case 1: return LEVEL_1_TIME;
        case 2: return LEVEL_2_TIME;
        case 3: return LEVEL_3_TIME;
        case 4: return LEVEL_4_TIME;
        case 5: return LEVEL_5_TIME;
        case 6: return LEVEL_6_TIME;
        case 7: return LEVEL_7_TIME;
        case 8: return LEVEL_8_TIME;
        default: return 0;
    }
}