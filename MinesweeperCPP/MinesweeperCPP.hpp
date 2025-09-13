#pragma once

#include <stdlib.h> // clear
#include <cstdint>
#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <iomanip>
#include <random>
#include <algorithm>

namespace MinesweeperCPP {
    inline void console_clear() {
        system("clear");
    }
    using size_type = std::size_t;

    namespace Game {
        struct Cell {
            // Хранится в сохранении
            bool open = true; // Открыт ли
            bool danger = false; // Мина ли
            bool flag = false; // Помечен ли флагом (при этом закрыт)

            // НЕ хранится в сохранении. Вычисляется в реалтайме
            uint8_t count = 0; // Количество мин возле безопасной открытой ячейки, 0 - пустая ячейка
        };
        struct Grid {
            size_type width{};
            size_type height{};
            std::vector<Cell> data;

            Grid() = default;
            Grid(size_type w, size_type h) : width(w), height(h), data(w * h) {}

            size_type total() const noexcept { return width * height; }
            bool empty() const noexcept { return data.empty(); }

            void generate_mines(uint32_t amount) {
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

            Cell& at_flat(uint32_t idx) noexcept { return data[idx]; }
            Cell& at_xy(size_type x, size_type y) noexcept { return data[y * width + x]; }

            void set_flat(uint32_t idx, Cell v) noexcept { data[idx] = v; }
            void set_xy(size_type x, size_type y, Cell v) noexcept { data[y * width + x] = v; }

            void open_all() {
                for(int i = 0; i < total(); i++) {
                    data[i].open = true;
                }
            }

            bool open(size_type x, size_type y) {
                if(x >= width || y >= height) return false;
                if(at_xy(x, y).danger) { // Если попался на мину
                    return true;
                } else {
                    open_recurs(x, y);
                    return false;
                }
            }
            void flag(size_type x, size_type y) {
                if(x >= width || y >= height) return;
                Cell& cell = at_xy(x, y);
                if(!cell.open && !cell.flag) {
                    cell.flag = true;
                }
            }

            void open_recurs(size_type x, size_type y) {
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
            void generate_count() {
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
        };

        class MinesweeperGame {
        public:
            MinesweeperGame(const std::string& _name, const uint16_t& _width, const uint16_t& _height, const uint32_t _amount_mines)
                : name(_name), map_width(_width), map_height(_height), map_amount_mines(_amount_mines), map(map_width, map_height)
            {}

            void run() {
                while(true) {
                    console_clear();
                    std::string command = "";

                    map_render();

                    std::cout << std::endl;

                    std::cout <<
                        "# - Закрытая ячейка\n"
                        "1-9 - Соседние мины открытой безопасной ячейки\n" <<
                        "0 - Мина\n" <<
                        "! - Закрытая ячейка помеченная флагом\n\n";

                    if(!defeat) {
                        std::cout <<
                            "open - Открыть ячейку\n";
                        if(!starter) {
                            std::cout << "flag - Пометить закрытую ячейку флагом\n";
                        }
                    } else {
                        std::cout << "Вы проиграли :(" << std::endl;
                        std::cout << "Введите imaloser чтобы выйти в главное меню" << std::endl;
                    }

                    std::cout << std::endl << "> ";
                    std::cin >> command;

                    if(defeat) {
                        if(command == "imaloser") {
                            break;
                        }
                        continue;
                    }
                    if(command == "open" || command == "flag") {
                        size_type cx, cy;

                        std::cout << "Введите координату x: ";
                        std::cin >> cx;

                        std::cout << "Введите координату y: ";
                        std::cin >> cy;

                        if(command == "open") {
                            if(starter) {
                                map.generate_mines(map_amount_mines);
                                map.generate_count();
                                starter = false;
                            }
                            if(map.open(cx, cy)) {
                                defeat = true;
                                map.open_all();
                            }
                        }
                        if(command == "flag" && !starter) {
                            map.flag(cx, cy);
                        }
                    }
                }
            }

            void map_render() {
                size_type max_width = 2;

                std::cout << "   ";
                for(size_type x = 0; x < map.width; ++x) {
                    std::cout << " " << std::setw(max_width) << x;
                }
                std::cout << '\n';

                std::string line;
                line.reserve((max_width + 1) * map.width + 4);
                line += "   ";
                for(size_type i = 0; i < map.width; ++i) {
                    line += "+"; line += std::string(max_width, '-');
                }
                line += '+';
                std::cout << line << '\n';

                for(size_type y = 0; y < map.height; ++y) {
                    std::cout << std::setw(2) << y << " ";
                    for(size_type x = 0; x < map.width; ++x) {
                        Game::Cell cell = map.at_xy(x, y);
                        std::cout << '|' << std::setw(static_cast<int>(max_width));

                        if(!cell.open) { // Ячейка не открыта
                            if(cell.flag) { // Ячейка помечена флагом
                                std::cout << "\033[43m!" << std::string(max_width - 1, ' ') << "\033[0m";
                            } else {
                                std::cout << "\033[47m#" << std::string(max_width - 1, ' ') << "\033[0m";
                            }
                        } else { // Ячейка открыта
                            if(cell.danger) { // Если мина
                                std::cout << "\033[0;31m0" << std::string(max_width - 1, ' ') << "\033[0m";
                            } else { // Если не мина
                                if(cell.count == 0) { // 0 значит просто пустая клетка
                                    std::cout << " ";
                                } else {
                                    const char* colors[8] = {
                                        "\033[0;32m", // Зеленый
                                        "\033[1;32m", // Ярко-зеленый
                                        "\033[0;36m", // Бирюзовый (переход к желтому)
                                        "\033[0;33m", // Желтый
                                        "\033[1;33m", // Ярко-желтый (почти оранжевый)
                                        "\033[0;31m", // Красный
                                        "\033[1;31m", // Ярко-красный
                                        "\033[0;35m"  // Пурпурный (насыщенный красный край)
                                    };
                                    std::cout << colors[cell.count - 1] << std::to_string(cell.count) << std::string(max_width - 1, ' ') << "\033[0m";
                                }
                            }
                        }
                    }
                    std::cout << '|' << '\n' << line << '\n';
                }
            }

        private:
            const std::string name;
            const uint16_t map_width, map_height;
            const uint32_t map_amount_mines;
            Grid map;
            bool starter = true;
            bool defeat = false;

        };
    };

    inline std::unique_ptr<Game::MinesweeperGame> game = nullptr;

    // Сцены
    namespace Scenes {
        inline void game_new() {
            std::string command = "";

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

            game->run();
        }
        inline void game_load() {
            std::string command = "";


        }
        inline void menu_main() {
            std::string command = "";

            std::cout << "- MinesweeperCPP by NoName24" << std::endl << std::endl;
            std::cout << "new - Начать новую игру" << std::endl;
            std::cout << "load - Загрузить игру из файла" << std::endl << std::endl;

            /*game = std::make_unique<Game::MinesweeperGame>("test", 16, 16, 32);
            game->run();*/

            std::cout << "> ";
            std::cin >> command;

            if(command == "new") {
                game_new();
            }
            if(command == "load") {
                game_load();
            }
        }
    };

    inline void starter() {
        while(true) {
            console_clear();
            Scenes::menu_main();
        }
    }
};
