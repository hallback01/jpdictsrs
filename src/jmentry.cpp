#include "jmentry.h"

void JMEntry::add_keb(std::string& keb) {
    m_kebs.push_back(keb);
}


void JMEntry::add_reb(std::string& reb) {
    m_rebs.push_back(reb);
}

void JMEntry::add_sense(JMEntrySense& reb) {
    m_senses.push_back(reb);
}

size_t JMEntry::keb_count() {
    return m_kebs.size();
}

size_t JMEntry::reb_count() {
    return m_rebs.size();
}

size_t JMEntry::sense_count() {
    return m_senses.size();
}

std::string* JMEntry::keb(size_t index) {
    return (index < m_kebs.size()) ? &m_kebs[index] : nullptr;
}

std::string* JMEntry::reb(size_t index) {
    return (index < m_rebs.size()) ? &m_rebs[index] : nullptr;
}

JMEntrySense* JMEntry::sense(size_t index) {
    return (index < m_senses.size()) ? &m_senses[index] : nullptr;
}

size_t JMEntry::sequence_index() {
    return m_sequence_index;
}