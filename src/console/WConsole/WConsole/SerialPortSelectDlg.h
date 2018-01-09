#pragma once

#include <list>
#include "Serial.h"

class SerialPort
{
public:
	SerialPort(int nPort, LPCTSTR lpszName) : m_nPort(nPort), m_strName(lpszName) {}

public:
	int			m_nPort;
	CString		m_strName;
};

// CSerialPortSelectDlg 대화 상자입니다.

class CSerialPortSelectDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CSerialPortSelectDlg)

public:
	CSerialPortSelectDlg(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CSerialPortSelectDlg();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SELECT_PORT };
#endif

private:
	std::list<SerialPort>		m_SerialPorts;

public:
	CString						m_sPortName;
	int							m_sPortNum;
	CSerial::EBaudrate			m_sBaudRate;
	CSerial::EDataBits			m_sDataBits;
	CSerial::EParity			m_sParity;
	CSerial::EStopBits			m_sStopBits;
	CSerial::EHandshake			m_sFlowControl;

private:
	BOOL						UpdateData(BOOL bSaveAndValidate);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	CComboBox m_wndPort;
	int m_nPort;
	int m_nBaudRate;
	int m_nDataBits;
	int m_nParity;
	int m_nStopBits;
	int m_nFlowControl;
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
};
