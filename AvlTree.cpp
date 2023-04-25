//
// Created by luis.berrospi on 4/17/23.
//

#include <fstream>
#include <iostream>
#include "AvlTree.h"
#include "functions.h"
#include <tuple>
#include <sstream>
#include <vector>
#include <cstring>

#define data_size sizeof(GameRecord::user_id) + sizeof(GameRecord::name) + sizeof(GameRecord::hours)
#define metada_size (3*sizeof(long) + 1)
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
//    std::cout << "inserting " << record.name << std::endl;
    std::fstream file(file_name, std::ios::in | std::ios::out | std::ios::binary);
    file.seekg(0, std::ios::end);
    if (file.tellg() != 0) {
        file.write("\n", 1);
    }
    long pos = ((long) (file.tellg()) - (sizeof(long) + 1)) / (LINE_SIZE);
    file.write(record.user_id, sizeof(record.user_id));
    file.write(record.name, sizeof(record.name));
    file.write((char *) &record.hours, sizeof(float));
    long trash = -1;
    long height = 0;
    file.write((char *) &trash, sizeof(trash));
    file.write((char *) &trash, sizeof(trash));
    file.write((char *) &height, sizeof(height));
    file.seekg(0, std::ios::beg);
    long header;
    file.read((char *) &header, sizeof(long));
    file.seekg(1, std::ios::cur);
    if (header == -1) {
        file.seekg(0, std::ios::beg);
        file.write((char *) &pos, sizeof(long));
        return true;
    } else {
        insert((char *) record.name, header, pos, file);
    }
    file.close();

}

void AvlTree::insert(char key[85], long current_pos, long pos, std::fstream &file) {
    file.clear();
    long file_pos = sizeof(long) + 1;
    file_pos += current_pos * LINE_SIZE;
    file.seekg(file_pos, std::ios::beg);
    file.seekg(sizeof(GameRecord::user_id), std::ios::cur);
    char current_name[85];
    file.read(current_name, sizeof(current_name));
    file.seekg(sizeof(GameRecord::hours), std::ios::cur);
    long left;
    long right;
    long left_pos = file.tellg();
    file.read((char *) &left, sizeof(long));
    long right_pos = file.tellg();
    file.read((char *) &right, sizeof(long));
    int comparison = Functions::compare(current_name, key);
    if (comparison == 1) {
        if (right == -1) {
            file.seekg(right_pos, std::ios::beg);
            file.write((char *) &pos, sizeof(long));
            right = pos;
        } else {
            AvlTree::insert(key, right, pos, file);
        }
    } else if (comparison == -1) {
        if (left == -1) {
            file.seekg(left_pos, std::ios::beg);
            file.write((char *) &pos, sizeof(long));
            left = pos;
        } else {
            AvlTree::insert(key, left, pos, file);
        }
    } else {
        //delete last line of file
//        file.seekg(0, std::ios::end);
//        long size = file.tellg();
//        file.seekg(0, std::ios::beg);
//        char *buffer = new char[size];
//        file.read(buffer, size);
//        file.close();
//        file.open(file_name, std::ios::out | std::ios::binary);
//        file.write(buffer, size - LINE_SIZE);
//        delete[] buffer;
        return;
    }
    update_height(file, current_pos);

    auto balance = balancing_factor(current_pos, file);


    long header;
    file.seekg(0, std::ios::beg);
    file.read((char*) &header, sizeof(long));
    auto current = get_record(current_pos);
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
    if (header == current_pos) {
        file.seekg(0, std::ios::beg);
        file.write((char*)&current.right, sizeof(long));
    } else {
        auto parent = get_parent_pos(header, current_pos);
        auto parentRecord = get_record(parent);
        file.seekg(get_file_pos_from_pos(parent) + data_size, std::ios::beg);
        if (parentRecord.left == current_pos) {
            file.write((char*)&current.right, sizeof(long));
        } else {
            file.seekg(sizeof (long), std::ios::cur);
            file.write((char*)&current.right, sizeof(long));
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
    if (header == current_pos) {
        file.seekg(0, std::ios::beg);
        file.write((char*)&current.left, sizeof(long));
    } else {
        auto parent = get_parent_pos(header, current_pos);
        auto parentRecord = get_record(parent);
        file.seekg(get_file_pos_from_pos(parent) + data_size, std::ios::beg);
        if (parentRecord.right == current_pos) {
            file.seekg(sizeof (long), std::ios::cur);
            file.write((char*)&current.left, sizeof(long));
        } else {
            file.write((char*)&current.left, sizeof(long));

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

        char user_id[sizeof(GameRecord::user_id)];
        char name[sizeof(GameRecord::name)];
        float hours;
        file.read(user_id, sizeof(user_id));
        file.read(name, sizeof(name));
        file.read((char *) &hours, sizeof(float));
        if (file.eof()) {
            break;
        }
        std::cout << "Count: " << count++ << " ";
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

        char user_id[sizeof(GameRecord::user_id)];
        char name[sizeof(GameRecord::name)];
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

    file.read(record.user_id, sizeof(GameRecord::user_id));
    file.read(record.name, sizeof(GameRecord::name));
    file.read((char *) &record.hours, sizeof(GameRecord::hours));
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
        strcpy(record.user_id, tokens[0].c_str());
        std::cout << line_count << " " << tokens[1].c_str() << std::endl;
        strcpy(record.name, tokens[1].c_str());
        record.hours = std::stof(tokens[2]);
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
    char user_id[sizeof(GameRecord::user_id)];
    char name[sizeof(GameRecord::name)];
    float hours;
    file.read(user_id, sizeof(user_id));
    file.read(name, sizeof(name));
    file.read((char *) &hours, sizeof(float));

    long left, right, height;
    file.read((char *) &left, sizeof(long));
    file.read((char *) &right, sizeof(long));
    file.read((char *) &height, sizeof(long));
    print_preorder(file, left, count);
    std::cout << "User ID: " << user_id << " ";
    std::cout << "Name: " << name << " ";
    std::cout << "Platform: " << hours << " ";


    std::cout << "Left: " << left << " ";
    std::cout << "Right: " << right << " ";
    std::cout << "Height: " << height << std::endl;
    print_preorder(file, right, count);
}