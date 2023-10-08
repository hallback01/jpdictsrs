#pragma once
#include <vector>
#include <string>
#include "jmentrysense.h"

class JMEntry {
    public:
        JMEntry() = default;
        JMEntry(size_t sequence_index) : m_sequence_index(sequence_index) {}
        void add_keb(std::string& keb);
        void add_reb(std::string& reb);
        void add_sense(JMEntrySense& reb);

        size_t sequence_index();
        size_t keb_count();
        size_t reb_count();
        size_t sense_count();
        std::string* keb(size_t index);
        std::string* reb(size_t index);
        JMEntrySense* sense(size_t index);
    private:
        size_t m_sequence_index;
        std::vector<std::string> m_kebs;
        std::vector<std::string> m_rebs;
        std::vector<JMEntrySense> m_senses;
};