#include "../../MinesweeperCPP.hpp"

namespace MinesweeperCPP {
    namespace Game {
        void MinesweeperGame::run_default() {
            Keyboard::set_raw_mode(true);
            while(true) {
                int key;
                bool got = false;
                bool quit = false;
                while((key = Keyboard::pop_key()) != -1) {
                    run_default_handle_movement(key);
                    run_default_handle_save(key);
                    run_default_handle_step(key);
                    if(run_default_handle_final(key) || run_default_handle_exit(key)) {
                        quit = true;
                    }

                    run_default_handle_debug(key);
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
                        logger.log_error("ioctl");
                        perror("ioctl");
                    }
                    map.viewport_width = w.ws_col / 2;
                    map.viewport_height = w.ws_row / 2;

                    console_clear();

                    uit_map_flags flag_count_total = map.flag_count_total();

                    std::cout << get_title() << "\n\n";

                    map_render();

                    std::cout << std::endl;

                    std::cout << "{" << cursor_position_x << ", " << cursor_position_y << "}" << std::endl;

                    std::cout << "Поставлено " << flag_count_total << " из " << map_amount_mines << " флагов\n";
                    std::cout << "Сделано " << step_counter << " шагов\n";

                    /*std::cout <<
                     *               "\033[47m#\033[0m - Закрытая ячейка\n"
                     *               "\033[0;32m1\033[0m-\033[0;35m8\033[0m - Соседние мины открытой безопасной ячейки\n" <<
                     *               "\033[0;31m0\033[0m - Мина\n" <<
                     *               "\033[43m!\033[0m - Закрытая ячейка, помеченная флагом\n\n";*/
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
                    std::cout << "s - Сохранить игру на текущем моменте\n";

                    std::cout << "\nПередвижение курсора по карте:\n";
                    std::cout << "h - Влево / ";
                    std::cout << "j - Вниз / ";
                    std::cout << "k - Вверх / ";
                    std::cout << "l - Вправо\n";

                    Notifications::output();
                    Notifications::step();
                }
                usleep(1000);
            }
        }

        // Handle
        void MinesweeperGame::run_default_handle_movement(const int& key) {
            uit_map_width dump_x;
            uit_map_heigth dump_y;
            switch(key) {
                case 'h': // Влево
                    dump_x = cursor_position_x + 1;
                    dump_x--;
                    if(dump_x <= 1) {
                        cursor_position_x = 0;
                    } else {
                        cursor_position_x--;
                    }
                    break;
                case 'j': // Вниз
                    dump_y = cursor_position_y + 1;
                    dump_y--;
                    if(dump_y <= 1) {
                        cursor_position_y = 0;
                    } else {
                        cursor_position_y--;
                    }
                    break;
                case 'k': // Вверх
                    cursor_position_y++;
                    if(cursor_position_y >= map_height) {
                        cursor_position_y = map_height - 1;
                    }
                    break;
                case 'l': // Вправо
                    cursor_position_x++;
                    if(cursor_position_x >= map_width) {
                        cursor_position_x = map_width - 1;
                    }
                    break;
            }
        }
        void MinesweeperGame::run_default_handle_save(const int& key) {
            if(key == 's') {
                save(name);
            }
        }
        void MinesweeperGame::run_default_handle_step(const int& key) {
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
                // Запись в историю
                history_new_now(true, false);
            } else
                if(key == 'f' && !starter) {
                    if(map.flag(cursor_position_x, cursor_position_y, map_amount_mines, step_counter)) {
                        winner = true;
                    }
                    // Запись в историю
                    history_new_now(false, true);
                }
        }
        bool MinesweeperGame::run_default_handle_final(const int& key) {
            if(defeat || winner) {
                if(key == 'c') {
                    return true;
                }
            }
            return false;
        }
        bool MinesweeperGame::run_default_handle_exit(const int& key) {
            if(key == 'q') {
                return true;
            }
            return false;
        }
        void MinesweeperGame::run_default_handle_debug(const int& key) {
            switch(key) {
                case '1': // Открытие всех ячеек
                    map.open_all();
                    break;
                case '2': // Смена состояния starter true/false
                    starter = !starter;
                    break;
                case '3': // Смена состояния defeat true/false
                    defeat = !defeat;
                    break;
                case '4': // Смена состояния winner true/false
                    winner = !winner;
                    break;
            }
        }
    };
};
