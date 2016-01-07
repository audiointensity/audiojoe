grammar joe;

joefile
    : blocks EOF
    ;

blocks
    : NEWLINE* block
    | NEWLINE* block '--' NEWLINE+ blocks
    ;

block
    : commands
    ;

commands
    : command NEWLINE*
    | command NEWLINE+ commands
    ;

command
    : event_directive
    | text_directive
    | time_directive
    | if_block
    ;

if_block
    :
    | if_directive NEWLINE+
        commands NEWLINE+
      (elseif_chain)?
      endif_directive
    ;

if_directive
    : 'if' DIGIT+
    ;

elseif_chain
    : elseif_directive NEWLINE+
        commands NEWLINE+
      (elseif_chain)?
    ;

elseif_directive
    : 'elseif' DIGIT+
    ;

endif_directive
    : 'endif'
    ;

event_directive
    : '!' EVENT_NAME
    ;

text_directive
    : 'text' STRING
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
        -> channel(HIDDEN)
    ;

NEWLINE
    : ('\r' | '\n' | '\r\n') 
    ;

STRING
    : '"' ~[\r\n\"]* '"' 
    ;
