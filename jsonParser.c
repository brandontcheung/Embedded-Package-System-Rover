#include "jsonParser.h"
#include "uart_term.h"
#include "jsmn.h"

// JAKE PAN OCT.23

static int jsoneq(const char *json, jsmntok_t *tok, const char *s)
{
    if (tok->type == JSMN_STRING && (int) strlen(s) == tok->end - tok->start
            && strncmp(json + tok->start, s, tok->end - tok->start) == 0)
    {
        return 0;
    }
    return -1;
}
//int jsonParser_MQTT(char *JSON_STRING, char bufferRecipient, char bufferSender, char bufferMsgID)

int jsonParser_MQTT(const char *JSON_STRING, char *bufferRecipient, char *bufferSender, char *bufferMsgID, char *bufferBody ,char *bufferElse)
{
    int i;
    int r;
    jsmn_parser p;
    jsmntok_t t[128]; /* We expect no more than 256 tokens */
    //need try 256 512 and setting of thread size

    //change to 512  still need change setting for thread or queue size .
    char tempValue1[64]; // Recipient
    char tempValue2[64]; // Sender
    char tempValue3[64]; // MSG_ID
    char tempValue4[64]; // Body
    char tempValue5[64]; // Else


    jsmn_init(&p);
    r = jsmn_parse(&p, JSON_STRING, strlen(JSON_STRING), t,
                   sizeof(t) / sizeof(t[0]));
    if (r < 0) {
//      printf("Failed to parse JSON: %d\n", r);
      return 1;
    }

    /* Assume the top-level element is an object */
    if (r < 1 || t[0].type != JSMN_OBJECT) {
//      printf("Object expected\n");
      return 1;
    }

    /* Loop over all keys of the root object */
    for (i = 1; i < r; i++)
    {
        if (jsoneq(JSON_STRING, &t[i], "Recipient") == 0)
        {
            /* We may use strndup() to fetch string value */
//        printf("- User: %.*s\n", t[i + 1].end - t[i + 1].start,
//               JSON_STRING + t[i + 1].start);
            memcpy((void*)tempValue1, (const void*)(JSON_STRING + t[i + 1].start),
                   t[i + 1].end - t[i + 1].start);
            sprintf(bufferRecipient, "%s", tempValue1);
            i++;
        }
        else if (jsoneq(JSON_STRING, &t[i], "Sender") == 0)
        {
            /* We may additionally check if the value is either "true" or "false" */
//        printf("- Admin: %.*s\n", t[i + 1].end - t[i + 1].start,
//               JSON_STRING + t[i + 1].start);
            memcpy((void*)tempValue2, (const void*)(JSON_STRING + t[i + 1].start),
                   t[i + 1].end - t[i + 1].start);

            sprintf(bufferSender, "%s", tempValue2);

            i++;
        }
        else if (jsoneq(JSON_STRING, &t[i], "MSGID") == 0)
        {
            /* We may want to do strtol() here to get numeric value */
//        printf("- UID: %.*s\n", t[i + 1].end - t[i + 1].start,
//               JSON_STRING + t[i + 1].start);
            memcpy((void*)tempValue3, (const void*)(JSON_STRING + t[i + 1].start),
                   t[i + 1].end - t[i + 1].start);
            sprintf(bufferMsgID, "%s", tempValue3);
            i++;
        }
        else if (jsoneq(JSON_STRING, &t[i], "Body") == 0)
        {
            /* We may want to do strtol() here to get numeric value */
            //        printf("- UID: %.*s\n", t[i + 1].end - t[i + 1].start,
            //               JSON_STRING + t[i + 1].start);
            memcpy((void*)tempValue4, (const void*)(JSON_STRING + t[i + 1].start),
                   t[i + 1].end - t[i + 1].start);
            sprintf(bufferBody, "%s", tempValue4);
            i++;
        }
        else
        {
            memcpy((void*)tempValue5, (const void*)(JSON_STRING + t[i + 1].start),
                   t[i + 1].end - t[i + 1].start);
            sprintf(bufferElse, "%s", tempValue5);
            i++;
        }
    }

    return 0;

}
