#include "../MinesweeperCPP.hpp"

#include <fstream>
#include <filesystem>

namespace MinesweeperCPP {
    namespace Game {
        void MinesweeperGame::save(const std::string& file_name) {
            // NoName24 MinesweeperCPP Save File = N24MSCSF
            std::ofstream file(std::filesystem::path(std::filesystem::current_path() / (file_name + ".n24mscsf")), std::ios::binary);
            if(!file.is_open()) return;

            // magic numbers - NoName24
            const char magic_numbers_1[] = "NoName24";
            file.write(magic_numbers_1, sizeof(magic_numbers_1) - 1);

            // magic numbers - MinesweeperCPP Save File (MSCSF)
            const char magic_numbers_2[] = "MSCSF";
            file.write(magic_numbers_2, sizeof(magic_numbers_2) - 1);

            // Название игры. Максимальная длина - 255 символов
            uint8_t name_len = name.size();
            file.write(reinterpret_cast<const char*>(&name_len), sizeof(name_len));
            file.write(name.data(), name_len);

            file.write(reinterpret_cast<const char*>(&map_width), sizeof(map_width)); // uint16_t
            file.write(reinterpret_cast<const char*>(&map_height), sizeof(map_height)); // uint16_t
            file.write(reinterpret_cast<const char*>(&map_amount_mines), sizeof(map_amount_mines)); // uint32_t
            file.write(reinterpret_cast<const char*>(&step_counter), sizeof(step_counter)); // uint32_t

            // Cursor
            file.write(reinterpret_cast<const char*>(&cursor_position_x), sizeof(cursor_position_x)); // uint16_t
            file.write(reinterpret_cast<const char*>(&cursor_position_y), sizeof(cursor_position_y)); // uint16_t

            file.write(reinterpret_cast<const char*>(&starter), sizeof(starter)); // char
            file.write(reinterpret_cast<const char*>(&defeat), sizeof(defeat)); // char
            file.write(reinterpret_cast<const char*>(&winner), sizeof(winner)); // char

            // Карта
            for(int i = 0; i < map.data.size(); i++) {
                Cell& cell = map.at_flat(i);
                uint8_t packed = cell.pack();
                file.write(reinterpret_cast<const char*>(&packed), sizeof(packed));
            }

            // История
            uint32_t history_len = history.size();
            file.write(reinterpret_cast<const char*>(&history_len), sizeof(history_len));
            for(int i = 0; i < history_len; i++) {
                std::vector<uint8_t> packed = history[i].pack();
                for(int j = 0; j < packed.size(); j++) {
                    file.write(reinterpret_cast<const char*>(&packed[j]), sizeof(packed[j]));
                }
            }

            file.close();
        }
        bool MinesweeperGame::load(const std::string& file_name) {
            std::ifstream file(std::filesystem::path(std::filesystem::current_path() / (file_name + ".n24mscsf")), std::ios::binary);
            if(!file.is_open()) return false;

            // NoName24
            char magic_numbers_1[9] = {}; // +1 для '\0'
            file.read(magic_numbers_1, 8);
            magic_numbers_1[8] = '\0';
            if(std::string(magic_numbers_1) != "NoName24") { return false; }

            // MSCSF
            char magic_numbers_2[6] = {};
            file.read(magic_numbers_2, 5);
            magic_numbers_2[5] = '\0';
            if(std::string(magic_numbers_2) != "MSCSF") { return false; }

            uint8_t name_len;
            file.read(reinterpret_cast<char*>(&name_len), sizeof(name_len));
            name.resize(name_len);
            file.read(name.data(), name_len);

            file.read(reinterpret_cast<char*>(&map_width), sizeof(map_width));
            file.read(reinterpret_cast<char*>(&map_height), sizeof(map_height));
            file.read(reinterpret_cast<char*>(&map_amount_mines), sizeof(map_amount_mines));
            file.read(reinterpret_cast<char*>(&step_counter), sizeof(step_counter));

            // Cursor
            file.read(reinterpret_cast<char*>(&cursor_position_x), sizeof(cursor_position_x));
            file.read(reinterpret_cast<char*>(&cursor_position_y), sizeof(cursor_position_y));

            file.read(reinterpret_cast<char*>(&starter), sizeof(starter));
            file.read(reinterpret_cast<char*>(&defeat), sizeof(defeat));
            file.read(reinterpret_cast<char*>(&winner), sizeof(winner));

            // Карта
            map = Grid(map_width, map_height);
            for(int i = 0; i < map.data.size(); i++) {
                Cell& cell = map.at_flat(i);
                uint8_t packed;
                file.read(reinterpret_cast<char*>(&packed), sizeof(packed));
                cell.unpack(packed);
            }

            // История
            uint32_t history_len;
            file.read(reinterpret_cast<char*>(&history_len), sizeof(history_len));

            uint8_t history_len_byte; // Длина в байтах для правильного смещения
            StepHistory history_len_byte_buffer(0, 0, false, false);
            std::vector<uint8_t> history_len_byte_buffer_vector = history_len_byte_buffer.pack();
            history_len_byte = history_len_byte_buffer_vector.size();
            std::cout << "history_len_byte: " << history_len_byte << std::endl;

            for(int i = 0; i < history_len; i++) {
                std::vector<uint8_t> buffer_vector;
                for(int j = 0; j < history_len_byte; j++) {
                    uint8_t buffer;
                    file.read(reinterpret_cast<char*>(&buffer), sizeof(buffer));
                    buffer_vector.push_back(buffer);
                }
                StepHistory buffer_history;
                buffer_history.unpack(buffer_vector);
                history.push_back(buffer_history);
            }

            map.generate_count();
            return true;
        }
    };
};
