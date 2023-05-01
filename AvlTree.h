//
// Created by luis.berrospi on 4/17/23.
//

#ifndef BD2_PROJECT_AVLTREE_H
#define BD2_PROJECT_AVLTREE_H

#include <string>
#include <fstream>
#include <vector>


struct GameRecord {
    char publisher[90];
    char gameTitle[150]; //primary key
    float price;
};

struct GameRecordMetaData {
    GameRecord gameRecord;
    long left; //primary key
    long right;
    long height;
    long deleted;
};

class AvlTree {
private:
    std::string file_name;
    void insert(char *key, long current_pos, long pos, std::fstream &file);
    std::tuple<long, long> update_height(std::fstream &file, long current_pos);
    GameRecordMetaData get_record(long pos);
    int insert_memory_accesses = 0;
    int search_memory_accesses = 0;

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

    bool delete_item(char *key);

    GameRecordMetaData search_by_name(std::fstream &file, long current_pos, char *name);

    std::vector<GameRecord> search_by_name_range(char *name1, char *name2);

    void search_by_name_range(std::fstream &file, std::vector<GameRecord> &records, long current_pos, char *left_name,
                              char *right_name);

    GameRecord search_by_name(char *name);

    void test_from_csv(const char *string, int number, char delimiter, const std::vector<int> &breaks);

    void test(int line_count, std::string &file_name);
};


#endif //BD2_PROJECT_AVLTREE_H
