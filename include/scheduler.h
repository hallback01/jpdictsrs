#pragma once
#include <unordered_map>
#include <string>
#include <vector>
#include "vocabulary.h"

class Scheduler {

    public:
        Scheduler();
        ~Scheduler();
        bool add(uint64_t seq_id);
        void remove(uint64_t seq_id);
        std::vector<Vocabulary*> reviewable_vocabulary();
        size_t reviewable_vocabulary_count();
        size_t vocabulary_inside_time_interval_count(int64_t seconds);
        std::vector<Vocabulary*> vocabulary_inside_time_interval(int64_t seconds);
        Vocabulary* vocabulary(uint64_t sequence_id);
        void sort();

    private:
        std::vector<Vocabulary*> m_vocabulary; //sorted by next level time!
        std::unordered_map<uint64_t, Vocabulary*> m_sequence_to_vocab;
};