/* Ostapenko D. V.
 * NIKTES 2019-03-22
 * Base widget for display state channel of zrm
 *
 */

#ifndef ZRMBASEWIDGET_H
#define ZRMBASEWIDGET_H

#include <qwidget.h>
#include <qtextcodec.h>
#include <zrm_connectivity.hpp>
#include <qlabel.h>
#include <qspinbox.h>
#include <qicon.h>


// Timer for simultaneous flashing objects
class ZrmFlashTimer: public QObject
{
	Q_OBJECT
public:
	ZrmFlashTimer(QObject* parent = Q_NULLPTR);
	void start_flash();
	void stop_flash ();
	bool is_flash_on();
signals:
	void  flash(bool flash_on);
private slots:
	void  on_timeout();
private:
	int    m_flash_state = 0;
	QTimer m_timer;
};


class ZrmBaseWidget: public QWidget
{
	Q_OBJECT
	friend class ZrmGroupWidget;
public:
	explicit      ZrmBaseWidget(QWidget* parent);
	virtual      ~ZrmBaseWidget() ;
	zrm::ZrmConnectivity* connectivity() {return  m_source;}
	virtual void          bind(zrm::ZrmConnectivity*    src, uint16_t chan, bool _connect_signals = true);
	virtual void          update_ui() {};
	bool          load_ui(const QString& ui_file);
	uint16_t      channel() {return m_channel;}
	QString       channel_name(uint16_t channel);
	bool          channel_is_stopped  (uint16_t channel);
	bool          channel_is_executing(uint16_t channel);
	bool          channel_is_paused   (uint16_t channel);

	QVariant      param_get  ( uint16_t channel, zrm::zrm_param_t param)
	{return (m_source && channel) ? m_source->param_get(channel, param) : QVariant();}


	static QLatin1String  codec_name();
	static void           set_codec_name  (const QLatin1String& str);
	static QString        to_utf(const char* str, int len);
	static QTextCodec*    codec();
	static void           addShadow(QWidget* w, qreal offset, qreal blurRadius, QColor color = Qt::GlobalColor::gray);
	template <typename T>
	static void setWidgetsShadow(QWidget* parent, qreal offset, qreal blurRadius);


protected slots:
	void    slot_connected       ( bool       conn_state);
	void    slot_recv_packet     ( QByteArray packet    );
	void    slot_send_packet     ( QByteArray packet    );
	void    slot_ioerror         ( QString    error_string );
	void    slot_param_changes   (unsigned    channel,  zrm::params_list_t params_list);
	void    slot_source_destroyed(QObject* obj);

protected:
	virtual void    connect_signals      (zrm::ZrmConnectivity* conn_obj, bool conn);
	virtual void    source_destroyed     (zrm::ZrmConnectivity* src);
	virtual void    on_connected         (bool con_state);
	virtual void    on_ioerror           (const QString& error_string);
	virtual void    channel_recv_packet  (unsigned channel, const zrm::recv_header_t* recv_data);
	virtual void    channel_send_packet  (unsigned channel, const zrm::send_header_t* send_data);
	virtual void    channel_param_changed(unsigned channel, const zrm::params_list_t& params_list  );

	virtual void    update_controls      ();
	virtual void    clear_controls       () {}
	virtual void    channel_session      (unsigned ch_num)  ;
	virtual void    onActivate() {}
	virtual void    onDeactivate() {}
	virtual void    showEvent(QShowEvent* event) override;
	virtual void    hideEvent(QHideEvent* event) override;
	virtual void    init_ui(QWidget* widget ) {Q_UNUSED(widget)}
	QString    zrm_method_duration_text(const zrm::zrm_method_t& method);
	template <typename T>
	static  void  set_number_value (T* text_widget, double value, int precision, const QString& zero_replace = QString()) ;
	template <typename T>
	static  void  set_number_value (T* text_widget, int value, int width, const QString& zero_replace = QString());
	static QString number_text( double value, int precision);
	static QString number_text(int value, int width, int base = 10);

	zrm::ZrmConnectivity*    m_source      =  Q_NULLPTR;
	uint16_t                 m_channel     =  0;

	static QLatin1String            m_codec_name  ;
	static QTextCodec*              m_text_codec ;

	static QString                  infinity_symbol;
	static QString                  no_value;
	static ZrmFlashTimer            flash_timer;
};



