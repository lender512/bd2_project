//
// Created by luis.berrospi on 4/17/23.
//

#ifndef BD2_PROJECT_AVLTREE_H
#define BD2_PROJECT_AVLTREE_H

#include <string>
#include <fstream>
#include <vector>
#include <fstream>
#include <iostream>
#include <algorithm>

#include <tuple>
#include <sstream>
#include <vector>
#include <cstring>
#include <chrono>

/*

=PROMEDIO(Hoja1!B2; Hoja2!B2; Hoja3!B2; Hoja4!B2; Hoja5!B2; Hoja6!B2; Hoja7!B2; Hoja8!B2; Hoja9!B2; Hoja10!B2)

*/

struct MovieRecord {
    int id;
    char primaryTitle[419];
    char year[5];
    char genres[32];
    static constexpr long size = sizeof(id) + sizeof(primaryTitle) + sizeof(year) + sizeof(genres);
};

struct GameRecord {
    char publisher[90];
    char gameTitle[150]; //primary gameTitle
    float price;
    static constexpr long size = sizeof(publisher) + sizeof(gameTitle) + sizeof(price);
};

static constexpr long metada_size = (3 * sizeof(long) + sizeof(int) + 1);

template<class Record>
struct Line_size {
    static constexpr long size = Record::size + metada_size;
};


template<class Record>
struct RecordMetaData {
    Record record;
    long left;
    long right;
    long height;
    int deleted;
};

template<typename Record, typename T>
static int compare(T a, T b) {
    if constexpr (std::is_same<Record, GameRecord>::value) {
        int i = 0;
        while (a[i] != '\0' && b[i] != '\0') {
            if (a[i] < b[i]) {
                return 1;
            } else if (a[i] > b[i]) {
                return -1;
            }
            i++;
        }
        if (a[i] == '\0' && b[i] == '\0') {
            return 1;
        } else if (a[i] == '\0') {
            return 1;
        } else {
            return -1;
        }
    }

    if constexpr (std::is_same<Record, MovieRecord>::value) {
        return -1 * ((a <= b) * -1 + (a > b) * 1);
    }

}


template<class Record>
class AvlTree {
private:
    std::string file_name;


    int insert_memory_accesses = 0;
    int search_memory_accesses = 0;

public:
    AvlTree(std::string &file_name) {
        this->file_name = file_name;
        auto file = std::fstream(file_name, std::ios::in | std::ios::out | std::ios::binary);
        file.seekg(0, std::ios::end);
        if (file.tellg() == 0) {
            long trash = -1;
            file.write((char *) &trash, sizeof(long));
        }
        file.close();
    }

    template<typename T>
    void insert(T key, long current_pos, long pos, std::fstream &file) {
        file.clear();
        long file_pos = sizeof(long) + 1;
        file_pos += current_pos * Line_size<Record>::size;

        auto current = get_record(current_pos);

        insert_memory_accesses += 6;

        int comparison = 0;
        if constexpr (std::is_same<Record, GameRecord>::value) {
            comparison = compare<Record>(current.record.gameTitle, key);
        }
        if constexpr (std::is_same<Record, MovieRecord>::value) {
            comparison = compare<Record>(current.record.id, key);
        }

        if (comparison == 1) {
            if (current.right == -1) {
                file.seekg(get_file_pos_from_pos(current_pos) + Line_size<Record>::size - metada_size + sizeof(long),
                           std::ios::beg);
                file.write((char *) &pos, sizeof(long));
                insert_memory_accesses += 2;
//                right = pos;
            } else {
                insert(key, current.right, pos, file);
            }
        } else if (comparison == -1) {
            if (current.left == -1) {
                file.seekg(get_file_pos_from_pos(current_pos) + Line_size<Record>::size - metada_size, std::ios::beg);
                file.write((char *) &pos, sizeof(long));
                insert_memory_accesses += 2;
//                left = pos;
            } else {
                insert(key, current.left, pos, file);
            }
        } else {
            return;
        }
        update_height(file, current_pos);

        auto balance = balancing_factor(current_pos, file);


        long header;
        file.seekg(0, std::ios::beg);
        file.read((char *) &header, sizeof(long));
        current = get_record(current_pos);
        insert_memory_accesses += 8 + 2;
        if (balance > 1) {
            if (balancing_factor(current.left, file) < 0) {
                // left child of B is now right child of A
                rotate_left(current.left, file);
            }
            rotate_right(current_pos, file);
        } else if (balance < -1) {
            if (balancing_factor(current.right, file) > 0) {
                rotate_right(current.right, file);

            }
            rotate_left(current_pos, file);
        }

        update_height(file, current_pos);
    }


