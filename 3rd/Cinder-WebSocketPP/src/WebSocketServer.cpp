/*
 * Copyright (c) 2015, Wieden+Kennedy
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or
 * without modification, are permitted provided that the following
 * conditions are met:
 *
 * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in
 * the documentation and/or other materials provided with the
 * distribution.
 *
 * Neither the name of the Ban the Rewind nor the names of its
 * contributors may be used to endorse or promote products
 * derived from this software without specific prior written
 * permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include "WebSocketServer.h"

#include "cinder/Log.h"
#include "cinder/Utilities.h"

using namespace ci;
using namespace std;

WebSocketServer::WebSocketServer()
{
	mServer.set_access_channels( websocketpp::log::alevel::all );
	mServer.clear_access_channels( websocketpp::log::alevel::frame_payload );
	
	mServer.init_asio();
	
	mServer.set_close_handler(			[&](websocketpp::connection_hdl handle) { onClose(handle); });
	mServer.set_fail_handler(			[&](websocketpp::connection_hdl handle) { onFail(handle); });
	mServer.set_http_handler(			[&](websocketpp::connection_hdl handle) { onHttp(handle); });
	mServer.set_interrupt_handler(		[&](websocketpp::connection_hdl handle) { onInterrupt(handle); });
	mServer.set_message_handler(		[&](websocketpp::connection_hdl handle, MessageRef msg) { onMessage(handle, msg); });
	mServer.set_open_handler(			[&](websocketpp::connection_hdl handle) { onOpen(handle); });
	mServer.set_ping_handler(			[&](websocketpp::connection_hdl handle, std::string msg) { return onPing(handle, msg); });
	mServer.set_socket_init_handler(	[&](websocketpp::connection_hdl handle, asio::ip::tcp::socket& socket) { onSocketInit(handle, socket); });
	mServer.set_tcp_post_init_handler(	[&](websocketpp::connection_hdl handle) { onTcpPostInit(handle); });
	mServer.set_tcp_pre_init_handler(	[&](websocketpp::connection_hdl handle) { onTcpPreInit(handle); });
	mServer.set_validate_handler(		[&](websocketpp::connection_hdl handle) { return onValidate(handle); });
}

WebSocketServer::~WebSocketServer()
{
	if ( !mServer.stopped() ) {
		cancel();
		mServer.stop();
	}
}

void WebSocketServer::cancel()
{
	try {
		mServer.stop_listening();
	} catch ( const std::exception& ex ) {
		if ( mFailEventHandler != nullptr ) {
			mFailEventHandler( ex.what() );
		}
    } catch ( websocketpp::lib::error_code err ) {
		if ( mFailEventHandler != nullptr ) {
			mFailEventHandler( err.message() );
		}
    } catch ( ... ) {
		if ( mFailEventHandler != nullptr ) {
			mFailEventHandler( "An unknown exception occurred." );
		}
    }
}

void WebSocketServer::listen( uint16_t port )
{
	try {
		mServer.listen( port );
		mServer.start_accept();
	} catch ( const std::exception& ex ) {
		if ( mFailEventHandler != nullptr ) {
			mFailEventHandler( ex.what() );
		}
    } catch ( websocketpp::lib::error_code err ) {
		if ( mFailEventHandler != nullptr ) {
			mFailEventHandler( err.message() );
		}
    } catch ( ... ) {
		if ( mFailEventHandler != nullptr ) {
			mFailEventHandler( "An unknown exception occurred." );
		}
    }
}

void WebSocketServer::ping( const string& msg )
{
	try {
		mServer.get_con_from_hdl( mHandle )->pong( msg );
	} catch( ... ) {
		if ( mFailEventHandler != nullptr ) {
			mFailEventHandler( "Ping failed." );
		}
	}
}

void WebSocketServer::poll()
{
	mServer.poll();
}

void WebSocketServer::run()
{
	mServer.run();
}

void WebSocketServer::write( void const * msg, size_t len )
{
	if (len == 0){
		if ( mFailEventHandler != nullptr ) {
			mFailEventHandler( "Cannot send empty message." );
		}
	} else {
		websocketpp::lib::error_code err;
		mServer.send(mHandle,
					 msg,
					 len,
					 websocketpp::frame::opcode::BINARY,
					 err);
		if (err) {
			if (mFailEventHandler != nullptr) {
				mFailEventHandler(err.message());
			}
		} else {
			if (mWriteEventHandler != nullptr) {
				mWriteEventHandler();
			}
		}
	}
}

void WebSocketServer::write( const std::string& msg )
{
	if ( msg.empty() ) {
		if ( mFailEventHandler != nullptr ) {
			mFailEventHandler( "Cannot send empty message." );
		}
	} else {
		websocketpp::lib::error_code err;
		mServer.send( mHandle, msg, websocketpp::frame::opcode::TEXT, err );
		if ( err ) {
			if ( mFailEventHandler != nullptr ) {
				mFailEventHandler( err.message() );
			}
		} else {
			if ( mWriteEventHandler != nullptr ) {
				mWriteEventHandler();
			}
		}
	}
}

WebSocketServer::Server& WebSocketServer::getServer()
{
	return mServer;
}

const WebSocketServer::Server& WebSocketServer::getServer() const
{
	return mServer;
}

void WebSocketServer::onClose(websocketpp::connection_hdl handle )
{
	if ( mCloseEventHandler != nullptr ) {
		mCloseEventHandler();
	}
}

void WebSocketServer::onFail( websocketpp::connection_hdl handle )
{
	mHandle = handle;
	if ( mFailEventHandler != nullptr ) {
		mFailEventHandler( "Transfer failed." );
	}
}

void WebSocketServer::onHttp( websocketpp::connection_hdl handle )
{
	mHandle = handle;
	if ( mHttpEventHandler != nullptr ) {
		mHttpEventHandler();
	}
}

void WebSocketServer::onInterrupt( websocketpp::connection_hdl handle )
{
	mHandle = handle;
	if ( mInterruptEventHandler != nullptr ) {
		mInterruptEventHandler();
	}
}

void WebSocketServer::onMessage( websocketpp::connection_hdl handle, MessageRef msg )
{
	mHandle = handle;
	if ( mMessageEventHandler != nullptr ) {
		mMessageEventHandler( msg->get_payload() );
	}
}

void WebSocketServer::onOpen( websocketpp::connection_hdl handle )
{
	mHandle = handle;
	if ( mOpenEventHandler != nullptr ) {
		mOpenEventHandler();
	}
}

bool WebSocketServer::onPing( websocketpp::connection_hdl handle, string msg )
{
	mHandle = handle;
	if ( mPingEventHandler != nullptr ) {
		mPingEventHandler( msg );
	}
	return true;
}

void WebSocketServer::onSocketInit( websocketpp::connection_hdl handle, asio::ip::tcp::socket& socket )
{
	mHandle = handle;
	mSocket = &socket;
	if ( mSocketInitEventHandler != nullptr ) {
		mSocketInitEventHandler();
	}
}

void WebSocketServer::onTcpPostInit( websocketpp::connection_hdl handle )
{
	mHandle = handle;
	if ( mTcpPostInitEventHandler != nullptr ) {
		mTcpPostInitEventHandler();
	}
}

void WebSocketServer::onTcpPreInit( websocketpp::connection_hdl handle )
{
	mHandle = handle;
	if ( mTcpPreInitEventHandler != nullptr ) {
		mTcpPreInitEventHandler();
	}
}

bool WebSocketServer::onValidate( websocketpp::connection_hdl handle )
{
	mHandle = handle;
	if ( mValidateEventHandler != nullptr ) {
		mValidateEventHandler();
	}
	return true;
}
