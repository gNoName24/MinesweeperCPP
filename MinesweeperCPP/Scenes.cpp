#include "MinesweeperCPP.hpp"

namespace MinesweeperCPP {
    namespace Scenes {
        bool menu_main(const int& key) {
            console_clear();
            std::cout << "- MinesweeperCPP by NoName24\n\n";

            std::cout << "n - Начать новую игру\n";
            std::cout << "l - Загрузить игру из файла\n";
            std::cout << "q - Завершить работу программы\n";
            std::cout << "\n";

            Notifications::output();
            Notifications::step();

            switch(key) {
                case 'q':
                    return true;
                    break;

                case 'n':
                    game_new();
                    break;
                case 'l':
                    game_load();
                    break;

                    // DEBUG
                case 'd':
                    /*game = std::make_unique<Game::MinesweeperGame>("DEBUGGAME", 16, 16, 16);
                    game->run();*/
                    game = std::make_unique<Game::MinesweeperGame>("LOAD", 1, 1, 0);
                    if(game->load("DEBUGGAME")) {}
                    game->mode_history = true;
                    break;

                case -2:
                    break;
                default:
                    std::cout << "Неизвестная команда\n";
                    break;
            }
            return false;
        }
        void game_load() {
            Keyboard::pause_input();
            Keyboard::set_raw_mode(false);
            std::string file_name;
            int mode = 0;
            std::string input;

            std::cout << "В каком режиме загрузить сохранение?\n";
            std::cout << "1 - В обычном режиме\n";
            std::cout << "2 - В режиме просмотра истории\n";
            std::cout << "\n> ";
            std::getline(std::cin, input);
            std::stringstream ss(input);
            if(ss >> mode && ss.eof()) {
                if(mode == 1 || mode == 2) {
                    std::cout << "\nВведите полной название файла по текущему пути: ";
                    std::getline(std::cin >> std::ws, file_name);

                    game = std::make_unique<Game::MinesweeperGame>("LOAD", 1, 1, 0);
                    bool game_b = game->load(file_name);
                    if(!game_b) {
                        game.reset();
                    } else {
                        switch(mode) {
                            case 1:
                                break;
                            case 2:
                                game->mode_history = true;
                                break;
                        }
                    }
                } else {
                    Notifications::add_warning("Введен невалидный режим");
                }
            } else {
                Notifications::add_warning("Введено не число");
            }
            menu_main(-2);

            Keyboard::set_raw_mode(true);
            Keyboard::resume_input();
        }
        void game_new() {
            Keyboard::pause_input();
            Keyboard::set_raw_mode(false);
            std::string game_name;
            uint16_t game_map_width, game_map_height;
            uint32_t game_amount_mines;

            // game_name
            std::cout << "Введите название игры: ";
            std::getline(std::cin >> std::ws, game_name);

            // game_map_width
            while(true) {
                std::string input;
                std::cout << "Введите ширину карты: ";
                std::getline(std::cin, input);

                std::stringstream ss(input);
                unsigned int tmp;
                if(ss >> tmp && ss.eof() && tmp > 0 && tmp <= std::numeric_limits<uint16_t>::max()) {
                    game_map_width = static_cast<uint16_t>(tmp);
                    break;
                }
                std::cout << "Введите корректное число от 1 до " << std::numeric_limits<uint16_t>::max() << "\n";
            }

            // game_map_height
            while(true) {
                std::string input;
                std::cout << "Введите высоту карты: ";
                std::getline(std::cin, input);

                std::stringstream ss(input);
                unsigned int tmp;
                if(ss >> tmp && ss.eof() && tmp > 0 && tmp <= std::numeric_limits<uint16_t>::max()) {
                    game_map_height = static_cast<uint16_t>(tmp);
                    break;
                }
                std::cout << "Введите корректное число от 1 до " << std::numeric_limits<uint16_t>::max() << "\n";
            }

            // game_amount_mines
            while(true) {
                std::string input;
                std::cout << "Введите количество мин на всю карту (не больше "  << (game_map_width * game_map_height) << "): ";
                std::getline(std::cin, input);

                std::stringstream ss(input);
                unsigned long long tmp;
                if(ss >> tmp && ss.eof() && tmp <= static_cast<unsigned long long>(game_map_width) * game_map_height) {
                    game_amount_mines = static_cast<uint32_t>(tmp);
                    break;
                }
                std::cout << "Введите корректное число от 0 до " << (game_map_width * game_map_height) << "\n";
            }

            game = std::make_unique<Game::MinesweeperGame>(game_name, game_map_width, game_map_height, game_amount_mines);
            Keyboard::set_raw_mode(true);
            Keyboard::resume_input();
        }
    };
};
