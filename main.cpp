#include <iostream>
#include "AvlTree.h"
// load from https://www.kaggle.com/datasets/tamber/steam-video-games

int main() {
    std::string file_name = "games.dat";
    AvlTree<GameRecord> avlTree(file_name);
    //ranges = [100, 200, 400, 800, 1600, 3200, 6400, 12800, 25600, 51200]
//    avlTree.load_from_csv("games.csv", 10, '|');
//    avlTree.test_from_csv("movies.csv", 51201, '|', {100, 200, 400, 800, 1600, 3200, 6400, 12800, 25600, 51200});
//    int title = 142032;
    avlTree.delete_item("Counter-Strike: Global Offensive");

//    int title = 3315550;
//    char* title = "PUBG: BATTLEGROUNDS";
    auto a = avlTree.search_by_name("Counter-Strike: Global Offensive");
//    std::cout << a.id << " " << a.primaryTitle << " "<< a.year << " " << a.genres << std::endl;
//    std::cout << a.publisher << std::endl;
//    std::cout << a.gameTitle << std::endl;
//    std::cout << a.price << std::endl;
    char left[150] = "A";
    char right[150] = "G";
    auto result = avlTree.search_by_name_range(left, right);
    for (auto &i : result) {
//        std::cout << i.id << " " << i.primaryTitle << " "<< i.year << " " << i.genres << std::endl;
        std::cout << i.publisher << " "  << i.gameTitle <<" " <<i.publisher << std::endl;
    }

//    avlTree.print_debug();
//    avlTree.print_preorder();
    return 0;
}