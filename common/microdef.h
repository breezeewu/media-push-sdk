/***************************************************************************************************************
 * filename     microdef.h
 * describe     Sunvalley cloud storage push sdk config define file
 * author       Created by dawson on 2019/04/25
 * Copyright    ©2007 - 2029 Sunvally. All Rights Reserved.
 **************************************************************************************************************/
#ifndef _MICRO_DEF_H_
#define _MICRO_DEF_H_
// support PRId64
#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif
#define SEND_FRAME_BUFFER_WITH_BUFFER_QUEUE
#ifndef _WIN32
    #define SOCKET_VALID(x) (x > 0)
#else
    #define SOCKET_VALID(x) (x!=INVALID_SOCKET)
#endif
#define MUXER_RTMP_PACKET_DIRECTORY
//#define ENABLE_MEMORY_DEBUG
//#define LARGE_MEMORY_MANAGER
#ifdef LARGE_MEMORY_MANAGER
#include "memorymgr.h"
#define sv_malloc(owner, size)     (char*)CPoolMgr::get_inst()->mem_malloc(owner, (int64_t)size)
#define sv_free(owner, pmem)       CPoolMgr::get_inst()->mem_free(owner, (void**)&pmem)
#define sv_pool_mgr_init(total_size)   CPoolMgr::get_inst()->init(total_size)
#define sv_pool_mgr_deinit()            CPoolMgr::get_inst()->destroy_inst();
#define sv_init_pool(owner, max_size, max_blk)     CPoolMgr::get_inst()->get_memory_pool(owner, NULL, max_size, max_blk)
#define sv_deinit_pool(owner)     CPoolMgr::get_inst()->free_memory_pool(owner)
#define sv_get_free_memory(owner)       CPoolMgr::get_inst()->get_free_memory(owner)
#define SV_MIN_FREE_BUFFER_SIZE         150 * 1024
#else
#define sv_malloc(owner, size) new char[size];
#define sv_free(owner, pmem)		if(pmem) { delete[] (char*)pmem; pmem = NULL;}
#define sv_pool_mgr_init(total_size)
#define sv_pool_mgr_deinit()
#define sv_init_pool(owner, max_size, max_blk)
#define sv_deinit_pool(owner)
#define sv_get_free_memory(owner)       1024*1024
#define SRS_MIN_FREE_BUFFER_SIZE         150 * 1024
#endif
// write media data micro define
//#define USE_FWRITE_MEADIA_LOG
//#define USE_FWRITE_PRITNF_LOG
//#define USE_FWRITE_IOV_LOG
#define ENABLE_ENCRYPTION
//#define DISABLE_RTMP_DATA_ENCRYPTION


#define MIN_FRAME_BUFFER_SIZE   64 * 1024

// rtmp output chunk size
#define DEAULT_OUTPUT_CHUNK_SIZE 60000
// default vhost of rtmp
//#define SV_CONSTS_RTMP_DEFAULT_VHOST "__defaultVhost__"
// default port of rtmp
//#define SV_CONSTS_RTMP_DEFAULT_PORT "1935"
#define INVALID_SOCKET -1
// utility
//#define CheckPtr(p, ret) if(NULL == p) { return ret;}
//#define CheckResult(ret) if(ret != 0) { sv_error("checkreulst failed, ret:%d", ret); return ret;}
//#define CheckFailed(ret) ret != 0 ? 1:0


#define MAX_RTMP_CONNECTION_MEMORY_SIZE      1024*1024
#define MAX_RTMP_CONNECTION_BLOCK_SIZE    1024

#define MAX_DURATION_OF_TIMESTAMP  150
#define svautofree(className, instance) \
impl__SVAutoFree<className> _auto_free_##instance(&instance, false)
//#define svautofreea(className, instance) impl__SVAutoFree<className> _auto_free_array_##instance(&instance, true)
template<class T>
class impl__SVAutoFree
{
private:
    T** ptr;
    bool is_array;
public:
    /**
     * auto delete the ptr.
     */
    impl__SVAutoFree(T** p, bool array) {
        ptr = p;
        is_array = array;
    }
    
    virtual ~impl__SVAutoFree() {
        if (ptr == NULL || *ptr == NULL) {
            return;
        }
        
        if (is_array) {
            //sv_free(*ptr);
            assert(0);
            //delete[] *ptr;
        } else {
            //printf("delete ptr:%p\n", ptr);
            delete *ptr;
        }
        
        *ptr = NULL;
    }
};

#define SYSTEM_ERROR errno < 0 ? errno : (-errno)
#define svfreep(p)  if(p) { delete p; p = NULL;}
//#define svfreepa(p)  if(p) { delete[] p; p = NULL;}
#define svmax(a, b) a > b ? a : b
#define svmin(a, b) a > b ? b : a


