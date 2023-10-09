#pragma once
#include <string>
#include "buffer.h"

#define LEVEL_0_TIME 0
#define LEVEL_1_TIME 14400
#define LEVEL_2_TIME 28800
#define LEVEL_3_TIME 72000
#define LEVEL_4_TIME 144000
#define LEVEL_5_TIME 576000
#define LEVEL_6_TIME 1180800
#define LEVEL_7_TIME 2390400
#define LEVEL_8_TIME 9648000

class Vocabulary {
    public:
        Vocabulary();
        Vocabulary(uint64_t sequence_id);
        static Vocabulary* from_buffer(Buffer& buffer);

        void decrease_level();
        void increase_level();
        int64_t next_level_time();
        uint8_t level();
        bool is_completed(); //a.k.a is level equal to 8?
        uint64_t sequence_id();

    private:
        int64_t level_time(uint8_t level);

        uint64_t m_seq_id;
        int64_t m_next_level_time;
        uint8_t m_level;
};