    bool insert(Record &record) {
        insert_memory_accesses = 0;

//    std::cout << "inserting " << record.name << std::endl;
        std::fstream file(file_name, std::ios::in | std::ios::out | std::ios::binary);
        insert_memory_accesses++;
        file.seekg(0, std::ios::end);
        insert_memory_accesses++;
        if (file.tellg() != 0) {
            file.write("\n", 1);
            insert_memory_accesses++;

        }
        long pos = ((long) (file.tellg()) - (sizeof(long) + 1)) / (Line_size<Record>::size);
        if constexpr (std::is_same<Record, GameRecord>::value) {
            file.write(record.publisher, sizeof(record.publisher));
            file.write(record.gameTitle, sizeof(record.gameTitle));
            file.write((char *) &record.price, sizeof(float));
        }
        if constexpr (std::is_same<Record, MovieRecord>::value) {
            file.write((char *) &record.id, sizeof(Record::id));
            file.write(record.primaryTitle, sizeof(Record::primaryTitle));
            file.write(record.year, sizeof(Record::year));
            file.write(record.genres, sizeof(Record::genres));
        }

        long trash = -1;
        long height = 0;
        int deleted = 0;
        file.write((char *) &trash, sizeof(trash));
        file.write((char *) &trash, sizeof(trash));
        file.write((char *) &height, sizeof(height));
        file.write((char *) &deleted, sizeof(deleted));
        file.seekg(0, std::ios::beg);
        long header;
        file.read((char *) &header, sizeof(long));
        file.seekg(1, std::ios::cur);
        insert_memory_accesses += 10;
        if (header == -1) {
            file.seekg(0, std::ios::beg);
            file.write((char *) &pos, sizeof(long));
            insert_memory_accesses += 2;
        } else {
            if constexpr (std::is_same<Record, GameRecord>::value) {
                insert(record.gameTitle, header, pos, file);
            }
            if constexpr (std::is_same<Record, MovieRecord>::value) {
                insert(record.id, header, pos, file);
            }
        }
        file.close();
        return true;
    }

    template<typename T>
    bool delete_item(T key) {
        std::fstream file(file_name, std::ios::in | std::ios::out | std::ios::binary);
        long header;
        file.seekg(0, std::ios::beg);
        file.read((char *) &header, sizeof(long));
        file.seekg(1, std::ios::cur);
        auto val = delete_item(key, header, file);
        file.close();
        return val;
    }

    template<typename T>
    bool delete_item(T key, long &current_pos, std::fstream &file) {

        auto [record, pos] = search_by_name_delete(file, current_pos, key);
        if (record.height == -1 || record.deleted == 1) {
            return false;
        }
        file.seekg(get_file_pos_from_pos(pos) + Line_size<Record>::size - sizeof(int) - 1, std::ios::beg);
        int deleted = 1;
        file.write((char *) &deleted, sizeof(deleted));
        return true;
    }


    long balancing_factor(long &current_pos, std::fstream &file) {
        auto cur = get_record(current_pos);
        auto left = get_record(cur.left);
        auto right = get_record(cur.right);
        insert_memory_accesses += 3 * 8;
        return left.height - right.height;
    }


