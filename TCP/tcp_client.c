
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PORT 12345

int main() {
  int sock;
  struct sockaddr_in server_address;
  char buffer[1024] = {0};

  // Create socket
  sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0) {
    perror("Socket creation error");
    return -1;
  }

  server_address.sin_family = AF_INET;
  server_address.sin_addr.s_addr = inet_addr("127.0.0.1");
  server_address.sin_port = htons(PORT);

  if (connect(sock, (struct sockaddr *)&server_address,
              sizeof(server_address)) < 0) {
    perror("Connection Failed");
    return -1;
  }
  int flag = 0;
  while (1) {
    int valre = read(sock, buffer, 1024);
    buffer[valre] = '\0';
    printf("%s\n", buffer);

    if (strstr(buffer, "Your turn")) {
      char move[10];
      while (1) {
        printf("Enter your move (row and column): ");
        fgets(move, sizeof(move), stdin);

        move[strcspn(move, "\n")] = 0;

        send(sock, move, strlen(move), 0);

        valre = read(sock, buffer, 1024);
        buffer[valre] = '\0';
        printf("%s", buffer);

        if (strstr(buffer, "Invalid move! Try again.") ||
            strstr(buffer, "Invalid input! Try again.")) {
          continue;
        }

        break;
      }
    }

    if (strstr(buffer, "wins") || strstr(buffer, "draw")) {
      char replay[10];
      while (1) {
        printf("Do you want to play again? (yes/no): ");
        fgets(replay, sizeof(replay), stdin);
        replay[strcspn(replay, "\n")] = 0;

        send(sock, replay, strlen(replay), 0);

        valre = read(sock, buffer, 1024);
        buffer[valre] = '\0';
        printf("%s", buffer);

        if (strstr(buffer, "Your opponent") || strstr(buffer, "bye") ||
            strstr(buffer, "End")) {
          flag++;
        }

        valre = read(sock, buffer, 1024);
        buffer[valre] = '\0';
        printf("%s", buffer);

        if (strstr(buffer, "Your opponent") || strstr(buffer, "bye") ||
            strstr(buffer, "End")) {
          flag++;
        }

        if (flag > 0) {
          close(sock);
          exit(0);
        }

        if (strstr(replay, "yes") || strstr(replay, "no")) {
          break;
        }
      }
      continue;
    }
  }

  close(sock);
  return 0;
}
