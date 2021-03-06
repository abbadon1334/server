<?php
// - - - - - - - - - - - - - BEGIN LICENSE BLOCK - - - - - - - - - - - - -
// Version: MPL 1.1/GPL 2.0/LGPL 2.1
//
// The contents of this file are subject to the Mozilla Public License Version
// 1.1 (the "License"); you may not use this file except in compliance with
// the License. You may obtain a copy of the License at
// http://www.mozilla.org/MPL/
//
// Software distributed under the License is distributed on an "AS IS" basis,
// WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
// for the specific language governing rights and limitations under the
// License.
//
// The Original Code is sitefusion.sourceforge.net code.
//
// The Initial Developer of the Original Code is
// FrontDoor Media Group.
// Portions created by the Initial Developer are Copyright (C) 2009
// the Initial Developer. All Rights Reserved.
//
// Contributor(s):
//   Nikki Auburger <nikki@thefrontdoor.nl> (original author)
//   Tom Peeters <tom@thefrontdoor.nl>
//
// - - - - - - - - - - - - - - END LICENSE BLOCK - - - - - - - - - - - - -


/**
 * @package Daemon
 * @subpackage Services
*/


include_once( 'extendedprocess.php.inc' );


/**
 * @package Daemon
 * @subpackage Services
*/

class ServiceProcess extends ExtendedSiteFusionProcess
{
	static public $Started;
	static private $ServiceStarted = FALSE;
	static public $Service = NULL;
	static private $Clients = array();
	static private $SocketPath;

	static public function StartServer( $parentSocket, $eventPipe, $serviceId, $owner, $service, $args, $sessionId ) {
		if( self::$ServiceStarted || self::$ProcessType )
			return;

		self::$ProcessType = 'service';
		self::$ServiceStarted = TRUE;

		self::SetSignalHandlers();

		self::$SessionID = $sessionId;

		self::$ParentSocket = $parentSocket;
		self::$EventPipe = $eventPipe;

		if( self::$Options['debug'] )
			self::$DebugSocket = self::OpenUnixSocket( self::$Options['runtimeDirectory'] . '/' . 'debug'.posix_getpid().'.sock' );

		self::$SocketPath = self::$Options['runtimeDirectory'] . '/service' . posix_getpid() . '.sock';
		self::$Socket = self::OpenUnixSocket( self::$SocketPath );
		self::$Started = time();
		self::$LastKeepalive = time();

		$GLOBALS['include_path'] = self::$Options['sitefusionPath'] . '/';
		$path = $owner . ($owner == '' ? '':'/') . $service;
		self::$ApplicationPath = $path;

		if( preg_match( '/[^a-zA-Z0-9_\-\/]/', $path ) || (! file_exists(self::$Options['sitefusionPath'].'/app/' . $path . '.service.php.inc')) ) {
			self::Log( 'Service does not exist: ' . $path);
			self::DatabaseQuery('DELETE FROM services WHERE id = ' . intval($serviceId));
			exit();
		}

		include_once( 'sys/service.php.inc' );
		include_once( 'app/' . $path . '.service.php.inc' );

		$class = getService( $args );

		self::$Service = new $class;
		self::$Service->init( $args );
		self::$Service->setDefaultEventGroup();

		self::UpdateDatabase();
		self::ServiceLoop();
		exit();
	}

	static private function StopServiceServer() {
		socket_close( self::$Socket );
		socket_close( self::$ParentSocket );
		socket_close( self::$EventPipe );
		if( self::$Options['debug'] )
			socket_close( self::$DebugSocket );
	}

	static protected function ServiceLoop( $subloop = FALSE ) {
		try {
			while(1) {
				if( self::$Shutdown ) {
					self::StopServiceServer();
					return;
				}

				if( time() - self::$LastKeepalive > self::$Options['keepaliveInterval'] ) {
					self::WriteCommand( self::$ParentSocket, 'LIVE' );
					self::$LastKeepalive = time();
				}

				for( $n = 0; $n < count(self::$Clients); $n++ ) {
					if( time() - self::$Clients[$n]->lastKeepalive > self::$Options['serviceClientTimeout'] )
						self::CloseClient( self::$Clients[$n] );
				}

				// Update database every two seconds
				if( (!isset($lastTime)) || time() - $lastTime > 1 ) {
					self::UpdateDatabase();
					$lastTime = time();
				}

				if( self::$Options['debug'] ) {
					if( self::$ConnectedDebugger )
						self::ListenToDebugger();
					else
						self::$ConnectedDebugger = self::PollForDebugger();
				}

				if(! self::$DebugPaused ) {
					self::ListenToParent();
					self::ListenToClients();
					self::CheckTimers();

					if( $client = self::PollForClient() ) {
						try {
							$cmd = self::ReadCommand( $client );

							switch ( $cmd->command ) {
								case 'GETSERVICE': self::HandleNewClient( $client, $cmd ); break;
							}
						}
						catch ( SFException $ex ) {
							throw $ex;
						}
					}
					//else usleep( self::$Options['daemonCycleInterval'] );
				}
				else usleep( self::$Options['daemonCycleInterval'] );

				if( $subloop && !self::$DebugPaused )
					return;
			}
		}
		catch ( SFException $ex ) {
			if( self::$ConnectedDebugger )
				self::ToDebuggerShowStack( $ex->getTrace(), FALSE, $ex->getMessage() . ' in ' . $ex->getFile() . ' at line ' . $ex->getLine() );

			self::Log( 'Uncaught Exception in service ('.self::$SessionID.'): ' . $ex->getFormattedReport() );
			self::StopServiceServer();
			return;
		}
	}

