#include "MinesweeperCPP.hpp"

#include <random>
#include <algorithm>

namespace MinesweeperCPP {
    namespace Game {
        // Grid
        void Grid::reset_funny() {
            for(int i = 0; i < total(); i++) {
                Cell& cell = data[i];
                cell.open = false;
                cell.flag = false;
                cell.count = 0;
            }
            generate_count();
        }
        void Grid::generate_mines(uint32_t amount) {
            std::cout << "-> Генерация мин" << std::endl;
            std::vector<std::pair<int, int>> coords;
            coords.reserve(width * height);

            for(int y = 0; y < height; ++y) {
                for(int x = 0; x < width; ++x) {
                    coords.emplace_back(x, y);
                }
            }

            thread_local std::random_device rd;
            thread_local std::mt19937 gen(rd());
            std::shuffle(coords.begin(), coords.end(), gen);

            for(uint32_t i = 0; i < amount && i < coords.size(); ++i) {
                at_xy(coords[i].first, coords[i].second).danger = true;
            }
        }

        void Grid::generate_count() {
            std::cout << "-> Генерация количества" << std::endl;
            for(size_type row = 0; row < height; ++row) {
                for(size_type col = 0; col < width; ++col) {
                    Cell &cell = at_xy(col, row);

                    if(cell.danger) { // если это мина
                        cell.count = 0;
                        continue;
                    }

                    int neighbour_mines = 0;

                    for(int offset_row = -1; offset_row <= 1; ++offset_row) {
                        for(int offset_col = -1; offset_col <= 1; ++offset_col) {
                            if(offset_row == 0 && offset_col == 0) continue; // пропускаем саму клетку

                            size_type neighbour_col = col + offset_col;
                            size_type neighbour_row = row + offset_row;

                            if(neighbour_col < width && neighbour_row < height) {
                                if(at_xy(neighbour_col, neighbour_row).danger) {
                                    neighbour_mines++;
                                }
                            }
                        }
                    }

                    cell.count = neighbour_mines;
                }
            }
        }

        bool Grid::open(size_type x, size_type y, uint32_t& step_counter) {
            std::cout << "-> Открытие ячейки " << x << "x" << y << "" << std::endl;
            if(x >= width || y >= height) return false;
            if(at_xy(x, y).danger) { // Если попался на мину
                return true;
            } else {
                open_recurs(x, y);
                step_counter++;
                return false;
            }
        }
        bool Grid::flag(size_type x, size_type y, const size_type& total_mines, uint32_t& step_counter) {
            std::cout << "-> Пометка ячейки " << x << "x" << y << " флагом" << std::endl;
            if(x >= width || y >= height) return false;
            Cell& cell = at_xy(x, y);
            if(!cell.open) {
                if(!cell.flag) {
                    if(flag_count_total() < total_mines) {
                        cell.flag = true;
                        step_counter++;
                    }
                } else {
                    cell.flag = false;
                    step_counter++;
                }
                return check_win(total_mines);
            }
            return false;
        }

        void Grid::open_recurs(size_type x, size_type y) {
            if(x >= width || y >= height) return;
            Cell &cell = at_xy(x, y);

            // Если уже открыта или стоит флаг
            if(cell.open || cell.flag) return;

            cell.open = true;

            // Если рядом есть мины
            if(cell.count > 0) return;

            for(int dy = -1; dy <= 1; ++dy) {
                for(int dx = -1; dx <= 1; ++dx) {
                    if(dx == 0 && dy == 0) continue;

                    int nx = static_cast<int>(x) + dx;
                    int ny = static_cast<int>(y) + dy;

                    if(nx >= 0 && ny >= 0 && nx < static_cast<int>(width) && ny < static_cast<int>(height)) {
                        open_recurs(nx, ny);
                    }
                }
            }
        }

        size_type Grid::flag_count_total() const {
            size_type flag_total = 0;
            for(int i = 0; i < total(); i++) {
                if(data[i].flag) {
                    flag_total++;
                }
            }
            return flag_total;
        }
        size_type Grid::flag_count_success() const {
            size_type flag_success = 0;
            for(int i = 0; i < total(); i++) {
                if(data[i].danger && !data[i].open && data[i].flag) {
                    flag_success++;
                }
            }
            return flag_success;
        }

        bool Grid::check_win(const size_type& total_mines) const {
            size_type success_flags = flag_count_success();
            if(success_flags == total_mines) {
                return true;
            }
            return false;
        }
    };
};
