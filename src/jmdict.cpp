#include <iostream>
#include <fstream>
#include <sstream>
#include <locale>
#include <codecvt>
#include "jmdict.h"

static std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> utf32conv;

#define ENTRY_LENGTH            7
#define ENTRY_END_LENGTH        8
#define ENTITY_LENGTH           9
#define ENTITY_END_LENGTH       2
#define MIDDLE_LEFT_TO_RIGHT    2

JMDict::JMDict(const std::string& location) {
    std::ifstream file(location);
    if(file.is_open()) {
        std::stringstream jmdict_stream;
        jmdict_stream << file.rdbuf();
        std::string jmdict = jmdict_stream.str();

        if(!parse_entities(jmdict)) {
            add_error("Could not parse entities");
        }

        size_t position = 0;
        bool parsing = true;
        while(parsing) {
            if(!parse_entry(position, jmdict)) {
                parsing = false;
            }
        }
    } else {
        add_error("Could not open file \"" + location + "\"");
    }
}

bool JMDict::has_errors() {
    return m_errors.size() > 0;
}

void JMDict::print_errors() {
    std::cout << m_errors.size() << " errors parsing jmdict file\n";
    for(size_t i = 0; i < m_errors.size(); i++) {
        std::cout << i+1 << ": " << m_errors[i] << "\n";
        if(i < m_errors.size() - 1) {
            std::cout << "\n";
        }
    }
}

void JMDict::add_error(const std::string& error) {
    m_errors.push_back(error);
}

bool JMDict::parse_entities(std::string& jmdict) {
    
    size_t position = 0;
    while(true) {
        size_t entitiy_pos = jmdict.find("<!ENTITY", position);
        if(entitiy_pos == std::string::npos) {
            break;
        }
        position = entitiy_pos + ENTITY_LENGTH;

        size_t entity_end_pos = jmdict.find("\">", position);
        if(entity_end_pos == std::string::npos) {
            return false;
        }

        size_t middle_left = jmdict.find(" \"", position);
        if(middle_left == std::string::npos) {
            return false;
        }
        size_t middle_right = middle_left + MIDDLE_LEFT_TO_RIGHT;

        std::string entity_key = jmdict.substr(position, middle_left - position);
        entity_key.insert(0, 1, '&');
        entity_key.push_back(';');
        std::string entity_content = jmdict.substr(middle_right, entity_end_pos - middle_right);

        position = entity_end_pos + ENTITY_END_LENGTH;
        m_entities.insert({entity_key, entity_content});
    }

    if(m_entities.size() == 0) {
        return false;
    }
    return true;
}