/**
 5. Protocol Control Messages
 RTMP reserves message type IDs 1-7 for protocol control messages.
 These messages contain information needed by the RTM Chunk Stream
 protocol or RTMP itself. Protocol messages with IDs 1 & 2 are
 reserved for usage with RTM Chunk Stream protocol. Protocol messages
 with IDs 3-6 are reserved for usage of RTMP. Protocol message with ID
 7 is used between edge server and origin server.
 */
#define RTMP_MSG_SetChunkSize                   0x01
#define RTMP_MSG_AbortMessage                   0x02
#define RTMP_MSG_Acknowledgement                0x03
#define RTMP_MSG_UserControlMessage             0x04
#define RTMP_MSG_WindowAcknowledgementSize      0x05
#define RTMP_MSG_SetPeerBandwidth               0x06
#define RTMP_MSG_EdgeAndOriginServerCommand     0x07
/**
 3. Types of messages
 The server and the client send messages over the network to
 communicate with each other. The messages can be of any type which
 includes audio messages, video messages, command messages, shared
 object messages, data messages, and user control messages.
 3.1. Command message
 Command messages carry the AMF-encoded commands between the client
 and the server. These messages have been assigned message type value
 of 20 for AMF0 encoding and message type value of 17 for AMF3
 encoding. These messages are sent to perform some operations like
 connect, createStream, publish, play, pause on the peer. Command
 messages like onstatus, result etc. are used to inform the sender
 about the status of the requested commands. A command message
 consists of command name, transaction ID, and command object that
 contains related parameters. A client or a server can request Remote
 Procedure Calls (RPC) over streams that are communicated using the
 command messages to the peer.
 */
#define RTMP_MSG_AMF3CommandMessage             17 // 0x11
#define RTMP_MSG_AMF0CommandMessage             20 // 0x14
/**
 3.2. Data message
 The client or the server sends this message to send Metadata or any
 user data to the peer. Metadata includes details about the
 data(audio, video etc.) like creation time, duration, theme and so
 on. These messages have been assigned message type value of 18 for
 AMF0 and message type value of 15 for AMF3.
 */
#define RTMP_MSG_AMF0DataMessage                18 // 0x12
#define RTMP_MSG_AMF3DataMessage                15 // 0x0F
/**
 3.3. Shared object message
 A shared object is a Flash object (a collection of name value pairs)
 that are in synchronization across multiple clients, instances, and
 so on. The message types kMsgContainer=19 for AMF0 and
 kMsgContainerEx=16 for AMF3 are reserved for shared object events.
 Each message can contain multiple events.
 */
#define RTMP_MSG_AMF3SharedObject               16 // 0x10
#define RTMP_MSG_AMF0SharedObject               19 // 0x13
/**
 3.4. Audio message
 The client or the server sends this message to send audio data to the
 peer. The message type value of 8 is reserved for audio messages.
 */
#define RTMP_MSG_AudioMessage                   8 // 0x08
/* *
 3.5. Video message
 The client or the server sends this message to send video data to the
 peer. The message type value of 9 is reserved for video messages.
 These messages are large and can delay the sending of other type of
 messages. To avoid such a situation, the video message is assigned
 the lowest priority.
 */
#define RTMP_MSG_VideoMessage                   9 // 0x09
/**
 3.6. Aggregate message
 An aggregate message is a single message that contains a list of submessages.
 The message type value of 22 is reserved for aggregate
 messages.
 */
#define RTMP_MSG_AggregateMessage               22 // 0x16

/****************************************************************************
 *****************************************************************************
 ****************************************************************************/
/**
 * the chunk stream id used for some under-layer message,
 * for example, the PC(protocol control) message.
 */
#define RTMP_CID_ProtocolControl                0x02
/**
 * the AMF0/AMF3 command message, invoke method and return the result, over NetConnection.
 * generally use 0x03.
 */
#define RTMP_CID_OverConnection                 0x03
/**
 * the AMF0/AMF3 command message, invoke method and return the result, over NetConnection,
 * the midst state(we guess).
 * rarely used, e.g. onStatus(NetStream.Play.Reset).
 */
#define RTMP_CID_OverConnection2                0x04
/**
 * the stream message(amf0/amf3), over NetStream.
 * generally use 0x05.
 */
#define RTMP_CID_OverStream                     0x05
/**
 * the stream message(amf0/amf3), over NetStream, the midst state(we guess).
 * rarely used, e.g. play("mp4:mystram.f4v")
 */
#define RTMP_CID_OverStream2                    0x08
/**
 * the stream message(video), over NetStream
 * generally use 0x06.
 */
#define RTMP_CID_Video                          0x06
/**
 * the stream message(audio), over NetStream.
 * generally use 0x07.
 */
#define RTMP_CID_Audio                          0x07

