#include "postgres.h"
#include "fmgr.h"
#include "utils/builtins.h"
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <arpa/inet.h>

/* pgvector 호환 구조체 */
typedef struct Vector {
    int32 vl_len_;
    uint16 dim;
    uint16 unused;
    float x[FLEXIBLE_ARRAY_MEMBER];
} Vector;

PG_MODULE_MAGIC;
PG_FUNCTION_INFO_V1(tobgem3vector_c);

#define SOCKET_PATH "/tmp/bge_m3_model.sock"

char *escape_json(const char *src) {
    if (src == NULL) return NULL;

    int len = strlen(src);
    // 모든 문자가 이스케이프 대상일 경우를 대비해 넉넉히 할당
    char *dest = palloc(len * 2 + 1);
    char *p = dest;

    while (*src) {
        switch (*src) {
            case '\\': *p++ = '\\'; *p++ = '\\'; break;
            case '"':  *p++ = '\\'; *p++ = '"';  break;
            case '\n': *p++ = '\\'; *p++ = 'n';  break;
            case '\r': *p++ = '\\'; *p++ = 'r';  break;
            case '\t': *p++ = '\\'; *p++ = 't';  break;
            default:
                // ASCII 제어 문자(0x00~0x1F) 처리 (선택사항이나 권장)
                if ((unsigned char)*src < 0x20) {
                    // 무시하거나 공백으로 대체
                    *p++ = ' ';
                } else {
                    *p++ = *src;
                }
                break;
        }
        src++;
    }
    *p = '\0';
    return dest;
}

Datum
tobgem3vector_c(PG_FUNCTION_ARGS)
{
    text *input_text = PG_GETARG_TEXT_PP(0);
    char *query = text_to_cstring(input_text);
    int sock;
    struct sockaddr_un server_addr;
    char *escaped_query = escape_json(query); // 이스케이프 처리 추가
    char *payload = psprintf("{\"query\": \"%s\"}", escaped_query);
    uint32_t payload_len = (uint32_t)strlen(payload);
    uint32_t net_len = htonl(payload_len);

    if ((sock = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) elog(ERROR, "Socket error");
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, SOCKET_PATH, sizeof(server_addr.sun_path) - 1);

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        close(sock);
        elog(ERROR, "Connect error");
    }

    write(sock, &net_len, 4);
    write(sock, payload, payload_len);

    uint32_t resp_net_len;
    if (recv(sock, &resp_net_len, 4, MSG_WAITALL) != 4) {
        close(sock);
        elog(ERROR, "Header error");
    }
    uint32_t resp_len = ntohl(resp_net_len);
    uint16 dim = (uint16)(resp_len / sizeof(float));

    // 중요: offsetof 대신 정확한 크기 계산
    size_t vec_size = sizeof(Vector) + resp_len - sizeof(float);
    Vector *result = (Vector *) palloc0(vec_size);

    // SET_VARSIZE 대신 직접 비트 연산 (PostgreSQL 13+ 표준 방식)
    result->vl_len_ = (int32) (vec_size << 2);
    result->dim = dim;

    if (recv(sock, result->x, resp_len, MSG_WAITALL) != resp_len) {
        close(sock);
        elog(ERROR, "Recv error");
    }
    close(sock);

    PG_RETURN_POINTER(result);
}
