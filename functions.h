//
// Created by luis.berrospi on 4/17/23.
//

#ifndef BD2_PROJECT_FUNCTIONS_H
#define BD2_PROJECT_FUNCTIONS_H

#include "AvlTree.h"


namespace Functions {
    template <typename Record, typename T>
    static int compare(T a, T b) {
        if constexpr (std::is_same<Record, GameRecord>::value) {
            int i = 0;
            while (a[i] != '\0' && b[i] != '\0') {
                if (a[i] < b[i]) {
                    return 1;
                } else if (a[i] > b[i]) {
                    return -1;
                }
                i++;
            }
            if (a[i] == '\0' && b[i] == '\0') {
                return 1;
            } else if (a[i] == '\0') {
                return 1;
            } else {
                return -1;
            }
        }

        if constexpr (std::is_same<Record, MovieRecord>::value) {
            return -1*((a <= b) * -1 + (a > b) * 1);
        }

    }

};


#endif //BD2_PROJECT_FUNCTIONS_H
