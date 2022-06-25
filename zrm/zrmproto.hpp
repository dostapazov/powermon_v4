/* ZRM exchange protocol
 * Ostapenko D.V. NIKTES 2019-03-11
 */

#ifndef PROTODEF_H
#define PROTODEF_H

#include <stdint.h>
#include <dev_proto.hpp>
#include <vector>
#include <map>
#include <mutex>
#include <math.h>
#include <string.h>

namespace zrm {

#pragma pack(push,1)

enum zrm_work_mode_t {  as_power, as_charger};

enum   sync_types_t
{
    PS_PC = 0xa5  // pc -> cu
    , PS_CU = 0x5a // cu -> pc
};

enum param_write_mode_t
{
    WM_NONE                   = 0,
    WM_CHECK_FOR_MODIFICATION = 1,
    WM_PROCESS                = 2,
    WM_PROCESS_AND_WRITE      = 3
};

// Session type
enum session_types_t
{
    ST_FINISH    = 0x00,
    ST_CONTROL   = 0x01,
    ST_READONLY  = 0x02
};


enum zrm_param_t
{
    PARAM_CHANNEL_WORKMODE = -1 // Изменен режим работы канала
    , PARAM_CON        = 0       // Данные подключения 0
    , PARAM_STATE      = 1       // Состояние 1
    , PARAM_WTIME      = 2       // Время работы 2
    , PARAM_LTIME      = 3       // Ограничение времени 3
    , PARAM_CUR        = 4       // Ток 4
    , PARAM_LCUR       = 5       // Ограничение тока 5
    , PARAM_VOLT       = 6       // Напряжение 6
    , PARAM_LVOLT      = 7       // Оганичение напряжения 7
    , PARAM_CAP        = 8       // Ёмкость 8
    , PARAM_TEMP       = 9       // Температура АКБ 9
    , PARAM_STG_NUM    = 10      // Номер этапа 10
    , PARAM_LOOP_NUM   = 11      // Номер цикла 11
    , PARAM_ERROR_STATE = 12     // Код ошибки состояния ЗРМ 12
    , PARAM_FAULTL_DEV = 13      // Номер неисправного ЗРМ 13
    , PARAM_MID        = 14      // ID выполняемого метода 14
    , PARAM_TRECT      = 15      // Температура
    , PARAM_TCONV      = 16      // Температура преобразователя
    , PARAM_VOUT       = 17      // Напряжение на выходе ЗРМ    // Напряжение на электролитах
    , PARAM_DOUT       = 18      // Дискретные выходы
    , PARAM_DIN        = 19      // Дискретные входы
    , PARAM_CCNT       = 20      // Количество элементов МАС АКБ

    , PARAM_ZRMMODE    = 21      // Режим работы (состояние) ЗРМ
    , PARAM_ADDR       = 22      // Адрес ЗРМ

    , PARAM_CALIB_VOLT = 23
    , PARAM_CALIB_CURR = 24
    , PARAM_FAN_PERCENT = 25

    , PARAM_TCP_SETTINGS = 47   // Смена настроек соединения
    , PARAM_CALIB_U = 48        // Калибровка U
    , PARAM_CALIB_I = 49        // Калибровка I

//    ,PARAM_KIC        = 50    // К1 тока заряда
//    ,PARAM_KU         = 51
//    ,PARAM_KEX        = 52
//    ,PARAM_KID        = 53    //K1 тока разряда
//    ,PARAM_BIC        = 54    //B тока заряда
//    ,PARAM_BU         = 55   //B напряжения
//    ,PARAM_BEX        = 56
//    ,PARAM_BID        = 57   //B тока разряда
//    ,PARAM_DIC        = 58   //D тока заряда
//    ,PARAM_DU         = 59
//    ,PARAM_DEX        = 60
//    ,PARAM_DID        = 61   //D тока разряда
//    ,PARAM_CMD        = 62   //Команда
//    ,PARAM_WMODE      = 63   //Режим работы