    void rotate_left(long &current_pos, std::fstream &file) {
        file.seekg(0, std::ios::beg);
        long header;
        file.read((char *) &header, sizeof(long));
        auto current = get_record(current_pos);
        auto right = get_record(current.right);
        file.seekg(get_file_pos_from_pos(current_pos) + (Line_size<Record>::size - metada_size) + sizeof(long),
                   std::ios::beg);
        file.write((char *) &right.left, sizeof(long));
        //swap height
//    file.write((char*)&right.height, sizeof(long));
        file.seekg(get_file_pos_from_pos(current.right) + (Line_size<Record>::size - metada_size), std::ios::beg);
        file.write((char *) &current_pos, sizeof(long));
        file.seekg(sizeof(long), std::ios::cur);
//    file.write((char*)&current.height, sizeof(long));
        insert_memory_accesses += 7 + 2 * 8;
        if (header == current_pos) {
            file.seekg(0, std::ios::beg);
            file.write((char *) &current.right, sizeof(long));
            insert_memory_accesses += 2;
        } else {
            auto parent = get_parent_pos(header, current_pos);
            auto parentRecord = get_record(parent);
            insert_memory_accesses += 8;
            file.seekg(get_file_pos_from_pos(parent) + (Line_size<Record>::size - metada_size), std::ios::beg);
            insert_memory_accesses += 1;
            if (parentRecord.left == current_pos) {
                file.write((char *) &current.right, sizeof(long));
                insert_memory_accesses += 1;
            } else {
                file.seekg(sizeof(long), std::ios::cur);
                file.write((char *) &current.right, sizeof(long));
                insert_memory_accesses += 2;
            }
        }
        update_height(file, current_pos);
//    update_height(file, current.right);

    }


    void rotate_right(long &current_pos, std::fstream &file) {
        file.clear();
        file.seekg(0, std::ios::beg);
        long header;
        file.read((char *) &header, sizeof(long));
        auto current = get_record(current_pos);
        auto left = get_record(current.left);
        file.seekg(get_file_pos_from_pos(current_pos) + (Line_size<Record>::size - metada_size),
                   std::ios::beg);
        file.write((char *) &left.right, sizeof(long));
        //swap height
        file.seekg(sizeof(long), std::ios::cur);
        file.write((char *) &left.height, sizeof(long));

        file.seekg(get_file_pos_from_pos(current.left) + (Line_size<Record>::size - metada_size) + sizeof(long),
                   std::ios::beg);
        file.write((char *) &current_pos, sizeof(long));
        file.write((char *) &current.height, sizeof(long));
        insert_memory_accesses += 7 + 2 * 8;
        if (header == current_pos) {
            file.seekg(0, std::ios::beg);
            file.write((char *) &current.left, sizeof(long));
            insert_memory_accesses += 2;
        } else {
            auto parent = get_parent_pos(header, current_pos);
            auto parentRecord = get_record(parent);
            insert_memory_accesses += 8;
            file.seekg(get_file_pos_from_pos(parent) + (Line_size<Record>::size - metada_size), std::ios::beg);
            if (parentRecord.right == current_pos) {
                file.seekg(sizeof(long), std::ios::cur);
                file.write((char *) &current.left, sizeof(long));
                insert_memory_accesses += 2;
            } else {
                file.write((char *) &current.left, sizeof(long));
                insert_memory_accesses += 1;
            }
        }
        update_height(file, current_pos);
//    update_height(file, current.left);

    }


    long get_parent_pos(long pos, long search_pos) {
        //open file name in readmode
        std::fstream file;
        file.open(file_name, std::ios::in | std::ios::binary);
        auto curRecord = get_record(pos);
        insert_memory_accesses += 8;
        if (curRecord.left == search_pos || curRecord.right == search_pos) {
            return pos;
        }
        if (curRecord.left != -1) {
            long left_parent = get_parent_pos(curRecord.left, search_pos);
            if (left_parent != -1) {
                return left_parent;
            }
        }
        if (curRecord.right != -1) {
            long right_parent = get_parent_pos(curRecord.right, search_pos);
            if (right_parent != -1) {
                return right_parent;
            }
        }
        return -1;
    }


    size_t get_file_pos_from_pos(long left_left_pos) const {
        return sizeof(long) + 1 + Line_size<Record>::size * left_left_pos;
    }


    std::tuple<long, long> update_height(std::fstream &file, long current_pos) {

        auto current = get_record(current_pos);
        auto left = get_record(current.left);
        auto right = get_record(current.right);

        long height = std::max(left.height, right.height) + 1;
        file.seekg(get_file_pos_from_pos(current_pos) + (Line_size<Record>::size - metada_size) +
                   2 * sizeof(long), std::ios::beg);
        file.write((char *) &height, sizeof(long));
        insert_memory_accesses += 2;

        return std::tuple<long, long>{left.height, right.height};
    }


