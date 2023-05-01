import random
lines = open('games.csv').readlines()
random.shuffle(lines)
open('games.csv', 'w').writelines(lines)

