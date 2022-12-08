#include "itech_dxl.h"

DynamixelShield dxl;
uint8_t user_pkt_buf[user_pkt_buf_cap];

sr_data_t sr_data[DXL_ID_CNT];
DYNAMIXEL::InfoSyncReadInst_t sr_infos;
DYNAMIXEL::XELInfoSyncRead_t info_xels_sr[DXL_ID_CNT];

sw_data_t sw_data[DXL_ID_CNT];
DYNAMIXEL::InfoSyncWriteInst_t sw_infos;
DYNAMIXEL::XELInfoSyncWrite_t info_xels_sw[DXL_ID_CNT];

bool has_new_cmd = false;

void SetupDxl(){
    dxl.begin(57600);
    dxl.setPortProtocolVersion(DYNAMIXEL_PROTOCOL_VERSION);

    dxl.torqueOff(DXL_ID_LEFT);
    dxl.setOperatingMode(DXL_ID_LEFT,OP_EXTENDED_POSITION);
    dxl.torqueOff(DXL_ID_RIGHT);
    dxl.setOperatingMode(DXL_ID_RIGHT,OP_EXTENDED_POSITION);
    dxl.torqueOn(BROADCAST_ID);

    //prepare read packet
    sr_infos.packet.p_buf = user_pkt_buf;
    sr_infos.packet.buf_capacity = user_pkt_buf_cap;
    sr_infos.packet.is_completed = false;
    sr_infos.addr = SR_START_ADDR;
    sr_infos.addr_length = SR_ADDR_LEN;
    sr_infos.p_xels = info_xels_sr;
    sr_infos.xel_count = 0;

    info_xels_sr[0].id = DXL_ID_RIGHT;
    info_xels_sr[0].p_recv_buf = (uint8_t*)&sr_data[0];
    sr_infos.xel_count++;

    info_xels_sr[1].id = DXL_ID_LEFT;
    info_xels_sr[1].p_recv_buf = (uint8_t*)&sr_data[1];
    sr_infos.xel_count++;

    sr_infos.is_info_changed = true;

    //prepare write packet
    sw_infos.packet.p_buf = nullptr;
    sw_infos.packet.is_completed = false;
    sw_infos.addr = SW_START_ADDR;
    sw_infos.addr_length = SW_ADDR_LEN;
    sw_infos.p_xels = info_xels_sw;
    sw_infos.xel_count = 0;

    info_xels_sw[0].id = DXL_ID_RIGHT;
    info_xels_sw[0].p_data = (uint8_t*)&sw_data[0].goal_position;
    sw_infos.xel_count++;

    info_xels_sw[1].id = DXL_ID_LEFT;
    info_xels_sw[1].p_data = (uint8_t*)&sw_data[1].goal_position;
    sw_infos.xel_count++;

    sw_infos.is_info_changed = true;
}

void SetNewTarget(int32_t target_left, int32_t target_right){
    sw_data[1].goal_position = target_left;
    sw_data[0].goal_position = target_right;
    sw_infos.is_info_changed = true;
    has_new_cmd = true;
}

void SetAndExecute(){
    if (has_new_cmd && sw_infos.is_info_changed)
    {
        Serial.println("New cmd arrived");
        has_new_cmd = false;
        if (dxl.syncWrite(&sw_infos)){
            Serial.println("SW success");
        }
        else{
            Serial.print("SW failed: ");
            Serial.println(dxl.getLastLibErrCode());
        }
    }
    
}