    , PARAM_MVOLT      = 64  //Макс. напряжение
    , PARAM_MCUR       = 65  //Макс. ток
    , PARAM_MCURD      = 66  //Макс. ток разряда
    , PARAM_DCNT       = 67  //Кол-во разрядных модулей
    , PARAM_GCAP       = 68  //Кол-во модулей в группе
    , PARAM_GCNT       = 69  //Кол-во групп
//    ,PARAM_FLCFG      = 70   //Биты (флаги) конфигурации
    , PARAM_TRYCT      = 71  //Кол-во попыток запуска
    , PARAM_RSTTO      = 72  //Таймаут между перезапусками
    , PARAM_VSTRT      = 73  //Напряжение автозапуска
    , PARAM_VRDEV      = 74  //Версия блока
    , PARAM_RVDEV      = 75  //Модификация блока
    , PARAM_RVSW       = 76  //Модификация ПО
    , PARAM_SERNM      = 77  //Зав. номер
    , PARAM_MEMFR      = 78  //Свободная память
    , PARAM_METH_DESCR = 79  //Метод только описание
    , PARAM_STAGE_OF_METH = 80  //этап метода
    , PARAM_DPOW       = 81  //Max. мощность разряда
    , PARAM_UMAP       = 82  //Карта используемых модулей. Бит0 – модуль вкл/выкл. биты 1-7 – номер ЗРМ
    , PARAM_SOFT_REV   = 83  //Модификация ПО
    , PARAM_MOD_DATA   = 84  //Данные установки (расположения) модуля
    , PARAM_MOD_CHMAP  = 85  //Карта каналов (канал и его адрес)
    , PARAM_MAX_CHP    = 86  //Максимальная мощьность заряда

    , PARAM_RD_EPROM_METHOD = 87
    , PARAM_METHOD_STAGES   = 88 //Метод с этапами

    , PARAM_MAXTEMP         = 96  // Макс. температура АКБ
    , PARAM_MXCVOLT         = 97  // Макс. напряжение банки
    , PARAM_MNCVOLT         = 98  //Мин. напряжение банки
    , PARAM_BTYPE           = 99  //Тип АКБ (0-щелочной 1-кислотный)
    , PARAM_CELL            = 100  //Параметры элементов АКБ

    , PARAM_METH_EXEC_RESULT = 101   //Результаты выполнения метода

    , PARAM_VOLT_HIGH_VOLT_BUS = 103 // Напряжение высоковольтной шины
    , PARAM_VOLT_SUPPLY = 104       // Напряжение питающей сети
    , PARAM_CUR_CONSUMPTION = 105   // Потребляемый ток

    , PARAM_LOG_ID = 106    // id лога
    , PARAM_LOG_POINT = 107 // Параметры из лога
    , PARAM_LOG_COUNT = 108 // Количество логов

    , PARAM_METH_EXEC_RESULT_SENSOR = 109   //Результаты выполнения метода - показания датчиковза один этап

