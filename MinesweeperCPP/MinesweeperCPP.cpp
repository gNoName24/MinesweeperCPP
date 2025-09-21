#include "MinesweeperCPP.hpp"

#include <regex>

namespace MinesweeperCPP {
    InitConsole::Logger logger("logger");

    void console_clear() {
        std::cout << "\033[2J\033[1;1H";
    }

    std::string reverse_colors(const std::string& s) {
        std::regex sgr_re("\x1B\\[([0-9;]*)m");
        std::smatch m;
        int fg = -1, bg = -1;

        if(std::regex_search(s, m, sgr_re)) {
            std::string codes = m[1].str();
            std::stringstream ss(codes);
            std::string tok;
            while (std::getline(ss, tok, ';')) {
                if(tok.empty()) continue;
                int code = std::stoi(tok);
                if((code >= 30 && code <= 37) || (code >= 90 && code <= 97)) {
                    fg = code;
                } else if((code >= 40 && code <= 47) || (code >= 100 && code <= 107)) {
                    bg = code;
                }
            }
        }

        std::string plain = std::regex_replace(s, sgr_re, std::string(""));

        int new_fg = -1, new_bg = -1;

        if(bg != -1) {
            // bg -> fg: 40..47 -> 30..37 ; 100..107 -> 90..97
            if(bg >= 40 && bg <= 47) new_fg = bg - 10;
            else if(bg >= 100 && bg <= 107) new_fg = bg - 10; // 100->90 etc
        }
        if(fg != -1) {
            // fg -> bg: 30..37 -> 40..47 ; 90..97 -> 100..107
            if(fg >= 30 && fg <= 37) new_bg = fg + 10;
            else if(fg >= 90 && fg <= 97) new_bg = fg + 10; // 90->100 etc
        }

        if(new_fg == -1 && new_bg == -1) {
            new_fg = 30;
            new_bg = 47;
        } else {
            if(new_fg == -1) new_fg = 37;
            if(new_bg == -1) new_bg = 40;
        }

        std::string prefix = "\033[" + std::to_string(new_fg) + ";" + std::to_string(new_bg) + "m";
        std::string suffix = "\033[0m";
        return prefix + plain + suffix;
    }
};