bool JMDict::parse_entry(size_t& position, std::string& jmdict) {

    std::string entry = get_element("entry", jmdict, position, &position);
    if(position == std::string::npos) {
        return false;
    }

    size_t new_entry_position = m_entries.size();

    //get ent_seq
    size_t ent_seq_pos;
    std::string ent_seq = get_element("ent_seq", entry, 0, &ent_seq_pos);
    if(ent_seq_pos == std::string::npos) {
        return false;
    }
    size_t sequence_index = stoi(ent_seq);
    JMEntry jmentry(sequence_index);

    //get all k_ele
    size_t entry_pos = 0;
    while(true) {
        std::string k_ele = get_element("k_ele", entry, entry_pos, &entry_pos);
        if(entry_pos != std::string::npos) {
            std::string keb = get_element("keb", k_ele, 0, nullptr);
            m_kebs_to_entry[keb].insert(new_entry_position);
            jmentry.add_keb(keb);
        } else {
            break;
        }
    }

    entry_pos = 0;
    while(true) {
        std::string r_ele = get_element("r_ele", entry, entry_pos, &entry_pos);
        if(entry_pos != std::string::npos) {
            //reb
            std::string reb = get_element("reb", r_ele, 0, nullptr);
            jmentry.add_reb(reb);
        } else {
            break;
        }
    }

    //senses
    entry_pos = 0;
    while(true) {
        std::string sense = get_element("sense", entry, entry_pos, &entry_pos);
        if(entry_pos == std::string::npos) {
            break;
        }
        JMEntrySense jmsense;

        //part of speech
        size_t sense_pos = 0;
        while(true) {
            std::string pos = get_element("pos", sense, sense_pos, &sense_pos);
            if(sense_pos == std::string::npos) {
                break;
            }
            jmsense.add_pos(m_entities[pos]);
        }

        //glosses
        sense_pos = 0;
        while(true) {
            std::string gloss = get_element("gloss", sense, sense_pos, &sense_pos);
            if(sense_pos == std::string::npos) {
                break;
            }
            jmsense.add_gloss(gloss);
        }

        //antonym
        sense_pos = 0;
        while(true) {
            std::string ant = get_element("ant", sense, sense_pos, &sense_pos);
            if(sense_pos == std::string::npos) {
                break;
            }
            ant = std::string("Antonym \"") + ant + std::string("\"");
            jmsense.add_extra_info(ant);
        }

        //xref
        sense_pos = 0;
        while(true) {
            std::string xref = get_element("xref", sense, sense_pos, &sense_pos);
            if(sense_pos == std::string::npos) {
                break;
            }
            xref = std::string("See also \"") + xref + std::string("\"");
            jmsense.add_extra_info(xref);
        }

        //s_inf
        sense_pos = 0;
        while(true) {
            std::string sinfo = get_element("s_inf", sense, sense_pos, &sense_pos);
            if(sense_pos == std::string::npos) {
                break;
            }
            jmsense.add_extra_info(sinfo);
        }

        //misc
        sense_pos = 0;
        while(true) {
            std::string misc = get_element("misc", sense, sense_pos, &sense_pos);
            if(sense_pos == std::string::npos) {
                break;
            }
            jmsense.add_extra_info(m_entities[misc]);
        }

        jmentry.add_sense(jmsense);
    }

    m_entries.push_back(jmentry);
    m_sequenced_to_entry.insert({sequence_index, new_entry_position});
    return true;
}

std::string JMDict::get_element(const std::string& element, std::string& from, size_t position, size_t* out_end_pos) {
    std::string start = "<" + element + ">";
    std::string end = "</" + element + ">";
    size_t element_start_position = from.find(start, position);
    size_t element_end_position = from.find(end, position);
    if(element_start_position == std::string::npos || element_end_position == std::string::npos) {
        if(out_end_pos) {
            *out_end_pos = -1;
        }
        return "";
    }

    if(out_end_pos) {
        *out_end_pos = element_end_position + end.size();
    }

    return from.substr(element_start_position + start.size(), element_end_position - (element_start_position + start.size()));
}

std::vector<size_t> JMDict::search(const std::string& prompt, size_t limit) {
    static std::vector<size_t> results;

    results.clear();

    for(size_t i = 0; i < m_entries.size(); i++) {

        JMEntry& entry = m_entries[i];

        for(size_t j = 0; j < entry.keb_count(); j++) {
            if(entry.keb(j)->find(prompt) != std::string::npos) {
                results.push_back(i);
            } 
        }

        for(size_t j = 0; j < entry.reb_count(); j++) {
            if(entry.reb(j)->find(prompt) != std::string::npos) {
                results.push_back(i);
            } 
        }

        for(size_t j = 0; j < entry.sense_count(); j++) {
            if(entry.sense(j)->gloss().find(prompt) != std::string::npos) {
                results.push_back(i);
            }
        }

        results.erase(unique(results.begin(), results.end()), results.end());

        if(limit != 0 && results.size() == limit) {
            break;
        }
    }
    return results;
}

JMEntry* JMDict::entry(size_t index) {
    return (index < m_entries.size()) ? &m_entries[index] : nullptr;
}

JMEntry* JMDict::entry_from_sequence(size_t sequence_index) {
    auto it = m_sequenced_to_entry.find(sequence_index);
    if(it != m_sequenced_to_entry.end()) {
        return entry(it->second);
    } else {
        return nullptr;
    }
}