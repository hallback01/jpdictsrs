#pragma once
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <vector>
#include "jmentry.h"

class JMDict {

    public:
        JMDict() = delete;
        JMDict(const std::string& location);

        std::vector<size_t> search(const std::string& prompt, size_t limit = 0);
        JMEntry* entry(size_t index);
        JMEntry* entry_from_sequence(size_t sequence_index);

        bool has_errors();
        void print_errors();

    private:

        bool parse_entities(std::string& jmdict);
        bool parse_entry(size_t& position, std::string& jmdict);

        std::string get_element(const std::string& element, std::string& from, size_t position, size_t* out_end_pos);

        std::unordered_map<std::string, std::string> m_entities;
        std::unordered_map<std::string, std::unordered_set<size_t>> m_kebs_to_entry;
        std::unordered_map<size_t, size_t> m_sequenced_to_entry;
        std::vector<JMEntry> m_entries;

        //error
        void add_error(const std::string& error);
        std::vector<std::string> m_errors;
};