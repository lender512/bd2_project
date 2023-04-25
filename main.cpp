#include <iostream>
#include "AvlTree.h"
// load from https://www.kaggle.com/datasets/tamber/steam-video-games

int main() {
    std::string file_name = "games.dat";
    AvlTree avlTree(file_name);
    avlTree.load_from_csv("games.csv", 10000, '|');


//    avlTree.print_debug();
    avlTree.print_preorder();
    return 0;
}