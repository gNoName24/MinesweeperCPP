#include "MinesweeperCPP.hpp"

namespace MinesweeperCPP {
    namespace Game {
        void MinesweeperGame::map_render() {
            size_type max_width = 2;

            uint16_t camera_position_x = cursor_position_x;
            uint16_t camera_position_y = cursor_position_y;

            int half_map_width = static_cast<int>(map.viewport_width)/2;
            int half_map_height = static_cast<int>(map.viewport_height)/2;

            for(int y = half_map_height + camera_position_y; y > -half_map_height + camera_position_y; --y) {
                for(int x = -half_map_width + camera_position_x; x < half_map_width + camera_position_x; ++x) {
                    if(x < 0 || x >= map_width || y < 0 || y >= map_height) {
                        std::cout << "  ";
                        continue;
                    }

                    Game::Cell cell = map.at_xy(x, y);

                    std::string cell_str;

                    if(!cell.open) {
                        if(cell.flag) {
                            cell_str = std::string("\033[43m") + "!" + " ";
                        } else {
                            cell_str = std::string("\033[47m") + "#" + " ";
                        }
                    } else {
                        if(cell.danger) {
                            cell_str = std::string("\033[0;31m") + "0" + " ";
                        } else if(cell.count == 0) {
                            cell_str = "  ";
                        } else {
                            const char* colors[8] = {
                                "\033[0;32m", "\033[1;32m", "\033[0;36m", "\033[0;33m",
                                "\033[1;33m", "\033[0;31m", "\033[1;31m", "\033[0;35m"
                            };
                            cell_str = std::string(colors[cell.count - 1]) + std::to_string(cell.count) + " ";
                        }
                    }

                    if(x == cursor_position_x && y == cursor_position_y) {
                        std::cout << reverse_colors(cell_str);
                    } else {
                        std::cout << cell_str << "\033[0m";
                    }
                }
                std::cout << '\n';
            }
        }

        void MinesweeperGame::run() {
            Keyboard::set_raw_mode(true);
            int key;
            while(true) {
                int key;
                bool got = false;
                bool quit = false;
                while((key = Keyboard::pop_key()) != -1) {
                    run_handle_movement(key);
                    run_handle_save(key);
                    run_handle_step(key);
                    if(run_handle_final(key) || run_handle_exit(key)) {
                        quit = true;
                    }

                    run_handle_debug(key);
                    got = true;
                }

                if(quit) {
                    console_clear();
                    Scenes::menu_main(-2);
                    break;
                }

                if(got) {
                    struct winsize w;
                    if(ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1) {
                        perror("ioctl");
                    }
                    map.viewport_width = w.ws_col / 2;
                    map.viewport_height = w.ws_row / 2;

                    console_clear();

                    size_type flag_count_total = map.flag_count_total();

                    std::cout << " - " << name << " - " << std::endl << std::endl;

                    map_render();

                    std::cout << std::endl;

                    std::cout << "{" << cursor_position_x << ", " << cursor_position_y << "}" << std::endl;

                    std::cout << "Поставлено " << flag_count_total << " из " << map_amount_mines << " флагов\n";
                    std::cout << "Сделано " << step_counter << " шагов\n";

                    std::cout <<
                        "\033[47m#\033[0m - Закрытая ячейка\n"
                        "\033[0;32m1\033[0m-\033[0;35m8\033[0m - Соседние мины открытой безопасной ячейки\n" <<
                        "\033[0;31m0\033[0m - Мина\n" <<
                        "\033[43m!\033[0m - Закрытая ячейка, помеченная флагом\n\n";
                    if(defeat) {
                        std::cout << "\nВы проиграли :(\n";
                        std::cout << "Нажмите c чтобы выйти в главное меню\n";
                    } else
                    if(winner) {
                        std::cout << "\nВы выиграли!\n";
                        std::cout << "Нажмите c чтобы выйти в главное меню\n";
                    } else {
                        std::cout << "o - Открыть ячейку на позиции курсора\n";
                        if(!starter && flag_count_total < map_amount_mines) {
                            std::cout << "f - Пометить закрытую ячейку флагом на позиции курсора\n";
                        }
                    }
                    std::cout << "q - Выйти сейчас же\n";
                    std::cout << "s - Сохранить игру на текущем моменте\n\n";

                    std::cout << "Передвижение курсора по карте:\n";
                    std::cout << "h - Вверх / ";
                    std::cout << "j - Вниз / ";
                    std::cout << "k - Влево / ";
                    std::cout << "l - Вправо\n";
                }
                usleep(1000);
            }
        }
        void MinesweeperGame::run_handle_movement(const int& key) {
            uint16_t dump_x, dump_y;
            switch(key) {
                case 'h':
                    cursor_position_y++;
                    if(cursor_position_y >= map_height) {
                        cursor_position_y = map_height - 1;
                    }
                    break;
                case 'j':
                    dump_y = cursor_position_y + 1;
                    dump_y--;
                    if(dump_y <= 1) {
                        cursor_position_y = 0;
                    } else {
                        cursor_position_y--;
                    }
                    break;
                case 'k':
                    dump_x = cursor_position_x + 1;
                    dump_x--;
                    if(dump_x <= 1) {
                        cursor_position_x = 0;
                    } else {
                        cursor_position_x--;
                    }
                    break;
                case 'l':
                    cursor_position_x++;
                    if(cursor_position_x >= map_width) {
                        cursor_position_x = map_width - 1;
                    }
                    break;
            }
        }
        void MinesweeperGame::run_handle_save(const int& key) {
            if(key == 's') {
                save(name);
            }
        }
        void MinesweeperGame::run_handle_step(const int& key) {
            if(key == 'o') {
                if(starter) {
                    map.generate_mines(map_amount_mines);
                    map.generate_count();
                    starter = false;
                }
                if(map.open(cursor_position_x, cursor_position_y, step_counter)) {
                    defeat = true;
                    map.open_all();
                }
            } else
            if(key == 'f' && !starter) {
                if(map.flag(cursor_position_x, cursor_position_y, map_amount_mines, step_counter)) {
                    winner = true;
                }
            }
        }
        bool MinesweeperGame::run_handle_final(const int& key) {
            if(defeat && key == 'c') {
                return true;
            }
            if(winner && key == 'c') {
                return true;
            }
            return false;
        }
        bool MinesweeperGame::run_handle_exit(const int& key) {
            if(key == 'q') {
                return true;
            }
            return false;
        }
        void MinesweeperGame::run_handle_debug(const int& key) {
            switch(key) {
                case '1': // Открытие всех ячеек
                    map.open_all();
                    break;
            }
        }
    };
};