    , PARAM_BOOT_LOADER  = 250 // Команда перехода в загрузчик
};

enum zrm_mode_command
{
    CMD_ERASE = 7
};


constexpr int MAX_CHANNEL_NUMBER = 255;

struct pc_prolog_t {uint8_t sync_byte =  PS_PC;};
struct cu_prolog_t {uint8_t sync_byte =  PS_CU;} ;

#ifndef PROTOCOL_PT_LINE

using CRC_TYPE = uint32_t;

enum packet_types_t
{
    PT_DATAREQ   = 0x0A, // Запрос данных
    PT_DATAREAD  = 0x0B, // Чтение данных
    PT_DATAWRITE = 0x0C, // Запись данных
    PT_DIAG      = 0x0D, // Диагностика
    PT_DEBUG     = 0x0F,
    PT_CONREQ    = 0x10,
    PT_CONCONF   = 0x11
};

struct proto_header
{
    uint16_t session_id;
    uint16_t packet_number;
    uint16_t channel;
    uint8_t  type;
    uint16_t data_size;
    proto_header(uint16_t _session_id, uint16_t _number, uint16_t _channel, uint8_t _type);
    size_t   operator()() const {return size_t(data_size);}
    void     operator()(size_t _dsz) { data_size = uint16_t(_dsz);}
};

inline proto_header::proto_header(uint16_t _session_id, uint16_t _number, uint16_t _channel, uint8_t _type)
    : session_id   (_session_id)
    , packet_number(_number    )
    , channel      (_channel   )
    , type         (_type      )
{}

#else

using CRC_TYPE = uint8_t;

enum packet_types_t
{
    PT_DATAREQ   = 0x07,// Запрос данных
    PT_DATAREAD  = 0x12, // Чтение данных
    PT_DATAWRITE = 0x09, // Запись данных
    PT_DIAG      = 0x0D, // Диагностика
    PT_DEBUG     = 0x0F,
    PT_CONREQ    = 0x10,
    PT_CONCONF   = 0x11
};

struct proto_header
{
    uint8_t channel;
    uint8_t session_id;
    uint8_t packet_number;
    uint8_t type;
    uint16_t data_size;
    proto_header(uint16_t _session_id, uint16_t _number, uint16_t _channel, uint8_t _type);
    size_t operator()() const { return size_t(data_size); }
    void operator()(size_t _dsz) { data_size = uint16_t(_dsz); }
};

inline proto_header::proto_header(uint16_t _session_id, uint16_t _number, uint16_t _channel, uint8_t _type)
    : channel      (_channel   )
    , session_id   (_session_id)
    , packet_number(_number    )
    , type         (_type      )
{}


#endif

using  pCrcFunc = CRC_TYPE(*)(const void*, size_t);

union session_t
{
    struct
    {
        uint8_t  mode;
        uint8_t  error;
        uint16_t ssID;
    } session_param;
    uint32_t    value;
    session_t(uint16_t id, uint8_t a_mode = ST_FINISH,  uint8_t a_error = 0)
    {
        session_param.mode = a_mode ;
        session_param.error = (a_error);
        session_param.ssID = (id);
    }
    bool is_active   () const { return session_param.mode != ST_FINISH;}
    bool is_read_only() const { return session_param.mode != ST_CONTROL;}
};


union oper_state_t
{
    struct
    {
        uint8_t relay_on       : 1; // замкнуть реле (подать нпряжение на выход)
        uint8_t start_rectifier: 1; // выпрямитель запущен (заряда)
        uint8_t start_load     : 1; // нагрузка включена (разряда)
        uint8_t i_stab         : 1; // выпрямитель работает в режиме стабилизации тока
        uint8_t ctr_stab       : 1; // контролировать режима стабилизации включен
        uint8_t auto_on        : 1; // автоматическое выполнение метода включено
        uint8_t start_pause    : 1; // включен режим "пауза"
        uint8_t fault_reset    : 1; // ошибка
        uint8_t p_limit        : 1; // ограничение мощности
        uint8_t soft_start     : 1; // включен режим плавного запуска
        uint8_t cooling        : 1; // охлаждение (температура слишком высока для запуска, ожидается снижение температуры до приемлемого значения)
        uint8_t slave_mode     : 1; // режим ведомого включен
        uint8_t cfg_not_save   : 1; // конфигурация изменилась, но не сохранена в памяти
        uint8_t reserv         : 3;
    } state_bits;
    uint16_t state = 0;
};


const uint8_t APS_METHOD = 0xAA;
const uint8_t APS_STAGE  = 0x55;


struct stage_exec_result_t
{
    uint8_t stage;       //этап
    uint8_t state;       //состояние
    int32_t curr_begin;  //ток на момент начала этапа
    int32_t curr_end;    // ток на момент окончания этапа
    int32_t volt_begin;  //напряжение на момент начала этапа
    int32_t volt_end;    //напряжение на момент окончания этапа
    int32_t capcacity;   //ёмкость этапа
    int8_t  duration[3]; //время этапа  hh::mm::ss
    uint8_t end_cond;    //условия завершения этапа
    stage_exec_result_t(const stage_exec_result_t& other) = default;
    stage_exec_result_t& operator = (const stage_exec_result_t& other) = default;
    bool operator <  (const stage_exec_result_t& other) const { return stage < other.stage;  }
    bool operator == (const stage_exec_result_t& other) const { return stage == other.stage; }
};

struct stage_exec_result_sensor_t
{
    int16_t temp;   // температура
    int16_t volt;   // напряжение
    stage_exec_result_sensor_t& operator = (const stage_exec_result_sensor_t& other) = default;
};

#pragma pack(pop)

struct stage_exec_result_sensors_t
{
    uint8_t stage;  // этап
    uint8_t count;  // количество датчиков
    using sensor_data_t = std::vector<stage_exec_result_sensor_t>;
    sensor_data_t sensors;    // датчик
};

using  lp_stage_exec_result_t = stage_exec_result_t* ;

using  method_exec_results_t  = std::vector<stage_exec_result_t>;
using  method_exec_results_sensors_t  = std::vector<stage_exec_result_sensors_t>;

constexpr int METHOD_NAME_SIZE = 33;
constexpr uint16_t METHOD_UNKNOWN_ID = uint16_t(-1);
constexpr uint16_t METHOD_MANUAL_ID = 0;
using method_hms = std::tuple<uint8_t, uint8_t, uint8_t>;

enum  method_kind_t { method_kind_unknown, method_kind_manual, method_kind_automatic };

#pragma pack(push,1)

struct method_t
{
    uint8_t  m_signature;
    char     m_name[METHOD_NAME_SIZE];
    double m_current;
    double m_voltage;
    uint8_t  m_hours;
    uint8_t  m_minutes;
    uint8_t  m_secs;
    uint8_t  m_cycles_count;
    uint8_t  m_stages;
    uint16_t m_id;
    double m_capacity;

