/* This is a simple TCP server that listens on port 1234 and provides lists
 * of files to clients, using a protocol defined in file_server.proto.
 *
 * It directly deserializes and serializes messages from network, minimizing
 * memory use.
 * 
 * For flexibility, this example is implemented using posix api.
 * In a real embedded system you would typically use some other kind of
 * a communication and filesystem layer.
 */

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>

#include "pb_encode.h"
#include "pb_decode.h"

#include "MessageProto.pb.h"
#include "common.h"


/* Handle one arriving client connection.
 * Clients are expected to send a ListFilesRequest, terminated by a '0'.
 * Server will respond with a ListFilesResponse message.
 */
void handle_connection(int fd) {

    /* Decode the message from the client and open the requested directory. */
    {
        pb_istream_t input = pb_istream_from_socket(fd);

        Message message = Message_init_zero;

        if (!pb_decode_delimited(&input, Message_fields, &message)) {
            printf("Decode failed: %s\n", PB_GET_ERROR(&input));
            return;
        }

        printf("server message: %d\n", message.int32Value);
    }

    {
        pb_ostream_t output = pb_ostream_from_socket(fd);

        Message message = Message_init_zero;

        message.int32Value = 46;

        if (!pb_encode_delimited(&output, Message_fields, &message)) {
            fprintf(stderr, "Encoding failed: %s\n", PB_GET_ERROR(&output));
            return;
        }
    }
}

int main(int argc, char **argv) {
    int listenfd, connfd;
    struct sockaddr_in servaddr;
    int reuse = 1;

    /* Listen on localhost:1234 for TCP connections */
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    servaddr.sin_port = htons(8080);
    if (bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) != 0) {
        perror("bind");
        return 1;
    }

    if (listen(listenfd, 5) != 0) {
        perror("listen");
        return 1;
    }

    for (;;) {
        /* Wait for a client */
        connfd = accept(listenfd, NULL, NULL);

        if (connfd < 0) {
            perror("accept");
            return 1;
        }

        printf("Got connection.\n");

        handle_connection(connfd);

        printf("Closing connection.\n");

        close(connfd);
    }

    return 0;
}
