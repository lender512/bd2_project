import random
lines = open('movies.csv').readlines()
random.shuffle(lines)
open('movies.csv', 'w').writelines(lines)

