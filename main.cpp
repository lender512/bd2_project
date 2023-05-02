#include <iostream>
#include "AvlTree.h"
// load from https://www.kaggle.com/datasets/tamber/steam-video-games

int main() {
    std::string file_name = "movies.dat";
    AvlTree<MovieRecord> avlTree(file_name);
    //ranges = [100, 200, 400, 800, 1600, 3200, 6400, 12800, 25600, 51200]
//    avlTree.load_from_csv("movies.csv", 100, '|');
    avlTree.test_from_csv("movies.csv", 51201, '|', {100, 200, 400, 800, 1600, 3200, 6400, 12800, 25600, 51200});
//    int title = 3;
//    auto a = avlTree.search_by_name(title);
//    std::cout << a.primaryTitle << std::endl;
//    std::cout << a.key << std::endl;
//    std::cout << a.text << std::endl;
//    std::cout << a.number << std::endl;


//    avlTree.print_debug();
//    avlTree.print_preorder();
    return 0;
}