    void print_debug() {
        auto file = std::fstream(file_name, std::ios::in | std::ios::out | std::ios::binary);
        file.seekg(0, std::ios::beg);
        long header;
        file.read((char *) &header, sizeof(header));
        std::cout << "Header: " << header << std::endl;
        long max_height = 0;
        int count = 0;
        while (true) {
            file.seekg(1, std::ios::cur);
            if constexpr (std::is_same<Record, GameRecord>::value) {
                char user_id[sizeof(Record::publisher)];
                char name[sizeof(Record::gameTitle)];
                float hours;
                file.read(user_id, sizeof(user_id));
                file.read(name, sizeof(name));
                file.read((char *) &hours, sizeof(float));
                if (file.eof()) {
                    break;
                }
                std::cout << "Count: " << count++ << " ";
                std::cout << "Name: " << name << " ";
                std::cout << "User ID: " << user_id << " ";
                std::cout << "Platform: " << hours << " ";
            }
            if constexpr (std::is_same<Record, MovieRecord>::value) {
                int id;
                char primaryTitle[sizeof(MovieRecord::primaryTitle)];
                char originalYear[sizeof(MovieRecord::year)];
                char genres[sizeof(MovieRecord::genres)];
                file.read((char *) &id, sizeof(int));
                file.read(primaryTitle, sizeof(primaryTitle));
                file.read(originalYear, sizeof(originalYear));
                file.read(genres, sizeof(genres));


                if (file.eof()) {
                    break;
                }
                std::cout << "Count: " << count++ << " ";
                std::cout << "ID: " << id << " ";
                std::cout << "Title: " << primaryTitle << " ";
                std::cout << "Year: " << originalYear << " ";
                std::cout << "Genres: " << genres << " ";
            }
            long left, right, height;
            int deleted;
            file.read((char *) &left, sizeof(long));
            file.read((char *) &right, sizeof(long));
            file.read((char *) &height, sizeof(long));
            file.read((char *) &deleted, sizeof(int));
            std::cout << "Left: " << left << " ";
            std::cout << "Right: " << right << " ";
            std::cout << "Height: " << height << " ";
            std::cout << "Deleted: " << deleted << std::endl;
            max_height = std::max(max_height, height);
        }
        std::cout << "Max height: " << max_height << std::endl;

        file.close();
    }


    void print_file(std::fstream &file) {
        file.seekg(0, std::ios::beg);
        long header;
        file.read((char *) &header, sizeof(header));
        std::cout << "Header: " << header << std::endl;
        while (true) {
            file.seekg(1, std::ios::cur);
            if constexpr (std::is_same<Record, GameRecord>::value) {
                char user_id[sizeof(Record::publisher)];
                char name[sizeof(Record::gameTitle)];
                float hours;
                file.read(user_id, sizeof(user_id));
                file.read(name, sizeof(name));
                file.read((char *) &hours, sizeof(float));
                if (file.eof()) {
                    break;
                }

                std::cout << "User ID: " << user_id << " ";
                std::cout << "Name: " << name << " ";
                std::cout << "Platform: " << hours << " ";
                long left, right, height;
                int deleted;
                file.read((char *) &left, sizeof(long));
                file.read((char *) &right, sizeof(long));
                file.read((char *) &height, sizeof(long));
                file.read((char *) &deleted, sizeof(long));
                std::cout << "Left: " << left << " ";
                std::cout << "Right: " << right << " ";
                std::cout << "Height: " << height << " ";
                std::cout << "Deleted: " << deleted << std::endl;
            }

        }
        file.close();
    }


    RecordMetaData<Record> get_record(long pos) {
        std::ifstream file(file_name, std::ios::in | std::ios::binary);
        file.seekg(0, std::ios::end);
        if (get_file_pos_from_pos(pos) >= file.tellg()) {
            file.close();
            RecordMetaData<Record> trash{};
            trash.height = -1;
            return trash;
        }

        file.seekg(get_file_pos_from_pos(pos), std::ios::beg);
        float hours;
//        Record record{};
        RecordMetaData<Record> record_data{};

        if constexpr (std::is_same<Record, GameRecord>::value) {
            file.read(record_data.record.publisher, sizeof(GameRecord::publisher));
            file.read(record_data.record.gameTitle, sizeof(GameRecord::gameTitle));
            file.read((char *) &record_data.record.price, sizeof(GameRecord::price));


        } else if constexpr (std::is_same<Record, MovieRecord>::value) {
            file.read((char *) &record_data.record.id, sizeof(MovieRecord::id));
            file.read(record_data.record.primaryTitle, sizeof(MovieRecord::primaryTitle));
            file.read(record_data.record.year, sizeof(MovieRecord::year));
            file.read(record_data.record.genres, sizeof(MovieRecord::genres));

        }

        file.read((char *) &record_data.left, sizeof(long));
        file.read((char *) &record_data.right, sizeof(long));
        file.read((char *) &record_data.height, sizeof(long));
        file.read((char *) &record_data.deleted, sizeof(int));
        //        record_data.record = record;
        file.close();
        return record_data;
    }

//function to load from a csv with optionial line price

