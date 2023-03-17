#include <ucanopen_c28x/tests/tests.h>
#include <settings/settings.h>


namespace ucanopen {

namespace tests {

Object* Server::_object;
unsigned char object_alloc[sizeof(Object)] __attribute__((section("shared_ucanopen_test_object")));


namespace od {

SdoAbortCode get_device_name(ExpeditedSdoData& retval) {
    const size_t len = strlen(sysinfo::device_name) + 1;
    const size_t word_count = (len + 3) / 4;
    static size_t counter = 0;

    char word[4] = {0};
    strncpy(word, sysinfo::device_name + 4*counter, 4);
    emb::c28x::from_bytes<uint32_t>(retval.u32, reinterpret_cast<uint8_t*>(word));

    counter = (counter + 1) % word_count;

    return SdoAbortCode::no_error;
}


SdoAbortCode get_hardware_version(ExpeditedSdoData& retval) {
    const size_t len = strlen(sysinfo::hardware_version) + 1;
    const size_t word_count = (len + 3) / 4;
    static size_t counter = 0;

    char word[4] = {0};
    strncpy(word, sysinfo::hardware_version + 4*counter, 4);
    emb::c28x::from_bytes<uint32_t>(retval.u32, reinterpret_cast<uint8_t*>(word));

    counter = (counter + 1) % word_count;

    return SdoAbortCode::no_error;
}


SdoAbortCode get_firmware_version(ExpeditedSdoData& retval) {
    const size_t len = strlen(sysinfo::firmware_version) + 1;
    const size_t word_count = (len + 3) / 4;
    static size_t counter = 0;

    char word[4] = {0};
    strncpy(word, sysinfo::firmware_version + 4*counter, 4);
    emb::c28x::from_bytes<uint32_t>(retval.u32, reinterpret_cast<uint8_t*>(word));

    counter = (counter + 1) % word_count;

    return SdoAbortCode::no_error;
}


SdoAbortCode save_all_parameters(ExpeditedSdoData val) {
    return SdoAbortCode::no_error;
}


SdoAbortCode restore_all_default_parameters(ExpeditedSdoData val) {
    return SdoAbortCode::no_error;
}


SdoAbortCode get_serial_number(ExpeditedSdoData& retval) {
    uint32_t* uid_ptr = reinterpret_cast<uint32_t*>(0x000703CC);
    retval.u32 = *uid_ptr;
    return SdoAbortCode::no_error;
}


inline SdoAbortCode reset_device(ExpeditedSdoData val) {
    syslog::add_message(sys::Message::device_resetting);
    mcu::chrono::system_clock::register_delayed_task(mcu::reset_device, emb::chrono::milliseconds(2000));
    return SdoAbortCode::no_error;
}


inline SdoAbortCode reset_errors(ExpeditedSdoData val) {
    syslog::reset_errors_warnings();
    return SdoAbortCode::no_error;
}


inline SdoAbortCode get_syslog_message(ExpeditedSdoData& retval) {
    retval.u32 = syslog::read_message().underlying_value();
    syslog::pop_message();
    return SdoAbortCode::no_error;
}

inline SdoAbortCode get_uptime(ExpeditedSdoData& retval) {
    retval.f32 = mcu::chrono::system_clock::now().count() / 1000.f;
    return SdoAbortCode::no_error;
}

} // namespace od


ODEntry object_dictionary[] = {
{{0x1008, 0x00}, {"sys", "info", "device_name", "", OD_STRING, OD_ACCESS_CONST, OD_NO_DIRECT_ACCESS, od::get_device_name, OD_NO_INDIRECT_WRITE_ACCESS}},
{{0x1009, 0x00}, {"sys", "info", "hardware_version", "", OD_STRING, OD_ACCESS_CONST, OD_NO_DIRECT_ACCESS, od::get_hardware_version, OD_NO_INDIRECT_WRITE_ACCESS}},
{{0x100A, 0x00}, {"sys", "info", "firmware_version", "", OD_STRING, OD_ACCESS_CONST, OD_NO_DIRECT_ACCESS, od::get_firmware_version, OD_NO_INDIRECT_WRITE_ACCESS}},

{{0x1010, 0x01}, {"sys", "ctl", "save_all_parameters", "", OD_EXEC, OD_ACCESS_WO, OD_NO_DIRECT_ACCESS, OD_NO_INDIRECT_READ_ACCESS, od::save_all_parameters}},
{{0x1011, 0x01}, {"sys", "ctl", "restore_all_default_parameters", "", OD_EXEC, OD_ACCESS_WO, OD_NO_DIRECT_ACCESS, OD_NO_INDIRECT_READ_ACCESS, od::restore_all_default_parameters}},

{{0x1018, 0x04}, {"sys", "info", "serial_number", "", OD_UINT32, OD_ACCESS_CONST, OD_NO_DIRECT_ACCESS, od::get_serial_number, OD_NO_INDIRECT_WRITE_ACCESS}},

{{0x2000, 0x01}, {"sys", "ctl", "reset_device", "", OD_EXEC, OD_ACCESS_WO, OD_NO_DIRECT_ACCESS, OD_NO_INDIRECT_READ_ACCESS, od::reset_device}},
{{0x2000, 0x02}, {"sys", "ctl", "reset_errors", "", OD_EXEC, OD_ACCESS_WO, OD_NO_DIRECT_ACCESS, OD_NO_INDIRECT_READ_ACCESS, od::reset_errors}},

{{0x5000, 0x01}, {"watch", "watch", "uptime", "s", OD_FLOAT32, OD_ACCESS_RO, OD_NO_DIRECT_ACCESS, od::get_uptime, OD_NO_INDIRECT_WRITE_ACCESS}},
{{0x5000, 0x02}, {"watch", "watch", "syslog_message", "", OD_UINT32, OD_ACCESS_RO, OD_NO_DIRECT_ACCESS, od::get_syslog_message, OD_NO_INDIRECT_WRITE_ACCESS}},

{{0x3000, 0x01}, {"config", "ucanopen", "node_id", "", OD_UINT32, OD_ACCESS_RW, OD_PTR(&settings::configs.ucanopen_server.node_id), OD_NO_INDIRECT_READ_ACCESS, OD_NO_INDIRECT_WRITE_ACCESS}},
{{0x3000, 0x02}, {"config", "ucanopen", "heartbeat_period", "ms", OD_UINT32, OD_ACCESS_RW, OD_PTR(&settings::configs.ucanopen_server.heartbeat_period_ms), OD_NO_INDIRECT_READ_ACCESS, OD_NO_INDIRECT_WRITE_ACCESS}},
{{0x3000, 0x03}, {"config", "ucanopen", "tpdo1_period", "ms", OD_UINT32, OD_ACCESS_RW, OD_PTR(&settings::configs.ucanopen_server.tpdo1_period_ms), OD_NO_INDIRECT_READ_ACCESS, OD_NO_INDIRECT_WRITE_ACCESS}},
{{0x3000, 0x04}, {"config", "ucanopen", "tpdo2_period", "ms", OD_UINT32, OD_ACCESS_RW, OD_PTR(&settings::configs.ucanopen_server.tpdo2_period_ms), OD_NO_INDIRECT_READ_ACCESS, OD_NO_INDIRECT_WRITE_ACCESS}},
{{0x3000, 0x05}, {"config", "ucanopen", "tpdo3_period", "ms", OD_UINT32, OD_ACCESS_RW, OD_PTR(&settings::configs.ucanopen_server.tpdo3_period_ms), OD_NO_INDIRECT_READ_ACCESS, OD_NO_INDIRECT_WRITE_ACCESS}},
{{0x3000, 0x06}, {"config", "ucanopen", "tpdo4_period", "ms", OD_UINT32, OD_ACCESS_RW, OD_PTR(&settings::configs.ucanopen_server.tpdo4_period_ms), OD_NO_INDIRECT_READ_ACCESS, OD_NO_INDIRECT_WRITE_ACCESS}},
{{0x3000, 0x07}, {"config", "ucanopen", "rpdo1_timeout", "ms", OD_UINT32, OD_ACCESS_RW, OD_PTR(&settings::configs.ucanopen_server.rpdo1_timeout_ms), OD_NO_INDIRECT_READ_ACCESS, OD_NO_INDIRECT_WRITE_ACCESS}},
{{0x3000, 0x08}, {"config", "ucanopen", "rpdo2_timeout", "ms", OD_UINT32, OD_ACCESS_RW, OD_PTR(&settings::configs.ucanopen_server.rpdo2_timeout_ms), OD_NO_INDIRECT_READ_ACCESS, OD_NO_INDIRECT_WRITE_ACCESS}},
{{0x3000, 0x09}, {"config", "ucanopen", "rpdo3_timeout", "ms", OD_UINT32, OD_ACCESS_RW, OD_PTR(&settings::configs.ucanopen_server.rpdo3_timeout_ms), OD_NO_INDIRECT_READ_ACCESS, OD_NO_INDIRECT_WRITE_ACCESS}},
{{0x3000, 0x0A}, {"config", "ucanopen", "rpdo4_timeout", "ms", OD_UINT32, OD_ACCESS_RW, OD_PTR(&settings::configs.ucanopen_server.rpdo4_timeout_ms), OD_NO_INDIRECT_READ_ACCESS, OD_NO_INDIRECT_WRITE_ACCESS}},
{{0x3000, 0x0B}, {"config", "ucanopen", "rpdo1_id", "", OD_UINT32, OD_ACCESS_RW, OD_PTR(&settings::configs.ucanopen_server.rpdo1_id), OD_NO_INDIRECT_READ_ACCESS, OD_NO_INDIRECT_WRITE_ACCESS}},
{{0x3000, 0x0C}, {"config", "ucanopen", "rpdo2_id", "", OD_UINT32, OD_ACCESS_RW, OD_PTR(&settings::configs.ucanopen_server.rpdo2_id), OD_NO_INDIRECT_READ_ACCESS, OD_NO_INDIRECT_WRITE_ACCESS}},
{{0x3000, 0x0D}, {"config", "ucanopen", "rpdo3_id", "", OD_UINT32, OD_ACCESS_RW, OD_PTR(&settings::configs.ucanopen_server.rpdo3_id), OD_NO_INDIRECT_READ_ACCESS, OD_NO_INDIRECT_WRITE_ACCESS}},
{{0x3000, 0x0E}, {"config", "ucanopen", "rpdo4_id", "", OD_UINT32, OD_ACCESS_RW, OD_PTR(&settings::configs.ucanopen_server.rpdo4_id), OD_NO_INDIRECT_READ_ACCESS, OD_NO_INDIRECT_WRITE_ACCESS}},


};


const size_t object_dictionary_size = sizeof(object_dictionary) / sizeof(object_dictionary[0]);

} // namespace tests

} // namespace ucanopen


