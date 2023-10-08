#include <iostream>
#include <locale.h>
#include "jmdict.h"
#include "app.h"

int main() {
    setlocale(LC_ALL, "");
    JMDict* jmdict = new JMDict("JMdict_e");
    if(jmdict->has_errors()) {
        jmdict->print_errors();
    }

    App app(jmdict, nullptr);

    /*for(auto result : jmdict->search("やばい", 0)) {
        JMEntry* entry = jmdict->entry(result);

        for(int i = 0; i < entry->keb_count(); i++) {
            std::cout << *entry->keb(i) << ", ";
        }
        std::cout << "\n";
        for(int i = 0; i < entry->reb_count(); i++) {
            std::cout << *entry->reb(i) << ", ";
        }
        std::cout << "\n\n";

        for(int i = 0; i < entry->sense_count(); i++) {
            std::cout << entry->sense(i)->pos() << "\n";
            std::cout << entry->sense(i)->gloss() << "\n";
            std::cout << entry->sense(i)->extra_info() << "\n\n";
        }
        
        std::cout << "\n";
    }*/

    delete jmdict;
    return 0;
}