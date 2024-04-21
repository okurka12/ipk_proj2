/*****************
**  Vit Pavlik  **
**   xpavli0a   **
**    251301    **
**     IPK2     **
**     2024     **
*****************/

#ifndef _I_O_T_A_H_
#define _I_O_T_A_H_

#define LF "\n"

#define HELP_TXT \
"Usage: " \
"ipk24chat-server [-l ADDRESS] [-p PORT] [-d TIMEOUT] [-r RETRIES] [-h]" LF \
"todo"

/* maximal length of field `Username` */
#define MAX_UNAME_LEN 20

/* maximal length of field `ChannelID` */
#define MAX_CHID_LEN 20

/* maximal length of field `Secret` */
#define MAX_SECRET_LEN 128

/* maximal length of field `DisplayName` */
#define MAX_DNAME_LEN 20

/* maximal length of field `MessageContent` */
#define MAX_MSGCONT_LEN 1400

#define DEFAULT_CHANNEL "general"


#endif  // ifndef _I_O_T_A_H_
