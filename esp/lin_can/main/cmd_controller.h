#ifndef CMD_CONTROLLER_H
#define CMD_CONTROLLER_H

enum CMD_ST
{
	CMD_ST_IDLE = 0,
	CMD_ST_EXEC
};

void          init_cmd();
enum CMD_ST  exec_cmd();

int     senario( char *data , int size );

#endif
