#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <netinet/in.h>
#include <unistd.h>
#include "pb_encode.h"
#include "pb_decode.h"
#include "common.h"
#include "MessageProto.pb.h"

bool send_message(int fd) {

    {
        pb_ostream_t output = pb_ostream_from_socket(fd);

        Message message = Message_init_zero;

        message.int32Value = 45;

        if (!pb_encode_delimited(&output, Message_fields, &message)) {
            fprintf(stderr, "Encoding failed: %s\n", PB_GET_ERROR(&output));
            return false;
        }
    }

    {
        pb_istream_t input = pb_istream_from_socket(fd);

        Message message = Message_init_zero;

        if (!pb_decode_delimited(&input, Message_fields, &message)) {
            fprintf(stderr, "Decode failed: %s\n", PB_GET_ERROR(&input));
            return false;
        }

        printf("client message: %d\n", message.int32Value);

    }

    return true;
}

int main(int argc, char **argv) {
    int sockfd;
    struct sockaddr_in servaddr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    /* Connect to server running on localhost:1234 */
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    servaddr.sin_port = htons(8080);

    if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) != 0) {
        perror("connect");
        return 1;
    }

    /* Send the directory listing request */
    if (!send_message(sockfd))
        return 2;

    /* Close connection */
    close(sockfd);

    return 0;
}