    method_t(uint16_t id = METHOD_UNKNOWN_ID)
    {
        memset(this, 0, sizeof(*this));
        m_signature = APS_METHOD;
        m_id      = id;
        m_cycles_count = 1;
        m_current = m_voltage = m_capacity = .0;
    }

    explicit method_t(const method_t& other)
    {
        *this = other;
    }

    method_t& operator = (const method_t& other)
    {
        memcpy(this, &other, sizeof(*this));
        m_signature = APS_METHOD;
        return *this;
    }
    void     set_name(const char* name, size_t len)
    { if (len < METHOD_NAME_SIZE) {strcpy(m_name, name);} else {memcpy(m_name, name, METHOD_NAME_SIZE);} }
    size_t   name_length() const {return m_name[METHOD_NAME_SIZE - 1] ? METHOD_NAME_SIZE : strlen(m_name);}
    bool     operator    <  (method_t& other) const {return m_id < other.m_id;}
    bool     operator    == (method_t& other) const {return m_id == other.m_id;}
    double   current        () const         { return m_current ; }
    double   voltage        () const         { return m_voltage ; }
    double   capacity       () const         { return m_capacity; }

    void     set_current    (double value)   { m_current  = value < 0 ? -value : value; }
    void     set_voltage    (double value)   { m_voltage  = value; }
    void     set_capacity   (double value)   { m_capacity = value < 0 ? -value : value; }
    double   current_ratio  (bool in_percent) const { return  (in_percent ? 100.0 : 1.0) * m_current / m_capacity; }
    uint32_t duration       ()const          { return uint32_t(m_hours) * 3600 + uint32_t(m_minutes) * 60 + uint32_t(m_secs);  }
    void     set_duration   (uint32_t value);
    uint8_t  cycles         () { return m_cycles_count; }
    void     set_cycles     (uint8_t val) { m_cycles_count = val; }
    method_kind_t method_kind   ();
    static   method_hms      secunds2hms(uint32_t duration);
    static   uint32_t        hms2secunds(const method_hms& hms);
    static   uint32_t        hms2secunds(uint8_t h, uint8_t m, uint8_t s);
    static   double          max_voltage() {return 1000;}
    static   double          max_capacity() {return 2000;}
    static   double          value_step  () {return 0.1; }

};

struct pack_method_t
{
    uint8_t  m_signature;
    char     m_name[METHOD_NAME_SIZE];
    uint16_t m_current;
    uint16_t m_voltage;
    uint8_t  m_hours;
    uint8_t  m_minutes;
    uint8_t  m_secs;
    uint8_t  m_cycles_count;
    uint8_t  m_stages;
    uint8_t  m_factor;
    uint16_t m_id;
    uint16_t m_capacity;

