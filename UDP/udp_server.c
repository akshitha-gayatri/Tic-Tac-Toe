#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PORT 12345
#define MAX_CLIENTS 2
#define BUFFER_SIZE 1024

typedef struct {
  char board[9]; // 3x3 board
  char current_player;
} TicTacToe;


int sockfd;

void print_board(TicTacToe *game) {
  printf("%c | %c | %c\n", game->board[0], game->board[1], game->board[2]);
  printf("-------\n");
  printf("%c | %c | %c\n", game->board[3], game->board[4], game->board[5]);
  printf("-------\n");
  printf("%c | %c | %c\n", game->board[6], game->board[7], game->board[8]);
}

int make_move(TicTacToe *game, int row, int col) {
  int position = row * 3 + col; // Convert (row, col) to position
  if (game->board[position] == ' ') {
    game->board[position] = game->current_player;
    return 1;
  }
  return 0;
}

char check_winner(TicTacToe *game) {
  int win_conditions[8][3] = {{0, 1, 2}, {3, 4, 5}, {6, 7, 8}, {0, 3, 6},
                              {1, 4, 7}, {2, 5, 8}, {0, 4, 8}, {2, 4, 6}};
  for (int i = 0; i < 8; i++) {
    if (game->board[win_conditions[i][0]] ==
            game->board[win_conditions[i][1]] &&
        game->board[win_conditions[i][1]] ==
            game->board[win_conditions[i][2]] &&
        game->board[win_conditions[i][0]] != ' ') {
      return game->board[win_conditions[i][0]];
    }
  }
  return 0;
}

int is_full(TicTacToe *game) {
  for (int i = 0; i < 9; i++) {
    if (game->board[i] == ' ')
      return 0;
  }
  return 1;
}

void send_board(TicTacToe *game, struct sockaddr_in *clients,
                socklen_t addr_len) {
  char message[BUFFER_SIZE];

  // Create a structured message for the board
  snprintf(message, sizeof(message),
           "Current board:\n"
           "%c | %c | %c\n"
           "-------\n"
           "%c | %c | %c\n"
           "-------\n"
           "%c | %c | %c\n",
           game->board[0], game->board[1], game->board[2], game->board[3],
           game->board[4], game->board[5], game->board[6], game->board[7],
           game->board[8]);

  for (int i = 0; i < MAX_CLIENTS; i++) {
    sendto(sockfd, message, strlen(message), 0,
           (const struct sockaddr *)&clients[i], addr_len);
  }
}

void handle_replay(struct sockaddr_in *clients, socklen_t addr_len) {
  char buffer[BUFFER_SIZE];
  char response[4];                 
  int responses[MAX_CLIENTS] = {0}; 
  int players_ready = 0;

  for (int i = 0; i < MAX_CLIENTS; i++) {

    int n = recvfrom(sockfd, response, sizeof(response), 0, NULL, NULL);
    response[n] = '\0';
    printf("response %s\n", response);
    if (strcmp(response, "yes") == 0) {
      responses[i] = 1;
      players_ready++;
    } else if (strcmp(response, "no") == 0) {
      responses[i] = 0;
      players_ready++;
    } else {
      responses[i] = 0;
      players_ready++;
    }
  }

  if (responses[0] == 1 && responses[1] == 1) {
    snprintf(buffer, sizeof(buffer), "Restarting Game.\n");
    for (int i = 0; i < MAX_CLIENTS; i++) {
      sendto(sockfd, buffer, strlen(buffer), 0,
             (const struct sockaddr *)&clients[i], addr_len);
    }
    printf("Both players want to play again.\n");
    return; 
  } else if (responses[0] == 0 && responses[1] == 0) {
    snprintf(buffer, sizeof(buffer), "End.\n");
    for (int i = 0; i < MAX_CLIENTS; i++) {
      sendto(sockfd, buffer, strlen(buffer), 0,
             (const struct sockaddr *)&clients[i], addr_len);
    }
    printf("Both players do not want to play again. Closing the connection.\n");
  } else {
    if (responses[0] == 1) {

      snprintf(buffer, sizeof(buffer),
               "Your opponent does not wish to play.\n");
      sendto(sockfd, buffer, strlen(buffer), 0,
             (const struct sockaddr *)&clients[0], addr_len);

      snprintf(buffer, sizeof(buffer), "Xppppppppp.\n");
      sendto(sockfd, buffer, strlen(buffer), 0,
             (const struct sockaddr *)&clients[1], addr_len);

      printf("Player 1 wants to continue, but Player 2 does not. Closing the "
             "connection.\n");
    } else {
      snprintf(buffer, sizeof(buffer),
               "Your opponent does not wish to play.\n");
      sendto(sockfd, buffer, strlen(buffer), 0,
             (const struct sockaddr *)&clients[1], addr_len);

      snprintf(buffer, sizeof(buffer), "Xppppppppp.\n");
      sendto(sockfd, buffer, strlen(buffer), 0,
             (const struct sockaddr *)&clients[0], addr_len);

      printf("Player 2 wants to continue, but Player 1 does not. Closing the "
             "connection.\n");
    }
  }

  close(sockfd);
  exit(0);
}

