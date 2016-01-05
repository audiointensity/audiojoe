grammar joe;

joefile
    : blocks EOF
    ;

blocks
    : NEWLINE* block
    | NEWLINE* block '--' SPACE? NEWLINE+ blocks
    ;

block
    : lines
    ;

lines
    : line SPACE? NEWLINE*
    | line SPACE? NEWLINE+ lines
    ;

line
    : event_directive
    | text_directive
    | time_directive
    ;

event_directive
    : '!' EVENT_NAME
    ;

text_directive
    : 'text' SPACE STRING
    ;

time_directive
    : (DIGIT+ 'h')
    | (DIGIT+ 'm')
    | (DIGIT+ 's')
    | (DIGIT+ 'h') (DIGIT+ 'm')
    | (DIGIT+ 'h') (DIGIT+ 's')
    | (DIGIT+ 'm') (DIGIT+ 's')
    | (DIGIT+ 'h') (DIGIT+ 'm') (DIGIT+ 's')
    ;

EVENT_NAME
    : 'start'
    | 'peak'
    | 'end'
    ;

DIGIT
    : '0'..'9'
    ;

SPACE
    : (' '|'\t')+
    ;

NEWLINE
    : ('\r' | '\n' | '\r\n') 
    ;

STRING
    : '"' ~[\r\n\"]* '"' 
    ;
