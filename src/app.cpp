#include <ncurses.h>
#include <iostream>
#include <locale>
#include <codecvt>
#include <cstring>
#include "scheduler.h"
#include "app.h"
#include "jmdict.h"

#define COLOR_BRIGHT    0
#define COLOR_MIDDLE    1

#define MAX_TEMP_VOCAB  8

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
                choice = no_results();
                break;
            }

            case 4: {
                choice = remove_vocab_confirm();
                break;
            }

            case 5: {
                choice = review();
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
    //move(0, y/2 - 5);
    //printw("JPDictSRS");

    move(0, 0);
    size_t reviewable_vocab = m_scheduler->reviewable_vocabulary_count();
    if(reviewable_vocab != 0) {
        printw("%d vocab now!", reviewable_vocab);
    } else {
        size_t seconds = m_scheduler->seconds_until_next_review();
        size_t hours = seconds / 3600;
        size_t minutes = (seconds - (hours * 3600)) / 60;
        size_t sec = (seconds - (hours * 3600) - (minutes * 60));
        const char* ha = hours < 10 ? "0" : "";
        const char* ma = minutes < 10 ? "0" : "";
        const char* sa = sec < 10 ? "0" : "";
        printw("%d vocab in 24h! Next is in %s%d:%s%d:%s%d hours!", m_scheduler->vocabulary_inside_time_interval_count(86400), ha, hours, ma, minutes, sa, sec);
    }
    move(1, 0);

    size_t total_inputted_vocab = m_scheduler->total_vocabulary_count();
    size_t digits = total_inputted_vocab > 0 ? (int) log10 ((double) total_inputted_vocab) + 1 : 1;
    move(0, y - digits - 7);
    printw("Total: %d", total_inputted_vocab);

    attroff(COLOR_PAIR(5));
}

void App::action_bar(const std::string& string) {
    int32_t x, y;
    getmaxyx(stdscr, y, x);
    attron(COLOR_PAIR(5));
    move(y - 1, 0);
    for(int i = 0; i < x; i++) {
        printw(" ");
    }
    move(y - 1, 0);
    printw(string.c_str());
    attroff(COLOR_PAIR(5));
}

