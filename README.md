# Project3Schetroma

//Project 3 is a spell checking server that is designed to accept connections 
//from clients and perform spell checking on the words received. 
//It uses a predefined dictionary of words to determine whether a word is spelled correctly or not. 
//The server can handle multiple client connections simultaneously because of the use of threads.


//The code defines a server that listens for incoming connections on port 8888 by default.
//It uses multithreading to handle incoming connections and spell checking concurrently. Three types of threads are created: worker threads, logger threads, and client threads.
//The worker threads are responsible for processing incoming client connections, spell checking words, and logging the results.
//The logger threads dequeue log messages from a log queue and print them to the console.
//The client threads are used to demonstrate how clients can connect to the server and send words for spell checking.
//The server uses a predefined dictionary of words (e.g., /usr/share/dict/words) to check the spelling of words received from clients.
//Spell checking results are logged, indicating whether a word is spelled correctly ("OK") or misspelled ("MISSPELLED").

//DEBUGGING AND TESTING
// For debugging, there were many instances of trial and error with the use of threads.
// The most difficult part through the debugging process was to find a way to 
// access the dictionary and compare the input from the client to the dictionary.
// In the end I did not attain proper functionality because of a bug within opening 
// and accessing the dictionary, as well as comparing the input of the client with the dictionary.
// For testing, I ran the server as well as another client that accessed the server to use the spell checker.
//The client was able to receive feedback from the server although it was incorrect.
//
