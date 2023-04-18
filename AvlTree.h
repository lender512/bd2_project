//
// Created by luis.berrospi on 4/17/23.
//

#ifndef BD2_PROJECT_AVLTREE_H
#define BD2_PROJECT_AVLTREE_H

#include <string>
#include <fstream>

struct GameRecord {
    char user_id[9];
    char name[85]; //primary key
    float hours;
};

class AvlTree {
private:
    std::fstream file;

    void insert(char *key, long current_pos, long pos);

public:
    explicit AvlTree(const std::string &file_name);

    bool insert(const GameRecord &record);

    void print_debug();
};


#endif //BD2_PROJECT_AVLTREE_H
