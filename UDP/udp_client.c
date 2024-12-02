#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PORT 12345
#define BUFFER_SIZE 1024

int main() {
  int sockfd;
  struct sockaddr_in server_addr;
  char buffer[BUFFER_SIZE];

  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd < 0) {
    perror("Socket creation failed");
    exit(EXIT_FAILURE);
  }

  memset(&server_addr, 0, sizeof(server_addr));

  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  server_addr.sin_port = htons(PORT);

  sendto(sockfd, "C", strlen("C"), 0, (const struct sockaddr *)&server_addr,
         sizeof(server_addr));

  while (1) {
    int n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, NULL, NULL);
    buffer[n] = '\0';
    printf("%s\n", buffer);
    if (strstr(buffer, "En") != NULL || strstr(buffer, "Your") != NULL) {
      break;
    }

    if (strstr(buffer, "wins!") != NULL || strstr(buffer, "draw!") != NULL) {
      char play_again[4];
      printf("Do you want to play again? (yes/no): \n");
      scanf("%s", play_again);

      sendto(sockfd, play_again, strlen(play_again), 0,
             (const struct sockaddr *)&server_addr, sizeof(server_addr));

      int n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, NULL, NULL);
      buffer[n] = '\0';
      if (strstr(buffer, "En") != NULL || strstr(buffer, "Xp") != NULL) {
        break;
      }
      printf("%s\n", buffer);
      if (strstr(buffer, "Your") != NULL) {
        break;
      }
      continue;
    }

    if (strstr(buffer, "turn") != NULL) {
      printf("Your move (row col): ");
      int row, col;
      scanf("%d %d", &row, &col);
      snprintf(buffer, sizeof(buffer), "%d %d", row, col);
      sendto(sockfd, buffer, strlen(buffer), 0,
             (const struct sockaddr *)&server_addr, sizeof(server_addr));
    }
  }

  close(sockfd);
  printf("Game over. Connection closed.\n");
  return 0;
}