	static private function ListenToParent() {
		$read = array(self::$ParentSocket,self::$EventPipe);
		$write = NULL;
		$except = NULL;

		while( 1 ) {
			try {
				if( socket_select($read, $write, $except, 0) < 1 )
					return;
			}
			catch ( SFException $ex ) { return; }

			if( count($read) ) {
				foreach ( $read as $socket ) {
					try {
						$cmd = self::ReadCommand( $socket );

						switch ( $cmd->command ) {
							case 'EVENT': self::HandleEvent( $cmd ); break;
						}
					}
					catch ( SFException $ex ) { self::Log( 'Uncaught Exception in service ('.self::$SessionID.'): ' . $ex->getFormattedReport() ); }
				}
			}
			else return;
		}
	}

	static private function ListenToClients() {
		$read = array();
		$write = NULL;
		$except = NULL;

		foreach ( self::$Clients as $c ) {
			$read[] = $c->socket;
		}

		if( ! count($read) )
			return;
		try {
			if( socket_select($read, $write, $except, 0) < 1 )
				return;
		}
		catch ( SFException $ex ) { return; }

		foreach ( $read as $socket ) {
			$client = self::GetClientBySocket( $socket );

			try {
				$cmd = self::ReadCommand( $socket );

				switch ( $cmd->command ) {
					case 'LIVE': self::HandleClientCmdKeepalive( $client ); break;
					case 'CALL': self::HandleClientCmdCall( $client, $cmd ); break;
					case 'CLOSE': self::CloseClient( $client ); break;
				}
			}
			catch ( SFException $ex ) {}
		}
	}

	static private function PollForClient() {
		try {
			$read = array( self::$Socket );
			$write = NULL;
			$except = NULL;
			if( @socket_select( $read, $write, $except, 0, self::$Options['daemonCycleInterval'] ) > 0 ) {
				$client = socket_accept( self::$Socket );
			}
			else {
				return NULL;
			}
		} catch(SFException $ex) {
			return FALSE;
		}

		return $client;
	}

	static private function CloseClient( $client ) {
		if( ($index = array_search( $client, self::$Clients, TRUE )) !== FALSE ) {
			try {
				@socket_close(self::$Clients[$index]);
			} catch (SFException $ex) {

			}

			self::$Clients[$index]->connected = FALSE;
			self::$Service->handleCall( '__disconnect', array( self::$Clients[$index] ) );
			array_splice( self::$Clients, $index, 1 );
		}
	}

	static private function HandleNewClient( $client, $cmd ) {
		$clientObj = new ServiceClient;
		$clientObj->socket = $client;
		$clientObj->started = time();
		$clientObj->lastKeepalive = time();

		self::$Clients[] = $clientObj;

		self::$Service->handleCall( '__connect', array( $clientObj ) );
	}

	static private function HandleClientCmdKeepalive( $client ) {
		$client->lastKeepalive = time();
	}

	static private function HandleClientCmdCall( $client, $cmd ) {
		try {
			$args = unserialize( $cmd->data );
		}
		catch ( SFException $ex ) {
			$ret = $ex;
		}

		if( ! isset($ret) ) {
		//	ob_start();

			array_unshift( $args, $client );

			try {
				$ret = self::$Service->handleCall( $cmd->name, $args );
			}
			catch ( SFException $ex ) {
				self::Log( 'Uncaught Exception in service ('.self::$SessionID.'): ' . $ex->getFormattedReport() );
				$ret = $ex;
			}

		//	ob_end_clean();
		}

		self::WriteCommand( $client->socket, 'CALL', NULL, serialize($ret) );
	}

	static private function HandleEvent( $cmd ) {
		try {
			list($event,$args) = unserialize( $cmd->data );
		}
		catch ( SFException $ex ) {
			return;
		}

	//	ob_start();

		try {
			self::$Service->handleGlobalEvent( $event, $args );
		}
		catch ( SFException $ex ) {
			self::Log( 'Uncaught Exception in service ('.self::$SessionID.'): ' . $ex->getFormattedReport() );
		}

	//	ob_end_clean();
	}

	static private function CheckTimers() {
		if( isset(self::$Timers[0]) && self::$Timers[0][0] <= microtime(TRUE) ) {
			try {
				list($time,$timer) = array_shift( self::$Timers );
			//	ob_start();
				$timer->trigger();
			//	ob_end_clean();
			}
			catch ( SFException $ex ) {
				self::Log( 'Uncaught Exception in service ('.self::$SessionID.'): ' . $ex->getFormattedReport() );
			}
		}
	}

	static private function GetClientBySocket( $socket ) {
		foreach ( self::$Clients as $client ) {
			if( isset($client->socket) && $socket == $client->socket )
				return $client;
		}
	}

	static public function SendClientCallback( $client, $name, $args ) {
		if(! $client->connected )
			return FALSE;

		self::WriteCommand( $client->socket, 'CALLBACK', NULL, serialize( array($name,$args) ) );
		$cmd = self::ReadCommand( $client->socket );
		return ($cmd->command == 'OK');
	}

	static private function UpdateDatabase() {
		if( !self::$Service )
			return;

		self::DatabaseQuery( "UPDATE processes SET memory = ".memory_get_usage().", memory_peak = ".memory_get_peak_usage()." WHERE id = '".self::$SessionID."' LIMIT 1" );
	}
}


class ServiceClient
{
	public $socket;
	public $started;
	public $lastKeepalive;
	public $connected = TRUE;

	public function callBack( $name, $args = NULL ) {
		if( $args === NULL ) $args = array();

		return ServiceProcess::SendClientCallback( $this, $name, $args );
	}
}
