#include "../MinesweeperCPP.hpp"


namespace MinesweeperCPP {
    namespace Game {
        void MinesweeperGame::map_render() {
            it_camera_position_x camera_position_x = cursor_position_x;
            it_camera_position_y camera_position_y = cursor_position_y;

            it_viewport_width half_viewport_width = static_cast<int>(map.viewport_width)/2;
            it_viewport_heigth half_viewport_height = static_cast<int>(map.viewport_height)/2;

            for(it_camera_position_x y = half_viewport_height + camera_position_y; y > -half_viewport_height+ + camera_position_y; --y) {
                for(it_camera_position_y x = -half_viewport_width + camera_position_x; x < half_viewport_width + camera_position_x; ++x) {
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

        std::string MinesweeperGame::get_title() {
            std::string mode;
            if(!mode_history) {
                mode = "Обычный режим";
            }
            if(mode_history) {
                mode = "Режим истории";
            }

            std::string buffer;
            buffer =
                " - " +
                name + // Название
                " - " + std::to_string(map_width) + "x" + std::to_string(map_height) + // Ширина и высота
                " - " + std::to_string(map_amount_mines) + // Количество мин
                " - " + mode // Название режима
                + " - ";
            return buffer;
        }

        void MinesweeperGame::run() {
            if(!mode_history) {
                run_default();
            }
            if(mode_history) {
                mhistory_replay_step_max = history.size();
                run_history();
            }
        }
    };
};
