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

#include <sys/ioctl.h>

namespace MinesweeperCPP {
    inline void console_clear() {
        std::cout << "\033[2J\033[1;1H";
    }
    using size_type = std::size_t;

    inline std::string reverse_colors(const std::string& s) {
        std::regex sgr_re("\x1B\\[([0-9;]*)m");
        std::smatch m;
        int fg = -1, bg = -1;

        if(std::regex_search(s, m, sgr_re)) {
            std::string codes = m[1].str();
            std::stringstream ss(codes);
            std::string tok;
            while (std::getline(ss, tok, ';')) {
                if(tok.empty()) continue;
                int code = std::stoi(tok);
                if((code >= 30 && code <= 37) || (code >= 90 && code <= 97)) {
                    fg = code;
                } else if((code >= 40 && code <= 47) || (code >= 100 && code <= 107)) {
                    bg = code;
                }
            }
        }

        std::string plain = std::regex_replace(s, sgr_re, std::string(""));

        int new_fg = -1, new_bg = -1;

        if(bg != -1) {
            // bg -> fg: 40..47 -> 30..37 ; 100..107 -> 90..97
            if(bg >= 40 && bg <= 47) new_fg = bg - 10;
            else if(bg >= 100 && bg <= 107) new_fg = bg - 10; // 100->90 etc
        }
        if(fg != -1) {
            // fg -> bg: 30..37 -> 40..47 ; 90..97 -> 100..107
            if(fg >= 30 && fg <= 37) new_bg = fg + 10;
            else if(fg >= 90 && fg <= 97) new_bg = fg + 10; // 90->100 etc
        }

        if(new_fg == -1 && new_bg == -1) {
            new_fg = 30;
            new_bg = 47;
        } else {
            if(new_fg == -1) new_fg = 37;
            if(new_bg == -1) new_bg = 40;
        }

        std::string prefix = "\033[" + std::to_string(new_fg) + ";" + std::to_string(new_bg) + "m";
        std::string suffix = "\033[0m";
        return prefix + plain + suffix;
    }

    namespace Keyboard {
        inline std::atomic<int> last_key{-2};
        inline std::atomic<bool> running{true};
        inline std::atomic<bool> paused{ false };
        inline termios original_tio;
        inline bool saved = false;

        inline void init() {
            if(!saved) {
                tcgetattr(STDIN_FILENO, &original_tio);
                saved = true;
            }
        }
        inline void set_raw_mode(bool enable) {
            static struct termios newt;
            if(!saved) init();
            if(enable) {
                newt = original_tio;
                newt.c_lflag &= ~(ICANON | ECHO);
                tcsetattr(STDIN_FILENO, TCSANOW, &newt);
            } else {
                tcsetattr(STDIN_FILENO, TCSANOW, &original_tio);
            }
        }
        inline void pause_input() {
            paused.store(true, std::memory_order_relaxed);
            // даём потоку время выйти из poll/read
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        inline void resume_input() {
            // очистим ввод, чтобы случайные байты не попали в следующую обработку
            tcflush(STDIN_FILENO, TCIFLUSH);
            paused.store(false, std::memory_order_relaxed);
        }
        inline int kbhit() {
            int old_flags = fcntl(STDIN_FILENO, F_GETFL);
            fcntl(STDIN_FILENO, F_SETFL, old_flags | O_NONBLOCK);

            char ch;
            int nread = read(STDIN_FILENO, &ch, 1);

            fcntl(STDIN_FILENO, F_SETFL, old_flags);

            if(nread == 1) return ch;
            return -1;
        }
        inline std::mutex key_mutex;
        inline std::queue<int> key_queue;

        inline void push_key(int key) {
            std::lock_guard<std::mutex> lock(key_mutex);
            key_queue.push(key);
        }

        inline int pop_key() {
            std::lock_guard<std::mutex> lock(key_mutex);
            if (key_queue.empty()) return -1;
            int k = key_queue.front();
            key_queue.pop();
            return k;
        }

        inline void keyboard_thread() {
            struct pollfd pfd;
            pfd.fd = STDIN_FILENO;
            pfd.events = POLLIN;

            while(running.load(std::memory_order_relaxed)) {
                if(paused.load(std::memory_order_relaxed)) {
                    // когда приостановлен — не дергаем read, просто ждём
                    std::this_thread::sleep_for(std::chrono::milliseconds(50));
                    continue;
                }

                // poll с небольшим таймаутом (100 ms) — даёт отзывчивость к paused/running
                int ret = poll(&pfd, 1, 100);
                if(ret > 0 && (pfd.revents & POLLIN)) {
                    unsigned char buf[8];
                    ssize_t n = read(STDIN_FILENO, buf, sizeof(buf));
                    if(n > 0) {
                        for(ssize_t i = 0; i < n; i++) {
                            push_key(static_cast<int>(buf[i]));
                        }
                    }
                }
                // иначе ret==0 -> timeout, продолжим и проверим флаги
            }
        }
        inline std::string read_line_manual() {
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
        }
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
            uint8_t viewport_width = 15;
            uint8_t viewport_height = 15;

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
            bool run_handle_final(const std::string &command);
            bool run_handle_exit(const std::string &command);
            void run_handle_debug(const std::string &command);

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
            Keyboard::set_raw_mode(false);
            std::string file_name;

            std::cout << "Введите полной название файла по текущему пути: ";
            std::cin >> file_name;

            game = std::make_unique<Game::MinesweeperGame>("LOAD", 1, 1, 0);
            if(game->load(file_name)) {
                game->run();
            }
            Keyboard::set_raw_mode(true);
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
                    game->run();   // тут уже всё работает, т.к. поток клавиатуры активен
                    game.reset();
                }
            }
            usleep(5000);
        }

        Keyboard::running = false;
        t.join();
        Keyboard::set_raw_mode(false);
    }
};
