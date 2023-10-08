#pragma once

class JMDict;
class JMEntry;
class Scheduler;

class App {

    public:
        App(JMDict* dictionary, Scheduler* scheduler);
        ~App();

    private:

        struct Line {
            std::string string;
            uint8_t color;
            size_t line_increase;
        };

        uint8_t menu();
        uint8_t search();
        uint8_t search_result();

        void draw_entry(int32_t& scroll, bool hide_definition, std::vector<Line>& info);
        void fill_lines(JMEntry* entry, std::vector<Line>& info);
        void status_bar();

        uint8_t fetch_word_data();
        uint8_t fetch_failed();
        uint8_t confirm_add_new_word();

        JMDict* m_jmdict;
        Scheduler* m_scheduler;
};