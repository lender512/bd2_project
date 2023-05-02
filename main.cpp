#include <iostream>
#include "AvlTree.h"
// load from https://www.kaggle.com/datasets/tamber/steam-video-games

int main() {
    std::string file_name = "movies.dat";
    AvlTree<MovieRecord> avlTree(file_name);
    //ranges = [100, 200, 400, 800, 1600, 3200, 6400, 12800, 25600, 51200]
//    avlTree.load_from_csv("movies.csv", 1000, '|');
//    avlTree.test_from_csv("movies.csv", 51201, '|', {100, 200, 400, 800, 1600, 3200, 6400, 12800, 25600, 51200});
//    int title = 6037892;
//    char* title = "PUBG: BATTLEGROUNDS";
//    auto a = avlTree.search_by_name(title);
////    std::cout << a.id << " " << a.primaryTitle << " "<< a.year << " " << a.genres << std::endl;
//    std::cout << a.publisher << std::endl;
//    std::cout << a.gameTitle << std::endl;
//    std::cout << a.price << std::endl;

    auto result = avlTree.search_by_name_range(100000, 150000);
    for (auto &i : result) {
        std::cout << i.id << " " << i.primaryTitle << " "<< i.year << " " << i.genres << std::endl;
    }
//    avlTree.print_debug();
    avlTree.print_preorder();
    return 0;
}