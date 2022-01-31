# Hangman
This is a multithreaded hangman game written in C.
Firstly, when any client enters he/she gives his/her nickname.
If nickname colides, the server asks to input new nickname,
 and then send all the available rooms.
If the user enters a number bigger than the number of rooms available,
 a new room is created. 
Program waits until there are more than 1 person in the room.
On the server side, a room is treated as a leaderboard.

Compile & Run guide:
	There is makefile, so just type "make" at the project folder command line.
	Run server : ./server
	Run client : ./client 127.0.0.1 12345
