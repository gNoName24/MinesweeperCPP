#include "../../MinesweeperCPP.hpp"

namespace MinesweeperCPP {
    namespace Game {
        void MinesweeperGame::run_history() {
            Keyboard::set_raw_mode(true);
            while(true) {
                int key;
                bool got = false;
                bool quit = false;
                if(!mhistory_replay) {
                    while((key = Keyboard::pop_key()) != -1) {
                        run_default_handle_movement(key);
                        if(run_default_handle_exit(key)) {
                            quit = true;
                        }

                        run_history_handle_replay(key);

                        got = true;
                    }
                } else {
                    key = Keyboard::pop_key();
                    run_default_handle_movement(key);
                    run_history_handle_replay(key);
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

                    std::cout << get_title() << "\n\n";

                    map_render();

                    std::cout << std::endl;

                    std::cout << "{" << cursor_position_x << ", " << cursor_position_y << "}" << std::endl;

                    std::cout << "Воспроизводится ли: " << std::to_string(mhistory_replay) << "\n";
                    std::cout << "Текущий шаг: " << std::to_string(mhistory_replay_step) << "/" << (mhistory_replay_step_max-1) << "\n";
                    std::cout << "Задержка между шагами: " << std::to_string(mhistory_replay_sleep_milliseconds) << " ms\n";

                    std::cout << "\ns - ";
                    if(!mhistory_replay_step) { std::cout << "Начать";
                    } else { std::cout << "Остановить"; }
                    std::cout << " воспроизводить шаги начиная с текущей\n";

                    std::cout << "\nПередвижение курсора по карте:\n";
                    std::cout << "h - Влево / ";
                    std::cout << "j - Вниз / ";
                    std::cout << "k - Вверх / ";
                    std::cout << "l - Вправо\n";

                    Notifications::output();
                    Notifications::step();

                    // Если mhistory_replay запущен
                    if(mhistory_replay) {
                        std::this_thread::sleep_for(std::chrono::milliseconds(mhistory_replay_sleep_milliseconds));

                        run_history_step(mhistory_replay_step);

                        mhistory_replay_step++;
                        if(mhistory_replay_step >= mhistory_replay_step_max) {
                            mhistory_replay = false;
                            mhistory_replay_step = 0;
                        }
                        continue;
                    }
                }
                usleep(1000);
            }
        }
        void MinesweeperGame::run_history_step(int step) {
            StepHistory& buffer = history[step];
            if(buffer.set_open) {
                map.open(buffer.cursor_position_x, buffer.cursor_position_y, step_counter);
            }
            if(buffer.set_flag) {
                map.flag(buffer.cursor_position_x, buffer.cursor_position_y, map_amount_mines, step_counter);
            }
        }

        // Handle
        void MinesweeperGame::run_history_handle_replay(const int& key) {
            if(key == 's') {
                mhistory_replay = !mhistory_replay;
                if(mhistory_replay_step == 0) {
                    map.reset_funny();
                }
                //std::cout << "Нажмите на (почти) любую клавишу\n";
            }
        }
    };
};