int main() {
  struct sockaddr_in server_addr, client_addr[MAX_CLIENTS];
  char buffer[BUFFER_SIZE];
  socklen_t addr_len = sizeof(client_addr[0]);
  int client_count = 0;

  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd < 0) {
    perror("Socket creation failed");
    exit(EXIT_FAILURE);
  }

  memset(&server_addr, 0, sizeof(server_addr));
  memset(&client_addr, 0, sizeof(client_addr));

  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(PORT);

  if (bind(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) <
      0) {
    perror("Bind failed");
    close(sockfd);
    exit(EXIT_FAILURE);
  }

  printf("Server is running... Waiting for players to join.\n");

  while (client_count < MAX_CLIENTS) {
    int n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0,
                     (struct sockaddr *)&client_addr[client_count], &addr_len);
    buffer[n] = '\0';
    printf("Player %d has joined.\n", client_count + 1);
    client_count++;
  }

  while (1) {
    TicTacToe game = {{' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '}, 'X'};

    send_board(&game, client_addr, addr_len);

    while (1) {
      int current_player = game.current_player == 'X' ? 0 : 1;

      snprintf(
          buffer, sizeof(buffer),
          "Player %c's turn. Choose position (row col):", game.current_player);
      sendto(sockfd, buffer, strlen(buffer), 0,
             (const struct sockaddr *)&client_addr[current_player], addr_len);

      int n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, NULL, NULL);
      buffer[n] = '\0';

      int row, col;
      sscanf(buffer, "%d %d", &row, &col); 

      if (make_move(&game, row-1, col-1)) {
        print_board(&game);
        char winner = check_winner(&game);
        send_board(&game, client_addr,
                   addr_len); 
        if (winner) {
          snprintf(buffer, sizeof(buffer), "Player %c wins!", winner);
          sendto(sockfd, buffer, strlen(buffer), 0,
                 (const struct sockaddr *)&client_addr[0], addr_len);
          sendto(sockfd, buffer, strlen(buffer), 0,
                 (const struct sockaddr *)&client_addr[1], addr_len);
          printf("%s\n", buffer);
          break;
        }

        if (is_full(&game)) {
          snprintf(buffer, sizeof(buffer), "The game is a draw!");
          sendto(sockfd, buffer, strlen(buffer), 0,
                 (const struct sockaddr *)&client_addr[0], addr_len);
          sendto(sockfd, buffer, strlen(buffer), 0,
                 (const struct sockaddr *)&client_addr[1], addr_len);
          printf("%s\n", buffer);
          break;
        }

        game.current_player =
            (game.current_player == 'X') ? 'O' : 'X';
      } else {
        sendto(sockfd, "Invalid move. Try again.", 25, 0,
               (const struct sockaddr *)&client_addr[current_player], addr_len);
      }
    }

    handle_replay(client_addr, addr_len);
  }

  close(sockfd);
  return 0;
}
