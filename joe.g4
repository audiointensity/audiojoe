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
    | random_directive
    | text_directive
    | time_directive
    | if_block
    ;

if_block
    :
    | if_directive NEWLINE+
        commands NEWLINE+
      (elseif_chain)?
      (else_block)?
      endif_directive
    ;

if_directive
    : 'if' ID
    ;

elseif_chain
    : elseif_directive NEWLINE+
        commands NEWLINE+
      (elseif_chain)?
    ;

elseif_directive
    : 'elseif' ID
    ;

else_block
    : else_directive NEWLINE+
        commands NEWLINE+
    ;

else_directive
    : 'else'
    ;

endif_directive
    : 'endif'
    ;

event_directive
    : '!' EVENT_NAME
    ;

random_directive
    : 'random' state_decl+
    ;

state_decl
    : ID '=' number '%'
    ;

text_directive
    : 'text' STRING
    ;

time_directive
    : (INT 'h')
    | (INT 'm')
    | (INT 's')
    | (INT 'h') (INT 'm')
    | (INT 'h') (INT 's')
    | (INT 'm') (INT 's')
    | (INT 'h') (INT 'm') (INT 's')
    ;

number
    : REAL
    | INT
    ;

EVENT_NAME
    : 'start'
    | 'peak'
    | 'end'
    ;

REAL
    : DIGIT+ '.' DIGIT+
    ;

INT
    : DIGIT+
    ;

fragment
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

ID
    : [a-zA-Z_][a-zA-Z0-9]*
    ;

STRING
    : '"' ~[\r\n\"]* '"' 
    ;
