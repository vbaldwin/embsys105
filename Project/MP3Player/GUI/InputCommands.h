#ifndef INPUTCOMMANDS_H_
#define INPUTCOMMANDS_H_

typedef enum _input_command
{
    INPUTCOMMAND_PLAY = 0,
    INPUTCOMMAND_STOP,
    INPUTCOMMAND_SKIP,
    INPUTCOMMAND_RESTART,
    INPUTCOMMAND_FF,
    INPUTCOMMAND_RWD,
    INPUTCOMMAND_VOLUP,
    INPUTCOMMAND_VOLDOWN,
} INPUT_COMMAND;

#endif /* INPUTCOMMANDS_H_ */