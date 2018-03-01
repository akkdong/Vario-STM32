// DownloaderDlg.h
//

#pragma once

#include "SerialWnd.h"
#include "Packet.h"
#include <list>

class CProgramDlg;
class CUploadDlg;
class CMemoryTestDlg;


////////////////////////////////////////////////////////////////////////////////////
//

class CDownloaderDlg : public CDialogEx, protected PacketListener
{
	friend class CProgramDlg;
	friend class CUploadDlg;
	friend class CMemoryTestDlg;

public:
	CDownloaderDlg(CWnd* pParent = NULL);
	~CDownloaderDlg();

	enum State
	{
		_READY,
		_IDENTIFY,
		_RUN
	};

	// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DOWNLOADER};
#endif

protected:
	virtual void		DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.
	virtual void		OnPacketReceived(PACKET * pPacket);

	void				SendCommand(uint8_t * pData, uint16_t nDataLen);

private:
	void				OpenSerial(LPCTSTR lpszDevice, CSerial::EBaudrate, CSerial::EDataBits, CSerial::EParity, CSerial::EStopBits, CSerial::EHandshake);
	void				CloseSerial();

	BOOL				UpdateData(BOOL bSaveAndValidate);
	void				UpdateTitle();

	void				SendResetRequest();
	void				SendIdentify();

	void				Log(LPCTSTR format, ...);

	void				Route(PACKET * pPacket);

// 구현입니다.
protected:
	//
	HICON				m_hIcon;

	//
	CSerialWnd			m_Serial;
	BOOL				m_bConnected;

	State				m_State;
	uint32_t			m_subState;

	//
	UINT				m_nTimerID;

	//
	PacketParser		m_Parser;
	PacketListener *	m_pPacketListener;


	// Serial Settings
	int					m_nPortNum;
	CSerial::EBaudrate	m_nBaudRate;
	CSerial::EDataBits	m_nDataBits;
	CSerial::EParity	m_nParity;
	CSerial::EStopBits	m_nStopBits;

	//
	CString				m_strDevID;
	CString				m_strFWVer;

	uint16_t			m_nDevID;
	uint16_t			m_nFWVer;

	CListBox			m_wndReport;

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
	afx_msg void		OnMemoryTest();
	afx_msg void		OnIdentify();

	afx_msg void		OnTest();
	afx_msg void		OnTest2();

	DECLARE_MESSAGE_MAP()
};

