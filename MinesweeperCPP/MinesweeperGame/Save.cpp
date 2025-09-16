#include "../MinesweeperCPP.hpp"

#include <fstream>
#include <filesystem>

namespace MinesweeperCPP {
    namespace Game {
        uint8_t MinesweeperGame::save_cellpack(const Cell& cell) {
            uint8_t b = 0;
            b |= (cell.open   ? 1 : 0) << 0;
            b |= (cell.danger ? 1 : 0) << 1;
            b |= (cell.flag   ? 1 : 0) << 2;
            return b;
        }
        Cell MinesweeperGame::save_cellunpack(uint8_t b) {
            Cell cell;
            cell.open   = b & (1 << 0);
            cell.danger = b & (1 << 1);
            cell.flag   = b & (1 << 2);
            return cell;
        }

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
            file.write(reinterpret_cast<const char*>(&cursor_position_x), sizeof(cursor_position_x)); // cursor_position_x
            file.write(reinterpret_cast<const char*>(&cursor_position_y), sizeof(cursor_position_y)); // cursor_position_x

            file.write(reinterpret_cast<const char*>(&starter), sizeof(starter)); // char
            file.write(reinterpret_cast<const char*>(&defeat), sizeof(defeat)); // char
            file.write(reinterpret_cast<const char*>(&winner), sizeof(winner)); // char

            for(int i = 0; i < map.data.size(); i++) {
                const Cell& cell = map.at_flat(i);
                uint8_t packed = save_cellpack(cell);
                file.write(reinterpret_cast<const char*>(&packed), sizeof(packed));
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

            map = Grid(map_width, map_height);

            for(auto& cell : map.data) {
                uint8_t packed;
                file.read(reinterpret_cast<char*>(&packed), sizeof(packed));
                cell = save_cellunpack(packed);
            }

            map.generate_count();
            return true;
        }
    };
};