    void load_from_csv(std::string file_name, int line_price, char delimiter) {
        std::ifstream file(file_name);
        //skip first line
        std::string line;
        std::getline(file, line);
        int line_count = 0;
        while (std::getline(file, line)) {
            if (line_count == line_price) break;
            std::stringstream ss(line);
            std::string token;
            std::vector<std::string> tokens;
            while (std::getline(ss, token, delimiter)) {
                tokens.push_back(token);
            }
            Record record{};
            if constexpr (std::is_same<Record, GameRecord>::value) {
                strcpy(record.publisher, tokens[0].c_str());
                strcpy(record.gameTitle, tokens[1].c_str());
                record.price = std::stof(tokens[2]);
            }

            if constexpr (std::is_same<Record, MovieRecord>::value) {
                record.id = std::stoi(tokens[0]);
                strcpy(record.primaryTitle, tokens[1].c_str());
                strcpy(record.year, tokens[2].c_str());
                strcpy(record.genres, tokens[3].c_str());
            }

            insert(record);
            line_count++;
        }
        file.close();
    }

//print the tree in preorder

    void print_preorder() {
        std::fstream file(file_name, std::ios::in | std::ios::binary);
        file.seekg(0, std::ios::beg);
        long header;
        file.read((char *) &header, sizeof(header));
        std::cout << "Header: " << header << std::endl << std::endl;

        int count = 0;
        print_preorder(file, header, count);
        std::cout << "Count: " << count << std::endl;
        file.close();
    }


    void print_preorder(std::fstream &file, long pos, int &count) {
        if (pos == -1) return;
        count++;
        file.seekg(get_file_pos_from_pos(pos), std::ios::beg);
        if constexpr (std::is_same<Record, GameRecord>::value) {
            char user_id[sizeof(Record::publisher)];
            char name[sizeof(Record::gameTitle)];
            float hours;
            file.read(user_id, sizeof(user_id));
            file.read(name, sizeof(name));
            file.read((char *) &hours, sizeof(float));
            long left, right, height;
            int deleted;
            file.read((char *) &left, sizeof(long));
            file.read((char *) &right, sizeof(long));
            file.read((char *) &height, sizeof(long));
            file.read((char *) &deleted, sizeof(int));
            print_preorder(file, left, count);

            if (deleted != 1) {
                std::cout << "Name: " << name << " ";
                std::cout << "User ID: " << user_id << " ";
                std::cout << "Platform: " << hours << " ";
                std::cout << "Left: " << left << " ";
                std::cout << "Right: " << right << " ";
                std::cout << "Height: " << height << " ";
                std::cout << "Deleted: " << deleted << std::endl;
            }


            print_preorder(file, right, count);
        }
        if constexpr (std::is_same<Record, MovieRecord>::value) {
            int id;
            char primaryTitle[sizeof(MovieRecord::primaryTitle)];
            char year[sizeof(MovieRecord::year)];
            char genres[sizeof(MovieRecord::genres)];
            file.read((char *) &id, sizeof(MovieRecord::id));
            file.read(primaryTitle, sizeof(MovieRecord::primaryTitle));
            file.read(year, sizeof(year));
            file.read(genres, sizeof(MovieRecord::genres));

            long left, right, height;
            int deleted;
            file.read((char *) &left, sizeof(long));
            file.read((char *) &right, sizeof(long));
            file.read((char *) &height, sizeof(long));
            file.read((char *) &deleted, sizeof(int));
            print_preorder(file, left, count);
            if (deleted != 1) {
                std::cout << "ID: " << id << " ";
                std::cout << "Primary Title: " << primaryTitle << " ";
                std::cout << "Year: " << year << " ";
                std::cout << "Genres: " << genres << " ";
                std::cout << "Left: " << left << " ";
                std::cout << "Right: " << right << " ";
                std::cout << "Height: " << height << " ";
                std::cout << "Deleted: " << deleted << std::endl;
            }

            print_preorder(file, right, count);
        }


    }


