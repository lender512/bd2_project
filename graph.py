import pandas as pd
import numpy as np
import matplotlib.pyplot as plt

df = pd.read_csv("games_data.csv", sep="\t")
plt.clf()
plt.plot(df["n"], df["search_time"])
plt.savefig("graph/games_running_time_search.png")
plt.clf()
plt.plot(df["n"], df["insert_time"])
plt.savefig("graph/games_running_time_insert.png")
plt.clf()
plt.plot(df["n"], df["search_memory"])
plt.savefig("graph/games_secondary_memory_search.png")
plt.clf()
plt.plot(df["n"], df["insert_memory"])
plt.savefig("graph/games_secondary_memory_insert.png")
plt.clf()

df = pd.read_csv("movies_data.csv", sep="\t")
plt.plot(df["n"], df["search_time"])
plt.savefig("graph/movies_running_time_search.png")
plt.clf()
plt.plot(df["n"], df["insert_time"])
plt.savefig("graph/movies_running_time_insert.png")
plt.clf()
plt.plot(df["n"], df["search_memory"])
plt.savefig("graph/movies_secondary_memory_search.png")
plt.clf()
plt.plot(df["n"], df["insert_memory"])
plt.savefig("graph/movies_secondary_memory_insert.png")
plt.clf()

