
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "networkPriv.h"

#include "connection.h"
#include "connectionDescription.h"
#include "global.h"
#include "network.h"
#include "nodePriv.h"
#include "packet.h"
#include "pipeNetwork.h"
#include "sessionPriv.h"
#include "socketNetwork.h"

#include <eq/base/log.h>
#include <alloca.h>

using namespace eqNet::priv;
using namespace std;

Network::Network( const uint id, Session* session )
        : eqNet::Network(id),
          _session(session),
          _state(STATE_STOPPED)
{}

Network::~Network()
{
    // TODO: ConnectionDescription cleanup
}

Network* Network::create( const uint id, Session* session, 
    const eqNet::NetworkProtocol protocol )
{
    Network* network;
    switch( protocol )
    {
        case eqNet::PROTO_TCPIP:
            network = new SocketNetwork( id, session );

        case eqNet::PROTO_PIPE:
            network = new PipeNetwork( id, session );

        default:
            WARN << "Protocol not implemented" << endl;
            return NULL;
    }
    network->_protocol = protocol;
    return network;
}

void Network::addNode( const uint nodeID,
    const eqNet::ConnectionDescription& description )
{
    ConnectionDescription *desc = new ConnectionDescription();
    *desc = description;

    if( typeid(this) == typeid( PipeNetwork* ))
    {
        if( description.parameters.PIPE.entryFunc != NULL)
            desc->parameters.PIPE.entryFunc =
                strdup( description.parameters.PIPE.entryFunc );
    }
   
    _descriptions[nodeID] = desc;
    _nodeStates[nodeID]   = NODE_STOPPED;
}

void Network::setStarted( const uint nodeID )
{
    ASSERT( _descriptions.count(nodeID)!=0 );

    _nodeStates[nodeID]  = NODE_RUNNING;
}

void Network::setStarted( const uint nodeID, Connection* connection )
{
    setStarted( _session->getNodeByID( nodeID ), connection );
}

void Network::setStarted( Node* node, Connection* connection )
{
    const uint nodeID = node->getID();
    ASSERT( _descriptions.count(nodeID)!=0 );
    ASSERT( connection->getState() == Connection::STATE_CONNECTED );

    _connectionSet.addConnection( connection, this, node );
    _nodeStates[nodeID]  = NODE_RUNNING;
}

const char* Network::_createLaunchCommand( const uint nodeID, 
                                           const char* args )
{
    IDHash<ConnectionDescription*>::iterator iter = _descriptions.find(nodeID);

    if( iter == _descriptions.end() )
        return NULL;

    ConnectionDescription* description = (*iter).second;
    const char*          launchCommand = description->launchCommand;

    if( !launchCommand )
        return NULL;

    const size_t      launchCommandLen = strlen( launchCommand );
    size_t                  resultSize = 256;
    char*                       result = (char*)alloca(resultSize);
    size_t                 resultIndex = 0;
    bool                  commandFound = false;

    for( size_t i=0; i<launchCommandLen-1; i++ )
    {
        if( launchCommand[i] == '%' )
        {
             char* replacement = NULL;
            switch( launchCommand[i+1] )
            {
                case 'c':
                {
                    const char* programName = Global::getProgramName();
                    replacement  = (char*)alloca( strlen(programName) +
                                                  strlen( args ) + 2 );
                    sprintf( replacement, "%s %s", programName, args );
                    commandFound = true;
                } break;
            }

            if( replacement )
            {
                // check string length
                const size_t replacementLen = strlen( replacement );
                size_t       newSize        = resultSize;
                
                while( newSize <= resultIndex + replacementLen )
                    newSize = newSize << 1;
                if( newSize > resultSize )
                {
                    char* newResult = (char*)alloca(newSize);
                    memcpy( newResult, result, resultSize );
                    result     = newResult;
                    resultSize = newSize;
                }

                // replace
                memcpy( &result[resultIndex], replacement, replacementLen );
                resultIndex += replacementLen;
                i++;
            }
        }
        else
        {
            result[resultIndex++] = launchCommand[i];
            if( resultIndex == resultSize ) // check string length
            {
                resultSize      = resultSize << 1;
                char* newResult = (char*)alloca(resultSize);
                memcpy( newResult, result, resultSize >> 1 );
                result = newResult;
            }
        }
    }

    if( !commandFound )
    { 
        // check string length
        const char* programName = Global::getProgramName();
        char*       command     = (char*)alloca( strlen(programName) +
                                                 strlen( args ) + 2 );
        sprintf( command, "%s %s", programName, args );
        const size_t commandLen = strlen( command );
        size_t       newSize    = resultSize;
                
        while( newSize <= resultIndex + commandLen )
            newSize = newSize << 1;

        if( newSize > resultSize )
        {
            char* newResult = (char*)alloca(newSize);
            memcpy( newResult, result, resultSize );
            result     = newResult;
            resultSize = newSize;
        }

        // append command
        memcpy( &result[resultIndex], command, commandLen );
        resultIndex += commandLen;
   }

    result[resultIndex] = '\0';
    INFO << "Launch command: " << result << endl;
    return strdup(result);
}

void Network::send( Node* toNode, const Packet& packet )
{
    ASSERT( _nodeStates[toNode->getID()] == NODE_RUNNING );
    
    Connection* connection = _connectionSet.getConnection( toNode );
    
    if( !connection )
    {
        connection = Connection::create( _protocol );
        ConnectionDescription* desc = _descriptions[toNode->getID()];
        ASSERT( desc );

        const bool connected = connection->connect( *desc );
        if( !connected )
        {
            ERROR << "Can't connect to node " << toNode << endl;
            delete connection;
            return;
        }
        _connectionSet.addConnection( connection, this, toNode );
    }

    connection->send( &packet, packet.size );
}