    pack_method_t(uint16_t id = METHOD_UNKNOWN_ID, uint8_t factor = 2)
    {
        memset(this, 0, sizeof(*this));
        m_signature = APS_METHOD;
        m_id      = id;
        m_factor  = factor;
    }

    pack_method_t(const method_t& method)
    {
        memset(this, 0, sizeof(*this));
        m_signature = method.m_signature;
        memcpy(m_name, method.m_name, METHOD_NAME_SIZE);
        m_factor = 2;
        if (method.m_current * 100 > 65535 || method.m_voltage * 100 > 65535)
            m_factor = 1;
        if (method.m_current * 100 > 16777215 || method.m_voltage * 100 > 16777215)
            m_factor = 0;
        m_current = uint16_t(method.m_current * pow(10, m_factor));
        m_voltage = uint16_t(method.m_voltage * pow(10, m_factor));
        m_hours = method.m_hours;
        m_minutes = method.m_minutes;
        m_secs = method.m_secs;
        m_cycles_count = method.m_cycles_count;
        m_stages = method.m_stages;
        m_id = method.m_id;
        m_capacity = uint16_t(method.current_ratio(true) * 100.);
    }

    void get_method(method_t& method) const
    {
        method.m_signature = m_signature;
        memcpy(method.m_name, m_name, METHOD_NAME_SIZE);
        method.m_current = double(m_current) / pow(10, double(m_factor));
        method.m_voltage = double(m_voltage) / pow(10, double(m_factor));
        method.m_hours = m_hours;
        method.m_minutes = m_minutes;
        method.m_secs = m_secs;
        method.m_cycles_count = m_cycles_count;
        method.m_stages = m_stages;
        method.m_id = m_id;
        method.m_capacity = m_current * 100. / m_capacity;
    }
};

typedef       method_t* lp_method_t;
typedef const pack_method_t* lpc_method_t;


enum stage_type_t
{
    STT_PAUSE = 0,
    STT_CHARGE,
    STT_DISCHARGE,
    STT_IMPULSE
};

enum stage_flags_t
{
    STFL_CAPACITY_MEASHURE = 1
    , STFL_CONDITION_CHECK   = 2
};

enum  stage_end_flags_t
{
    stage_end_time           = 0x01 //Окончание по времени
    , stage_end_current        = 0x02 //Окончание по току
    , stage_end_voltage        = 0x04 //Окончание по напряжению
    , stage_end_delta_voltage  = 0x08 //Окончание по изменению напряжения
    , stage_end_temper         = 0x10 //Окончание по температуре
    , stage_end_capacity       = 0x20 //Окончание по ёмкости
    , stage_end_cell_voltage   = 0x40 //Окончание по напряжению банки
};



constexpr double  stage_precision1 = 10.0;
constexpr double  stage_precision2 = 100.0;
constexpr double  stage_percent    = 100.0 * stage_precision2;

struct stage_t
{
    uint8_t  m_signature;
    uint8_t  m_type;
    uint8_t  m_finish_flags;
    uint16_t m_char_curr;
    uint16_t m_char_volt;
    uint16_t m_dis_curr;
    uint16_t m_dis_volt;
    uint8_t  m_char_time;
    uint8_t  m_dis_time;
    uint16_t m_end_cur;
    uint16_t m_end_volt;
    uint16_t m_end_delta_volt;
    uint16_t m_end_temper;
    uint16_t m_end_cap;
    uint8_t  m_hours;
    uint8_t  m_minutes;
    uint8_t  m_secs;
    uint8_t  m_number;
    uint8_t  m_stage_flags;
    uint16_t m_method_id;
    uint16_t m_end_elem_volt;

