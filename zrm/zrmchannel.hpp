#ifndef ZRMCHANNEL_H
#define ZRMCHANNEL_H

#include "zrmproto.hpp"
#include <tuple>
#include <algorithm>
#include <QByteArrayList>
#include <QElapsedTimer>

namespace zrm {


struct zrm_maskab_param_t
{
	double dU = .0;
	double dT = .0;
};


constexpr uint32_t CHANNEL_DEFAULT_COLOR = 0x4682b4;
constexpr uint16_t SESSION_ID_DEFAULT = 555;
#ifndef PROTOCOL_PT_LINE
	constexpr int      SEND_DELAY_DEFAULT   = 10;
#else
	constexpr int      SEND_DELAY_DEFAULT   = 100;
#endif

class ZrmChannel
{
public:
// typedef  std::recursive_mutex                  mutex_t;
// typedef  std::lock_guard<mutex_t>              locker_t;

	struct Attributes
	{
		int box_number = 0;
		int device_number = 0;
		uint32_t color = CHANNEL_DEFAULT_COLOR;
		bool operator == (const Attributes& other) const
		{return (box_number == other.box_number && device_number == other.device_number && color == other.color);}
	};


	ZrmChannel(uint16_t channel = uint16_t(-1), zrm_work_mode_t work_mode = zrm_work_mode_t::as_charger);
	virtual ~ZrmChannel();

	virtual int      handle_recv (const zrm::recv_header_t& recv_data);
	zrm_work_mode_t work_mode   () const { return m_work_mode;}
	void     set_work_mode(zrm_work_mode_t wm) {m_work_mode = wm;}
	uint16_t channel     () const;
	void     set_channel (uint16_t channel);
	int      ping_period() const;
	void     set_ping_period(int value);
	bool     ping_check     (int timer_value);
	void     ping_reset     () const;


	session_types_t  session_request       () const;
	void      set_session_request   (session_types_t sess_type);
	session_t session               () const;
	bool      session_active        () const;
	bool      session_readonly      () const;
	void      session_reset         ();


	void     param_request_add    (zrm_param_t param);
	void     params_request_add   (const params_t&   params);
	void     param_request_remove (const zrm_param_t param);
	void     params_request_remove(const params_t&   params);
	bool     param_is_requested   (zrm_param_t param);
	const params_t&   params_list          () const;
	param_variant     param_get            (zrm_param_t param) const;
	bool     params_is_changed    (zrm_param_t param) const;
	const params_list_t& params_current     () const;

	oper_state_t  get_state(bool prev = false) const;

	const params_list_t& changes()   const;
	void          clear_changes();

	int           results_get(method_exec_results_t&) const;
	void          results_clear();

	void          method_clear();
	void          method_set  (const zrm_method_t& method);
	const zrm_method_t& method_get (bool eprom = false) const    { return eprom ? m_eprom_method : m_current_method;}
	zrm_method_t& method_get (bool eprom = false)          { return eprom ? m_eprom_method : m_current_method;}
	bool          method_is_auto() const { return m_current_method.m_method.m_id > 0 ; }

	bool          is_executing () const;
	bool          is_paused  () const;
	bool          is_stopped () const;

	const method_exec_results_t&          results_get() const ;
	const method_exec_results_sensors_t&  results_sensor_get() const ;
	zrm_cells_t         cells_get() const;
	zrm_maskab_param_t  masakb_param();
	void                set_masakb_param(const zrm_maskab_param_t& map);

	void  setAttributes(const Attributes& attrs) {m_Attributes = attrs;}
	const Attributes& getAttributes() const {return m_Attributes;}

	static std::string  time_param (const param_variant& pv);
	static std::string  trect_param(const param_variant& pv);
	static std::string  fan_param  (const param_variant& pv);

	/*   methods after refactoring */

	qint64 getRespondTime();
	void   queryParams(size_t psize, const void* params);
	void   queuePacket( packet_types_t type, size_t dataSize, const void* data);

	QByteArray getNextPacket();
	bool   readyToSend() const;
	bool   hasPacket() const;
	void   clearSend();

	void   startSession();
	void   stopSession();
	bool   isWriteEnabled( uint8_t type);
	void   pingChannel();


