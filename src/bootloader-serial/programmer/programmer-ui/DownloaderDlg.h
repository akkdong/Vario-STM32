// DownloaderDlg.h
//

#pragma once

#include "SerialWnd.h"
//#include "Device.h"
#include "Packet.h"

#include <list>

////////////////////////////////////////////////////////////////////////////////////
//

class CProgramDlg;
class CUploadDlg;
class CMemoryDumpDlg;


////////////////////////////////////////////////////////////////////////////////////
//

class SerialPort
{
public:
	SerialPort(int nPort, LPCTSTR lpszName) : m_nPort(nPort), m_strName(lpszName) {}

public:
	int			m_nPort;
	CString		m_strName;
};


////////////////////////////////////////////////////////////////////////////////////
//

class CDownloaderDlg : public CDialogEx, protected BPacketListener
{
	friend class CProgramDlg;
	friend class CUploadDlg;
	friend class CMemoryDumpDlg;

public:
	CDownloaderDlg(CWnd* pParent = NULL);
	~CDownloaderDlg();

	enum State
	{
		_READY,
		_CHECK_DEVICE,
		_IDENTIFY_DEVICE,
		_IDENTIFY_BOOTLOADER,
		_RUN
	};

	enum LogLevel
	{
		_INFO,
		_VERBO
	};

	// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DOWNLOADER};
#endif

protected:
	virtual void		DoDataExchange(CDataExchange* pDX);
	virtual void		OnPacketReceived(BPacket * pPacket);

	void				SendCommand(uint8_t * pData, uint16_t nDataLen);

private:
	LRESULT				OpenSerial(LPCTSTR lpszDevice, CSerial::EBaudrate, CSerial::EDataBits, CSerial::EParity, CSerial::EStopBits, CSerial::EHandshake);
	void				CloseSerial();

	void				Connect(BOOL bDeviceCheck);
	void				Disconnect();

	BOOL				UpdateData(BOOL bSaveAndValidate);
	void				UpdateTitle();
	void				UpdateControls();

	void				RefillSerialPort();

	void				SendFirmwareVersion();
	void				SendRebootRequest();
	void				SendIdentify();
	void				SendStartCommand();

	void				Route(BPacket * pPacket);

	void				Log(LogLevel level, LPCTSTR format, ...);

	CString				GetDigest(void * pData, uint16_t nDataLen);
	CString				GetDigest(BPacket * pPacket);

// 구현입니다.
protected:
	//
	HICON				m_hIcon;

	//
	CSerialWnd			m_Serial;
	BOOL				m_bConnected;

	State				m_State;
	uint32_t			m_SubState;

	//
	UINT				m_nTimerID;

	//
	BPacketParser		m_Parser;
	BPacketListener *	m_pPacketListener;


	// Serial Settings
	CString				m_strPortName;
	int					m_sPortNum;
	CSerial::EBaudrate	m_sBaudRate;
	CSerial::EDataBits	m_sDataBits;
	CSerial::EParity	m_sParity;
	CSerial::EStopBits	m_sStopBits;
	CSerial::EHandshake	m_sHandshake;

	//
	CString				m_strDevID;
	CString				m_strBootVer;
	CString				m_strFWVer;

	uint16_t			m_nDevID;
	uint16_t			m_nBootVer;
	uint16_t			m_nFWVer;

	BOOL				m_bVerbo;

	CListCtrl			m_wndHistory;
	CComboBox			m_wndSerialPort;
	CComboBox			m_wndSerialBaudRate;
	CComboBox			m_wndSerialDataBits;
	CComboBox			m_wndSerialParity;
	CComboBox			m_wndSerialStopBits;
	CComboBox			m_wndSerialHandshake;

	std::list<SerialPort> m_SerialPorts;

protected:
	virtual BOOL		OnInitDialog();
	virtual void		OnOK();
	virtual void		OnCancel();

	afx_msg void		OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void		OnPaint();
	afx_msg HCURSOR		OnQueryDragIcon();
	afx_msg void		OnTimer(UINT nIDEvent);
	afx_msg LRESULT		OnSerialMessage(WPARAM wParam, LPARAM lParam);
	afx_msg void		OnConnect();
	afx_msg void		OnDownload();
	afx_msg void		OnUpload();
	afx_msg void		OnMemoryDump();
	afx_msg void		OnCheckDevice();

	afx_msg void		OnRefresh();
	afx_msg void		OnClearLog();
	afx_msg void		OnRunUserProgram();
	afx_msg void		OnChangeLogLevel();

	DECLARE_MESSAGE_MAP()
};