    stage_t()
    {
        memset(this, 0, sizeof (*this));
        m_signature = APS_STAGE;
    }

    bool operator <  (const stage_t& other) const {return m_number <  other.m_number;}
    bool operator == (const stage_t& other) const {return m_number == other.m_number;}

    double charge_volt    (double base            ) const {return base * double(m_char_volt    ) / stage_percent;}
    double charge_volt    (const method_t& ms    ) const {return charge_volt(ms.voltage());}

    double charge_curr    (double base            ) const {return base * double(m_char_curr     ) / stage_percent;}
    double charge_curr    (const method_t& ms    ) const {return charge_curr(ms.current());}

    double discharge_volt (double base            ) const {return base * double(m_dis_volt     ) / stage_percent;}
    double discharge_volt (const method_t& ms    ) const {return discharge_volt(ms.voltage());}

    double discharge_curr (double base            ) const {return base * double(m_dis_curr      ) / stage_percent;}
    double discharge_curr (const method_t& ms    ) const {return discharge_curr(ms.current());}

    double end_curr       (double base            ) const {return base * double(m_end_cur      ) / stage_percent;}
    double end_curr       (const method_t& ms    ) const {return end_curr(ms.current());}

    double end_volt       (double base            ) const {return base * double(m_end_volt     ) / stage_percent;}
    double end_volt       (const method_t& ms    ) const {return end_volt(ms.voltage());}

    double end_delta_volt (double base            ) const {return base * double(m_end_delta_volt) / stage_percent;}
    double end_delta_volt (const method_t& ms    ) const {return end_delta_volt(ms.voltage());}

    double end_capacity   (double base            ) const {return base * double(m_end_cap      ) / stage_percent;}
    double end_capacity   (const method_t& ms    ) const {return end_capacity(ms.capacity());}

    double end_temp       () const           {return m_end_temper   / stage_precision2;}
    double end_cell_volt  () const           {return m_end_elem_volt / stage_precision2;}

    void set_charge_volt   (double value, double base  )  { m_char_volt = uint16_t(stage_percent * value / base + 0.5); }
    void set_charge_volt   (double value, const method_t& ms)  { set_charge_volt(value, ms.voltage()); }

    void set_charge_curr   (double value, double base   )  { m_char_curr = uint16_t(stage_percent * value / base + 0.5); }
    void set_charge_curr   (double value, const method_t& ms)  { set_charge_curr(value, ms.current()); }

    void set_discharge_volt(double value, double base   )  { m_dis_volt = uint16_t(stage_percent * value / base + 0.5); }
    void set_discharge_volt(double value, const method_t& ms)  { set_discharge_volt(value, ms.voltage()); }

    void set_discharge_curr(double value, double base   )  { m_dis_curr = uint16_t(stage_percent * value / base + 0.5); }
    void set_discharge_curr(double value, const method_t& ms)  { set_discharge_curr(value, ms.current()); }

    void set_end_curr      (double value, double base   )  { m_end_cur = uint16_t(stage_percent * value / base + 0.5); }
    void set_end_curr      (double value, const method_t& ms)  { set_end_curr(value, ms.current()); }

    void set_end_volt      (double value, double base   )  { m_end_volt = uint16_t(stage_percent * value / base + 0.5); }
    void set_end_volt      (double value, const method_t& ms)  { set_end_volt(value, ms.voltage()); }

    void set_end_delta_volt(double value, double base   )  { m_end_delta_volt = uint16_t(stage_percent * value / base + 0.5); }
    void set_end_delta_volt(double value, const method_t& ms)  { set_end_delta_volt(value, ms.voltage()); }

    void set_end_capacity  (double value, double base   )  { m_end_cap  = uint16_t(stage_percent * value / base + 0.5); }
    void set_end_capacity  (double value, const method_t& ms)  { set_end_capacity(value, ms.capacity()); }