    void test_from_csv(const char *string, int price, char delimiter, const std::vector<int> &breaks) {
        std::cout << "n|search_time|search_memory|insert_time|insert_memory\n";
        std::ifstream file(string);
        std::string line;
        int line_count = 0;
        bool need_test = false;
        std::getline(file, line);
        while (std::getline(file, line)) {
            if (line_count == price) break;
            if (std::find(breaks.begin(), breaks.end(), line_count) != breaks.end()) {
                std::cout << line_count << "|";
                need_test = true;
                test(line_count, file_name);
            }
            std::stringstream ss(line);
            std::string token;
            std::vector<std::string> tokens;
            while (std::getline(ss, token, delimiter)) {
                tokens.push_back(token);
            }
            Record record{};
            if constexpr (std::is_same<Record, GameRecord>::value) {
                strcpy(record.publisher, tokens[0].c_str());
                strcpy(record.gameTitle, tokens[1].c_str());
                record.price = std::stof(tokens[2]);
            }

            if constexpr (std::is_same<Record, MovieRecord>::value) {

                record.id = std::stoi(tokens[0]);
                strcpy(record.primaryTitle, tokens[1].c_str());
                strcpy(record.year, tokens[2].c_str());
                strcpy(record.genres, tokens[3].c_str());
            }
            //insert
            std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
            insert(record);
            std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
            if (need_test) {
                std::cout << std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count()
                          << "|";
                std::cout << insert_memory_accesses << "\n";
            }
            line_count++;
            need_test = false;

        }
        file.close();
    }


    void test(int line_count, std::string &file_name) {
        //get 10 random games
        std::vector<int> random_games;
        std::fstream file(file_name, std::ios::in | std::ios::binary);
        long avg = 0;
        int avg_mem = 0;
        for (int i = 0; i < line_count / 10; i++) {
            //seek to random position
            int random_pos = rand() % line_count;
            file.seekg(get_file_pos_from_pos(random_pos), std::ios::beg);
            //read game
            if constexpr (std::is_same<Record, GameRecord>::value) {
                char user_id[sizeof(Record::publisher)];
                char name[sizeof(Record::gameTitle)];
                float hours;
                file.read(user_id, sizeof(user_id));
                file.read(name, sizeof(name));
                file.read((char *) &hours, sizeof(float));
                //get time for searching that name
                std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
                search_by_name(name);
                std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
                avg += std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count();
                avg_mem += search_memory_accesses;
            }
            if constexpr (std::is_same<Record, MovieRecord>::value) {
                int id;
                char primaryTitle[sizeof(MovieRecord::primaryTitle)];
                char year[sizeof(MovieRecord::year)];
                char genres[sizeof(MovieRecord::genres)];
                file.read((char *) &id, sizeof(id));
                file.read(primaryTitle, sizeof(primaryTitle));
                file.read(year, sizeof(year));
                file.read(genres, sizeof(genres));
                //get time for searching that name
                std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
                search_by_name(id);
                std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
                avg += std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count();
                avg_mem += search_memory_accesses;
            }
        }
        avg /= line_count / 10;
        avg_mem /= line_count / 10;
        std::cout << avg << "|";
        std::cout << avg_mem << "|";
    }

    template<typename T>
    Record search_by_name(T name) {
        std::fstream file(file_name, std::ios::in | std::ios::binary);
        search_memory_accesses = 0;
        file.seekg(0, std::ios::beg);
        long header;
        file.read((char *) &header, sizeof(header));
        search_memory_accesses += 2;
        if (header == -1) {
            file.close();
            RecordMetaData<Record> trash{};
            trash.height = -1;
            return trash.record;
        }
        RecordMetaData<Record> record = search_by_name(file, header, name);
        file.close();
        return record.record;
    }

