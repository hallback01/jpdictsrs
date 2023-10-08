#pragma once
#include <string>

class JMEntrySense {

    public:
        void add_pos(std::string& pos);
        void add_gloss(std::string& gloss);
        void add_extra_info(std::string& info);

        std::string pos();
        std::string gloss();
        std::string extra_info();
    private:
        std::string m_part_of_speech;
        std::string m_extra_info;
        std::string m_gloss;
};