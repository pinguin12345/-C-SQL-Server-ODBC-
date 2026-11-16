#include "log.h"
#include <fstream>

// Короткий журнал рядом с программой. В базу он не пишется.
void logStroka(const std::string& text) {
    std::ofstream f("zhurnal_programmy.txt", std::ios::app);
    if (f) f << text << "\n";
}
