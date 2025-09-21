#include "MinesweeperCPP.hpp"

namespace MinesweeperCPP {
    namespace Notifications {
        std::vector<Note> list;

        void clear() { list.clear(); }
        void step() {
            for(int i = 0; i < list.size(); i++) {
                if(i < 0 || i >= list.size()) continue;
                Note& note = list[i];
                note.life--;
                if(note.life <= 0) {
                    list.erase(list.begin() + i);
                    i--;
                }
            }
        }
        void output() {
            for(int i = 0; i < list.size(); i++) {
                Note& note = list[i];
                if(note.type == "info" || note.type == "warning") {
                    std::cout << "## [" + std::to_string(note.life) + "] ";
                    if(note.type == "info") {
                        std::cout << "Информация: ";
                    } else
                    if(note.type == "warning") {
                        std::cout << "Предупреждение: ";
                    }
                    std::cout << note.message << std::endl;
                } else if(note.type == "error") {
                    std::cout << "## ";
                    logger.log_error_full(
                        note.message,
                        note.macros_file_name,
                        note.macros_file_line,
                        note.macros_function_name
                    );
                }
            }
        }

        void add_info(const std::string& message) {
            Note buffer{"info", message};
            buffer.life = 2;
            list.push_back(buffer);
        }
        void add_warning(const std::string& message) {
            Note buffer{"warning", message};
            buffer.life = 3;
            list.push_back(buffer);
        }
        void add_error_full(const std::string& message, const std::string& file_name, int file_line, const std::string& function_name) {
            Note buffer{"error", message};
            buffer.life = 4;
            buffer.macros_file_name = file_name;
            buffer.macros_file_line = file_line;
            buffer.macros_function_name = function_name;
            list.push_back(buffer);
        }
    };
};
