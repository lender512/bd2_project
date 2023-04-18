//
// Created by luis.berrospi on 4/17/23.
//

#include <fstream>
#include <iostream>
#include <cstring>
#include "AvlTree.h"
#include "functions.h"

AvlTree::AvlTree(const std::string &file_name) {
    this->file = std::fstream(file_name, std::ios::in | std::ios::out | std::ios::binary);
    this->file.seekg(0, std::ios::end);
    if (file.tellg() == 0) {
        long trash = -1;
        this->file.write((char *) &trash, sizeof(long));
    }
}

bool AvlTree::insert(const GameRecord &record) {
    file.clear();
    this->file.seekg(0, std::ios::end);
    if (file.tellg() != 0) {
        this->file.write("\n", 1);
    }
    long pos = ((file.tellg()) - (sizeof(long) + 1))/(sizeof(GameRecord::user_id) + sizeof(GameRecord::name) + sizeof(GameRecord::hours) + 2*sizeof(long) + 1);
    this->file.write(record.user_id, sizeof(record.user_id));
    this->file.write(record.name, sizeof(record.name));
    this->file.write((char *) &record.hours, sizeof(float));
    long trash = -1;
    this->file.write((char *) &trash, sizeof(trash));
    this->file.write((char *) &trash, sizeof(trash));
    this->file.seekg(0, std::ios::beg);
    long header;
    this->file.read((char *) &header, sizeof(long));
    this->file.seekg(1, std::ios::cur);
    if (header == -1) {
        this->file.seekg(0, std::ios::beg);
        this->file.write((char *)&pos, sizeof(long));
        return true;
    } else {
        insert((char *) record.name, header, pos);
    }
}

void AvlTree::insert(char key[85], long current_pos, long pos) {
    file.clear();
    long file_pos = sizeof (long) + 1;
    file_pos += current_pos * (sizeof(GameRecord::user_id) + sizeof(GameRecord::name) + sizeof(GameRecord::hours) + 2*sizeof(long) + 1);
    this->file.seekg(file_pos, std::ios::beg);
    this->file.seekg(sizeof(GameRecord::user_id), std::ios::cur);
    char current_name[85];
    this->file.read(current_name, sizeof(current_name));
    this->file.seekg(sizeof(GameRecord::hours), std::ios::cur);
    long left;
    long right;
    long left_pos = file.tellg();
    this->file.read((char*)&left, sizeof(long));
    long right_pos = file.tellg();
    this->file.read((char*)&right, sizeof(long));
    if (Functions::compare(current_name, key) == 1) {
        if (right == -1) {
            this->file.seekg(right_pos, std::ios::beg);
            this->file.write((char*)&pos, sizeof(long));
            return;
        }
        AvlTree::insert(key, right, pos);
    } else {
        if (left == -1) {
            this->file.seekg(left_pos, std::ios::beg);
            this->file.write((char*)&pos, sizeof(long));
            return;
        }
        AvlTree::insert(key, left, pos);
    }

}


void AvlTree::print_debug() {
    this->file.seekg(0, std::ios::beg);
    long header;
    this->file.read((char *) &header, sizeof(header));
    std::cout << "Header: " << header << std::endl;
    while (true) {
        this->file.seekg(1, std::ios::cur);

        char user_id[9];
        char name[85];
        float hours;
        file.read(user_id, sizeof(user_id));

        file.read(name, sizeof(name));
        file.read((char*)&hours, sizeof(float));
        if (file.eof()) {
            break;
        }
        std::cout << "User ID: " << user_id << " ";
        std::cout << "Name: " << name << " ";
        std::cout << "Platform: " << hours << " ";
        long left;
        long right;
        file.read((char*)&left, sizeof(long));
        file.read((char*)&right, sizeof(long));
        std::cout << "Left: " << left << " ";
        std::cout << "Right: " << right << std::endl;
    }
}

