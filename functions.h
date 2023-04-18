//
// Created by luis.berrospi on 4/17/23.
//

#ifndef BD2_PROJECT_FUNCTIONS_H
#define BD2_PROJECT_FUNCTIONS_H


namespace Functions {
    static int compare(char *a, char *b) {
        int i = 0;
        while (a[i] != '\0' && b[i] != '\0') {
            if (a[i] < b[i]) {
                return -1;
            } else if (a[i] > b[i]) {
                return 1;
            }
            i++;
        }
        if (a[i] == '\0' && b[i] == '\0') {
            return 0;
        } else if (a[i] == '\0') {
            return -1;
        } else {
            return 1;
        }
    }
};


#endif //BD2_PROJECT_FUNCTIONS_H