    void set_end_temp      (double value )            { m_end_temper    = uint16_t(value * stage_precision2);}
    void set_end_cell_volt (double value )            { m_end_elem_volt = uint16_t(value * stage_precision2);}
    bool is_charge         () const {return m_type & STT_CHARGE;}
    bool is_discharge      () const {return m_type & STT_DISCHARGE;}
    bool is_pause          () const {return m_type == STT_PAUSE;}
    bool is_impulse        () const {return m_type == STT_IMPULSE;}

    const char* stage_type_name (zrm_work_mode_t work_mode )
    { return stage_type_name(work_mode, stage_type_t (m_type) );}
    static const char* stage_type_name (zrm_work_mode_t work_mode, stage_type_t stage_type  );

    static const char* st_types_power  [4] ;
    static const char* st_types_charger[4] ;

};

typedef const stage_t* lpc_stage_t;
typedef       stage_t* lp_stage_t;
typedef       std::vector<stage_t>   stages_t;

#pragma pack(pop)

struct  zrm_method_t
{
    method_t  m_method;
    stages_t  m_stages;
    zrm_method_t() { m_method = method_t(); m_stages = stages_t(); }
    // на android не создается zrm_method_t(const zrm_method_t & m) по умолчанию ?
    zrm_method_t(const zrm_method_t& m): m_method(m.m_method), m_stages(m.m_stages) {}
    zrm_method_t& operator = (const zrm_method_t& other)
    {
        m_method = other.m_method;
        m_stages = other.m_stages;
        return *this;
    }
    stages_t::size_type stages_count() const {return m_stages.size();}
    zrm_method_t& operator += (const stage_t& stage)
    {
        m_stages.insert(m_stages.end(), stage);
        m_method.m_stages = static_cast<uint8_t>(stages_count());
        return *this;
    }
};

#pragma pack(push,1)

struct zrm_cell_t
{
    uint32_t m_volt;
    int32_t  m_temp;
    zrm_cell_t() { m_volt = m_temp = 0; }
    static constexpr double SCALE_FACTOR = 1000;
    double volt() { return double(m_volt) / SCALE_FACTOR; }
    double temp() { return double(m_temp) / SCALE_FACTOR; }
};

typedef       zrm_cell_t* lp_zrm_cell_t;
typedef const zrm_cell_t* lpc_zrm_cell_t;

typedef std::vector<zrm_cell_t> zrm_cells_t;


#pragma pack(pop)

typedef devproto::t_hdr<pc_prolog_t, proto_header, uint16_t> send_header_t, *lpsend_header_t;
typedef devproto::t_hdr<cu_prolog_t, proto_header, uint16_t> recv_header_t, *lprecv_header_t;

using recv_buffer_t  = devproto::proto_buffer<recv_header_t, CRC_TYPE>;
using _send_buffer_t = devproto::proto_buffer<send_header_t, CRC_TYPE>;

typedef devproto::storage_t                   params_t;

struct param_variant
{
    param_variant() = default;
    size_t       size = 0;
    union
    {
        int8_t    sbyte;
        uint8_t   ubyte;
        int16_t   sword;
        uint16_t  uword;
        int32_t   sdword;
        uint32_t  udword;
        int64_t   sqword;
        uint64_t  uqword ;
        double    v_double;
        float     v_float;
        uint8_t   puchar[128] = {0};
    };
    bool is_valid() const {return size && size <= sizeof(puchar);}
    template <typename T>
    T    value(bool as_signed) const;

};

template <typename T>
param_variant init_variant(T value)
{
    param_variant pv;
    pv.size =  sizeof (value);
    if (pv.size > sizeof(pv.puchar) )
        pv.size = sizeof (pv.puchar);
    memcpy(pv.puchar,  &value, pv.size);
    return pv;
}

template <typename T>
param_variant init_variant(T* value, int cnt)
{
    param_variant pv;
    pv.size =  sizeof (*value) * cnt;
    if (pv.size > sizeof(pv.puchar) )
        pv.size = sizeof (pv.puchar);
    memcpy(pv.puchar,  value, pv.size);
    return pv;
}


template <typename T>
T    param_variant::value(bool as_signed) const
{
    T res = T(0);
    switch (size)
    {
        case sizeof(int8_t ) :
            res = as_signed ? T(sbyte)  : T(ubyte);
            break;
        case sizeof(int16_t) :
            res = as_signed ? T(sword)  : T(uword);
            break;
        case sizeof(int32_t) :
            res = as_signed ? T(sdword) : T(udword);
            break;
        case sizeof(int64_t) :
            res = as_signed ? T(sqword) : T(uqword);
            break;
        default :
            break;
    }
    return res;
}

typedef  std::map<zrm_param_t, param_variant> params_list_t;


class   send_buffer_t : public _send_buffer_t
{

public:

