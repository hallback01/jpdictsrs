#include <ncurses.h>
#include <iostream>
#include <locale>
#include <codecvt>
#include <cstring>
#include "app.h"
#include "jmdict.h"

#define COLOR_BRIGHT    0
#define COLOR_MIDDLE    1

App::App(JMDict* dictionary, Scheduler* scheduler) {

    m_jmdict = dictionary;
    m_scheduler = scheduler;

    //ncurses
    initscr();
    //resizeterm(30, 30);
    start_color();
    if(!has_colors() || !can_change_color()) {
        std::cout << "Colors aren't supported in this terminal\n";
        exit(1);
    }

    init_color(50, 500, 500, 500); //MIDDLE
    init_pair(COLOR_BRIGHT, COLOR_WHITE, COLOR_BLACK);
    init_pair(COLOR_MIDDLE, 50, COLOR_BLACK);

    init_pair(5, COLOR_BLACK, COLOR_WHITE);

    refresh();

    uint8_t choice = 0;
    bool is_running = true;
    while(is_running) {
        clear();
        switch(choice) {
            case 0: {
                choice = menu();
                break;
            }

            case 1: {
                choice = search();
                break;
            }

            case 2: {
                choice = search_result();
                break;
            }

            case 3: {
                //choice = fetch_failed();
                break;
            }

            case 4: {
                //choice = confirm_add_new_word();
                break;
            }

            case 20: {
                is_running = false;
                break;
            }
        }
    }
}

App::~App() {
    endwin();
}

void App::status_bar() {
    int32_t x, y;
    getmaxyx(stdscr, x, y);

    bool extra = (y % 2) == 1;

    attron(COLOR_PAIR(5));
    for(size_t i = 0; i < y; i++) {
        printw(" ");
    }
    move(0, y/2 - 5);
    printw("JPDictSRS");

    move(0, 0);
    size_t reviewable_vocab = 0;//m_scheduler->reviewable_vocabulary_count();
    printw("%d vocabulary to review now", reviewable_vocab);
    move(1, 0);

    attroff(COLOR_PAIR(5));
}

uint8_t App::menu() {
    status_bar();
    curs_set(0);
    keypad(stdscr, true);
    noecho();

    std::string choices[3] = {"Review", "Search", "Quit"};
    int choice;
    int highlight = 0;

    while(true) {
        int32_t x, y;
        getmaxyx(stdscr, y, x);
        raw();

        for(size_t i = 0; i < 3; i++) {
            if(i == highlight) {
                attron(A_REVERSE);
            }
            mvwprintw(stdscr, i+ (y/2), (x / 2) - (choices[i].size() / 2), choices[i].c_str());
            attroff(A_REVERSE);
        }
        choice = getch();
        switch(choice) {
            case KEY_UP: {
                highlight--;
                if(highlight == -1) {
                    highlight = 2;
                }
                break;
            }

            case KEY_DOWN: {
                highlight++;
                if(highlight == 3) {
                    highlight = 0;
                }
                break;
            }
            default: break;
        }
        if(choice == 10) {

            if(highlight == 1) {
                return 1;
            }

            if(highlight == 2) {
                return 20;
            }
        }
    }
    return 0;
}

char search_string[256];

uint8_t App::search() {
    memset(search_string, 0, 256);
    int32_t x, y;
    getmaxyx(stdscr, y, x);
    echo();
    status_bar();
    printw("Type in what you want to search for\nYou can search with english, kana and kanji\nPress enter to continue");
    move(y - 1, 0);
    curs_set(2);
    noraw();
    getstr(search_string);

    return 2;
}

void App::draw_entry(int32_t& scroll, bool hide_definition, std::vector<Line>& info) {
    int32_t x, y;
    getmaxyx(stdscr, y, x);
    int32_t max_scroll_height = y - 5;

    size_t how_many = hide_definition ? 1 : info.size();

    size_t height = 0;
    for(size_t i = 0; i < how_many; i++) {
        info[i].line_increase = (info[i].string.size() / (x - 1)) + 1;
        height += info[i].line_increase;
    }

    if(scroll < 0) {
        scroll = 0;
    }

    if(height < max_scroll_height) {
        scroll = 0;
    }

    if(height - max_scroll_height - 1 <= scroll) {
        scroll = height - max_scroll_height - 1;
    }

    size_t line = 0;
    for(size_t i = scroll; i < how_many; i++) {
        attron(COLOR_PAIR(info[i].color));
        mvwprintw(stdscr, line + 2, 1, "%s\n", info[i].string.c_str());
        line += info[i].line_increase;
        attroff(COLOR_PAIR(info[i].color));

        if((i - scroll) >= max_scroll_height) {
            break;
        }
    }
}

void App::fill_lines(JMEntry* entry, std::vector<Line>& info) {

    if(entry->keb_count() != 0) {
        info.push_back({*entry->keb(0), COLOR_BRIGHT});
        info.push_back({*entry->reb(0), COLOR_MIDDLE});
    } else {
        info.push_back({*entry->reb(0), COLOR_BRIGHT});
    }
    info.push_back({"", 0});

    //senses
    for(size_t i = 0; i < entry->sense_count(); i++) {
        JMEntrySense* sense = entry->sense(i);
        info.push_back({sense->pos(), COLOR_MIDDLE});
        info.push_back({sense->gloss(), COLOR_BRIGHT});
        if(sense->extra_info().size() != 0) {
            info.push_back({sense->extra_info(), COLOR_MIDDLE});
        }
        if(i != entry->sense_count() - 1) {
            info.push_back({"", 0});
        }
    }
}

uint8_t App::search_result() {
    std::vector<size_t> entries = m_jmdict->search(search_string);
    noecho();
    raw();
    curs_set(0);
    int32_t current_word = 0;
    int32_t scroll = 0;
    while(true) {

        static std::vector<Line> lines;
        lines.clear();
        fill_lines(m_jmdict->entry(entries[current_word]), lines);

        int32_t x, y;
        getmaxyx(stdscr, y, x);
        clear();
        status_bar();
        std::string info = "Entries found " + std::to_string(current_word + 1) + "/" + std::to_string(entries.size());
        move(1, x/2 - (info.size() / 2)-1);
        printw(info.c_str());
        move(y - 1, 0);
        attron(COLOR_PAIR(5));
        for(int i = 0; i < x; i++) {
            printw(" ");
        }
        move(y - 1, 0);
        printw("[Q]uit, [B]ack, [A]dd word, [R]emove word, Navigate with Arrow Keys");
        attroff(COLOR_PAIR(5));
        draw_entry(scroll, false, lines);
        refresh();
        int32_t input = getch();
        switch(input) {
            case 'q': return 0;
            case 'b': return 1;
            case KEY_RIGHT: {
                current_word++;
                scroll = 0;
                break;
            }
            case KEY_LEFT: {
                current_word--;
                scroll = 0;
                break;
            }
            case KEY_UP: {
                scroll--;
                break;
            }
            case KEY_DOWN: {
                scroll++;
                break;
            }
            default: break;
        }

        //clamp
        if(current_word < 0) {
            current_word = entries.size() - 1;
        }
        if(current_word == entries.size()) {
            current_word = 0;
        }
    }

    return 0;
}