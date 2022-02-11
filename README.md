# Hangman
1. The player connects to the server.
2. The player sends his nickname
a. if the name is already taken, the server asks for a different nickname
3. The player goes to the screen of available rooms with the name and number of players
4. The player chooses a room or creates a new one
5. After hitting a game room, the currently guessing word is displayed (default = empty
characters), his gallows, player ranking, and images with the current other players' gallows
a. in case the player is the only person in the room is displayed information about
waiting for more players.
6. The game starts as soon as more than one player joins the room.
7. The server sends all players blanks for letters, along with the word category.
8. If the player correctly guesses a letter, each occurrence of that letter is shown in the
respective gaps, otherwise, another line is added to the drawing hangman.
9. 7. If the drawing is completed, the player scores -2 points. I the player reveals all letters
score 1 point.
10. When at least half of the players have finished solving the puzzle, it begins
11. A countdown after which the round ends and everyone who has not finished solving puzzles
receives 0 points.
12. If a player has played more than 20 rounds, the result of the oldest round is subtracted from
his ranking.