uint8_t App::menu() {

    std::string choices[3] = {"Review", "Search", "Quit"};
    int choice;
    int highlight = 0;

    while(true) {
        clear();
        status_bar();
        curs_set(0);
        keypad(stdscr, true);
        noecho();
        action_bar("Enter to choose, Navigate with Arrow Keys");
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

            if(highlight == 0) {
                return 5;
            }

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
size_t saved_seq_id = 0;
int32_t saved_scroll = 0;
int32_t saved_current_word = 0;
void reset_saved_search() {
    saved_seq_id = 0;
    saved_scroll = 0;
    saved_current_word = 0;
    memset(search_string, 0, 256);
}

uint8_t App::search() {
    reset_saved_search();
    int32_t x, y;
    getmaxyx(stdscr, y, x);
    echo();
    status_bar();
    curs_set(2);
    printw("Type in what you want to search for\nYou can search with english, kana and kanji");
    action_bar("Press enter to search");
    move(y - 2, 0);
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
    if(entries.size() == 0) {
        return 3;
    }
    noecho();
    raw();
    curs_set(0);
    int32_t current_word = saved_current_word;
    int32_t scroll = saved_scroll;
    while(true) {

        static std::vector<Line> lines;
        lines.clear();
        JMEntry* jmentry = m_jmdict->entry(entries[current_word]);
        fill_lines(jmentry, lines);

        int32_t x, y;
        getmaxyx(stdscr, y, x);
        clear();
        status_bar();
        std::string info = "Entries found " + std::to_string(current_word + 1) + "/" + std::to_string(entries.size());
        move(1, x/2 - (info.size() / 2)-1);
        printw(info.c_str());

        Vocabulary* vocab = m_scheduler->vocabulary(jmentry->sequence_index());
        bool can_add = true;
        if(vocab) {
            action_bar("[Q]uit, [B]ack, [R]emove word, Navigate with Arrow Keys");
            can_add = false;
        } else {
            action_bar("[Q]uit, [B]ack, [A]dd word, Navigate with Arrow Keys");
        }

        draw_entry(scroll, false, lines);
        refresh();
        int32_t input = getch();
        switch(input) {
            case 'q': return 0;
            case 'b': return 1;
            case 'a': {
                if(can_add) {
                    //add vocab
                    m_scheduler->add(jmentry->sequence_index());
                }
                break;
            }
            case 'r': {
                if(!can_add) {
                    saved_seq_id = jmentry->sequence_index();
                    return 4;
                }
                break;
            }
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

        saved_current_word = current_word;
        saved_scroll = scroll;

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

uint8_t App::no_results() {
    status_bar();
    curs_set(0);
    printw("Search \"%s\" yielded no results", search_string);
    action_bar("Press any key to return to the main menu");
    getch();
    return 0;
}

uint8_t App::remove_vocab_confirm() {
    status_bar();
    curs_set(0);
    JMEntry* jmentry = m_jmdict->entry_from_sequence(saved_seq_id);

    if(jmentry) {
        if(jmentry->keb_count() > 0) {
            printw("Are you sure that you want to remove\n%s?", jmentry->keb(0)->c_str());
        } else {
            printw("Are you sure that you want to remove\n%s?", jmentry->reb(0)->c_str());
        }
    }

    action_bar("[Y]es, [N]o");
    while(true) {
        int32_t input = getch();
        if(input == 'n') {
            return 2;
        } else if(input == 'y') {
            m_scheduler->remove(saved_seq_id);
            return 2;
        }
    }
    return 0;
}

uint8_t App::review() {

    struct TempVocab {
        Vocabulary* vocab;
        bool has_answered = false;
    };

    std::vector<Vocabulary*> reviewable_vocab = m_scheduler->reviewable_vocabulary();
    
    std::vector<TempVocab> temporary_vocabs;

    for(size_t i = 0; i < MAX_TEMP_VOCAB; i++) {
        if(reviewable_vocab.size() > 0) {
            size_t which = rand() % reviewable_vocab.size();
            TempVocab tv = {reviewable_vocab[which], false};
            reviewable_vocab.erase(reviewable_vocab.begin() + which);
            temporary_vocabs.push_back(tv);
        }
    }

    while(temporary_vocabs.size() > 0) {

        size_t random_vocab = rand() % temporary_vocabs.size();
        JMEntry* jmentry = m_jmdict->entry_from_sequence(temporary_vocabs[random_vocab].vocab->sequence_id());
        static std::vector<Line> lines;
        lines.clear();
        fill_lines(jmentry, lines);
        int32_t scroll = 0;

        clear();
        status_bar();

        std::string level = "Level " + std::to_string(temporary_vocabs[random_vocab].vocab->level());
        int32_t x, y;
        getmaxyx(stdscr, y, x);
        move(1, x/2 - level.size()/2-1);
        printw(level.c_str());

        draw_entry(scroll, true, lines);
        action_bar("Press any key to reveal definition");
        refresh();
        getch();
        while(true) {
            clear();
            move(0, 0);
            status_bar();
            std::string dyktw("Did you know the word?");
            int32_t x, y;
            getmaxyx(stdscr, y, x);
            move(1, x/2 - dyktw.size()/2);
            printw(dyktw.c_str());
            draw_entry(scroll, false, lines);
            action_bar("[Y]es to level up, [N]o to level down, [H]ard - stay at level /w half the time");

            int32_t input = getch();

            if(input == 'y') {

                //decrease level if a previous attempt was unsuccessful
                if(temporary_vocabs[random_vocab].has_answered) {
                    temporary_vocabs[random_vocab].vocab->decrease_level();
                } else {
                    //otherwise increase the level if this attempt was the first one
                    temporary_vocabs[random_vocab].vocab->increase_level();
                }
                m_scheduler->sort();

                //replace the vocab with a new one
                temporary_vocabs.erase(temporary_vocabs.begin() + random_vocab);
                if(reviewable_vocab.size() > 0) {
                    size_t which = rand() % reviewable_vocab.size();
                    TempVocab tv = {reviewable_vocab[which], false};
                    reviewable_vocab.erase(reviewable_vocab.begin() + which);
                    temporary_vocabs.push_back(tv);
                }
                break;
            } else if(input == 'n') {
                temporary_vocabs[random_vocab].has_answered = true;
                break;
            } else if(input == 'h') {
                
                //decrease level if a previous attempt was unsuccessful
                if(temporary_vocabs[random_vocab].has_answered) {
                    temporary_vocabs[random_vocab].vocab->decrease_level();
                } else {
                    //otherwise keep same level but half the time
                    temporary_vocabs[random_vocab].vocab->stay_at_level();
                }
                m_scheduler->sort();

                //replace the vocab with a new one
                temporary_vocabs.erase(temporary_vocabs.begin() + random_vocab);
                if(reviewable_vocab.size() > 0) {
                    size_t which = rand() % reviewable_vocab.size();
                    TempVocab tv = {reviewable_vocab[which], false};
                    reviewable_vocab.erase(reviewable_vocab.begin() + which);
                    temporary_vocabs.push_back(tv);
                }
                break;

            } else if(input == KEY_UP) {
                scroll--;
            } else if(input == KEY_DOWN) {
                scroll++;
            }
        }
    }

    return 0;
}