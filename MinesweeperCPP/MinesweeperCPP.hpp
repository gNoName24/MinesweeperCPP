#pragma once

/*#include <stdlib.h> // clear
#include <cstdint>
#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <iomanip>
#include <random>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <thread>
#include <atomic>
#include <chrono>
#include <regex>
#include <sstream>

#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <mutex>
#include <queue>

#include <sys/ioctl.h>*/

#include <iostream>
#include <cstdint>
#include <string>
#include <unistd.h>

// Для потоков
#include <thread>
#include <atomic>
#include <chrono>
#include <mutex>
#include <queue>

// Консоль
#include <termios.h> // Получение клавиш

namespace MinesweeperCPP {
    using size_type = std::size_t;

    void console_clear();
    std::string reverse_colors(const std::string& s);

    namespace Keyboard {
        extern std::atomic<int> last_key;
        extern std::atomic<bool> running;
        extern std::atomic<bool> paused;
        extern termios original_tio;
        extern bool saved;

        extern std::mutex key_mutex;
        extern std::queue<int> key_queue;

        void init();
        void set_raw_mode(bool enable);
        void pause_input();
        void resume_input();
        int kbhit();

        void push_key(int key);
        int pop_key();

        void keyboard_thread();
        /*std::string read_line_manual() {
            std::string line;
            unsigned char c;
            while(true) {
                int ret = poll((pollfd[]){ {STDIN_FILENO, POLLIN, 0} }, 1, -1); // блокируем до ввода
                if(ret > 0) {
                    ssize_t n = read(STDIN_FILENO, &c, 1);
                    if(n == 1) {
                        if(c == '\n' || c == '\r') {
                            write(STDOUT_FILENO, "\n", 1);
                            break;
                        } else if (c == 127 || c == 8) { // backspace
                            if(!line.empty()) {
                                line.pop_back();
                                // простой backspace-эмулятор
                                const char *bs = "\b \b";
                                write(STDOUT_FILENO, bs, 3);
                            }
                        } else {
                            line.push_back(static_cast<char>(c));
                            write(STDOUT_FILENO, &c, 1);
                        }
                    }
                }
            }
            return line;
        }*/
    };

    namespace Game {
        struct Cell {
            // Хранится в сохранении
            bool open = false; // Открыт
            bool danger = false; // Мина
            bool flag = false; // Помечен флагом (при этом закрыт)

            // НЕ хранится в сохранении / Вычисляется в реалтайме
            uint8_t count = 0; // Количество мин возле безопасной открытой ячейки, 0 - пустая ячейка
        };
        struct Grid {
            size_type width{};
            size_type height{};
            std::vector<Cell> data;

            // Размеры видимой области
            uint8_t viewport_width, viewport_height;

            Grid() = default;
            Grid(size_type w, size_type h) : width(w), height(h), data(w * h) {}

            size_type total() const noexcept { return width * height; }
            bool empty() const noexcept { return data.empty(); }

            Cell& at_flat(uint32_t idx) noexcept { return data[idx]; }
            Cell& at_xy(size_type x, size_type y) noexcept { return data[y * width + x]; }

            void set_flat(uint32_t idx, Cell v) noexcept { data[idx] = v; }
            void set_xy(size_type x, size_type y, Cell v) noexcept { data[y * width + x] = v; }

            // Ресет карты. При этом, ресетуются только open, flag и count
            void reset_funny();

            // Генерация указанного количества мин в рандомных местах
            // Сначала равномерно ставится указанное количество мин, а потом все элементы карты перемешиваются
            void generate_mines(uint32_t amount);

            // Генерация числа соседних мин к безопасным ячейкам
            void generate_count();

            void open_all() {
                for(int i = 0; i < total(); i++) { data[i].open = true; }
            }
            void flag_all() {
                for(int i = 0; i < total(); i++) { data[i].flag = true; }
            }

            bool open(size_type x, size_type y, uint32_t& step_counter);
            bool flag(size_type x, size_type y, const size_type& total_mines, uint32_t& step_counter);

            void open_recurs(size_type x, size_type y);

            // Общее количество установленных флагов
            size_type flag_count_total() const;
            // Общее количество правильно установленных флагов
            size_type flag_count_success() const;

            bool check_win(const size_type& total_mines) const;
        };

        class MinesweeperGame {
        public:
            MinesweeperGame(const std::string& _name, const uint16_t& _width, const uint16_t& _height, const uint32_t _amount_mines)
                : name(_name), map_width(_width), map_height(_height), map_amount_mines(_amount_mines), map(map_width, map_height)
            {}

            // Сохранение игры в файл
            void save(const std::string& file_name);
            bool load(const std::string& file_name);
            uint8_t save_cellpack(const Cell& cell); // Упаковка ячейки в один байт
            Cell save_cellunpack(uint8_t b); // Распаковка одного байта в ячейку

            void map_render();

            void run();
            void run_handle_movement(const int& key);
            void run_handle_save(const int& key);
            void run_handle_step(const int& key);
            bool run_handle_final(const int& key);
            bool run_handle_exit(const int& key);
            void run_handle_debug(const int& key);

        private:
            std::string name;
            uint16_t map_width, map_height;
            uint32_t map_amount_mines;
            uint32_t step_counter = 0;
            Grid map;
            bool starter = true;
            bool defeat = false;
            bool winner = false;

            // Cursor
            uint16_t cursor_position_x = 0;
            uint16_t cursor_position_y = 0;

        };
    };

    inline std::unique_ptr<Game::MinesweeperGame> game = nullptr;

    // Сцены
    namespace Scenes {
        inline void game_new() {
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
        inline void game_load() {
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
        inline bool menu_main(const int& key) {
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
    };

    inline void starter() {
        Keyboard::init();
        Keyboard::set_raw_mode(true);
        std::thread t(Keyboard::keyboard_thread);

        console_clear();
        Scenes::menu_main(-2);

        int key;
        while(true) {
            key = Keyboard::pop_key();
            if(key != -1) {
                console_clear();
                if(Scenes::menu_main(key)) {
                    std::cout << "Завершение" << std::endl;
                    break;
                }
                if (game) {
                    console_clear();
                    std::cout << "Нажмите на (почти) любую клавишу чтобы начать игру\n";
                    game->run();
                    game.reset();
                }
            }
            usleep(1000);
        }

        Keyboard::running = false;
        t.join();
        Keyboard::set_raw_mode(false);
    }
};
