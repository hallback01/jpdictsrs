#include <iostream>
#include <locale.h>
#include "scheduler.h"
#include "jmdict.h"
#include "app.h"

int main() {
    setlocale(LC_ALL, "");
    JMDict* jmdict = new JMDict("JMdict_e");
    if(jmdict->has_errors()) {
        jmdict->print_errors();
    }
    Scheduler* scheduler = new Scheduler();

    App app(jmdict, scheduler);

    delete scheduler;
    delete jmdict;
    return 0;
}