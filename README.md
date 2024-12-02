# Networking

### 1. XOXO
#### TCP:
The server creates and initializes a TCP socket on port 12345, accepts connections from two players, and manages the game logic, including player turns and board updates.The updated board is sent to both the clients by the server.It checks for valid moves and determines if there’s a winner or if the game ends in a draw, broadcasting the current board state and game results to both players after each turn. After the game concludes, the server prompts both players to decide whether to play again, resetting the board if both agree or closing the connections if they do not. The client establishes a connection to the server on 127.0.0.1 and interacts with the game by reading server messages, displaying the current game state, and sending the player's moves. It also handles prompts for replaying the game, ensuring a seamless multiplayer experience over the network.


#### UDP :
The server manages the game state, including the 3x3 board, current player, and game logic such as making moves, checking for a winner, and determining if the board is full. When a player joins, their address is recorded, and the server sends the current game board after each move. Clients send their moves to the server, which validates them and updates the game state accordingly. Once a game concludes, players are prompted to decide whether to play again. The server and client communicate using structured messages, and the server handles reconnections and responses based on player input.

## To run :
Open 3 terminals out of which one should run the server and the other 2 should run the client.First server should be compiled and the clients should be compiled.
