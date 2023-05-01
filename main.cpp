#include <iostream>
#include "AvlTree.h"
// load from https://www.kaggle.com/datasets/tamber/steam-video-games

int main() {
    std::string file_name = "games.dat";
    AvlTree avlTree(file_name);
    //ranges = [100, 200, 400, 800, 1600, 3200, 6400, 12800, 25600, 51200]
//    avlTree.load_from_csv("games.csv", 1600, '|');
    avlTree.test_from_csv("games.csv", 51201, '|', {100, 200, 400, 800, 1600, 3200, 6400, 12800, 25600, 51200});


//    avlTree.print_debug();
//    avlTree.print_preorder();
    return 0;
}