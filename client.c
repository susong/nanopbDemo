#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <netinet/in.h>
#include <unistd.h>
#include "pb_encode.h"
#include "pb_decode.h"
#include "common.h"
#include "MessageProto.pb.h"
#include "pb.h"

bool int32ListValue_callback(pb_ostream_t *stream, const pb_field_t *field, void *const *arg) {
    uint32_t int32ListValue[5] = {1, 2, 3, 4, 5};

    if (!pb_encode_tag_for_field(stream, field))
        return false;

    return pb_encode_varint(stream, 42);
}

bool stringValue_callback(pb_ostream_t *stream, const pb_field_t *field, void *const *arg) {
    char *str = "this is stringValue";
    if (!pb_encode_tag_for_field(stream, field)) {
        return false;
    }
    return pb_encode_string(stream, (uint8_t *) str, strlen(str));
}

bool stringListValue_callback(pb_ostream_t *stream, const pb_field_t *field, void *const *arg) {
    char *str[4] = {"Hello world!", "", "Test", "Test2"};
    int i;
    for (i = 0; i < 4; i++) {
        if (!pb_encode_tag_for_field(stream, field)) {
            return false;
        }
        if (!pb_encode_string(stream, (uint8_t *) str[i], strlen(str[i]))) {
            return false;
        }
    }
    return true;
}

bool head_msg_callback(pb_ostream_t *stream, const pb_field_t *field, void *const *arg) {
    char *str = "this is head msg";
    if (!pb_encode_tag_for_field(stream, field)) {
        return false;
    }
    return pb_encode_string(stream, (uint8_t *) str, strlen(str));
}

bool send_message(int fd) {

    {
        pb_ostream_t output = pb_ostream_from_socket(fd);

        Message message = Message_init_zero;

        message.int32Value = 45;

        message.int32ListValue.funcs.encode = &int32ListValue_callback;

        message.stringValue.funcs.encode = &stringValue_callback;

        char *stringValue128 = "this is stringValue128";
        message.has_stringValue128 = true;
        strcpy(message.stringValue128, stringValue128);

        message.stringListValue.funcs.encode = &stringListValue_callback;

        message.has_floatValue = true;
        message.floatValue = 3.14;

        message.has_doubleValue = true;
        message.doubleValue = 0.16;

        message.has_boolValue = true;
        message.boolValue = true;

        message.has_status = true;
        message.status = StatusEnum_OPEN;

        Head head = Head_init_zero;
        head.has_code = true;
        head.code = 100;

        message.has_head = true;
        message.head = head;
        message.head.msg.funcs.encode = &head_msg_callback;

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