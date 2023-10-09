#pragma once
#include <string>
#include "buffer.h"

#define LEVEL_0_TIME 0          /*Right away*/
#define LEVEL_1_TIME 14400      /*4 hours*/
#define LEVEL_2_TIME 28800      /*8 hours*/
#define LEVEL_3_TIME 72000      /*20 hours*/
#define LEVEL_4_TIME 144000     /*40 hours*/
#define LEVEL_5_TIME 576000     /*6 2/3 days*/
#define LEVEL_6_TIME 1180800    /*13 2/3 days*/
#define LEVEL_7_TIME 2390400    /*27 2/3 days*/
#define LEVEL_8_TIME 9648000    /*3 2/3 months*/

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