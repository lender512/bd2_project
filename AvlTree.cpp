//
// Created by luis.berrospi on 4/17/23.
//

#include <fstream>
#include <iostream>
#include <algorithm>
#include "AvlTree.h"
#include "functions.h"
#include <tuple>
#include <sstream>
#include <vector>
#include <cstring>
#include <chrono>

#define data_size sizeof(GameRecord::publisher) + sizeof(GameRecord::gameTitle) + sizeof(GameRecord::price)
#define metada_size (3*sizeof(long) + sizeof(int) + 1)
#define LINE_SIZE (data_size + metada_size)

AvlTree::AvlTree(std::string &file_name) {
    this->file_name = file_name;
    auto file = std::fstream(file_name, std::ios::in | std::ios::out | std::ios::binary);
    file.seekg(0, std::ios::end);
    if (file.tellg() == 0) {
        long trash = -1;
        file.write((char *) &trash, sizeof(long));
    }
    file.close();
}

bool AvlTree::insert(const GameRecord &record) {
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
    long pos = ((long) (file.tellg()) - (sizeof(long) + 1)) / (LINE_SIZE);
    file.write(record.publisher, sizeof(record.publisher));
    file.write(record.gameTitle, sizeof(record.gameTitle));
    file.write((char *) &record.price, sizeof(float));
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
    insert_memory_accesses+=10;
    if (header == -1) {
        file.seekg(0, std::ios::beg);
        file.write((char *) &pos, sizeof(long));
        insert_memory_accesses+=2;

        return true;
    } else {
        insert((char *) record.gameTitle, header, pos, file);
    }
    file.close();
}

bool AvlTree::delete_item(char key[sizeof(GameRecord::gameTitle)]) {
    std::fstream file(file_name, std::ios::in | std::ios::out | std::ios::binary);
    file.seekg(0, std::ios::beg);
    long header;
    file.read((char *) &header, sizeof(long));
    if (header == -1) {
        return false;
    }
    long current_pos = header;
    long current_pos_parent = -1;
    long current_pos_parent_pos = -1;
    while (current_pos != -1) {
        file.seekg(sizeof(GameRecord::publisher), std::ios::cur);
        char current_name[sizeof(GameRecord::gameTitle)];
        file.read(current_name, sizeof(current_name));
        file.seekg(sizeof(GameRecord::price), std::ios::cur);
        long left;
        long right;
        long left_pos = file.tellg();
        file.read((char *) &left, sizeof(long));
        long right_pos = file.tellg();
        file.read((char *) &right, sizeof(long));
        int comparison = Functions::compare(current_name, key);
        if (comparison == 1) {
            current_pos_parent = current_pos;
            current_pos_parent_pos = left_pos;
            current_pos = left;
        } else if (comparison == -1) {
            current_pos_parent = current_pos;
            current_pos_parent_pos = right_pos;
            current_pos = right;
        } else {
            break;
        }
    }

}

