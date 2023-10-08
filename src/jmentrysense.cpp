#include "jmentrysense.h"

void JMEntrySense::add_pos(std::string& pos) {
    if(m_part_of_speech.size() != 0) {
        m_part_of_speech.append(", ");
    }
    m_part_of_speech.append(pos);
}

void JMEntrySense::add_gloss(std::string& gloss) {
    if(m_gloss.size() != 0) {
        m_gloss.append("; ");
    }
    m_gloss.append(gloss);
}

void JMEntrySense::add_extra_info(std::string& info) {
    if(m_extra_info.size() != 0) {
        m_extra_info.append(", ");
    }
    m_extra_info.append(info);
}

std::string JMEntrySense::pos() {
    return m_part_of_speech;
}

std::string JMEntrySense::gloss() {
    return m_gloss;
}

std::string JMEntrySense::extra_info() {
    return m_extra_info;
}