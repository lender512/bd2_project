//
// Created by luis.berrospi on 4/17/23.
//

#ifndef BD2_PROJECT_AVLTREE_H
#define BD2_PROJECT_AVLTREE_H

#include <string>
#include <fstream>



struct GameRecord {
    char user_id[10];
    char name[85]; //primary key
    float hours;
};

struct GameRecordMetaData {
    GameRecord gameRecord;
    long left; //primary key
    long right;
    long height;
};

class AvlTree {
private:
    std::string file_name;
    void insert(char *key, long current_pos, long pos, std::fstream &file);
    std::tuple<long, long> update_height(std::fstream &file, long current_pos);
    GameRecordMetaData get_record(long pos);

public:
    explicit AvlTree(std::string &file_name);

    bool insert(const GameRecord &record);

    void print_debug();

    size_t get_file_pos_from_pos(long left_left_pos) const;

    long get_parent_pos(long pos, long search_pos);

    void rotate_left(long &current_pos, std::fstream &file);

    void rotate_right(long &current_pos, std::fstream &file);

    long balancing_factor(long &current_pos, std::fstream &file);

    void print_file(std::fstream &file);

    void load_from_csv(std::string file_name, int line_number = -1, char delimiter = ',');

    void print_preorder();

    void print_preorder(std::fstream &file, long pos, int &count);
};


#endif //BD2_PROJECT_AVLTREE_H
