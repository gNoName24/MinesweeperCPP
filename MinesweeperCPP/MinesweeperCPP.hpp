#pragma once

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
        bool menu_main(const int& key);
        void game_new();
        void game_load();
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