void AvlTree::insert(char key[sizeof(GameRecord::gameTitle)], long current_pos, long pos, std::fstream &file) {
    file.clear();
    long file_pos = sizeof(long) + 1;
    file_pos += current_pos * LINE_SIZE;
    file.seekg(file_pos, std::ios::beg);
    file.seekg(sizeof(GameRecord::publisher), std::ios::cur);
    char current_name[sizeof(GameRecord::gameTitle)];
    file.read(current_name, sizeof(current_name));
    file.seekg(sizeof(GameRecord::price), std::ios::cur);
    long left;
    long right;
    long left_pos = file.tellg();
    file.read((char *) &left, sizeof(long));
    long right_pos = file.tellg();
    file.read((char *) &right, sizeof(long));
    insert_memory_accesses+=6;

    int comparison = Functions::compare(current_name, key);
    if (comparison == 1) {
        if (right == -1) {
            file.seekg(right_pos, std::ios::beg);
            file.write((char *) &pos, sizeof(long));
            insert_memory_accesses += 2;
            right = pos;
        } else {
            AvlTree::insert(key, right, pos, file);
        }
    } else if (comparison == -1) {
        if (left == -1) {
            file.seekg(left_pos, std::ios::beg);
            file.write((char *) &pos, sizeof(long));
            insert_memory_accesses += 2;
            left = pos;
        } else {
            AvlTree::insert(key, left, pos, file);
        }
    } else {
        return;
    }
    update_height(file, current_pos);

    auto balance = balancing_factor(current_pos, file);


    long header;
    file.seekg(0, std::ios::beg);
    file.read((char*) &header, sizeof(long));
    auto current = get_record(current_pos);
    insert_memory_accesses += 8+2;
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

long AvlTree::balancing_factor(long &current_pos, std::fstream &file) {
    auto cur = get_record(current_pos);
    auto left = get_record(cur.left);
    auto right = get_record(cur.right);
    insert_memory_accesses+=3*8;
    return left.height - right.height;
}



void AvlTree::rotate_left(long &current_pos, std::fstream &file) {
    file.seekg(0, std::ios::beg);
    long header;
    file.read((char*) &header, sizeof(long));
    auto current = get_record(current_pos);
    auto right = get_record(current.right);
    file.seekg(get_file_pos_from_pos(current_pos) + data_size + sizeof(long), std::ios::beg);
    file.write((char*)&right.left, sizeof(long));
    //swap height
//    file.write((char*)&right.height, sizeof(long));
    file.seekg(get_file_pos_from_pos(current.right) + data_size, std::ios::beg);
    file.write((char*)&current_pos, sizeof(long));
    file.seekg(sizeof(long), std::ios::cur);
//    file.write((char*)&current.height, sizeof(long));
    insert_memory_accesses += 7 + 2*8;
    if (header == current_pos) {
        file.seekg(0, std::ios::beg);
        file.write((char*)&current.right, sizeof(long));
        insert_memory_accesses += 2;
    } else {
        auto parent = get_parent_pos(header, current_pos);
        auto parentRecord = get_record(parent);
        insert_memory_accesses += 8;
        file.seekg(get_file_pos_from_pos(parent) + data_size, std::ios::beg);
        insert_memory_accesses += 1;
        if (parentRecord.left == current_pos) {
            file.write((char*)&current.right, sizeof(long));
            insert_memory_accesses += 1;
        } else {
            file.seekg(sizeof (long), std::ios::cur);
            file.write((char*)&current.right, sizeof(long));
            insert_memory_accesses += 2;
        }
    }
    update_height(file, current_pos);
//    update_height(file, current.right);

}

void AvlTree::rotate_right(long &current_pos, std::fstream &file) {
    file.clear();
    file.seekg(0, std::ios::beg);
    long header;
    file.read((char*) &header, sizeof(long));
    auto current = get_record(current_pos);
    auto left = get_record(current.left);
    file.seekg(get_file_pos_from_pos(current_pos) + data_size, std::ios::beg);
    file.write((char*)&left.right, sizeof(long));
    //swap height
    file.seekg(sizeof(long), std::ios::cur);
    file.write((char*)&left.height, sizeof(long));

    file.seekg(get_file_pos_from_pos(current.left) + data_size + sizeof(long), std::ios::beg);
    file.write((char*)&current_pos, sizeof(long));
    file.write((char*)&current.height, sizeof(long));
    insert_memory_accesses += 7 + 2*8;
    if (header == current_pos) {
        file.seekg(0, std::ios::beg);
        file.write((char*)&current.left, sizeof(long));
        insert_memory_accesses += 2;
    } else {
        auto parent = get_parent_pos(header, current_pos);
        auto parentRecord = get_record(parent);
        insert_memory_accesses += 8;
        file.seekg(get_file_pos_from_pos(parent) + data_size, std::ios::beg);
        if (parentRecord.right == current_pos) {
            file.seekg(sizeof (long), std::ios::cur);
            file.write((char*)&current.left, sizeof(long));
            insert_memory_accesses += 2;
        } else {
            file.write((char*)&current.left, sizeof(long));
            insert_memory_accesses += 1;
        }
    }
    update_height(file, current_pos);
//    update_height(file, current.left);

}

long AvlTree::get_parent_pos(long pos, long search_pos) {
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

size_t AvlTree::get_file_pos_from_pos(long left_left_pos) const {
    return sizeof(long) + 1 + LINE_SIZE * left_left_pos;
}

std::tuple<long, long> AvlTree::update_height(std::fstream &file, long current_pos) {

    auto current = get_record(current_pos);
    auto left = get_record(current.left);
    auto right = get_record(current.right);

    long height = std::max(left.height, right.height) + 1;
    file.seekg(get_file_pos_from_pos(current_pos) + data_size + 2* sizeof(long), std::ios::beg);
    file.write((char*)&height, sizeof(long));
    insert_memory_accesses+=2;

    return std::tuple<long, long>{left.height, right.height};
}


void AvlTree::print_debug() {
    auto file = std::fstream(file_name, std::ios::in | std::ios::out | std::ios::binary);
    file.seekg(0, std::ios::beg);
    long header;
    file.read((char *) &header, sizeof(header));
    std::cout << "Header: " << header << std::endl;
    long max_height = 0;
    int count = 0;
    while (true) {
        file.seekg(1, std::ios::cur);

        char user_id[sizeof(GameRecord::publisher)];
        char name[sizeof(GameRecord::gameTitle)];
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

void AvlTree::print_file(std::fstream &file) {
    file.seekg(0, std::ios::beg);
    long header;
    file.read((char *) &header, sizeof(header));
    std::cout << "Header: " << header << std::endl;
    while (true) {
        file.seekg(1, std::ios::cur);

        char user_id[sizeof(GameRecord::publisher)];
        char name[sizeof(GameRecord::gameTitle)];
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
        file.read((char *) &left, sizeof(long));
        file.read((char *) &right, sizeof(long));
        file.read((char *) &height, sizeof(long));
        std::cout << "Left: " << left << " ";
        std::cout << "Right: " << right << " ";
        std::cout << "Height: " << height << std::endl;
    }
    file.close();
}


GameRecordMetaData AvlTree::get_record(long pos) {
    std::ifstream file(file_name, std::ios::in | std::ios::binary);
    file.seekg(0, std::ios::end);
    if (get_file_pos_from_pos(pos) >= file.tellg()) {
        file.close();
        GameRecordMetaData trash{};
        trash.height = -1;
        return trash;
    }

    file.seekg(get_file_pos_from_pos(pos), std::ios::beg);
    float hours;
    GameRecord record{};

    file.read(record.publisher, sizeof(GameRecord::publisher));
    file.read(record.gameTitle, sizeof(GameRecord::gameTitle));
    file.read((char *) &record.price, sizeof(GameRecord::price));
    GameRecordMetaData record_data{};

    file.read((char *) &record_data.left, sizeof(long));
    file.read((char *) &record_data.right, sizeof(long));
    file.read((char *) &record_data.height, sizeof(long));

    record_data.gameRecord = record;
    file.close();
    return record_data;
}

//function to load from a csv with optionial line number
void AvlTree::load_from_csv(std::string file_name, int line_number, char delimiter) {
    std::ifstream file(file_name);
    std::string line;
    int line_count = 0;
    while (std::getline(file, line)) {
        if (line_count == line_number) break;
        std::stringstream ss(line);
        std::string token;
        std::vector<std::string> tokens;
        while (std::getline(ss, token, delimiter)) {
            tokens.push_back(token);
        }
        GameRecord record{};
        strcpy(record.publisher, tokens[0].c_str());
//        std::cout << line_count << " " << tokens[1].c_str() << std::endl;
        strcpy(record.gameTitle, tokens[1].c_str());
        record.price = std::stof(tokens[2]);
        insert(record);
        line_count++;
    }
    file.close();
}

//print the tree in preorder
void AvlTree::print_preorder() {
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

void AvlTree::print_preorder(std::fstream &file, long pos, int &count) {
    if (pos == -1) return;
    count++;
    file.seekg(get_file_pos_from_pos(pos), std::ios::beg);
    char user_id[sizeof(GameRecord::publisher)];
    char name[sizeof(GameRecord::gameTitle)];
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
    std::cout << "Name: " << name << " ";
    std::cout << "User ID: " << user_id << " ";
    std::cout << "Platform: " << hours << " ";
    std::cout << "Left: " << left << " ";
    std::cout << "Right: " << right << " ";
    std::cout << "Height: " << height << " ";
    std::cout << "Deleted: " << deleted << std::endl;

    print_preorder(file, right, count);
}

void AvlTree::test_from_csv(const char *string, int number, char delimiter, const std::vector<int> &breaks) {
    std::cout << "n|search_time|search_memory|insert_time|insert_memory\n";
    std::ifstream file(string);
    std::string line;
    int line_count = 0;
    bool need_test = false;
    while (std::getline(file, line)) {
        if (line_count == number) break;
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
        GameRecord record{};
        strcpy(record.publisher, tokens[0].c_str());
//        std::cout << line_count << " " << tokens[1].c_str() << std::endl;
        strcpy(record.gameTitle, tokens[1].c_str());
        record.price = std::stof(tokens[2]);
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

void AvlTree::test(int line_count, std::string &file_name) {
    //get 10 random games
    std::vector<int> random_games;
    std::fstream file(file_name, std::ios::in | std::ios::binary);
    long avg = 0;
    int avg_mem = 0;
    for (int i = 0; i < line_count/10; i++) {
        //seek to random position
        int random_pos = rand() % line_count;
        file.seekg(get_file_pos_from_pos(random_pos), std::ios::beg);
        //read game
        char user_id[sizeof(GameRecord::publisher)];
        char name[sizeof(GameRecord::gameTitle)];
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
    avg /= line_count/10;
    avg_mem /= line_count/10;
    std::cout << avg << "|";
    std::cout << avg_mem << "|";
}

GameRecord AvlTree::search_by_name(char name[sizeof(GameRecord::gameTitle)]) {
    std::fstream file(file_name, std::ios::in | std::ios::binary);
    search_memory_accesses = 0;
    file.seekg(0, std::ios::beg);
    long header;
    file.read((char *) &header, sizeof(header));
    search_memory_accesses += 2;
    if (header == -1) {
        file.close();
        GameRecordMetaData trash{};
        trash.height = -1;
        return trash.gameRecord;
    }
    GameRecordMetaData record = search_by_name(file, header, name);
    file.close();
    return record.gameRecord;
}

GameRecordMetaData AvlTree::search_by_name(std::fstream&file, long current_pos, char name[sizeof(GameRecord::gameTitle)]) {
    auto current = get_record(current_pos);
    search_memory_accesses += 8;
    if (current.height == -1) {
        return current;
    }
    int cmp = Functions::compare(name, current.gameRecord.gameTitle);
    if (!strcmp(current.gameRecord.gameTitle, name)) {
        return current;
    } else if (cmp > 0) {
        return search_by_name(file, current.left, name);
    } else {
        return search_by_name(file, current.right, name);
    }
}

//search by range of filne_name
std::vector<GameRecord> AvlTree::search_by_name_range(char name1[sizeof(GameRecord::gameTitle)],
                                                      char name2[sizeof(GameRecord::gameTitle)]) {
    std::fstream file(file_name, std::ios::in | std::ios::binary);
    file.seekg(0, std::ios::beg);
    long header;
    file.read((char *) &header, sizeof(header));
    if (header == -1) {
        file.close();
        GameRecordMetaData trash{};
        trash.height = -1;
        return std::vector<GameRecord>();
    }
    std::vector<GameRecord> records;
    search_by_name_range(file, records, header, name1, name2);
    file.close();
    return records;
}

void AvlTree::search_by_name_range(std::fstream &file, std::vector<GameRecord> &records, long current_pos,
                                   char left_name[sizeof(GameRecord::gameTitle)],
                                   char right_name[sizeof(GameRecord::gameTitle)]) {
    auto current = get_record(current_pos);

    int cmp_left = Functions::compare(left_name, current.gameRecord.gameTitle);
    int cmp_right = Functions::compare(right_name, current.gameRecord.gameTitle);
    if (cmp_left >= 0 && cmp_right <= 0) {
        records.push_back(current.gameRecord);
    }
    if (current.left != -1 && cmp_left > 0) {
        search_by_name_range(file, records, current.left, left_name, right_name);
    }
    if (current.right != -1 && cmp_right < 0) {
        search_by_name_range(file, records, current.right, left_name, right_name);
    }
}
