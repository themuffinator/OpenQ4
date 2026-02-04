

#include "../Game_local.h"

// This equates to about 1 second
#define MAX_VOICE_PACKET_SIZE	382

idCVar si_voiceChat( "si_voiceChat", "1", CVAR_GAME | CVAR_BOOL | PC_CVAR_ARCHIVE | CVAR_SERVERINFO | CVAR_INFO, "enables or disables voice chat through the server" );
idCVar g_voiceChatDebug( "g_voiceChatDebug", "0", CVAR_GAME | CVAR_INTEGER | CVAR_NOCHEAT, "display info on voicechat net traffic" );

void idMultiplayerGame::ReceiveAndForwardVoiceData( int clientNum, const idBitMsg &inMsg, int messageType ) {
	assert( clientNum >= 0 && clientNum < MAX_CLIENTS );

	idBitMsg	outMsg;
	int			i;
	byte		msgBuf[MAX_VOICE_PACKET_SIZE + 2];
	idPlayer *	from;
	
	from = ( idPlayer * )gameLocal.entities[clientNum];
	if( !gameLocal.serverInfo.GetBool( "si_voiceChat" ) || !from ) {
		return;
	}

	// Create a new packet with forwarded data
	outMsg.Init( msgBuf, sizeof( msgBuf ) );
	outMsg.WriteByte( GAME_UNRELIABLE_MESSAGE_VOICEDATA_SERVER );
	outMsg.WriteByte( clientNum );
	outMsg.WriteData( inMsg.GetReadData(), inMsg.GetRemainingData() );

	if( g_voiceChatDebug.GetInteger() & 2 ) {
		common->Printf( "VC: Received %d bytes, forwarding...\n", inMsg.GetRemainingData() );
	}

	// Forward to appropriate parties
	for( i = 0; i < gameLocal.numClients; i++ )  {
		idPlayer* to = ( idPlayer * )gameLocal.entities[i];
		if( to && to->GetUserInfo() && to->GetUserInfo()->GetBool( "s_voiceChatReceive" ) )
		{
			if( i != gameLocal.localClientNum && CanTalk( from, to, !!( messageType & 1 ) ) )
			{
				if( messageType & 2 )
				{
					// If "from" is testing - then only send back to him
					if( from == to )
					{
						gameLocal.SendUnreliableMessage( outMsg, to->entityNumber );
					}
				}
				else
				{
					if( to->AllowedVoiceDest( from->entityNumber ) )
					{
						gameLocal.SendUnreliableMessage( outMsg, to->entityNumber );
						if( g_voiceChatDebug.GetInteger() & 2 )
						{
							common->Printf( " ... to client %d\n", to->entityNumber );
						}
					}
					else
					{
						if( g_voiceChatDebug.GetInteger() )
						{
							common->Printf( " ... suppressed packet to client %d\n", to->entityNumber );
						}
					}
				}
			}
		}
	}

#ifdef _USE_VOICECHAT
	// Listen servers need to manually call the receive function
	if ( gameLocal.isListenServer ) {
		// Skip over control byte
		outMsg.ReadByte();
        
		idPlayer* to = gameLocal.GetLocalPlayer();
		if( to->GetUserInfo()->GetBool( "s_voiceChatReceive" ) )
		{
			if( CanTalk( from, to, !!( messageType & 1 ) ) )
			{
				if( messageType & 2 )
				{
					// If "from" is testing - then only send back to him
					if( from == to )
					{
						ReceiveAndPlayVoiceData( outMsg );
					}
				}
				else
				{
					if( to->AllowedVoiceDest( from->entityNumber ) )
					{
						if( g_voiceChatDebug.GetInteger() & 2 )
						{
							common->Printf( " ... to local client %d\n", gameLocal.localClientNum );
						}
						ReceiveAndPlayVoiceData( outMsg );
					}
				}
			}
		}
	}
#endif
}

bool idMultiplayerGame::CanTalk( idPlayer *from, idPlayer *to, bool echo ) {
	if ( !to ) {
		return false;
	}

	if ( !from ) {
		return false;
	}

	if ( from->spectating != to->spectating ) {
		return false;
	}

	if ( to->IsPlayerMuted( from ) ) {
		return false;
	}

	if ( to->GetArena() != from->GetArena() ) {
		return false;
	}

	if ( gameLocal.IsTeamGame() && from->team != to->team ) {
		return false;
	}

	if ( from == to ) {
		return echo;
	}

	return true;
}

#ifdef _USE_VOICECHAT

void idMultiplayerGame::XmitVoiceData( void )
{
	
}

void idMultiplayerGame::ReceiveAndPlayVoiceData( const idBitMsg &inMsg )
{
	
}

#endif

// end