/**
 * 6.1. Chunk Format
 * Extended timestamp: 0 or 4 bytes
 * This field MUST be sent when the normal timsestamp is set to
 * 0xffffff, it MUST NOT be sent if the normal timestamp is set to
 * anything else. So for values less than 0xffffff the normal
 * timestamp field SHOULD be used in which case the extended timestamp
 * MUST NOT be present. For values greater than or equal to 0xffffff
 * the normal timestamp field MUST NOT be used and MUST be set to
 * 0xffffff and the extended timestamp MUST be sent.
 */
#define RTMP_EXTENDED_TIMESTAMP                 0xFFFFFF

/**
 * amf0 command message, command name macros
 */
#define RTMP_AMF0_COMMAND_CONNECT               "connect"
#define RTMP_AMF0_COMMAND_CREATE_STREAM         "createStream"
#define RTMP_AMF0_COMMAND_CLOSE_STREAM          "closeStream"
#define RTMP_AMF0_COMMAND_PLAY                  "play"
#define RTMP_AMF0_COMMAND_PAUSE                 "pause"
#define RTMP_AMF0_COMMAND_ON_BW_DONE            "onBWDone"
#define RTMP_AMF0_COMMAND_ON_STATUS             "onStatus"
#define RTMP_AMF0_COMMAND_RESULT                "_result"
#define RTMP_AMF0_COMMAND_ERROR                 "_error"
#define RTMP_AMF0_COMMAND_RELEASE_STREAM        "releaseStream"
#define RTMP_AMF0_COMMAND_FC_PUBLISH            "FCPublish"
#define RTMP_AMF0_COMMAND_UNPUBLISH             "FCUnpublish"
#define RTMP_AMF0_COMMAND_PUBLISH               "publish"
#define RTMP_AMF0_DATA_SAMPLE_ACCESS            "|RtmpSampleAccess"

/**
 * the signature for packets to client.
 */
#define RTMP_SIG_FMS_VER                        "3,5,3,888"
#define RTMP_SIG_AMF0_VER                       0
#define RTMP_SIG_CLIENT_ID                      "ASAICiss"

/**
 * onStatus consts.
 */
#define StatusLevel                             "level"
#define StatusCode                              "code"
#define StatusDescription                       "description"
#define StatusDetails                           "details"
#define StatusClientId                          "clientid"
// status value
#define StatusLevelStatus                       "status"
// status error
#define StatusLevelError                        "error"
// code value
#define StatusCodeConnectSuccess                "NetConnection.Connect.Success"
#define StatusCodeConnectRejected               "NetConnection.Connect.Rejected"
#define StatusCodeStreamReset                   "NetStream.Play.Reset"
#define StatusCodeStreamStart                   "NetStream.Play.Start"
#define StatusCodeStreamPause                   "NetStream.Pause.Notify"
#define StatusCodeStreamUnpause                 "NetStream.Unpause.Notify"
#define StatusCodePublishStart                  "NetStream.Publish.Start"
#define StatusCodeDataStart                     "NetStream.Data.Start"
#define StatusCodeUnpublishSuccess              "NetStream.Unpublish.Success"


// AMF0 marker
#define RTMP_AMF0_Number                     0x00
#define RTMP_AMF0_Boolean                     0x01
#define RTMP_AMF0_String                     0x02
#define RTMP_AMF0_Object                     0x03
#define RTMP_AMF0_MovieClip                 0x04 // reserved, not supported
#define RTMP_AMF0_Null                      0x05
#define RTMP_AMF0_Undefined                 0x06
#define RTMP_AMF0_Reference                 0x07
#define RTMP_AMF0_EcmaArray                 0x08
#define RTMP_AMF0_ObjectEnd                 0x09
#define RTMP_AMF0_StrictArray                 0x0A
#define RTMP_AMF0_Date                         0x0B
#define RTMP_AMF0_LongString                 0x0C
#define RTMP_AMF0_UnSupported                 0x0D
#define RTMP_AMF0_RecordSet                 0x0E // reserved, not supported
#define RTMP_AMF0_XmlDocument                 0x0F
#define RTMP_AMF0_TypedObject                 0x10
// AVM+ object is the AMF3 object.
#define RTMP_AMF0_AVMplusObject             0x11
// origin array whos data takes the same form as LengthValueBytes
#define RTMP_AMF0_OriginStrictArray         0x20

// User defined
#define RTMP_AMF0_Invalid                     0x3F


/**
 * E.4.1 FLV Tag, page 75
 */
// 8 = audio
#define SRS_RTMP_TYPE_AUDIO 8
// 9 = video
#define SRS_RTMP_TYPE_VIDEO 9
// 18 = script data
#define SRS_RTMP_TYPE_SCRIPT 18

// the default recv buffer size, 128KB. change to 10KB
//#define SRS_DEFAULT_RECV_BUFFER_SIZE 131072
#define SRS_DEFAULT_RECV_BUFFER_SIZE 128

