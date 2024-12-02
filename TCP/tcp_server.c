
#include <arpa/inet.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PORT 12345
#define SIZE 3

char board[SIZE][SIZE];
int current_player = 0;
int players[2];

void reset_board() {
  for (int i = 0; i < SIZE; i++)
    for (int j = 0; j < SIZE; j++)
      board[i][j] = ' ';
}

void print_board(int sock) {
  char buffer[256] = "\nCurrent board:\n";
  for (int i = 0; i < SIZE; i++) {
    for (int j = 0; j < SIZE; j++) {
      strncat(buffer, &board[i][j], 1);
    }
    strncat(buffer, "\n", 1);
    if (i < SIZE - 1) {
      strncat(buffer, "---\n", 5);
    }
  }
  send(sock, buffer, strlen(buffer), 0);
}

int make_move(int player, int row, int col) {
  if (row < 0 || row >= SIZE || col < 0 || col >= SIZE ||
      board[row][col] != ' ') {
    return 0;
  }
  board[row][col] = (player == 0) ? 'X' : 'O';
  return 1;
}

int check_winner() {
  for (int i = 0; i < SIZE; i++)
    if (board[i][0] == board[i][1] && board[i][1] == board[i][2] &&
        board[i][0] != ' ')
      return 1;

  for (int i = 0; i < SIZE; i++)
    if (board[0][i] == board[1][i] && board[1][i] == board[2][i] &&
        board[0][i] != ' ')
      return 1;

  if (board[0][0] == board[1][1] && board[1][1] == board[2][2] &&
      board[0][0] != ' ')
    return 1;

  if (board[0][2] == board[1][1] && board[1][1] == board[2][0] &&
      board[0][2] != ' ')
    return 1;

  return 0;
}

int is_draw() {
  for (int i = 0; i < SIZE; i++)
    for (int j = 0; j < SIZE; j++)
      if (board[i][j] == ' ')
        return 0;
  return 1;
}

void broadcast(const char *message) {
  for (int i = 0; i < 2; i++)
    send(players[i], message, strlen(message), 0);
}

void handle_replay() {
  char replay_buffer[1024];
  int responses[2] = {0, 0};

  for (int i = 0; i < 2; i++) {
    int valread = read(players[i], replay_buffer, 1024);
    replay_buffer[valread] = '\0';

    for (char *p = replay_buffer; *p; ++p)
      *p = tolower(*p);

    if (strstr(replay_buffer, "yes")) {
      responses[i] = 1;
    } else if (strstr(replay_buffer, "no")) {
      responses[i] = 0;
    }
  }

  if (responses[0] == 1 && responses[1] == 1) {
    reset_board();
    current_player = 0;
    broadcast("Both players want to play again. Starting new game!\n");
    printf("Both players want to play again. Starting new game!\n");
    print_board(players[0]);
    return;
  } else if (responses[0] == 0 && responses[1] == 0) {
    broadcast("End\n");
    printf("Both players do not want to continue. Closing connection.\n");
    close(players[0]);
    close(players[1]);
    exit(0);
  }

  else {

    int quitting_player = (responses[0] == 0) ? 0 : 1;
    int continuing_player = 1 - quitting_player;

    char message[256];
    snprintf(message, sizeof(message), "bye.\n");
    broadcast(message);
    send(players[continuing_player],
         "Your opponent has chosen not to continue. Closing connection.\n", 63,
         0);
    close(players[continuing_player]);
    exit(0);
  }
}

int main() {
  int server_fd, new_socket;
  struct sockaddr_in address;
  int addrlen = sizeof(address);
  char buffer[1024] = {0};

  server_fd = socket(AF_INET, SOCK_STREAM, 0);
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(PORT);

  bind(server_fd, (struct sockaddr *)&address, sizeof(address));
  listen(server_fd, 2);
  printf("Waiting for players to connect...\n");

  for (int i = 0; i < 2; i++) {
    new_socket =
        accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
    printf("Player %d connected.\n", i + 1);
    players[i] = new_socket;
  }

  while (1) {
    reset_board();
    current_player = 0;
    broadcast("Game started! Player 1 is 'X' and Player 2 is 'O'.\n");
    print_board(players[0]);
    print_board(players[1]);

    while (1) {
      int player = current_player;

      while (1) {
        send(players[player],
             "Your turn! Enter row and column (0-2) separated by a space:\n",
             64, 0);
        int valread = read(players[player], buffer, 1024);
        buffer[valread] = '\0';

        int row, col;
        if (sscanf(buffer, "%d %d", &row, &col) != 2 || row - 1 < 0 ||
            row - 1 >= SIZE || col - 1 < 0 || col - 1 >= SIZE) {
          send(players[player], "Invalid input! Try again.\n", 27, 0);
          continue;
        }

        if (make_move(player, row - 1, col - 1)) {
          print_board(players[0]);
          print_board(players[1]);

          if (check_winner()) {
            char message[256];
            snprintf(message, sizeof(message), "Player %d wins!\n", player + 1);
            broadcast(message);
            break;
          }

          if (is_draw()) {
            broadcast("It's a draw!\n");
            break;
          }

          current_player = 1 - current_player;
          break;
        } else {
          send(players[player], "Invalid move! Try again.\n", 26, 0);
        }
      }

      if (check_winner() || is_draw()) {
        handle_replay();
        break;
      }
    }
  }

  close(players[0]);
  close(players[1]);
  close(server_fd);
  return 0;
}