    template<typename T>
    std::tuple<RecordMetaData<Record>, long> search_by_name_delete(std::fstream &file, long current_pos, T key) {
        auto current = get_record(current_pos);
        search_memory_accesses += 8;
        if (current.height == -1) {
            return {current, current_pos};
        }
        int comparison = 0;
        if constexpr (std::is_same<Record, GameRecord>::value) {
            comparison = compare<Record, T>(current.record.gameTitle, key);
        }
        if constexpr (std::is_same<Record, MovieRecord>::value) {
            comparison = compare<Record, T>(current.record.id, key);
        }
        int cmp2 = 0;
        if constexpr (std::is_same<Record, GameRecord>::value) {
            cmp2 = !strcmp(current.record.gameTitle, key);
            if (cmp2) {
                return {current, current_pos};
            } else if (comparison <= 0) {
                return search_by_name_delete(file, current.left, key);
            } else {
                return search_by_name_delete(file, current.right, key);
            }
        }
        if constexpr (std::is_same<Record, MovieRecord>::value) {
            cmp2 = key == current.record.id;
            if (cmp2) {
                return {current, current_pos};
            } else if (comparison <= 0) {
                return search_by_name_delete(file, current.left, key);
            } else {
                return search_by_name_delete(file, current.right, key);
            }
        }
    }


    template<typename T>
    RecordMetaData<Record> search_by_name(std::fstream &file, long current_pos, T key) {
        auto current = get_record(current_pos);
        search_memory_accesses += 8;
        if (current.height == -1) {
            return current;
        }
        int comparison = 0;
        if constexpr (std::is_same<Record, GameRecord>::value) {
            comparison = compare<Record, T>(current.record.gameTitle, key);
        }
        if constexpr (std::is_same<Record, MovieRecord>::value) {
            comparison = compare<Record, T>(current.record.id, key);
        }
        int cmp2 = 0;
        if constexpr (std::is_same<Record, GameRecord>::value) {
            cmp2 = !strcmp(current.record.gameTitle, key);
            if (cmp2) {
                if (current.deleted != 0) {
                    RecordMetaData<Record> trash{};
                    trash.height = -1;
                    return trash;
                }
                return current;
            } else if (comparison <= 0) {
                return search_by_name(file, current.left, key);
            } else {
                return search_by_name(file, current.right, key);
            }
        }
        if constexpr (std::is_same<Record, MovieRecord>::value) {
            cmp2 = key == current.record.id;
            if (cmp2) {
                if (current.deleted != 0) {
                    RecordMetaData<Record> trash{};
                    trash.height = -1;
                    return trash;
                }
                return current;
            } else if (comparison <= 0) {
                return search_by_name(file, current.left, key);
            } else {
                return search_by_name(file, current.right, key);
            }
        }
    }

//search by range of filne_name
    template<typename T>
    std::vector<Record> search_by_name_range(T name1,
                                             T name2) {
        std::fstream file(file_name, std::ios::in | std::ios::binary);
        file.seekg(0, std::ios::beg);
        long header;
        file.read((char *) &header, sizeof(header));
        if (header == -1) {
            file.close();
            RecordMetaData<Record> trash{};
            trash.height = -1;
            return std::vector<Record>();
        }
        std::vector<Record> records;
        search_by_name_range(file, records, header, name1, name2);
        file.close();
        return records;
    }

    template<typename T>
    void search_by_name_range(std::fstream &file, std::vector<Record> &records, long current_pos,
                              T left_name,
                              T right_name) {
        auto current = get_record(current_pos);
        int cmp_left = 0;
        int cmp_right = 0;
        if constexpr (std::is_same<Record, GameRecord>::value) {
            cmp_left = compare<Record>(left_name, current.record.gameTitle);
            cmp_right = compare<Record>(right_name, current.record.gameTitle);
        }
        if constexpr (std::is_same<Record, MovieRecord>::value) {
            cmp_left = compare<Record>(left_name, current.record.id);
            cmp_right = compare<Record>(right_name, current.record.id);

        }

        if (current.left != -1 && cmp_left > 0) {
            search_by_name_range(file, records, current.left, left_name, right_name);
        }
        if (cmp_left >= 0 && cmp_right <= 0) {
            if (current.deleted == 0) records.push_back(current.record);
        }
        if (current.right != -1 && cmp_right < 0) {
            search_by_name_range(file, records, current.right, left_name, right_name);
        }

    }

};


#endif //BD2_PROJECT_AVLTREE_H
