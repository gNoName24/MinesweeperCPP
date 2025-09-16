#include "MinesweeperCPP.hpp"

namespace MinesweeperCPP {
    namespace Scenes {
        bool menu_main(const int& key) {
            std::cout << "- MinesweeperCPP by NoName24\n\n";

            std::cout << "n - Начать новую игру\n";
            std::cout << "l - Загрузить игру из файла\n";
            std::cout << "q - Завершить работу программы\n";
            std::cout << "\n";

            // Завершение работы
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
                    game = std::make_unique<Game::MinesweeperGame>("DEBUGGAME", 32, 32, 128);
                    game->run();
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

            std::cout << "Введите полной название файла по текущему пути: ";
            std::cin >> file_name;

            game = std::make_unique<Game::MinesweeperGame>("LOAD", 1, 1, 0);
            if(game->load(file_name)) {}
            Keyboard::set_raw_mode(true);
            Keyboard::resume_input();
        }
        void game_new() {
            Keyboard::pause_input();
            Keyboard::set_raw_mode(false);
            std::string game_name;
            uint16_t game_map_width, game_map_heigth;
            uint32_t game_amount_mines;

            // game_name
            std::cout << "Введите название игры: ";
            std::cin >> game_name;

            // game_map_width
            std::cout << "Введите ширину карты: ";
            std::cin >> game_map_width;

            // game_map_heigth
            std::cout << "Введите высоту карты: ";
            std::cin >> game_map_heigth;

            // game_amount_mines
            std::cout << "Введите количество мин на всю карту (не больше " << (game_map_width * game_map_heigth) << "): ";
            std::cin >> game_amount_mines;

            game = std::make_unique<Game::MinesweeperGame>(game_name, game_map_width, game_map_heigth, game_amount_mines);
            Keyboard::set_raw_mode(true);
            Keyboard::resume_input();
        }
    };
};
