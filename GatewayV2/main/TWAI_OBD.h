#ifndef TWAI_OBD
#define TWAI_OBD

#define SINGLE_FRAME            0x08  //highest value frist byte can be for single frame
#define MULT_FRAME_FIRST        0x10  //first frame of multi CAN Frame msg
#define MULT_FRAME_CON          0x20  //first consecutive frame will be 0x21 then 0x22...
#define MULT_FRAME_FLOW         0x30  //Flow contorl frame for multi CAN Frame msg
#define SERVICE_PID_TC          0x43  //service/mode to grab TC
#define ID_DT_ECM               0x7E0 //From tester to all ECUs0x7DF
#define ID_DT_ECUs              0x7DF //From teseter to engine ECU only
#define ID_ECU                  0x7E8 //lower end of ECU responses
#define ID_ECU_Second           0x7EF //upper end of ECU responses

//request and responses to/from diagnostic tool
#define SHOW_LIVE_DATA_REQ      0x01  //service 01 - request live/current data
#define SHOW_FREEZE_FRAME_REQ   0x02  //service 02 - request freeze frame data
#define STORED_DTCS_REQ         0x03  //service 03 - request stored DTCs
#define CLEAR_DTCS_REQ          0x04  //service 04 - request to clear DTCs
#define PENDING_DTCS_REQ        0x07  //service 07 - request pending DTCs 
#define PERM_DTCS_REQ           0x0A  //service 0A - request perminate DTCs

#define SHOW_LIVE_DATA_RESP     0x41  //service 01 - response live/current data
#define SHOW_FREEZE_FRAME_RESP  0x42  //service 02 - response freeze frame data
#define CLEAR_DTCS_BAD_RESP     0x7F  //indicates negative response
#define STORED_DTCS_RESP        0x43  //service 03  - stored DTCs response mode byte
#define CLEAR_DTCS_GOOD_RESP    0x44  //service 04  - clear DTCs good response mode byte
#define PENDING_DTCS_RESP       0x47  //service 07  - pending DTCs response mode byte
#define PERM_DTCS_RESP          0x4A  //service 0A  - perminate DTCs response mode byte

#endif