	static QByteArray makeSendPacket
	(uint16_t ssid, uint16_t packetNumber,
		uint16_t channel, uint8_t packet_type,
		size_t data_size, const void* data
	);

protected:
	void     param_set           (zrm_param_t param, const param_variant& pv);
	void     handle_conreq       (const session_t* session);
	void     handle_data         (const uint8_t*    data, size_t size);
	uint16_t handle_results      (uint16_t data_size, const uint8_t* beg, const uint8_t* end);
	uint16_t handle_results_sensor(uint16_t data_size, const uint8_t* beg, const uint8_t* end);
	uint16_t handle_cells        (uint16_t data_size, const uint8_t* beg, const uint8_t* end);
	uint16_t handle_eprom_method (uint16_t data_size, const uint8_t* beg, const uint8_t* end);
	uint16_t handle_method_stages(uint16_t data_size, const uint8_t* beg, const uint8_t* end);
	static uint16_t handle_method_stages(zrm_method_t& method, uint16_t data_size, const uint8_t* beg, const uint8_t* end);

protected:
//mutable mutex_t          m_mut;
	uint16_t              m_LastPacketNumber = -1;
	int                   m_ping_period     =  1000;
	mutable int           m_ping_timeout    =  0;
	uint16_t              m_channel         =  uint16_t(-1);
	session_types_t       m_session_request = ST_CONTROL;
	params_list_t         m_curr_params;   //Список полученных   параметров

	mutable params_list_t m_chg_params;   //Список изменившихся параметров
	params_t              m_ctrl_params;  //Список параметров которые хочется контролировать
	oper_state_t          m_prev_state;

	zrm_cells_t           m_cells;
	zrm_work_mode_t       m_work_mode = as_charger;
	zrm_maskab_param_t    m_maskab_param;

	zrm_method_t          m_eprom_method;   //Метод запрошенный из памяти устройства
	zrm_method_t          m_current_method; //Текущий метод
	method_exec_results_t m_exec_results;
	method_exec_results_sensors_t m_exec_results_sensor;

	Attributes            m_Attributes;


	int                   m_SendDelay = SEND_DELAY_DEFAULT;
	QByteArrayList        m_SendQueue;
	uint16_t              m_PacketNumber = 0;
	QElapsedTimer         m_timeFromRecv;
	QElapsedTimer         m_timeFromSend;
	qint64                m_RespondTime = 0;
	bool                  m_waitReceive = false;
	uint16_t              m_SessionId = SESSION_ID_DEFAULT;
	bool                  m_LastPacketIsPing = false;


};

//********* inline implementations

inline void   ZrmChannel::ping_reset     () const
{
	m_ping_timeout = m_ping_period;
}

inline uint16_t ZrmChannel::channel() const
{
	return m_channel;
}

inline void     ZrmChannel::set_channel(uint16_t channel)
{
	m_channel = channel;
}

inline int      ZrmChannel::ping_period() const
{
	return m_ping_period;
}

inline void     ZrmChannel::set_ping_period(int value)
{
	m_ping_period = value;
}

inline session_types_t  ZrmChannel::session_request    () const
{
	return m_session_request;
}

inline         void     ZrmChannel::set_session_request(session_types_t sess_type)
{
	m_session_request =  sess_type == ST_FINISH ? ST_READONLY : sess_type;
}

inline const params_t&   ZrmChannel::params_list         () const
{
	return m_ctrl_params;
}

inline const params_list_t&  ZrmChannel::changes() const
{
	//locker_t l(m_mut);
	return m_chg_params;
}

inline void   ZrmChannel::clear_changes()
{
	//locker_t l(m_mut);
	m_chg_params.clear();
}

inline bool       ZrmChannel::is_executing() const
{
	return get_state().state_bits.auto_on;
}

inline bool       ZrmChannel::is_paused () const
{
	return get_state().state_bits.start_pause;
}

inline bool       ZrmChannel::is_stopped() const
{
	auto   st = get_state();
	return !(st.state_bits.auto_on | st.state_bits.start_pause);
}

inline const params_list_t& ZrmChannel::params_current() const
{
	//locker_t l(m_mut);
	return m_curr_params;
}

inline bool      ZrmChannel::session_active        () const
{
	return session().is_active();
}

inline bool      ZrmChannel::session_readonly      () const
{
	return session().is_read_only();
}

inline zrm_maskab_param_t ZrmChannel::masakb_param()
{
	return m_maskab_param;
}

inline void               ZrmChannel::set_masakb_param(const zrm_maskab_param_t& map)
{
	m_maskab_param = map;
}



inline const method_exec_results_t& ZrmChannel::results_get() const
{return  m_exec_results;}

inline const method_exec_results_sensors_t& ZrmChannel::results_sensor_get() const {return  m_exec_results_sensor; }

inline bool operator == (const param_variant& pv1, const param_variant& pv2)
{
	return pv1.size == pv2.size && pv1.uqword == pv2.uqword;
}

inline bool operator != (const param_variant& pv1, const param_variant& pv2)
{
	return !(pv1 == pv2);
}



} // namespace zrm

#endif // ZRMCHANNEL_H