    explicit send_buffer_t(size_t sz = 1024): _send_buffer_t(sz) {}
    size_t   queue_packet         (uint16_t channel, uint8_t packet_type, uint16_t data_size, const void* data = nullptr );
    size_t   queue_request        (uint16_t channel,  const devproto::storage_t& param_list);
    uint16_t session_id   ();
    void     set_sesion_id(uint16_t session_id);
    uint16_t packet_number();
    void     set_packet_number(uint16_t pn);

    static void  params_add(devproto::storage_t& data, param_write_mode_t wm, zrm_param_t  param, size_t val_sz = 0, const void* val_ptr = nullptr);
    template <typename _Type>
    static void  params_add(devproto::storage_t& data, param_write_mode_t wm, zrm_param_t  param, _Type value);

protected:
    uint16_t   m_packet_number = 0;
    uint16_t   m_session_id    = 0;
};

constexpr int SECUNDS_IN_MINUTE = 60;
constexpr int SECUNDS_IN_HOUR = 60 * SECUNDS_IN_MINUTE;

/* inline implementation*/


inline void     method_t::set_duration(uint32_t value)
{
    div_t h   = div(int(value), SECUNDS_IN_HOUR);
    div_t ms  = div(h.rem, SECUNDS_IN_MINUTE  );
    m_hours   = uint8_t(h.quot);
    m_minutes = uint8_t(ms.quot);
    m_secs    = uint8_t(ms.rem);
}

inline method_kind_t method_t::method_kind   ()
{
    if (m_id)
        return m_id == uint16_t(-1) ? method_kind_unknown : method_kind_automatic;
    return method_kind_manual;
}

inline uint16_t send_buffer_t::session_id   ()
{
    return m_session_id;
}

inline void     send_buffer_t::set_sesion_id(uint16_t session_id)
{
    m_session_id = session_id;
}

inline uint16_t send_buffer_t::packet_number()
{
    return m_packet_number;
}

inline void     send_buffer_t::set_packet_number(uint16_t pn)
{
    m_packet_number =    pn;
}

template <typename _Type>
void  send_buffer_t::params_add(devproto::storage_t& data, param_write_mode_t wm, zrm_param_t  param, _Type value)
{
    params_add(data, wm, param, sizeof (value), &value);
}


inline method_hms method_t::secunds2hms(uint32_t duration)
{
    div_t h  = div(int(duration), SECUNDS_IN_HOUR);
    div_t ms = div(h.rem, SECUNDS_IN_MINUTE  );
    return std::make_tuple(uint8_t(h.quot), uint8_t(ms.quot), uint8_t(ms.rem));
}

inline uint32_t  method_t::hms2secunds(uint8_t h, uint8_t m, uint8_t s)
{
    return h * SECUNDS_IN_HOUR + m * SECUNDS_IN_MINUTE + s;
}

inline uint32_t  method_t::hms2secunds(const method_hms& hms)
{
    return hms2secunds(std::get<0>(hms), std::get<1>(hms), std::get<2>(hms) );
}

} // end of namecpace zrm

#endif // PROTODEF_H