// limit user-space buffer to 256KB, for 3Mbps stream delivery.
//      800*2000/8=200000B(about 195KB).
// @remark it's ok for higher stream, the buffer is ok for one chunk is 256KB.
#define SRS_MAX_SOCKET_BUFFER 262144

// the max header size,
// @see SVProtocol::read_message_header().
#define SRS_RTMP_MAX_MESSAGE_HEADER 11

/**
 * 6.1.2. Chunk Message Header
 * There are four different formats for the chunk message header,
 * selected by the "fmt" field in the chunk basic header.
 */
// 6.1.2.1. Type 0
// Chunks of Type 0 are 11 bytes long. This type MUST be used at the
// start of a chunk stream, and whenever the stream timestamp goes
// backward (e.g., because of a backward seek).
#define RTMP_FMT_TYPE0                          0
// 6.1.2.2. Type 1
// Chunks of Type 1 are 7 bytes long. The message stream ID is not
// included; this chunk takes the same stream ID as the preceding chunk.
// Streams with variable-sized messages (for example, many video
// formats) SHOULD use this format for the first chunk of each new
// message after the first.
#define RTMP_FMT_TYPE1                          1
// 6.1.2.3. Type 2
// Chunks of Type 2 are 3 bytes long. Neither the stream ID nor the
// message length is included; this chunk has the same stream ID and
// message length as the preceding chunk. Streams with constant-sized
// messages (for example, some audio and data formats) SHOULD use this
// format for the first chunk of each message after the first.
#define RTMP_FMT_TYPE2                          2
// 6.1.2.4. Type 3
// Chunks of Type 3 have no header. Stream ID, message length and
// timestamp delta are not present; chunks of this type take values from
// the preceding chunk. When a single message is split into chunks, all
// chunks of a message except the first one, SHOULD use this type. Refer
// to example 2 in section 6.2.2. Stream consisting of messages of
// exactly the same size, stream ID and spacing in time SHOULD use this
// type for all chunks after chunk of Type 2. Refer to example 1 in
// section 6.2.1. If the delta between the first message and the second
// message is same as the time stamp of first message, then chunk of
// type 3 would immediately follow the chunk of type 0 as there is no
// need for a chunk of type 2 to register the delta. If Type 3 chunk
// follows a Type 0 chunk, then timestamp delta for this Type 3 chunk is
// the same as the timestamp of Type 0 chunk.
#define RTMP_FMT_TYPE3                          3

#define SRS_CONSTS_RTMP_MAX_FMT0_HEADER_SIZE 16
/**
 * max rtmp header size:
 *     1bytes basic header,
 *     4bytes timestamp header,
 * that is, 1+4=5bytes.
 */
// always use fmt0 as cache.
#define SRS_CONSTS_RTMP_MAX_FMT3_HEADER_SIZE 5

// server play control
#define SRS_BW_CHECK_START_PLAY                 "onSVBandCheckStartPlayBytes"
#define SRS_BW_CHECK_STARTING_PLAY              "onSVBandCheckStartingPlayBytes"
#define SRS_BW_CHECK_STOP_PLAY                  "onSVBandCheckStopPlayBytes"
#define SRS_BW_CHECK_STOPPED_PLAY               "onSVBandCheckStoppedPlayBytes"

// server publish control
#define SRS_BW_CHECK_START_PUBLISH              "onSVBandCheckStartPublishBytes"
#define SRS_BW_CHECK_STARTING_PUBLISH           "onSVBandCheckStartingPublishBytes"
#define SRS_BW_CHECK_STOP_PUBLISH               "onSVBandCheckStopPublishBytes"
// @remark, flash never send out this packet, for its queue is full.
#define SRS_BW_CHECK_STOPPED_PUBLISH            "onSVBandCheckStoppedPublishBytes"

// EOF control.
// the report packet when check finished.
#define SRS_BW_CHECK_FINISHED                   "onSVBandCheckFinished"
// @remark, flash never send out this packet, for its queue is full.
#define SRS_BW_CHECK_FINAL                      "finalClientPacket"

// data packets
#define SRS_BW_CHECK_PLAYING                    "onSVBandCheckPlaying"
#define SRS_BW_CHECK_PUBLISHING                 "onSVBandCheckPublishing"
#define SUN_DEV
#ifdef SUN_DEV
//#define GET_TEST_TOKEN_SERVER_URL               "http://52.53.92.190:5678/token/get"
#define GET_TEST_TOKEN_SERVER_URL               "http://54.177.131.71:5678/token/get"
#else
#define GET_TEST_TOKEN_SERVER_URL               "http://10.30.0.112:5678/token/get"
#endif
//#define GET_TOKEN_SERVER_URL                    "http://52.53.92.190:5678/token/get"
#endif
