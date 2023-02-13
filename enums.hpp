#ifndef ENUMS_HPP
# define ENUMS_HPP

enum	privilege { OPERATOR = 1, VOICE_PRIO = 2, NO_PRIO = 3, INVITED = 4 };
enum	modes { NONE = 0, SECRET = 1, PRIV = 2, MODERATED = 4, INVITE_ONLY = 8, TOPIC_RESTRICTED = 16, KEY_PROTECTED = 32 };

#endif