class ZrmChannelWidget: public ZrmBaseWidget
{
	Q_OBJECT
public:
	ZrmChannelWidget     (QWidget* parent): ZrmBaseWidget(parent) {}
	QVariant param_get   (zrm::zrm_param_t param) {return ZrmBaseWidget::param_get(uint16_t(channel()), param);}
	bool     is_stopped  ();
	bool     is_paused   ();
	bool     is_executing();

protected:
};


class ZrmGroupWidget : public ZrmBaseWidget
{
	Q_OBJECT
public:
	ZrmGroupWidget(QWidget* parent): ZrmBaseWidget(parent) {}
	virtual  void  bind(zrm::ZrmConnectivity*    src, uint16_t chan, bool _connect_signals = true) override;
	QList<ZrmBaseWidget*>   zrm_widgets() {if (!m_widgets.count()) {zrm_widgets_make();} return m_widgets;}
	void          update_ui() override;
protected:
	virtual void  channel_recv_packet  (unsigned channel, const zrm::recv_header_t* recv_data) override;
	virtual void  channel_send_packet  (unsigned channel, const zrm::send_header_t* send_data) override;
	virtual void  channel_param_changed(unsigned channel, const zrm::params_list_t& params_list)  override;
	virtual void  on_connected         (bool con_state) override;
	virtual void  on_ioerror           (const QString& error_string) override;
	virtual void  source_destroyed     (zrm::ZrmConnectivity* src) override;
	virtual void  update_controls      () override;
	virtual void  clear_controls       () override;



	void    zrm_widgets_clear    ();
	void    zrm_widgets_make     ();
	QList<ZrmBaseWidget*>        m_widgets;

};

/**/

inline bool          ZrmBaseWidget::channel_is_stopped  (uint16_t channel)
{
	return m_source && channel ? m_source->channel_is_stopped(channel) : true;
}

inline bool          ZrmBaseWidget::channel_is_executing(uint16_t channel)
{
	return m_source && channel ? m_source->channel_is_executing(channel) : false;
}

inline bool          ZrmBaseWidget::channel_is_paused   (uint16_t channel)
{
	return m_source && channel ? m_source->channel_is_paused(channel) : false;
}


inline QString  ZrmBaseWidget::to_utf(const char* str, int len)
{
	return m_text_codec ? m_text_codec->toUnicode(str, len) : QString::fromLocal8Bit(str, len);
}

inline QLatin1String  ZrmBaseWidget::codec_name()
{
	return m_codec_name;
}

inline QTextCodec* ZrmBaseWidget::codec()
{
	return m_text_codec;
}

template <typename T>
void    ZrmBaseWidget::set_number_value (T* text_widget, int value, int width, const QString& zero_replace )
{
	if (text_widget)
	{
		if (!value && !zero_replace.isEmpty())
			text_widget->setText(zero_replace);
		else
			text_widget->setText(number_text(value, width, 10));
	}
}

template <typename T>
void   ZrmBaseWidget::set_number_value (T* text_widget, double value, int precision, const QString& zero_replace)
{
	if (text_widget)
	{
		if (qFuzzyIsNull(value) && !zero_replace.isEmpty())
			text_widget->setText(zero_replace);
		else
			text_widget->setText(number_text(value, precision));
	}
}


template <>
inline void   ZrmBaseWidget::set_number_value<QDoubleSpinBox> (QDoubleSpinBox* text_widget, double value, int precision, const QString& zero_replace)
{
	text_widget->setSpecialValueText(zero_replace);
	text_widget->setDecimals(precision);
	text_widget->setValue(value);
}


template <typename T>
void ZrmBaseWidget::setWidgetsShadow(QWidget* parent, qreal offset, qreal blurRadius)
{
	for ( auto&& b : parent->findChildren<T*>())
	{
		ZrmBaseWidget::addShadow(b, offset, blurRadius);
	}
}




inline bool     ZrmChannelWidget::is_stopped  ()
{
	return m_source && m_channel ? m_source->channel_is_stopped(uint16_t(m_channel)) : true;
}

inline bool     ZrmChannelWidget::is_paused   ()
{
	return m_source && m_channel ? m_source->channel_is_paused(uint16_t(m_channel)) : true;
}

inline bool     ZrmChannelWidget::is_executing()
{
	return m_source && m_channel ? m_source->channel_is_executing(uint16_t(m_channel)) : true;
}





#endif // ZRMBASEWIDGET_H
