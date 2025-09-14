#include "MinesweeperCPP.hpp"

#include <fcntl.h>
#include <poll.h>

namespace MinesweeperCPP {
    namespace Keyboard {
        std::atomic<int> last_key{-2};
        std::atomic<bool> running{true};
        std::atomic<bool> paused{false};
        termios original_tio;
        bool saved = false;

        std::mutex key_mutex;
        std::queue<int> key_queue;

        void init() {
            if(!saved) {
                tcgetattr(STDIN_FILENO, &original_tio);
                saved = true;
            }
        }
        void set_raw_mode(bool enable) {
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
        void pause_input() {
            paused.store(true, std::memory_order_relaxed);
            // даём потоку время выйти из poll/read
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        void resume_input() {
            // очистим ввод, чтобы случайные байты не попали в следующую обработку
            tcflush(STDIN_FILENO, TCIFLUSH);
            paused.store(false, std::memory_order_relaxed);
        }
        int kbhit() {
            int old_flags = fcntl(STDIN_FILENO, F_GETFL);
            fcntl(STDIN_FILENO, F_SETFL, old_flags | O_NONBLOCK);

            char ch;
            int nread = read(STDIN_FILENO, &ch, 1);

            fcntl(STDIN_FILENO, F_SETFL, old_flags);

            if(nread == 1) return ch;
            return -1;
        }

        void push_key(int key) {
            std::lock_guard<std::mutex> lock(key_mutex);
            key_queue.push(key);
        }
        int pop_key() {
            std::lock_guard<std::mutex> lock(key_mutex);
            if (key_queue.empty()) return -1;
            int k = key_queue.front();
            key_queue.pop();
            return k;
        }
        void keyboard_thread() {
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
    };
};
