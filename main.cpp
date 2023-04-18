#include <iostream>
#include "AvlTree.h"
// load from https://www.kaggle.com/datasets/tamber/steam-video-games

int main() {
    AvlTree avlTree("games.dat");
    auto record1 = GameRecord{ "asdadsas", "Luis", 123.04};
    auto record2 = GameRecord{ "asdadsas", "Mario", 123.04};
    auto record3 = GameRecord{ "asdadsas", "Jose", 123.04};
    auto record4 = GameRecord{ "asdadsas", "Ignacio", 123.04};

    avlTree.insert(record1);
    avlTree.insert(record2);
    avlTree.insert(record3);
    avlTree.insert(record4);
    avlTree.print_debug();

    return 0;
}