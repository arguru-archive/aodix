/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Aodix Core IRC Implementation
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "./aodixcore.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
DWORD WINAPI irc_thread(void* pcore)
{
	// local information
	char nick_name[32];
	char user_name[64]={"adx_usr localhost localhost :aodix_irc_client"};
	char chan_name[32]={"#aodix"};
	char serv_name[32]={"irc.inet.tele.dk"};
	long serv_port=6666;

	// holder
	char buf[512];
	char stream_buf[512];

	// get core pointer
	CAodixCore* padx=(CAodixCore*)pcore;

	// format nickname
	sprintf(nick_name,"adx%s",padx->cfg.user_name);
	nick_name[8]=0;

	// init irc lines
	for(int il=0;il<MAX_IRC_LINES;il++)
	{
		padx->irc_line[il][0]='B';
		padx->irc_line[il][1]=0;
	}

	// init user irc vars
	sprintf(padx->irc_user_line,"Type CHAT Line Here...");

	// init irc properties
	padx->irc_num_nicks=0;
	padx->irc_nick_list_offset=0;
	padx->irc_num_lines=20;
	padx->irc_line_list_offset=0;

	// version format
	sprintf(stream_buf,"%d",padx->aodix_version);
	sprintf(buf,"B*** Aodix IRC Client Version %c.%c.%c.%c",stream_buf[0],stream_buf[1],stream_buf[2],stream_buf[3]);

	// irc headline message
	padx->irc_add_line("B");
	padx->irc_add_line(buf);
	padx->irc_add_line("B*** (C) Arguru Audio Software");
	padx->irc_add_line("B");

	// winsock startup
	WSADATA wsa_data;

	if(WSAStartup(0x101,&wsa_data))
	{
		padx->irc_add_line("E*** WSAStartup failed");
		return TRUE;
	}
	else
	{
		padx->irc_add_line("A*** WSAStartup done");
	}

	// connector socket
	SOCKET conn=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);

	if(conn==INVALID_SOCKET)
	{
		padx->irc_add_line("E*** Invalid socket");
		return TRUE;
	}
	else
	{
		sprintf(buf,"A*** Connector socket #%d",conn);
		padx->irc_add_line(buf);
	}

	// host ent
	hostent* hp=NULL;

	// retrieve server address
	if(inet_addr(serv_name)==INADDR_NONE)
	{
		hp=gethostbyname(serv_name);
	}
	else
	{
		unsigned long const addr=inet_addr(serv_name);
		hp=gethostbyaddr((char*)&addr,sizeof(addr),AF_INET);
	}

	// cant resolve
	if(hp==NULL)
	{
		sprintf(buf,"E*** Failed to resolve '%s'",serv_name);
		padx->irc_add_line(buf);
		closesocket(conn);
		return TRUE;
	}

	// server struct
	SOCKADDR_IN sin_s;

	// fill server struct
	sin_s.sin_addr.s_addr=*((unsigned long*)hp->h_addr);
	sin_s.sin_family=AF_INET;
	sin_s.sin_port=htons(serv_port);

	// log connection
	sprintf(buf,"A*** Connecting to '%s:%d'",serv_name,serv_port);
	padx->irc_add_line(buf);

	// connect to server
	if(connect(conn,(sockaddr*)&sin_s,sizeof(sin_s)))
	{
		padx->irc_add_line("E*** Connection failed");
		closesocket(conn);
		return TRUE;
	}

	// connection success
	padx->irc_add_line("A*** Connected to server");
	padx->irc_add_line("A");

	// send nick
	sprintf(buf,"NICK %s",nick_name);
	padx->irc_send(buf,conn);

	// send user
	sprintf(buf,"USER %s",user_name);
	padx->irc_send(buf,conn);

	// send channel
	sprintf(buf,"JOIN %s",chan_name);
	padx->irc_send(buf,conn);

	// receive data
	int y=0;

	// server line parser holder
	char parser_buf[512];
	int  parser_cnt=0;

	// loop receive
	while(y=recv(conn,stream_buf,512,0))
	{
		// parser server data
		if(y==SOCKET_ERROR)
		{
			// network error
			sprintf(buf,"E*** Network error code #%d",WSAGetLastError());
			padx->irc_add_line(buf);
		}
		else
		{
			// scan each character
			for(int c=0;c<y;c++)
			{
				// get character
				char const s_chr=stream_buf[c];

				// carriage return found
				if(s_chr=='\r')
				{
					// terminate parser string with null character
					parser_buf[parser_cnt]=0;

					// restart parser counter
					parser_cnt=0;

					// dispatched flag
					int parser_dispatched=0;

					// ping server messages
					if(strncmp(parser_buf,"PING",4)==0)
					{
						// verbose ping-pong
						padx->irc_add_line("A*** Ping? Pong!");

						// send pong back
						sprintf(buf,"PONG %s",parser_buf+6);
						padx->irc_send(buf,conn);

						// set dispatched
						parser_dispatched=1;
					}

					// notice auth server messages
					if(strncmp(parser_buf,"NOTICE AUTH",11)==0)
					{
						sprintf(buf,"N%s",parser_buf+13);
						padx->irc_add_line(buf);

						// set dispatched
						parser_dispatched=1;
					}

					// server client to client messages
					if(parser_buf[0]==':')
					{
						// set dispatched
						parser_dispatched=1;

						// message counter
						int msg_cnt=1;

						// get full message len
						int const str_len=strlen(parser_buf);

						// retrieve sender id buffer
						char sender_buf[512];
						int sender_cnt=0;

						// scan sender until space
						while(parser_buf[msg_cnt]!=' ')
							sender_buf[sender_cnt++]=parser_buf[msg_cnt++];

						// terminate sender string with null character
						sender_buf[sender_cnt]=0;

						// terminate nick
						for(int tni=0;tni<sender_cnt;tni++)
						{
							// look for '!' character
							if(sender_buf[tni]=='!')
								sender_buf[tni]=0;
						}

						// skip space
						msg_cnt++;

						// retrieve command buffer
						char command_buf[512];
						int command_cnt=0;

						// scan command until space
						while(parser_buf[msg_cnt]!=' ')
							command_buf[command_cnt++]=parser_buf[msg_cnt++];

						// terminate command string with null character
						command_buf[command_cnt]=0;

						// retrieve parameter
						char parameter_buf[512];

						// default null terminated
						parameter_buf[0]=0;

						// scan parameter
						for(msg_cnt=1;msg_cnt<str_len;msg_cnt++)
						{
							// find second ':'
							if(parser_buf[msg_cnt]==':')
							{
								sprintf(parameter_buf,parser_buf+msg_cnt+1);
								msg_cnt=str_len;
							}
						}

						// parse client commands
						if(strncmp(command_buf,"PRIVMSG",7)==0)
						{
							// ignore client to client protocol (ctcp) messages
							if(parameter_buf[0]!=0x01)
							{
								// private or channel message
								sprintf(buf,"P<%s> %s",sender_buf,parameter_buf);
								padx->irc_add_line(buf);
							}
						}
						else if(strncmp(command_buf,"QUIT",4)==0)
						{
							sprintf(buf,"S*** %s has quit IRC",sender_buf);
							padx->irc_del_nick(sender_buf);
							padx->irc_add_line(buf);
						}
						else if(strncmp(command_buf,"PART",4)==0)
						{
							sprintf(buf,"S*** %s has left channel",sender_buf);
							padx->irc_del_nick(sender_buf);
							padx->irc_add_line(buf);
						}
						else if(strncmp(command_buf,"JOIN",4)==0)
						{
							sprintf(buf,"S*** %s has joined %s",sender_buf,parameter_buf);
							padx->irc_add_nick(sender_buf);
							padx->irc_add_line(buf);
						}
						else if(strncmp(command_buf,"NICK",4)==0)
						{
							sprintf(buf,"S*** %s is now known as %s",sender_buf,parameter_buf);
							padx->irc_del_nick(sender_buf);
							padx->irc_add_nick(parameter_buf);
							padx->irc_add_line(buf);
						}
						else if(strncmp(command_buf,"MODE",4)==0)
						{
							sprintf(buf,"S*** %s sets mode: %s",sender_buf,parameter_buf);
							padx->irc_add_line(buf);
						}
						else if(strncmp(command_buf,"332",3)==0)
						{
							sprintf(buf,"A*** Topic is '%s'",parameter_buf);
							padx->irc_add_line(buf);
						}
						else if(strncmp(command_buf,"353",3)==0)
						{
							// get parameter num chars
							int const par_len=strlen(parameter_buf);

							// names parser buffers
							char names_buf[32];
							int names_cnt=0;

							// parse names list
							for(int ps=0;ps<par_len;ps++)
							{
								// scan spaces
								if(parameter_buf[ps]!=' ')
								{
									// fill new nick
									names_buf[names_cnt++]=parameter_buf[ps];
								}
								else
								{
									// add nick to list
									names_buf[names_cnt]=0;
									names_cnt=0;

									// add new nick
									padx->irc_add_nick(names_buf);
								}
							}
						}
						else
						{
							//	sprintf(buf,"S*** <%s> (%s) '%s'",sender_buf,command_buf,parameter_buf);
							//	padx->irc_add_line(buf);
						}
					}

					// undispatched parser data
					if(parser_dispatched==0)
					{
						sprintf(buf,"S*U* %s",parser_buf);
						padx->irc_add_line(parser_buf);
					}
				}
				else
				{
					// check that character is not line feed
					if(s_chr!='\n')
						parser_buf[parser_cnt++]=s_chr;
				}
			}
		}
	}

	// quit message
	padx->irc_send("QUIT",conn);

	// connection success
	padx->irc_add_line("A");
	padx->irc_add_line("A*** Connection Terminated");
	padx->irc_add_line("A");

	// close socket
	closesocket(conn);

	// de-initialize winsock
	WSACleanup();

	// end thread
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::irc_add_nick(char* buf)
{
	// if too many nicks, return inmediatelly
	if(irc_num_nicks>=MAX_IRC_NICKS)
		return;

	// lowercase nick strings
	char lwr_nick_a[32];
	char lwr_nick_b[32];

	// lowcase nick parameter
	sprintf(lwr_nick_a,buf);
	strlwr(lwr_nick_a);

	// scan nicks
	for(int n=0;n<irc_num_nicks;n++)
	{
		// lowcase nick list
		sprintf(lwr_nick_b,irc_nick[n]);
		strlwr(lwr_nick_b);

		// compare strings
		int const str_comp=strcmp(lwr_nick_a,lwr_nick_b);

		// nick already exist, return inmediatelly
		if(str_comp==0)
			return;

		// insert in 'n' pos if less
		if(str_comp<0)
		{
			// push all strings
			for(int p=irc_num_nicks;p>n;p--)
				sprintf(irc_nick[p],irc_nick[p-1]);

			// insert new nick
			sprintf(irc_nick[n],buf);

			// increment num nicks
			irc_num_nicks++;

			// update flag
			gui_update_flag=1;

			// exit function
			return;
		}
	}

	// add new nick at end
	sprintf(irc_nick[irc_num_nicks++],buf);

	// update flag
	gui_update_flag=1;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::irc_del_nick(char* buf)
{
	// scan nicks
	for(int n=0;n<irc_num_nicks;n++)
	{
		// compare strings
		if(strcmp(buf,irc_nick[n])==0)
		{
			// dec num nicks
			irc_num_nicks--;

			// pull all strings
			for(int p=n;p<irc_num_nicks;p++)
				sprintf(irc_nick[p],irc_nick[p+1]);

			// update flag
			gui_update_flag=1;

			// exit function
			return;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::irc_chg_nick(char* prv,char* now)
{
	// scan nicks
	for(int n=0;n<irc_num_nicks;n++)
	{
		// compare strings
		if(strcmp(prv,irc_nick[n])==0)
		{
			// change to new
			sprintf(irc_nick[n],now);

			// update flag
			gui_update_flag=1;

			// exit function
			return;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::irc_add_line(char* buf)
{
	// check num irc text lines
	if(irc_num_lines<MAX_IRC_LINES)
	{
		// add new line
		sprintf(irc_line[irc_num_lines++],buf);
	}
	else
	{
		// pull lines
		for(int l=0;l<(MAX_IRC_LINES-1);l++)
			sprintf(irc_line[l],irc_line[l+1]);

		// insert new last line
		sprintf(irc_line[MAX_IRC_LINES-1],buf);
	}

	// set irc line offset
	irc_line_list_offset=irc_num_lines-20;

	// update flag
	gui_update_flag=1;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::irc_send(char* buf,unsigned int const sck)
{
	// holder
	char s_buf[512];

	// get length
	int const s_len=strlen(buf);

	// copy buffer
	sprintf(s_buf,buf);

	// clamp buffer
	s_buf[s_len]='\r';
	s_buf[s_len+1]='\n';

	// send data
	send(sck,s_buf,s_len+2,0);
}
