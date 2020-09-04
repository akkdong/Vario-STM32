
// VarioConsoleDlg.h : ��� ����
//

#pragma once
#include "afxcmn.h"
#include "SerialWnd.h"
#include "VarioSettings.h"
#include "MatrixStatic.h"
#include <list>

#define MAX_SERIAL_BUFFER			(1024)

#define TIMER_CHECK_RESPONSE		(0x1001)

// CVarioConsoleDlg ��ȭ ����
class CVarioConsoleDlg : public CDialogEx
{
// �����Դϴ�.
public:
	CVarioConsoleDlg(CWnd* pParent = NULL);	// ǥ�� �������Դϴ�.

// ��ȭ ���� �������Դϴ�.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_WCONSOLE_DIALOG };
#endif

protected:
	virtual void		DoDataExchange(CDataExchange* pDX);	// DDX/DDV �����Դϴ�.

private:
	void				ParseReceivedMessage();
	void				ProcessReceivedMessage(WORD code, UINT param, LPCTSTR lpszData);

	BOOL				UpdateData(BOOL bSaveAndValidate);
	void				UpdateTitle();

	void				OpenSerial(LPCTSTR lpszDevice, CSerial::EBaudrate, CSerial::EDataBits, CSerial::EParity, CSerial::EStopBits, CSerial::EHandshake);
	void				CloseSerial();

// �����Դϴ�.
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

	// Serial Settings
	int					m_nPortNum;
	CSerial::EBaudrate	m_nBaudRate;
	CSerial::EDataBits	m_nDataBits;
	CSerial::EParity	m_nParity;
	CSerial::EStopBits	m_nStopBits;

	//
	CMatrixStatic		m_wndLatitude;
	CMatrixStatic		m_wndLongitude;

	CString				m_strLatitude;
	CString				m_strLongitude;


protected:
	// ������ �޽��� �� �Լ�
	virtual BOOL		OnInitDialog();
	virtual void		OnOK();
	virtual void		OnCancel();

	afx_msg void		OnPaint();
	afx_msg HCURSOR		OnQueryDragIcon();
	afx_msg void		OnTimer(UINT nIDEvent);
	afx_msg LRESULT		OnSerialMessage(WPARAM, LPARAM);
	afx_msg void		OnEditToneTable();
	afx_msg void		OnCalibration();
	afx_msg void		OnViewSensorData();
	afx_msg void		OnConnect();
	afx_msg void		OnSelectPort();
	afx_msg void		OnSendMessage();

	DECLARE_MESSAGE_MAP()

};
