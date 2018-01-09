
// VarioConsoleDlg.h : 헤더 파일
//

#pragma once
#include "afxcmn.h"
#include "SerialWnd.h"
#include "VarioSettings.h"
#include <list>

#define MAX_SERIAL_BUFFER			(1024)

#define TIMER_CHECK_RESPONSE		(0x1001)


struct VarioParameters
{
	// GliderInfo
	int				Glider_Type;
	CString			Glider_Manufacture;
	CString			Glider_Model;
	// IGC-Logger
	BOOL			Logger_Enable;
	int				Logger_TakeoffSpeed;
	int				Logger_LandingTimeout;
	int				Logger_LoggingInterval;
	CString			Logger_PilotName;
	int				Logger_Timezone;
	// Vario Settings
	float			Vario_ClimbThreshold;
	float			Vario_SinkThreshold;
	float			Vario_Sensitivity;
	int				Vario_Sentece;
	BOOL			Vario_BaroOnly;
	// Volume Settings
	int				Volume_Vario;
	int				Volume_Effect;
	// Threshold Settings
	float			Threshold_LowBattery;
	int				Threshold_ShutdownHoldtime;
	int				Threshold_AutoShutdownVario;
	int				Threshold_AutoShutdownUMS;
	// Kalman Parameters
	float			Kalman_VarZMeas;
	float			Kalman_VarZAccel;
	float			Kalman_VarAccelbias;
	// Calibration data
	float			Caldata_Accel[3];
	float			Caldata_Gyro[3];
	float			Caldata_Mag[3];

	// Tone table
	VarioTone		ToneTable[TONE_TABLE_COUNT];
};


// CVarioConsoleDlg 대화 상자
class CVarioConsoleDlg : public CDialogEx
{
// 생성입니다.
public:
	CVarioConsoleDlg(CWnd* pParent = NULL);	// 표준 생성자입니다.

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_WCONSOLE_DIALOG };
#endif

protected:
	virtual void		DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.

private:
	void				ParseReceivedMessage();
	void				ProcessReceivedMessage(WORD code, UINT param, LPCTSTR lpszData);

	BOOL				UpdateData(BOOL bSaveAndValidate);
	void				UpdateTitle();

	void				OpenSerial(LPCTSTR lpszDevice, CSerial::EBaudrate, CSerial::EDataBits, CSerial::EParity, CSerial::EStopBits, CSerial::EHandshake);
	void				CloseSerial();

// 구현입니다.
protected:
	//
	HICON				m_hIcon;

	//
	CSerialWnd			m_Serial;
	BOOL				m_bConnected;

	CHAR				m_pSerialBuf[MAX_SERIAL_BUFFER];
	int					m_nBufLen;

	std::list<CString>	m_RecvMsgs;
	std::list<CString>	m_SendMsgs;
	BOOL				m_bRecvVarioMsg;

	//
	UINT				m_nTimerID;

	// Vario Parameters
	VarioParameters		m_VarioParams;

	// Serial Settings
	int					m_nPortNum;
	CSerial::EBaudrate	m_nBaudRate;
	CSerial::EDataBits	m_nDataBits;
	CSerial::EParity	m_nParity;
	CSerial::EStopBits	m_nStopBits;

public:
	CComboBox			m_wndGliderType;
	int					m_nGliderType;
	CString				m_strGliderManufacture;
	CString				m_strGliderModel;
	int					m_nLoggerTakeoffSpeed;
	int					m_nLOggerLandingTimeout;
	int					m_nLoggerLoggingInterval;
	CString				m_strPilotName;
	CComboBox			m_wndTimezone;
	int					m_nTimezone;
	BOOL				m_bLoggerEnable;
	float				m_fVarioClimbThreshold;
	float				m_fVarioSinkThreshold;
	float				m_fVarioSensitivity;
	CComboBox			m_wndVarioSentence;
	int					m_nVarioSentence;
	BOOL				m_bVarioUseBaroOnly;
	float				m_fKalmanVarZmeas;
	float				m_fKalmanVarZaccel;
	float				m_fKalmanVarAccelbias;
	CSliderCtrl			m_wndVolumeVario;
	int					m_nVolumeVario;
	CString				m_strVolumeVario;
	CSliderCtrl			m_wndVolumeEffect;
	int					m_nVolumeEffect;
	CString				m_strVolumeEffect;
	float				m_fThresholdLowBattery;
	unsigned int		m_fThresholdShutdownHoldtime;
	unsigned int		m_fThresholdAutoPoweroffVario;
	unsigned int		m_fThresholdAutoPoweroffUMS;
	float				m_fCalDataAccelX;
	float				m_fCalDataAccelY;
	float				m_fCalDataAccelZ;
	float				m_fCalDataGyroX;
	float				m_fCalDataGyroY;
	float				m_fCalDataGyroZ;

protected:
	// 생성된 메시지 맵 함수
	virtual BOOL		OnInitDialog();
	virtual void		OnOK();
	virtual void		OnCancel();
	afx_msg void		OnPaint();
	afx_msg HCURSOR		OnQueryDragIcon();
	afx_msg void		OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void		OnTimer(UINT nIDEvent);
	afx_msg LRESULT		OnSerialMessage(WPARAM, LPARAM);
	afx_msg void		OnEditToneTable();
	afx_msg void		OnCalibration();
	afx_msg void		OnViewSensorData();
	afx_msg void		OnStore();
	afx_msg void		OnReload();
	afx_msg void		OnFactoryReset();
	afx_msg void		OnConnect();
	afx_msg void		OnSelectPort();
	afx_msg void		OnSendMessage();

	DECLARE_MESSAGE_MAP()

};
